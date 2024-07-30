// Goes from 0 to 1
layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec2 normal_direction;
layout (location = 2) in vec2 texture_coord;

out vec2 tex_coord;

uniform vec2 offset;
uniform float aspect_ratio;
uniform float zoom;
uniform vec2 map_size;
uniform float time;
uniform mat3 rotation;
uniform vec2 model_offset;
uniform float target_facing;
uniform float target_topview_fixup;
uniform uint subroutines_index;
uniform float counter_factor;

vec4 calc_gl_position(in vec3 v);

// A rotation so units can face were they are going
vec3 rotate_target(vec3 v) {
	vec3 k = vec3(0.f, 1.f, 0.f);
	float cos_theta = cos(target_facing - M_PI / 2.f);
	float sin_theta = sin(target_facing - M_PI / 2.f);
	return (v * cos_theta) + (cross(k, v) * sin_theta) + (k * dot(k, v)) * (1.f - cos_theta);
}

void main() {
	float vertical_factor = (map_size.x + map_size.y) / 4.f;
	vec3 world_pos = vec3(vertex_position.x, vertex_position.y, vertex_position.z);
	world_pos = rotate_target(world_pos);
	world_pos /= vec3(map_size.x, vertical_factor, map_size.y);
	world_pos += vec3(model_offset.x / map_size.x, 0.f, model_offset.y / map_size.y);
	gl_Position = calc_gl_position(world_pos);
	tex_coord = texture_coord;
}
