out vec4 outcolor;
in vec2 frag_texture_coord;

uniform sampler2D source;

void main() {
	vec2 uv = vec2(frag_texture_coord.x, 1.0 - frag_texture_coord.y);
	outcolor = texture(source, uv);
}
