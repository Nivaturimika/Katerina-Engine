// Goes from 0 to 1
layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec2 normal_direction;
layout (location = 2) in vec2 texture_coord;
layout (location = 3) in int bone_ids[4];
layout (location = 4) in float bone_weights[4];

out vec2 tex_coord;
out vec3 dbg_color;

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

#define MAX_BONES 100
uniform mat4 bones_matrices[MAX_BONES];

vec4 calc_gl_position(in vec3 v);

// A rotation so units can face were they are going
vec3 rotate_target(vec3 v) {
	vec3 k = vec3(0.f, 1.f, 0.f);
	float cos_theta = cos(target_facing + M_PI);
	float sin_theta = sin(target_facing + M_PI);
	return (v * cos_theta) + (cross(k, v) * sin_theta) + (k * dot(k, v)) * (1.f - cos_theta);
}

void main() {
	vec3 skin_pos = vec3(0.f);
	for(int i = 0 ; i < 4; i++) {
		if(bone_ids[i] == -1)
			break;
		vec4 local_pos = bones_matrices[bone_ids[i]] * vec4(vertex_position, 1.f);
		skin_pos += local_pos.xyz * bone_weights[i];
	}
	vec3 world_pos = vertex_position;
	//vec3 world_pos = skin_pos;

	world_pos.xz = world_pos.zx;
	world_pos.y *= -1.f;

	float vertical_factor = (map_size.x + map_size.y) / 4.f;
	world_pos = rotate_target(world_pos);
	world_pos /= vec3(map_size.x, vertical_factor, map_size.y);
	world_pos += vec3(model_offset.x / map_size.x, 0.f, model_offset.y / map_size.y);
	gl_Position = calc_gl_position(world_pos);

	dbg_color = abs(skin_pos - world_pos);
	tex_coord = texture_coord;
}
