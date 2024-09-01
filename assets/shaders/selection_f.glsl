out vec4 frag_color;
vec4 gamma_correct(in vec4 colour);

uniform sampler2D texture_sampler;

in vec2 tc;

void main() {
	vec4 colour = texture(texture_sampler, tc);
	frag_color = gamma_correct(colour);
}
