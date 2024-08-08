#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <vector>
#include "parsers.hpp"

namespace emfx {

struct xac_vector2f {
	float x = 0.f;
	float y = 0.f;

	xac_vector2f() = default;
	xac_vector2f(xac_vector2f&) = default;
	xac_vector2f(xac_vector2f const&) = default;
	xac_vector2f(xac_vector2f&&) = default;
	xac_vector2f& operator=(xac_vector2f&) = default;
	xac_vector2f& operator=(xac_vector2f const&) = default;

	xac_vector2f(float v) {
		x = y = v;
	}
	xac_vector2f(float x_, float y_) {
		x = x_;
		y = y_;
	}
	xac_vector2f operator+(xac_vector2f const& o) const {
		return xac_vector2f{ this->x + o.x, this->y + o.y };
	}
	xac_vector2f operator-(xac_vector2f const& o) const {
		return xac_vector2f{ this->x - o.x, this->y - o.y };
	}
	xac_vector2f operator/(xac_vector2f const& o) const {
		return xac_vector2f{ this->x / o.x, this->y / o.y };
	}
	xac_vector2f operator*(xac_vector2f const& o) const {
		return xac_vector2f{ this->x * o.x, this->y * o.y };
	}
};
struct xac_vector3f {
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;

	xac_vector3f() = default;
	xac_vector3f(xac_vector3f&) = default;
	xac_vector3f(xac_vector3f const&) = default;
	xac_vector3f(xac_vector3f&&) = default;
	xac_vector3f& operator=(xac_vector3f&) = default;
	xac_vector3f& operator=(xac_vector3f const&) = default;

	xac_vector3f(float v) {
		x = y = z = v;
	}
	xac_vector3f(float x_, float y_, float z_) {
		x = x_;
		y = y_;
		z = z_;
	}
	xac_vector3f(xac_vector2f& v, float z_) {
		x = v.x;
		y = v.y;
		z = z_;
	}
	xac_vector3f operator+(xac_vector3f const& o) const {
		return xac_vector3f{ this->x + o.x, this->y + o.y, this->z + o.z };
	}
	xac_vector3f operator-(xac_vector3f const& o) const {
		return xac_vector3f{ this->x - o.x, this->y - o.y, this->z - o.z };
	}
	xac_vector3f operator/(xac_vector3f const& o) const {
		return xac_vector3f{ this->x / o.x, this->y / o.y, this->z / o.z };
	}
	xac_vector3f operator*(xac_vector3f const& o) const {
		return xac_vector3f{ this->x * o.x, this->y * o.y, this->z * o.z };
	}
};
struct xac_vector4f {
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	float w = 0.f;

	xac_vector4f() = default;
	xac_vector4f(xac_vector4f&) = default;
	xac_vector4f(xac_vector4f const&) = default;
	xac_vector4f(xac_vector4f&&) = default;
	xac_vector4f& operator=(xac_vector4f&) = default;
	xac_vector4f& operator=(xac_vector4f const&) = default;

	xac_vector4f(float v) {
		x = y = z = w = v;
	}
	xac_vector4f(float x_, float y_, float z_, float w_) {
		x = x_;
		y = y_;
		z = z_;
		w = w_;
	}
	xac_vector4f(xac_vector3f& v, float w_) {
		x = v.x;
		y = v.y;
		z = v.z;
		w = w_;
	}
	xac_vector4f operator+(xac_vector4f const& o) const {
		return xac_vector4f{ this->x + o.x, this->y + o.y, this->z + o.z, this->w + o.w };
	}
	xac_vector4f operator-(xac_vector4f const& o) const {
		return xac_vector4f{ this->x - o.x, this->y - o.y, this->z - o.z, this->w - o.w };
	}
	xac_vector4f operator/(xac_vector4f const& o) const {
		return xac_vector4f{ this->x / o.x, this->y / o.y, this->z / o.z, this->w / o.w };
	}
	xac_vector4f operator*(xac_vector4f const& o) const {
		return xac_vector4f{ this->x * o.x, this->y * o.y, this->z * o.z, this->w * o.w };
	}
};
struct xac_color_rgba {
	float r = 0.f;
	float g = 0.f;
	float b = 0.f;
	float a = 0.f;
};
struct xac_color_rgb {
	float r = 0.f;
	float g = 0.f;
	float b = 0.f;
};
struct xac_vector4u16 {
	int16_t x = 0;
	int16_t y = 0;
	int16_t z = 0;
	int16_t w = 0;
	xac_vector4f to_vector4f() {
		return xac_vector4f{
			float(x) / 32767.f,
			float(y) / 32767.f,
			float(z) / 32767.f,
			float(w) / 32767.f,
		};
	}
};
struct xac_mat4x4 {
	float m[4][4] = {
		{ 0.f, 0.f, 0.f, 0.f },
		{ 0.f, 0.f, 0.f, 0.f },
		{ 0.f, 0.f, 0.f, 0.f },
		{ 0.f, 0.f, 0.f, 0.f },
	};
	xac_vector4f row_as_vector4f(uint32_t n) const {
		return xac_vector4f{ m[n][0], m[n][1], m[n][2], m[n][3] };
	}
};


















enum class xac_chunk_type : uint32_t {
	unknown_0 = 0,
	mesh = 1,
	skinning = 2,
	material_3 = 3,
	unknown_4 = 4,
	material = 5,
	metadata = 7,
	node_hierachy = 11,
	material_block = 13,
	count
};

//post-parse data
struct xac_pp_actor_submesh {
	std::vector<uint32_t> indices;
	std::vector<uint32_t> bone_ids;
	int32_t material_id = -1;
	uint32_t num_vertices = 0;
	uint32_t vertex_offset = 0;
};
struct xac_pp_bone_influence {
	float weight = 0.f;
	int32_t bone_id = -1;
};
struct xac_pp_actor_mesh {
	std::vector<xac_vector4f> weights;
	std::vector<xac_vector3f> vertices;
	std::vector<xac_vector3f> normals;
	std::vector<xac_vector2f> texcoords;
	std::vector<xac_pp_bone_influence> influences;
	std::vector<xac_pp_actor_submesh> submeshes;
	std::vector<uint32_t> influence_indices;
	std::vector<uint32_t> influence_starts;
	std::vector<uint32_t> influence_counts;
};
struct xac_pp_actor_node {
	std::string name;
	std::vector<xac_pp_actor_mesh> meshes;
	xac_vector4f rotation;
	xac_vector4f scale_rotation;
	xac_vector3f position;
	xac_vector3f scale;
	xac_mat4x4 transform;
	int32_t parent_id = -1;
	int32_t collision_mesh = -1;
	int32_t visual_mesh = -1;
};
enum class xac_pp_material_map_type : uint8_t {
	none = 0,
	diffuse = 2,
	specular = 3
};
struct xac_pp_actor_material_layer {
	std::string texture;
	float amount = 0.f;
	float u_offset = 0.f;
	float v_offset = 0.f;
	float u_tiling = 0.f;
	float v_tiling = 0.f;
	float rotation = 0.f; //radians?
	int16_t material_id = -1;
	xac_pp_material_map_type map_type = xac_pp_material_map_type::none;
};
struct xac_pp_actor_material {
	std::string name;
	std::vector<xac_pp_actor_material_layer> layers;
	xac_color_rgba ambient_color;
	xac_color_rgba diffuse_color;
	xac_color_rgba specular_color;
	xac_color_rgba emissive_color;
	float shine = 1.f;
	float shine_strength = 1.f;
	float opacity = 1.f;
	float ior = 1.f;
	bool double_sided = false;
	bool wireframe = false;
};
struct xac_context {
	std::vector<xac_pp_actor_node> root_nodes;
	std::vector<xac_pp_actor_node> nodes;
	std::vector<xac_pp_actor_material> materials;
	uint32_t max_standard_materials = 0; // Normal standard materials (diffuse, ambient, normal, etc)
	uint32_t max_fx_materials = 0; // Especial effect materials (smoke, fire, glow, etc)
	bool ignore_length = false; // Length of chunks sometimes doesn't align with their true size
};

template<typename T>
T parse_xac_any_binary(const char** start, const char* end, parsers::error_handler& err) {
	if(*start + sizeof(T) >= end) {
		err.accumulated_errors += "buffer overflow access! (" + err.file_name + ")\n";
		return T{};
	}
	T obj = *reinterpret_cast<const T*>(*start);
	*start += sizeof(T);
	return obj;
}

const char* parse_xac_cstring(const char* start, const char* end, parsers::error_handler& err);
const char* parse_xac_cstring_nodiscard(std::string& out, const char* start, const char* end, parsers::error_handler& err);

struct xac_metadata_chunk_header {
	uint32_t reposition_mask = 0;
	int32_t reposition_node = 0;
	uint8_t exporter_major_version = 0;
	uint8_t exporter_minor_version = 0;
	uint8_t unused[2] = { 0 };
	float retarget_root_offset = 0.f;
};

struct xac_material_block_v1_chunk_header {
	uint32_t num_total_materials = 0;
	uint32_t num_standard_materials = 0;
	uint32_t num_fx_materials = 0;
};

struct xac_material_v2_chunk_header {
	xac_color_rgba ambient_color;
	xac_color_rgba diffuse_color;
	xac_color_rgba specular_color;
	xac_color_rgba emissive_color;
	float shine = 1.f;
	float shine_strength = 1.f;
	float opacity = 1.f;
	float ior = 1.f;
	bool double_sided = false;
	bool wireframe = false;
	uint8_t unused = 0;
	uint8_t num_layers = 0;
};
struct xac_material_layer_v2_header {
	float amount = 1.f;
	float u_offset = 0.f;
	float v_offset = 0.f;
	float u_tiling = 0.f;
	float v_tiling = 0.f;
	float rotation = 0.f; //radians?
	int16_t material_id = -1;
	uint8_t map_type = 0;
	uint8_t unused = 0;
};

struct xac_node_hierachy_v1_chunk_header {
	uint32_t num_nodes = 0;
	uint32_t num_root_nodes = 0;
};
struct xac_node_hierachy_v1_node_header {
	xac_vector4f rotation;
	xac_vector4f scale_rotation;
	xac_vector3f position;
	xac_vector3f scale;
	float unused[3] = { 0 };
	uint32_t unknown[2] = { 0 };
	int32_t parent_id = -1;
	uint32_t num_children = 0;
	int32_t include_in_bounds_calc = 0;
	xac_mat4x4 transform;
	float importance_factor = 0.f;
};

struct xac_mesh_v1_chunk_header {
	int32_t node_id = -1;
	uint32_t num_influence_ranges = 0;
	uint32_t num_vertices = 0;
	uint32_t num_indices = 0;
	uint32_t num_sub_meshes = 0;
	uint32_t num_attribute_layers = 0;
	uint8_t is_collision_mesh = 0;
	uint8_t unused[3] = { 0 };
	// vertex blocks[vertex_block_count]
};
struct xac_vertex_block_v1_header {
	uint32_t ident = 0;
	uint32_t size = 0;
	uint8_t keep_original = 0;
	uint8_t is_scale_factor = 0;
	uint8_t unused[2] = { 0, 0 };
	// data[size * count];
};
enum class xac_vertex_block_v1_type : uint32_t {
	vertex = 0,
	normal = 1,
	weight = 2,
	texcoord = 3,
	color32 = 4,
	influence_indices = 5,
	color128 = 6,
	count
};
struct xac_submesh_v1_header {
	uint32_t num_indices = 0;
	uint32_t num_vertices = 0;
	int32_t material_id = -1;
	uint32_t num_bones = 0;
};

struct xac_skinning_v3_chunk_header {
	int32_t node_id = -1;
	uint32_t num_local_bones = 0;
	uint32_t num_influences = 0;
	uint8_t is_for_collision_mesh = 0;
	uint8_t unused[3] = { 0, 0, 0 };
	// vertex blocks[vertex_block_count]
};
struct xac_skinning_v3_influence_entry {
	float weight = 0.f;
	int16_t bone_id = -1;
	uint8_t unused[2] = { 0, 0 };
};
struct xac_skinning_v3_influence_range {
	uint32_t first_influence_index = 0;
	uint32_t num_influences = 0;
};

struct xac_header {
	uint8_t ident[4] = { 'X', 'A', 'C', ' ' };
	uint8_t major_version = 1;
	uint8_t minor_version = 0;
	uint8_t big_endian = 0;
	uint8_t multiply_order = 0;
};
struct xac_chunk_header {
	uint32_t ident = 0;
	uint32_t len = 0;
	uint32_t version = 0;
	// data[len]
};

emfx::xac_pp_actor_node* get_parent_node(xac_context& context, uint32_t node_index);
emfx::xac_pp_actor_node* get_visible_parent_node(xac_context& context, uint32_t node_index);
void parse_xac(xac_context& context, const char* start, const char* end, parsers::error_handler& err);
void finish(xac_context& context);

enum class xsm_chunk_type : uint8_t {
	metadata = 0xc9,
	bone_animation = 0xca,
};
template<typename T> struct xsm_animation_key {
	T value;
	float time;
};
struct xsm_animation {
	std::string node;
	emfx::xac_vector4f pose_rotation;
	emfx::xac_vector4f bind_pose_rotation;
	emfx::xac_vector4f pose_scale_rotation;
	emfx::xac_vector4f bind_pose_scale_rotation;
	emfx::xac_vector3f pose_position;
	emfx::xac_vector3f pose_scale;
	emfx::xac_vector3f bind_pose_position;
	emfx::xac_vector3f bind_pose_scale;
	std::vector<xsm_animation_key<emfx::xac_vector3f>> position_keys;
	std::vector<xsm_animation_key<emfx::xac_vector3f>> scale_keys;
	std::vector<xsm_animation_key<emfx::xac_vector4f>> rotation_keys;
	std::vector<xsm_animation_key<emfx::xac_vector4f>> scale_rotation_keys;
	float max_error = 0.f;
	int32_t bone_id = -1; //assigned by processer
	int32_t parent_id = -1;

	glm::mat4x4 bone_matrix = glm::mat4x4(1.f);
	glm::mat4x4 parent_matrix = glm::mat4x4(1.f);

	float total_anim_time = 0.f;
	float total_position_anim_time = 0.f;
	float total_rotation_anim_time = 0.f;
	float total_scale_anim_time = 0.f;
	float total_scale_rotation_anim_time = 0.f;

	xsm_animation_key<xac_vector3f> get_position_key(uint32_t i) const;
	xsm_animation_key<xac_vector4f> get_rotation_key(uint32_t i) const;
	xsm_animation_key<xac_vector3f> get_scale_key(uint32_t i) const;
	xsm_animation_key<xac_vector4f> get_scale_rotation_key(uint32_t i) const;

	uint32_t get_position_key_index(float time) const;
	uint32_t get_rotation_key_index(float time) const;
	uint32_t get_scale_key_index(float time) const;
	uint32_t get_scale_rotation_key_index(float time) const;

	float get_player_scale_factor(float t1, float t2, float time) const;
};
struct xsm_context {
	std::vector<xsm_animation> animations;
	bool use_quat_16 = false;
	bool ignore_length = false;
};
struct xsm_header : public xac_header { };
struct xsm_chunk_header : public xac_chunk_header { };

void parse_xsm(xsm_context& context, const char* start, const char* end, parsers::error_handler& err);
void finish(xsm_context& context);

// Known animations
enum class animation_type : uint8_t {
	idle = 0,
	move = 1,
	attack = 2,
	count
};

}
