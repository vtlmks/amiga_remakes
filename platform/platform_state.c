#define SCALE 3.3

#define CRT_ASPECT_NUM 4
#define CRT_ASPECT_DEN 3
#define CRT_ASPECT     ((float)CRT_ASPECT_NUM / (float)CRT_ASPECT_DEN)

#define FRAME_RATE_HZ 50.0
#define FRAME_TIME_NS ((uint64_t)(1000000000.0 / FRAME_RATE_HZ))

uint32_t buffer[BUFFER_WIDTH * BUFFER_HEIGHT] __attribute__((aligned(4096)));

struct options {
	uint8_t fullscreen;
	uint8_t crtshader;
	char *release_group;	// MAX 40 chars
	char *release_title;	// MAX 40 chars
	char *window_title;
};
struct options opt;

struct remake_state {
	struct { int32_t x, y, w, h; } viewport;
	int32_t mouse_dx;
	int32_t mouse_dy;

	float contrast;
	float saturation;
	float brightness;
	float tone_data[4];

	// OpenGL Objects
	GLuint shader_program;
	GLuint persistence_program;
	GLuint passthrough_program;

	GLuint texture;
	GLuint persistence_texture;
	GLuint persistence_output_texture;

	GLuint persistence_fbo;

	GLuint vao;
	GLuint vbo;
	GLuint ebo;

	// CRT Shader Uniforms
	GLuint uniform_resolution;
	GLuint uniform_src_image_size;
	GLuint uniform_brightness;
	GLuint uniform_tone;
	GLuint uniform_sampler_location;

	// Passthrough Shader Uniforms
	GLuint passthrough_uniform_source;

	// Phosphor persistence
	float persistence_decay;

	// Rendering & Dynamic Resolution
	uint32_t render_width;	// The actual remake resolution (e.g., 360)
	uint32_t render_height;	// The actual remake resolution (e.g., 270)
	uint32_t frame_number;
	uint8_t running;
	uint8_t toggle_crt_emulation;
	uint8_t fullscreen;
	uint8_t viewport_changed;

};

static struct remake_state state;
