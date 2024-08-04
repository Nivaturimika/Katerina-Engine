// Goes from 0 to 1
layout (location = 0) in vec2 vertex_position;
layout (location = 1) in vec2 normal_direction;
layout (location = 2) in float texture_coord;
layout (location = 3) in float distance;

out float tex_coord;
out float o_dist;
out vec2 map_coord;

uniform vec2 offset;
uniform float zoom;
uniform vec2 map_size;
uniform float width;
uniform float time;

vec4 calc_gl_position(in vec3 v);

void main() {
	vec2 normal_vector = normalize(normal_direction) * width;
	vec2 world_pos = vertex_position;

	world_pos.x *= map_size.x / map_size.y;
	world_pos += normal_vector;
	world_pos.x /= map_size.x / map_size.y;

	map_coord = world_pos;
	gl_Position = calc_gl_position(vec3(world_pos.x, 0.f, world_pos.y));
	tex_coord = texture_coord;
	o_dist = time + distance / (2.0f * width);
	map_coord = vertex_position;
}
