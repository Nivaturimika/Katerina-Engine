in vec2 tex_coord;
in vec3 normal;
in vec3 light_dir;
out vec4 frag_color;

uniform float time;
uniform sampler2D diffuse_texture;
vec4 gamma_correct(in vec4 colour);

void main() {
	float ndotl = max(0.f, dot(light_dir, normal));
	float diffuse = pow(0.5f * ndotl + 0.5f, 2.f) * 1.75f;
	// do a texel fetch
	vec2 tc = vec2(tex_coord.x, mod(tex_coord.y + time, 1.f));
	vec4 base_color = texture(diffuse_texture, tc);
	vec4 out_color = base_color * diffuse;
	out_color.a = base_color.a;
	frag_color = gamma_correct(out_color);
}
