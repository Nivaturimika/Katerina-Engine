out vec4 frag_color;
vec4 gamma_correct(in vec4 colour);

uniform sampler2D texture_sampler;

in vec2 tc;

void main() {
	vec4 colour = texture(texture_sampler, tc);
	colour.a *= (colour.r + colour.g + colour.b) == 0.f ? 0.f : 1.f;
	frag_color = gamma_correct(colour);
}
