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
	vec3 outer_color = vec3(0.1f);
	outer_color = mix(inner_color, outer_color, text_size * 40.f);
	vec4 color_in = texture(texture_sampler, vec2(tex_coord.xy));

	float segment_1 = 0.5f;
	float segment_2 = 0.475f;
	if(text_size * 40.f <= 0.5f) {
		segment_1 = 0.535f;
	} else {
		segment_1 = mix(0.535f, 0.5f, clamp(text_size * 40.f - 0.5f, 0.f, 1.f));
	}

	if(color_in.r > segment_1) {
		frag_color = vec4(inner_color, 1.0f);
	} else if(color_in.r > segment_2) {
		float t = (color_in.r - segment_2) / (segment_1 - segment_2);
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
