out vec4 outcolor;
in vec2 frag_texture_coord;

uniform sampler2D source;
uniform vec2 resolution;
uniform vec2 src_image_size;

void main() {
	// Match CRT shader coordinate system exactly
	vec2 half_pixel = 0.5 / src_image_size;
	vec2 fragCoord = vec2(frag_texture_coord.x, 1.0 - frag_texture_coord.y) + half_pixel;

	// Convert to pixel coordinates and add pixel centering offset
	vec2 ipos = fragCoord * resolution + vec2(0.5);

	// Convert to normalized device coordinates [-1, 1]
	vec2 pos = ipos * (2.0 / resolution) - vec2(1.0);

	// Apply barrel distortion (match CRT warp)
	vec2 warp = vec2(1.0 / 24.0, 1.0 / 16.0);
	pos *= vec2(
		1.0 + (pos.y * pos.y) * warp.x,
		1.0 + (pos.x * pos.x) * warp.y
	);

	// Convert back to UV coordinates
	vec2 uv = (pos + vec2(1.0)) * 0.5;

	// Sample from source with boundary check
	if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
		outcolor = vec4(0.0, 0.0, 0.0, 1.0);
	} else {
		// Sample directly (Y-flip already applied at line 11)
		outcolor = texture(source, uv, -16.0);
	}
}
