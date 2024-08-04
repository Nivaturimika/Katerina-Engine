in float tex_coord;
in float o_dist;
out vec4 frag_color;

uniform sampler2D line_texture;
uniform float time;
vec4 gamma_correct(in vec4 colour);

void main() {
	vec4 out_color = texture(line_texture, vec2(o_dist, tex_coord - time));
	frag_color = gamma_correct(out_color);
}
