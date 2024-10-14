out vec4 frag_color;
vec4 gamma_correct(in vec4 colour);

uniform sampler2D texture_sampler;
uniform float time;

in vec2 tc;

void main() {
	float mid = 0.5f;
	vec2 r = vec2(cos(time) * (tc.x - mid) + sin(time) * (tc.y - mid) + mid, cos(time) * (tc.y - mid) - sin(time) * (tc.x - mid) + mid);

	vec4 colour = texture(texture_sampler, r);
	colour.a = mix(0.f, colour.a, (colour.r + colour.g + colour.b));
	frag_color = gamma_correct(colour);
}
