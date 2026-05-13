out vec4 outcolor;
in vec2 frag_texture_coord;

uniform sampler2D source;
uniform vec2 src_texel_size;
uniform bool first_pass;
uniform float threshold;
uniform float knee;
uniform float halation_power;

// Soft-knee bright-pass filter. Linear ramp through [threshold-knee, threshold+knee]
// avoids the "binary on/off" outline look of a hard threshold. Uses max(r,g,b)
// rather than perceptual luminance so saturated reds/blues bloom equally with
// greens; luminance weights would let a bright red drop below threshold and tint
// the bloom green.
//
// halation_power makes the bloom response non-linear in brightness: pixels just
// above threshold contribute very little, while pixels near full brightness
// dominate. Models how a real CRT's phosphor saturates locally for bright spots,
// producing the "very bright stuff glows hard, mid-bright stuff stays clean" feel.
// 1.0 = linear (smoothstep-only), 2.0-3.0 = pronounced halation.
vec3 prefilter(vec3 c) {
	float brightness = max(c.r, max(c.g, c.b));
	float t = smoothstep(threshold - knee, threshold + knee, brightness);
	t = pow(t, halation_power);
	return c * t;
}

// Anti-firefly weight. Same reasoning as above - using max keeps a single bright
// red sample as suppressed as a bright green sample of the same intensity.
float karis_weight(vec3 c) {
	float m = max(c.r, max(c.g, c.b));
	return 1.0 / (1.0 + m);
}

vec3 karis_box(vec3 a, vec3 b, vec3 c, vec3 d) {
	float wa = karis_weight(a);
	float wb = karis_weight(b);
	float wc = karis_weight(c);
	float wd = karis_weight(d);
	return (a * wa + b * wb + c * wc + d * wd) / (wa + wb + wc + wd);
}

// 13-tap downsample (Karis / COD: Advanced Warfare SIGGRAPH 2014).
// Samples 5 overlapping 2x2 boxes - 4 corner boxes weighted 0.125 each,
// 1 inner box weighted 0.5 - giving a smooth, anti-aliased downsample.
void main() {
	vec2 uv = frag_texture_coord;
	vec2 t = src_texel_size;

	vec3 a = texture(source, uv + t * vec2(-2.0, -2.0)).rgb;
	vec3 b = texture(source, uv + t * vec2( 0.0, -2.0)).rgb;
	vec3 c = texture(source, uv + t * vec2( 2.0, -2.0)).rgb;
	vec3 d = texture(source, uv + t * vec2(-2.0,  0.0)).rgb;
	vec3 e = texture(source, uv + t * vec2( 0.0,  0.0)).rgb;
	vec3 f = texture(source, uv + t * vec2( 2.0,  0.0)).rgb;
	vec3 g = texture(source, uv + t * vec2(-2.0,  2.0)).rgb;
	vec3 h = texture(source, uv + t * vec2( 0.0,  2.0)).rgb;
	vec3 i = texture(source, uv + t * vec2( 2.0,  2.0)).rgb;
	vec3 j = texture(source, uv + t * vec2(-1.0, -1.0)).rgb;
	vec3 k = texture(source, uv + t * vec2( 1.0, -1.0)).rgb;
	vec3 l = texture(source, uv + t * vec2(-1.0,  1.0)).rgb;
	vec3 m = texture(source, uv + t * vec2( 1.0,  1.0)).rgb;

	vec3 result;

	if(first_pass) {
		a = prefilter(a); b = prefilter(b); c = prefilter(c);
		d = prefilter(d); e = prefilter(e); f = prefilter(f);
		g = prefilter(g); h = prefilter(h); i = prefilter(i);
		j = prefilter(j); k = prefilter(k); l = prefilter(l); m = prefilter(m);

		vec3 group_inner = karis_box(j, k, l, m);
		vec3 group_tl = karis_box(a, b, d, e);
		vec3 group_tr = karis_box(b, c, e, f);
		vec3 group_bl = karis_box(d, e, g, h);
		vec3 group_br = karis_box(e, f, h, i);

		result = group_inner * 0.5
				+ group_tl * 0.125
				+ group_tr * 0.125
				+ group_bl * 0.125
				+ group_br * 0.125;
	} else {
		result = e * 0.125
				+ (a + c + g + i) * 0.03125
				+ (b + d + f + h) * 0.0625
				+ (j + k + l + m) * 0.125;
	}

	outcolor = vec4(result, 1.0);
}
