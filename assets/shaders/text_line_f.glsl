in vec2 tex_coord;
in float opacity;
in float text_size;
out vec4 frag_color;

uniform float is_black;
uniform sampler2D texture_sampler;
vec4 gamma_correct(in vec4 colour);

void main() {
	float border_size = 0.022f;
	vec3 inner_color = vec3(1.f - is_black);
	vec3 outer_color = vec3(0.f, 0.1f, 0.f);
	outer_color = mix(inner_color, outer_color, text_size * 40.f);
	vec4 color_in = texture(texture_sampler, vec2(tex_coord.xy));

	if(color_in.r > 0.5f) {
		frag_color = vec4(inner_color, 1.0f);
	} else if(color_in.r > 0.475f) {
		float t = (color_in.r - 0.475f) * (1.f / (0.5f - 0.475f));
		frag_color = vec4(inner_color, t * t);
		//frag_color = vec4(mix(inner_color, outer_color, 1.0f - (color_in.r - 0.5f) * 100.f), 1.0f);
	} else {
		float t = max(0.0f, color_in.r * 2.0f - 0.5f);
		frag_color = vec4(outer_color, t * t * t);
	}

	frag_color.a *= 0.85f;
	//frag_color.a *= opacity;
	frag_color = gamma_correct(frag_color);
}
