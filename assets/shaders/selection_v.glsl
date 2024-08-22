layout (location = 0) in vec2 vertex_position;
layout (location = 1) in vec2 texcoord;

out vec2 tc;

vec4 calc_gl_position(in vec3 v);
void main() {
	tc = texcoord;
	gl_Position = calc_gl_position(vec3(vertex_position.x, 0.f, vertex_position.y));
}
