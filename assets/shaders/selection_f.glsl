out vec4 frag_color;
vec4 gamma_correct(in vec4 colour);

uniform sampler2D texture_sampler;

in vec2 tc;

void main() {
	const float contrast = 2.f;
	const float luminance = 0.5f;
	const float sat = 2.f;

	vec4 colour = texture(texture_sampler, tc);
	colour.a = (colour.r + colour.g + colour.b);
	colour = contrast * (colour - 0.5f) + 0.5f + luminance;
	colour.r = clamp(colour.r, 0.f, 1.f);
	colour.g = clamp(colour.g, 0.f, 1.f);
	colour.b = clamp(colour.b, 0.f, 1.f);

	colour = mix(colour * vec4(0.299f, 0.587f, 0.114f, 1.f), colour, sat);
	colour.r = clamp(colour.r, 0.f, 1.f);
	colour.g = clamp(colour.g, 0.f, 1.f);
	colour.b = clamp(colour.b, 0.f, 1.f);

	frag_color = gamma_correct(colour);
}
