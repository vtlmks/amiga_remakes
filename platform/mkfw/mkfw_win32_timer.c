#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <avrt.h>
#include <winternl.h>

#define MKFW_SPIN_THRESHOLD_NS 1000000	// NOTE(peter): 1000µs spin threshold for Windows

struct mkfw_timer_handle {
	uint64_t interval_ns;
	uint64_t qpc_frequency;
	uint64_t next_deadline;
	uint64_t spin_threshold_100ns;
	uint32_t running;

	HANDLE event;
	HANDLE timer_thread;
	HANDLE mmcss_handle;

#ifdef MKFW_TIMER_DEBUG
	uint64_t last_wait_start_ns;
#endif
};

typedef LONG NTSTATUS;
typedef NTSTATUS (__stdcall *mkfw_NtDelayExecution_t)(BOOLEAN Alertable, PLARGE_INTEGER DelayInterval);

static mkfw_NtDelayExecution_t mkfw_pNtDelayExecution;
static uint64_t mkfw_cached_qpc_frequency = 0;

static inline uint64_t mkfw_qpc_now_ns(uint64_t freq) {
	LARGE_INTEGER qpc;
	QueryPerformanceCounter(&qpc);

	uint64_t q = (uint64_t)qpc.QuadPart;
	uint64_t s = q / freq;
	uint64_t r = q % freq;

	return s * 1000000000ULL + (r * 1000000000ULL + (freq >> 1)) / freq;
}

static void mkfw_try_set_nt_timer_resolution(void) {
	typedef LONG NTSTATUS;
	typedef NTSTATUS (__stdcall *mkfw_NtSetTimerResolution_t)(ULONG, BOOLEAN, ULONG *);
	mkfw_NtSetTimerResolution_t mkfw_NtSetTimerResolution = (mkfw_NtSetTimerResolution_t)(uintptr_t)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtSetTimerResolution");
	if(mkfw_NtSetTimerResolution) {
		ULONG requested = 5000; /* 0.5ms = 5000 * 100ns */
		ULONG actual = 0;
		mkfw_NtSetTimerResolution(requested, TRUE, &actual);
	}
}

static void mkfw_set_realtime_priority(HANDLE *mmcss_handle) {
	DWORD task_index = 0;
	*mmcss_handle = AvSetMmThreadCharacteristicsW(L"Pro Audio", &task_index);
	if(!*mmcss_handle) {
		SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	}
}

static void mkfw_timer_sleep(uint64_t delay_100ns_units) {
	LONGLONG delay_100ns = -(LONGLONG)delay_100ns_units;
	if(delay_100ns < 0) {
		LARGE_INTEGER delay;
		delay.QuadPart = delay_100ns;
		mkfw_pNtDelayExecution(FALSE, &delay);
	}
}

static DWORD WINAPI mkfw_timer_thread_func(LPVOID arg) {
	struct mkfw_timer_handle *t = (struct mkfw_timer_handle *)arg;
	SetThreadAffinityMask(GetCurrentThread(), 1);
	mkfw_set_realtime_priority(&t->mmcss_handle);

	while(t->running) {
#ifdef MKFW_TIMER_DEBUG
		int64_t remaining_after_sleep_ns = -1;
#endif
		uint64_t now = mkfw_qpc_now_ns(t->qpc_frequency);

		if(now < t->next_deadline) {
			uint64_t diff = t->next_deadline - now;
			if(diff > MKFW_SPIN_THRESHOLD_NS) {
				uint64_t sleep_ns = diff - MKFW_SPIN_THRESHOLD_NS;
				mkfw_timer_sleep(sleep_ns / 100);
#ifdef MKFW_TIMER_DEBUG
				now = mkfw_qpc_now_ns(t->qpc_frequency);
				remaining_after_sleep_ns = (int64_t)(t->next_deadline - now);
#endif
			}
			while(mkfw_qpc_now_ns(t->qpc_frequency) < t->next_deadline) {
				_mm_pause();
			}
		}

		now = mkfw_qpc_now_ns(t->qpc_frequency);
		SetEvent(t->event);

#ifdef MKFW_TIMER_DEBUG
		if(t->last_wait_start_ns > 0) {
			int64_t overshoot_ns = (int64_t)(now - t->next_deadline);
			if(overshoot_ns < 0) overshoot_ns = 0;

			if(remaining_after_sleep_ns >= 0) {
				DEBUG_PRINT("[DEBUG] Woke up with %lld ns left. Overshoot: %5lld ns\n", remaining_after_sleep_ns, overshoot_ns);
			} else {
				DEBUG_PRINT("[DEBUG] No sleep. Overshoot: %lld ns\n", overshoot_ns);
			}
		}
		t->last_wait_start_ns = now;
#endif

		t->next_deadline += t->interval_ns;
	}

	return 0;
}

static void mkfw_timer_init(void) {
	if(!mkfw_pNtDelayExecution) {
		mkfw_pNtDelayExecution = (mkfw_NtDelayExecution_t)(uintptr_t)GetProcAddress( GetModuleHandleW(L"ntdll.dll"), "NtDelayExecution" );
	}
	if(!mkfw_cached_qpc_frequency) {
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		mkfw_cached_qpc_frequency = freq.QuadPart;
	}
	timeBeginPeriod(1);
	mkfw_try_set_nt_timer_resolution();
}

static void mkfw_timer_shutdown(void) {
	timeEndPeriod(1);
}


static struct mkfw_timer_handle *mkfw_timer_new(uint64_t interval_ns) {
	struct mkfw_timer_handle *t = calloc(1, sizeof(struct mkfw_timer_handle));

	t->qpc_frequency = mkfw_cached_qpc_frequency;
	t->interval_ns = interval_ns;
	t->spin_threshold_100ns = MKFW_SPIN_THRESHOLD_NS / 100;
	t->next_deadline = mkfw_qpc_now_ns(t->qpc_frequency) + interval_ns;
	t->running = 1;
	t->mmcss_handle = 0;

#ifdef MKFW_TIMER_DEBUG
	t->last_wait_start_ns = 0;
#endif

	t->event = CreateEvent(0, FALSE, FALSE, 0);
	t->timer_thread = CreateThread(0, 0, mkfw_timer_thread_func, t, 0, 0);

	return t;
}

static uint32_t mkfw_timer_wait(struct mkfw_timer_handle *t) {
	WaitForSingleObject(t->event, INFINITE);
	return 1;
}

static void mkfw_timer_destroy(struct mkfw_timer_handle *t) {
	t->running = 0;

	SetEvent(t->event);
	WaitForSingleObject(t->timer_thread, INFINITE);
	CloseHandle(t->timer_thread);
	CloseHandle(t->event);

	if(t->mmcss_handle) {
		AvRevertMmThreadCharacteristics(t->mmcss_handle);
	}

	free(t);
}
