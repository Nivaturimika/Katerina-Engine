in vec2 tex_coord;
out vec4 frag_color;
uniform sampler2D unit_arrow;
vec4 gamma_correct(in vec4 colour);

void main() {
	vec4 OutColor = texture( unit_arrow, tex_coord );
	frag_color = gamma_correct(OutColor);
}
