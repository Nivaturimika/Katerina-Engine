// Goes from 0 to 1
layout (location = 0) in vec2 vertex_position;
out vec2 tex_coord;

vec4 calc_gl_position(vec3 v);

void main() {
	gl_Position = calc_gl_position(vec3(vertex_position.x, 0.f, vertex_position.y));
	tex_coord = vertex_position;
}
