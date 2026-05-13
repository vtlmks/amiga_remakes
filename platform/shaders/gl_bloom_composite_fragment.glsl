out vec4 outcolor;
in vec2 frag_texture_coord;

uniform sampler2D crt_texture;
uniform sampler2D bloom_texture;
uniform float bloom_strength;
uniform vec2 viewport_size;

// 4x4 Bayer ordered dither, returns ~[-0.5, +0.5] / 16. Used to break visible
// banding when smooth gradients (vignette, bias floor) cross sRGB byte boundaries
// in dark areas - without dither each step shows as a concentric ring.
float bayer4x4(vec2 fp) {
	const float t[16] = float[](
		 0.0,  8.0,  2.0, 10.0,
		12.0,  4.0, 14.0,  6.0,
		 3.0, 11.0,  1.0,  9.0,
		15.0,  7.0, 13.0,  5.0
	);
	int x = int(mod(fp.x, 4.0));
	int y = int(mod(fp.y, 4.0));
	return t[y * 4 + x] / 16.0 - 0.46875;
}

void main() {
	vec3 crt_color = texture(crt_texture, frag_texture_coord).rgb;
	vec3 bloom_color = texture(bloom_texture, frag_texture_coord).rgb;

	// CRT bezel rounded-corner mask. Both crt and bloom are already in
	// warped screen space (warp was applied before bloom extract), so we
	// only need to clip the bezel here.
	vec2 ipos = frag_texture_coord * viewport_size;
	vec2 pos = ipos * (2.0 / viewport_size) - vec2(1.0);
	vec2 warp = vec2(1.0 / 24.0, 1.0 / 16.0);
	pos *= vec2(1.0 + (pos.y * pos.y) * warp.x, 1.0 + (pos.x * pos.x) * warp.y);
	vec2 uv = (pos + vec2(1.0)) * 0.5;

	float corner_radius = 0.05;
	vec2 edge_distance = abs(pos) - vec2(1.0 - corner_radius);
	float dist = length(max(edge_distance, 0.0));
	float edge_softness = 0.003;
	float mask = smoothstep(corner_radius + edge_softness, corner_radius - edge_softness, dist);
	mask *= (uv.x >= 0.0 && uv.x <= 1.0 && uv.y >= 0.0 && uv.y <= 1.0) ? 1.0 : 0.0;

	// CRT lens / phosphor edge falloff. length(pos) ~= 0 at center, ~1 at edge
	// midpoints, ~1.5 at corners (warped), so corners darken much more than edges.
	float vignette = 1.0 - INPUT_VIGNETTE * smoothstep(0.4, 1.4, length(pos));

	vec3 final = (crt_color + bloom_color * bloom_strength) * vignette * mask;
	// Dither in linear space - 1 sRGB byte step is ~0.0003 linear at low values
	// where banding shows up, so 1/256 noise magnitude is enough.
	final += vec3(bayer4x4(gl_FragCoord.xy) / 256.0);
	outcolor = vec4(final, 1.0);
}
