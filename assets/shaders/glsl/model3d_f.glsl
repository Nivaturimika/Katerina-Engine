in vec2 tex_coord;
out vec4 frag_color;

uniform float time;
uniform sampler2D diffuse_texture;
vec4 gamma_correct(in vec4 colour);

void main() {
	vec4 out_color = texture(diffuse_texture, vec2(tex_coord.x, mod(tex_coord.y + time, 1.f)));
	frag_color = gamma_correct(out_color);
}
