out vec4 outcolor;
in vec2 frag_texture_coord;

uniform sampler2D current_frame;
uniform sampler2D previous_frame;
uniform float decay;

void main() {
	vec3 current = texture(current_frame, frag_texture_coord).rgb;
	vec3 previous = texture(previous_frame, frag_texture_coord).rgb;

#if 0
	// Real phosphor has two-stage decay:
	// 1. Fast initial drop (bright to dim)
	// 2. Slower tail (the afterglow)

	// Apply non-linear decay curve to simulate real phosphor behavior
	// This gives a steep initial drop but keeps a subtle tail
	vec3 decayed = previous * decay;

	// The decay curve: fast drop for bright values, slower for dim
	// This prevents long trails on slow-moving objects while preserving
	// the subtle glow that makes motion feel smooth
	decayed = decayed * decayed; // Square the decay for non-linear falloff

	// Take the maximum to accumulate brightness (phosphor can't get darker from new light)
	vec3 result = max(current, decayed);
#else

	vec3 fast = vec3(0.25, 0.35, 0.20) * decay;
	vec3 slow = vec3(0.88, 0.92, 0.85) * decay;
	vec3 t = clamp(previous * 1.5, 0.0, 1.0);
	vec3 decayed = previous * mix(slow, fast, t);
	vec3 result = max(current, decayed);

#endif

	outcolor = vec4(result, 1.0);
}
