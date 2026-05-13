out vec4 outcolor;
in vec2 frag_texture_coord;

uniform vec2 resolution;
uniform vec2 src_image_size;
uniform float brightness;
uniform vec4 tone_data;
uniform sampler2D iChannel0;
uniform int use_mask_texture;
uniform int apply_mask;
uniform sampler2D mask_sampler;

//==============================================================
//                    FETCH FUNCTION
//==============================================================
vec3 CrtsFetch(vec2 uv) {
	return texture(iChannel0, uv, -16.0).rgb;
}

//==============================================================
//                      PHOSPHOR MASK
//==============================================================
vec3 CrtsMask(vec2 pos, float dark) {
#ifdef CRTS_MASK_GRILLE
	vec3 m = vec3(dark, dark, dark);
	float x = fract(pos.x * (1.0/3.0));

	if(x < (1.0/3.0)) {
		m.r = 1.0;

	} else if(x < (2.0/3.0)) {
		m.g = 1.0;

	} else {
		m.b = 1.0;

	}
	return m;
#endif

#ifdef CRTS_MASK_GRILLE_LITE
	vec3 m = vec3(1.0, 1.0, 1.0);
	float x = fract(pos.x * (1.0/3.0));
	if(x < (1.0/3.0)) {
		m.r = dark;

	} else if(x < (2.0/3.0)) {
		m.g = dark;

	} else {
		m.b = dark;

	}
	return m;
#endif

#ifdef CRTS_MASK_NONE
	return vec3(1.0, 1.0, 1.0);
#endif

#ifdef CRTS_MASK_SHADOW
	pos.x += pos.y * 3.0;
	vec3 m = vec3(dark, dark, dark);
	float x = fract(pos.x * (1.0/6.0));

	if(x < (1.0/3.0)) {
		m.r = 1.0;

	} else if(x < (2.0/3.0)) {
		m.g = 1.0;

	} else {
		m.b = 1.0;

	}
	return m;
#endif
}

//==============================================================
//                      CRTS FILTER
//==============================================================
vec3 CrtsFilter(vec2 ipos, vec2 inputSizeDivOutputSize, vec2 halfInputSize, vec2 rcpInputSize, vec2 rcpOutputSize, vec2 twoDivOutputSize, float inputHeight, vec2 warp, float thin, float blur, float mask, vec4 tone) {
	// Apply warp
	// Convert to {-1 to 1} range
	vec2 pos = ipos * twoDivOutputSize - vec2(1.0, 1.0);
	// Distort pushes image outside {-1 to 1} range
	pos *= vec2(1.0 + (pos.y * pos.y) * warp.x, 1.0 + (pos.x * pos.x) * warp.y);
	// Vignette disabled - rounded corners applied at end instead
	float vin = 1.0;
	// Leave in {0 to inputSize}
	pos = pos * halfInputSize + halfInputSize;

	// Snap to center of first scanline
	float y0 = floor(pos.y - 0.5) + 0.5;

	// Small horizontal bias to avoid nearest-neighbor stepping at non-integer scale ratios
	float x0 = floor(pos.x + 0.25 - 1.5) + 0.5;
	// Initial UV position
	vec2 p = vec2(x0 * rcpInputSize.x, y0 * rcpInputSize.y);
	// Fetch 4 nearest texels from 2 nearest scanlines
	vec3 colA0 = CrtsFetch(p);
	p.x += rcpInputSize.x;
	vec3 colA1 = CrtsFetch(p);
	p.x += rcpInputSize.x;
	vec3 colA2 = CrtsFetch(p);
	p.x += rcpInputSize.x;
	vec3 colA3 = CrtsFetch(p);
	p.y += rcpInputSize.y;
	vec3 colB3 = CrtsFetch(p);
	p.x -= rcpInputSize.x;
	vec3 colB2 = CrtsFetch(p);
	p.x -= rcpInputSize.x;
	vec3 colB1 = CrtsFetch(p);
	p.x -= rcpInputSize.x;
	vec3 colB0 = CrtsFetch(p);

	// Vertical filter - Scanline intensity using cosine wave.
	// Phosphor saturation widens the local scanline: bright beams bleed into the
	// dark gaps between scanlines, dim beams keep the narrow-line / wide-gap
	// look. INPUT_BEAM_BLOOM (opengl_shader.h) tunes how much bright beams overshoot.
	float max_a = max(max(max(colA0.r, max(colA0.g, colA0.b)), max(colA1.r, max(colA1.g, colA1.b))),
	                   max(max(colA2.r, max(colA2.g, colA2.b)), max(colA3.r, max(colA3.g, colA3.b))));
	float max_b = max(max(max(colB0.r, max(colB0.g, colB0.b)), max(colB1.r, max(colB1.g, colB1.b))),
	                   max(max(colB2.r, max(colB2.g, colB2.b)), max(colB3.r, max(colB3.g, colB3.b))));
	float thin_a = thin * (1.0 - max_a * INPUT_BEAM_BLOOM);
	float thin_b = thin * (1.0 - max_b * INPUT_BEAM_BLOOM);

	float off = pos.y - y0;
	float pi2 = 6.28318530717958;
	float hlf = 0.5;
	float scanA = cos(min(0.5,   off  * thin_a              ) * pi2) * hlf + hlf;
	float scanB = cos(min(0.5, (-off) * thin_b + thin_b      ) * pi2) * hlf + hlf;

	// Horizontal kernel is simple gaussian filter
	float off0 = pos.x - x0;
	float off1 = off0 - 1.0;
	float off2 = off0 - 2.0;
	float off3 = off0 - 3.0;
	float pix0 = exp2(blur * off0 * off0);
	float pix1 = exp2(blur * off1 * off1);
	float pix2 = exp2(blur * off2 * off2);
	float pix3 = exp2(blur * off3 * off3);
	float pixT = 1.0 / (pix0 + pix1 + pix2 + pix3);
	// Get rid of wrong pixels on edge
	pixT *= vin;
	scanA *= pixT;
	scanB *= pixT;
	// Apply horizontal and vertical filters
	vec3 color = (colA0 * pix0 + colA1 * pix1 + colA2 * pix2 + colA3 * pix3) * scanA + (colB0 * pix0 + colB1 * pix1 + colB2 * pix2 + colB3 * pix3) * scanB;

	// Apply phosphor mask
	if(use_mask_texture > 0) {
		float mask_u = (ipos.x + ipos.y * 3.0 + 0.5) / 6.0;
		color *= texture(mask_sampler, vec2(mask_u, 0.5)).rgb;

	} else {
		color *= CrtsMask(ipos, mask);
	}

	// Tonal control, start by protecting from /0
	float peak = max(1.0 / (256.0 * 65536.0), max(color.r, max(color.g, color.b)));
	// Compute the ratios of {R,G,B}
	vec3 ratio = color / peak;
	// Apply tonal curve to peak value
	peak = pow(peak, tone.x);
	peak = peak / (peak * tone.y + tone.z);
	// Apply saturation
	ratio = pow(ratio, vec3(tone.w, tone.w, tone.w));
	// Reconstruct color
	color = ratio * peak;

	// Add bias to prevent pure black (makes CRT glow more realistic)
	color = max(color, vec3(0.002333333));

	return color;
}

//==============================================================
//                         DITHER
//==============================================================
// 4x4 Bayer ordered dither, ~[-0.5, +0.5] / 16. Breaks the concentric banding
// caused by smooth gradients (vignette, bias floor) crossing sRGB byte
// boundaries in dark regions.
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

//==============================================================
//                         MAIN
//==============================================================
void main() {
	vec2 fragCoord = vec2(frag_texture_coord.x, 1.0 - frag_texture_coord.y);

	vec3 crt_color = CrtsFilter(
		fragCoord.xy * resolution,
		src_image_size / resolution,
		src_image_size * vec2(0.5),
		1.0 / src_image_size,
		1.0 / resolution,
		2.0 / resolution,
		src_image_size.y,
		vec2(1.0 / 24.0, 1.0 / 16.0),
		INPUT_THIN,
		INPUT_BLUR,
		INPUT_MASK,
		tone_data
	) * brightness;

	// Apply bezel rounded-corner mask only when bloom is disabled. When bloom
	// is active, the composite pass handles the bezel so it can clip both the
	// CRT output and the bloom together.
	if(apply_mask > 0) {
		vec2 ipos = fragCoord.xy * resolution;
		vec2 pos = ipos * (2.0 / resolution) - vec2(1.0);
		vec2 warp = vec2(1.0 / 24.0, 1.0 / 16.0);
		pos *= vec2(1.0 + (pos.y * pos.y) * warp.x, 1.0 + (pos.x * pos.x) * warp.y);
		vec2 uv = (pos + vec2(1.0)) * 0.5;

		float corner_radius = 0.05;
		vec2 edge_distance = abs(pos) - vec2(1.0 - corner_radius);
		float dist = length(max(edge_distance, vec2(0.0)));
		float edge_softness = 0.003;
		float mask = smoothstep(corner_radius + edge_softness, corner_radius - edge_softness, dist);
		mask *= (uv.x >= 0.0 && uv.x <= 1.0 && uv.y >= 0.0 && uv.y <= 1.0) ? 1.0 : 0.0;

		// CRT lens / phosphor edge falloff. Matches the vignette in the composite
		// shader so the look is identical whether bloom is on or off.
		float vignette = 1.0 - INPUT_VIGNETTE * smoothstep(0.4, 1.4, length(pos));

		crt_color *= vignette * mask;
		// Dither to break vignette+bias-floor banding when writing direct to screen.
		crt_color += vec3(bayer4x4(gl_FragCoord.xy) / 256.0);
	}

	outcolor.rgb = crt_color;
	outcolor.a = 1.0;
}
