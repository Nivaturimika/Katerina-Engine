in vec2 tex_coord;
out vec4 frag_color;

uniform float time;
uniform float gamma;

uniform sampler2D diffuse_texture;

vec4 gamma_correct(vec4 colour) {
	return vec4(pow(colour.rgb, vec3(1.f / gamma)), colour.a);
}

void main() {
	vec4 out_color = texture(diffuse_texture, vec2(tex_coord.x, mod(tex_coord.y + time, 1.f)));
	frag_color = gamma_correct(out_color);
}
