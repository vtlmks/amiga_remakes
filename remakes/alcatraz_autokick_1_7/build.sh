#!/bin/bash

# Set the project name here
PROJECT_NAME="remake"  # Change this when creating a new remake

GIT_ROOT=$(git rev-parse --show-toplevel)

# Process shared option selector assets (once, avoid race conditions)
process_selector() {
	local selector_dir="$GIT_ROOT/platform/$1"
	local needs_update=false
	for bmp in "$selector_dir"/*.bmp; do
		[ -f "$bmp" ] || continue
		local ugg="${bmp%.bmp}.ugg"
		[ ! -f "$ugg" ] || [ "$bmp" -nt "$ugg" ] && needs_update=true && break
	done
	[ "$needs_update" = true ] && (cd "$selector_dir" && [ -n "$(ls *.bmp 2>/dev/null)" ] && bmp2ugg -o . *.bmp)
	return 0
}

# Compiler
CC=gcc
WINCC=x86_64-w64-mingw32-gcc

# Base configuration common to all builds
CFLAGS="-std=gnu11 -mtune=generic "
CFLAGS+="-fno-argument-alias "
CFLAGS+="-mfunction-return=keep -mindirect-branch=keep "
CFLAGS+="-fwrapv -ffast-math -fno-trapping-math -fvisibility=hidden "
CFLAGS+="-fno-stack-protector -fno-PIE -no-pie -fcf-protection=none "
CFLAGS+="-fno-non-call-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables "
CFLAGS+="-Wall -Wextra -Wstrict-aliasing=3 "
CFLAGS+="-Wno-unused-parameter -Wno-sign-compare -Wno-trigraphs -Wno-maybe-uninitialized "
CFLAGS+="-Wno-unused-variable -Wno-unused-const-variable -Wno-unused-function -Wno-write-strings -Wno-missing-field-initializers "
CFLAGS+="-U_FORTIFY_SOURCE -fno-pic -fno-ident "

# harden semantics + warn on UB-prone patterns with near-zero build-time cost
CFLAGS+="-fno-strict-overflow -fno-delete-null-pointer-checks "
CFLAGS+="-Wstrict-overflow=5 -Warray-bounds -Wshift-overflow=2 -Woverflow -Wstringop-overflow=4 -Wstringop-truncation -Wvla "

# Enable occationally to check for errors..
# CFLAGS+="-fopt-info-vec-optimized "
# CFLAGS+="-fopt-info-optimized "
# CFLAGS+="-fopt-info-missed "
# CFLAGS+="-fopt-info-loop "
# CFLAGS+="-fanalyzer "

LDFLAGS="-Wl,--gc-sections -Wl,--as-needed "

# Base include paths
INCLUDE_PATHS="-I. -I$GIT_ROOT -I$GIT_ROOT/include -I$GIT_ROOT/platform -I$GIT_ROOT/platform/mkfw"

# Linux-specific includes and libraries
LINUX_CFLAGS="-ffunction-sections -fdata-sections"
LINUX_LDFLAGS="$LDFLAGS"
LINUX_INCLUDE=""
LINUX_LIBS="-lasound -lXi -lX11 -lGL -lm -ldl -pthread "

# Windows-specific includes and libraries
WINDOWS_CFLAGS="-ffunction-sections -D__USE_MINGW_ANSI_STDIO=0 "
WINDOWS_LDFLAGS="$LDFLAGS "
WINDOWS_INCLUDE=""
WINDOWS_LIBS="-lwinmm -lole32 -lmmdevapi -lavrt -lgdi32 -lopengl32 -luuid -mwindows "

# Determine build type-specific flags
BUILD_TYPE=$1

if [ -z "$BUILD_TYPE" ]; then
	BUILD_TYPE="normal"
fi

case "$BUILD_TYPE" in
	"normal")
		CFLAGS+="-ggdb -fno-omit-frame-pointer -O2 "
#  -pg # for gprof
		;;
	"release")
		CFLAGS+="-s -Wl,--strip-all -O2 "
		;;
	"profile")
		CFLAGS+="-O2 -fprofile-generate "
#  -ftest-coverage
		;;
	"profile_release")
		CFLAGS+="-s -Wl,--strip-all -O2 -fprofile-use "
		# CFLAGS+="-O2 -fprofile-use "
		;;
	"debug")
		CFLAGS+="-g -O0 -DMKFW_TIMER_DEBUG -DDEBUGPRINT "
		# tripwires for UB; non-recovering so failures are loud
#		CFLAGS+="-fsanitize=undefined,alignment,shift,signed-integer-overflow,pointer-overflow,object-size,unreachable "
#		CFLAGS+="-fno-sanitize-recover=undefined,alignment,shift,signed-integer-overflow,pointer-overflow,object-size,unreachable "
		LDFLAGS+="-fno-pie -no-pie "
		;;
	"coverage")
		gcov -b -c *.c
		exit 0
		;;

	"clean")
		rm -f *.gcda *.gcno *.gcov perf.data* *.alias *.o config.o ${PROJECT_NAME} ${PROJECT_NAME}.exe viewer viewer.exe
		rm -f data/*.ugg data/*/*.ugg data/*.dat
		exit 0
		;;
	*)
		echo "Unknown build type: $BUILD_TYPE"
		exit 1
		;;
esac

# Make sure compilation stops if any error happens.
set -e

# Process option selector assets
process_selector "option_selector_1"

# Process assets
(cd data && [ -n "$(ls *.bmp 2>/dev/null)" ] && bmp2ugg -o . *.bmp || true)

# Build Linux version
(
	$CC $CFLAGS $LINUX_CFLAGS remake.c -o ${PROJECT_NAME} $INCLUDE_PATHS $LINUX_INCLUDE $LINUX_LDFLAGS $LINUX_LIBS
) &

# Build Windows version
(
	$WINCC $CFLAGS $WINDOWS_CFLAGS remake.c -o ${PROJECT_NAME}.exe $INCLUDE_PATHS $WINDOWS_INCLUDE $WINDOWS_LDFLAGS $WINDOWS_LIBS
) &


wait
