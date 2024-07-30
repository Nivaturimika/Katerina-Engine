in float tex_coord;
in float o_dist;
in vec2 map_coord;
out vec4 frag_color;

uniform sampler2D line_texture;
uniform float time;
vec4 gamma_correct(in vec4 colour);

void main() {
	float v_factor = 0.5f - sqrt(map_coord.x * map_coord.y);
	vec4 out_color = texture(line_texture, vec2(o_dist, tex_coord - time * v_factor));
	frag_color = gamma_correct(out_color);
}
