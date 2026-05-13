out vec4 outcolor;
in vec2 frag_texture_coord;

uniform sampler2D source;
uniform vec2 src_texel_size;
uniform float intensity;

// 3x3 tent (pyramid) filter for upsampling. Weights are 1 2 1 / 2 4 2 / 1 2 1 / 16,
// which is the standard pyramid that gives a smooth, soft upsample without
// the box-filter artifacts of a flat 3x3 average.
//
// Output is added to the destination via additive blending (glBlendFunc(GL_ONE, GL_ONE)).
// Doing the sum on the GPU saves a texture read on the destination mip.
void main() {
	vec2 uv = frag_texture_coord;
	vec2 t = src_texel_size;

	vec3 a = texture(source, uv + t * vec2(-1.0,  1.0)).rgb;
	vec3 b = texture(source, uv + t * vec2( 0.0,  1.0)).rgb;
	vec3 c = texture(source, uv + t * vec2( 1.0,  1.0)).rgb;
	vec3 d = texture(source, uv + t * vec2(-1.0,  0.0)).rgb;
	vec3 e = texture(source, uv + t * vec2( 0.0,  0.0)).rgb;
	vec3 f = texture(source, uv + t * vec2( 1.0,  0.0)).rgb;
	vec3 g = texture(source, uv + t * vec2(-1.0, -1.0)).rgb;
	vec3 h = texture(source, uv + t * vec2( 0.0, -1.0)).rgb;
	vec3 i = texture(source, uv + t * vec2( 1.0, -1.0)).rgb;

	vec3 result = e * 4.0
				+ (b + d + f + h) * 2.0
				+ (a + c + g + i) * 1.0;
	result *= 1.0 / 16.0;

	outcolor = vec4(result * intensity, 1.0);
}
