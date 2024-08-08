// Goes from 0 to 1
layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec2 normal_direction;
layout (location = 2) in vec2 texture_coord;
layout (location = 3) in ivec4 bone_ids;
layout (location = 4) in vec4 bone_weights;

out vec2 tex_coord;

uniform vec2 offset;
uniform float zoom;
uniform vec2 map_size;
uniform float time;
uniform vec2 model_offset;
uniform float target_facing;

//#define HAVE_ANIMATIONS

#define MAX_BONES 100
uniform mat4 bones_matrices[MAX_BONES];

vec4 calc_gl_position(in vec3 v);

// A rotation so units can face were they are going
vec3 rotate_target(vec3 v, vec3 k, float s) {
	float cos_theta = cos(s);
	float sin_theta = sin(s);
	return (v * cos_theta) + (cross(k, v) * sin_theta) + (k * dot(k, v)) * (1.f - cos_theta);
}

void main() {
#ifndef HAVE_ANIMATIONS
	vec3 world_pos = vertex_position;
#elif
	vec4 skin_pos = vec4(0.f);
	for(int i = 0 ; i < 4; i++) {
		if(bone_ids[i] == -1)
			break;
		vec4 local_pos = vec4(vertex_position, 1.f) * bones_matrices[bone_ids[i]];
		skin_pos += local_pos * bone_weights[i];
	}
	vec3 world_pos = skin_pos.xyz;
	world_pos.z *= -1.f;
#endif
	world_pos.y *= -1.f;
	world_pos = rotate_target(world_pos, vec3(0.f, 1.f, 0.f), target_facing - PI / 2.f);
//
	float vertical_factor = (map_size.x + map_size.y) / 4.f;
	world_pos /= vec3(map_size.x, vertical_factor, map_size.y);
	world_pos += vec3(model_offset.x / map_size.x, 0.f, model_offset.y / map_size.y);
	vec4 t = calc_gl_position(world_pos);
	gl_Position = t;

	tex_coord = texture_coord * 4.f;
}
