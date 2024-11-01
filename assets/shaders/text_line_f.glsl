in vec2 tex_coord;
in float opacity;
in float text_size;
out vec4 frag_color;

uniform float is_black;
uniform sampler2D texture_sampler;
vec4 gamma_correct(in vec4 colour);

void main() {
	vec4 color_in = texture(texture_sampler, vec2(tex_coord.xy));
	if(is_black == 0.f) { //invert colours
		color_in.r = 1.f - color_in.r;
		color_in.g = 1.f - color_in.g;
		color_in.b = 1.f - color_in.b;
	}
	frag_color = gamma_correct(color_in);
}
