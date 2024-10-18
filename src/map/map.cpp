#include <cmath>
#include <numbers>
#include <glm/glm.hpp>
#include <glm/mat3x3.hpp>
#include <unordered_map>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/polar_coordinates.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "stb_image.h"
#include "system_state.hpp"
#include "parsers_declarations.hpp"
#include "math_fns.hpp"
#include "prng.hpp"
#include "demographics.hpp"
#include "map.hpp"
#include "glm/fwd.hpp"
#include "texture.hpp"
#include "province.hpp"
#include "province_templates.hpp"
#include "gui_element_types.hpp"
#include "pdqsort.h"
#include "reports.hpp"

#include "xac.hpp"

emfx::xac_pp_actor_material_layer get_diffuse_layer(emfx::xac_pp_actor_material const& mat) {
	for(const auto& layer : mat.layers) {
		if(layer.map_type == emfx::xac_pp_material_map_type::diffuse) {
			return layer;
		}
	}
	return mat.layers.empty() ? emfx::xac_pp_actor_material_layer{} : mat.layers[0];
}

namespace duplicates {
	glm::vec2 get_port_location(sys::state& state, dcon::province_id p) {
		if(auto pt = state.world.province_get_port_to(p); pt) {
			auto adj = state.world.get_province_adjacency_by_province_pair(p, pt);
			assert(adj);
			auto id = adj.index();
			auto const& border = state.map_state.map_data.borders[id];
			auto const& vertex = state.map_state.map_data.border_vertices[border.start_index + border.count / 2];
			glm::vec2 map_size = glm::vec2(state.map_state.map_data.size_x, state.map_state.map_data.size_y);

			glm::vec2 v1 = glm::vec2(vertex.position_.x, vertex.position_.y);
			v1 /= 65535.f;
			return v1 * map_size;
		}
		return glm::vec2{};
	}

	bool is_sea_province(sys::state& state, dcon::province_id prov_id) {
		return prov_id.index() >= state.province_definitions.first_sea_province.index();
	}

	glm::vec2 get_navy_location(sys::state& state, dcon::province_id prov_id) {
		if(is_sea_province(state, prov_id)) {
			return state.world.province_get_mid_point(prov_id);
		}
		return get_port_location(state, prov_id);
	}

	glm::vec2 get_army_location(sys::state& state, dcon::province_id prov_id) {
		return state.world.province_get_mid_point(prov_id);
	}
}

namespace map {

	void display_data::update_borders(sys::state& state) {

	}

	void add_nation_visible_provinces(sys::state& state, dcon::nation_id n) {
		for(auto pc : state.world.nation_get_province_control_as_nation(n)) {
			auto p = pc.get_province();
			state.map_state.visible_provinces[province::to_map_id(p)] = true;
			for(auto c : state.world.province_get_province_adjacency(p)) {
				auto p2 = c.get_connected_provinces(0) == p ? c.get_connected_provinces(1) : c.get_connected_provinces(0);
				state.map_state.visible_provinces[province::to_map_id(p2)] = true;
			}
		}
		for(auto ac : state.world.nation_get_army_control_as_controller(n)) {
			auto p = ac.get_army().get_location_from_army_location();
			state.map_state.visible_provinces[province::to_map_id(p)] = true;
			if(!ac.get_army().get_is_retreating() && !ac.get_army().get_black_flag()) {
				for(auto c : state.world.province_get_province_adjacency(p)) {
					auto p2 = c.get_connected_provinces(0) == p ? c.get_connected_provinces(1) : c.get_connected_provinces(0);
					state.map_state.visible_provinces[province::to_map_id(p2)] = true;
				}
			}
		}
		for(auto nc : state.world.nation_get_navy_control_as_controller(n)) {
			auto p = nc.get_navy().get_location_from_navy_location();
			state.map_state.visible_provinces[province::to_map_id(p)] = true;
			if(!nc.get_navy().get_is_retreating()) {
				for(auto c : state.world.province_get_province_adjacency(p)) {
					auto p2 = c.get_connected_provinces(0) == p ? c.get_connected_provinces(1) : c.get_connected_provinces(0);
					state.map_state.visible_provinces[province::to_map_id(p2)] = true;
				}
			}
		}
	}

	void display_data::update_fog_of_war(sys::state& state) {
		// update fog of war too
		std::vector<uint32_t> province_fows(state.world.province_size() + 1, 0xFFFFFFFF);
		state.map_state.visible_provinces.clear();
		if(state.local_player_nation != state.world.national_identity_get_nation_from_identity_holder(state.national_definitions.rebel_id)
		&& (state.user_settings.fow_enabled || state.network_mode != sys::network_mode_type::single_player)
		&& !state.current_scene.is_lobby) {
			state.map_state.visible_provinces.resize(state.world.province_size() + 1, false);
			add_nation_visible_provinces(state, state.local_player_nation);
			for(auto urel : state.world.nation_get_overlord_as_ruler(state.local_player_nation)) {
				add_nation_visible_provinces(state, urel.get_subject());
			}
			for(auto rel : state.world.nation_get_diplomatic_relation(state.local_player_nation)) {
				if(rel.get_are_allied()) {
					auto n = rel.get_related_nations(0) == state.local_player_nation ? rel.get_related_nations(1) : rel.get_related_nations(0);
					add_nation_visible_provinces(state, n);
					for(auto urel : state.world.nation_get_overlord_as_ruler(n)) {
						add_nation_visible_provinces(state, urel.get_subject());
					}
				}
			}
			for(auto n : state.world.in_nation) {
				if(n != state.local_player_nation && military::are_allied_in_war(state, n, state.local_player_nation)) {
					add_nation_visible_provinces(state, n);
				}
			}
			for(auto p : state.world.in_province) {
				province_fows[province::to_map_id(p)] = uint32_t(state.map_state.visible_provinces[province::to_map_id(p)] ? 0xFFFFFFFF : 0x7B7B7B7B);
			}
		} else {
			state.map_state.visible_provinces.resize(state.world.province_size() + 1, true);
		}
		gen_prov_color_texture(textures[texture_province_fow], province_fows, 1);
	}

	void create_textured_line_vbo(GLuint vbo, std::vector<textured_line_vertex>& data) {
		// Create and populate the border VBO
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		if(!data.empty()) {
			glBufferData(GL_ARRAY_BUFFER, sizeof(textured_line_vertex) * data.size(), data.data(), GL_STATIC_DRAW);
		}
		//glBindVertexBuffer(0, vbo, 0, sizeof(textured_line_vertex));
		glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(textured_line_vertex), (const void*)offsetof(textured_line_vertex, position_));
		glVertexAttribPointer(1, 2, GL_SHORT, GL_TRUE, sizeof(textured_line_vertex), (const void*)offsetof(textured_line_vertex, normal_direction_));
		glVertexAttribPointer(2, 1, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(textured_line_vertex), (const void*)offsetof(textured_line_vertex, texture_coord_));
		glVertexAttribPointer(3, 1, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(textured_line_vertex), (const void*)offsetof(textured_line_vertex, distance_));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
	}

	void create_textured_line_b_vbo(GLuint vbo, std::vector<textured_line_vertex_b>& data) {
		// Create and populate the border VBO
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		if(!data.empty()) {
			glBufferData(GL_ARRAY_BUFFER, sizeof(textured_line_vertex_b) * data.size(), data.data(), GL_STATIC_DRAW);
		}
		// Bind the VBO to 0 of the VAO
		//glBindVertexBuffer(0, vbo, 0, sizeof(textured_line_vertex_b));
		glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(textured_line_vertex_b), (const void*)offsetof(textured_line_vertex_b, position_));
		glVertexAttribPointer(1, 2, GL_SHORT, GL_TRUE, sizeof(textured_line_vertex_b), (const void*)offsetof(textured_line_vertex_b, previous_point_));
		glVertexAttribPointer(2, 2, GL_SHORT, GL_TRUE, sizeof(textured_line_vertex_b), (const void*)offsetof(textured_line_vertex_b, next_point_));
		glVertexAttribPointer(3, 1, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(textured_line_vertex_b), (const void*)offsetof(textured_line_vertex_b, texture_coord_));
		glVertexAttribPointer(4, 1, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(textured_line_vertex_b), (const void*)offsetof(textured_line_vertex_b, distance_));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
	}

	void create_unit_arrow_vbo(GLuint vbo, std::vector<curved_line_vertex>& data) {
		// Create and populate the border VBO
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		if(!data.empty()) {
			glBufferData(GL_ARRAY_BUFFER, sizeof(curved_line_vertex) * data.size(), data.data(), GL_STATIC_DRAW);
		}
		// Bind the VBO to 0 of the VAO
		//glBindVertexBuffer(0, vbo, 0, sizeof(curved_line_vertex));
		glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(curved_line_vertex), (const void*)offsetof(curved_line_vertex, position_));
		glVertexAttribPointer(1, 2, GL_SHORT, GL_TRUE, sizeof(curved_line_vertex), (const void*)offsetof(curved_line_vertex, normal_direction_));
		glVertexAttribPointer(2, 2, GL_SHORT, GL_TRUE, sizeof(curved_line_vertex), (const void*)offsetof(curved_line_vertex, direction_));
		glVertexAttribPointer(3, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(curved_line_vertex), (const void*)offsetof(curved_line_vertex, texture_coord_));
		glVertexAttribPointer(4, 1, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(curved_line_vertex), (const void*)offsetof(curved_line_vertex, type_));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
	}

	void create_text_line_vbo(GLuint vbo) {
		// Create and populate the border VBO
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		// Bind the VBO to 0 of the VAO
		//glBindVertexBuffer(0, vbo, 0, sizeof(text_line_vertex));
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(text_line_vertex), (const void*)offsetof(text_line_vertex, position_));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(text_line_vertex), (const void*)offsetof(text_line_vertex, normal_direction_));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(text_line_vertex), (const void*)offsetof(text_line_vertex, direction_));
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(text_line_vertex), (const void*)offsetof(text_line_vertex, texture_coord_));
		glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(text_line_vertex), (const void*)offsetof(text_line_vertex, thickness_));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
	}

	void create_drag_box_vbo(GLuint vbo) {
		// Create and populate the border VBO
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		//glBindVertexBuffer(0, vbo, 0, sizeof(screen_vertex));
		glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(screen_vertex), (const void*)offsetof(screen_vertex, position_));
		glEnableVertexAttribArray(0);
	}

	void create_textured_quad_vbo(GLuint vbo) {
		// Create and populate the border VBO
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		//glBindVertexBuffer(0, vbo, 0, sizeof(textured_screen_vertex));
		glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(textured_screen_vertex), (const void*)offsetof(textured_screen_vertex, position_));
		glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(textured_screen_vertex), (const void*)offsetof(textured_screen_vertex, texcoord_));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
	}

	void display_data::create_border_ogl_objects() {
		// TODO: remove unused function
	}

	void display_data::create_meshes() {
		reports::write_debug("Creating map static meshes\n");
		{
			std::vector<map_vertex> land_vertices;
			glm::vec2 last_pos(0, 0);
			glm::vec2 pos(0, 0);
			glm::vec2 map_size(size_x, size_y);
			glm::ivec2 sections(42, 1);
			for(int y = 0; y <= sections.y; y++) {
				pos.y = float(y) / float(sections.y);
				for(int x = 0; x <= sections.x; x++) {
					pos.x = float(x) / float(sections.x);
					land_vertices.emplace_back(pos.x, pos.y);
				}
			}
			map_indices.clear();
			for(int y = 0; y < sections.y; y++) {
				auto top_row_start = y * (sections.x + 1);
				auto bottom_row_start = (y + 1) * (sections.x + 1);
				map_indices.push_back(uint16_t(bottom_row_start + 0));
				map_indices.push_back(uint16_t(top_row_start + 0));
				for(int x = 0; x < sections.x; x++) {
					map_indices.push_back(uint16_t(bottom_row_start + 1 + x));
					map_indices.push_back(uint16_t(top_row_start + 1 + x));
				}
				map_indices.push_back(std::numeric_limits<uint16_t>::max());
			}
			glBindVertexArray(vao_array[vo_land]);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_land]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(map_vertex) * land_vertices.size(), land_vertices.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(map_vertex), (const void*)offsetof(map_vertex, position_));
			glEnableVertexAttribArray(0);
		}
		{
			std::vector<map_vertex> land_vertices;
			glm::vec2 last_pos(0, 0);
			glm::vec2 pos(0, 0);
			glm::vec2 map_size(size_x, size_y);
			glm::ivec2 sections(200, 200);
			for(int y = 0; y <= sections.y; y++) {
				pos.y = float(y) / float(sections.y);
				for(int x = 0; x <= sections.x; x++) {
					pos.x = float(x) / float(sections.x);
					land_vertices.emplace_back(pos.x, pos.y);
				}
			}
			map_globe_indices.clear();
			for(int y = 0; y < sections.y; y++) {
				auto top_row_start = y * (sections.x + 1);
				auto bottom_row_start = (y + 1) * (sections.x + 1);
				map_globe_indices.push_back(uint16_t(bottom_row_start + 0));
				map_globe_indices.push_back(uint16_t(top_row_start + 0));
				for(int x = 0; x < sections.x; x++) {
					map_globe_indices.push_back(uint16_t(bottom_row_start + 1 + x));
					map_globe_indices.push_back(uint16_t(top_row_start + 1 + x));
				}
				map_globe_indices.push_back(std::numeric_limits<uint16_t>::max());
			}
			glBindVertexArray(vao_array[vo_land_globe]);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_land_globe]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(map_vertex) * land_vertices.size(), land_vertices.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(map_vertex), (const void*)offsetof(map_vertex, position_));
			glEnableVertexAttribArray(0);
		}

		// Fill and bind the VAOs and VBOs
		glBindVertexArray(vao_array[vo_border]);
		create_textured_line_b_vbo(vbo_array[vo_border], border_vertices);
		glBindVertexArray(vao_array[vo_river]);
		create_textured_line_vbo(vbo_array[vo_river], river_vertices);
		glBindVertexArray(vao_array[vo_railroad]);
		create_textured_line_vbo(vbo_array[vo_railroad], railroad_vertices);
		glBindVertexArray(vao_array[vo_coastal]);
		create_textured_line_b_vbo(vbo_array[vo_coastal], coastal_vertices);
		glBindVertexArray(vao_array[vo_unit_arrow]);
		create_unit_arrow_vbo(vbo_array[vo_unit_arrow], unit_arrow_vertices);
		glBindVertexArray(vao_array[vo_attack_unit_arrow]);
		create_unit_arrow_vbo(vbo_array[vo_attack_unit_arrow], attack_unit_arrow_vertices);
		glBindVertexArray(vao_array[vo_retreat_unit_arrow]);
		create_unit_arrow_vbo(vbo_array[vo_retreat_unit_arrow], retreat_unit_arrow_vertices);
		glBindVertexArray(vao_array[vo_strategy_unit_arrow]);
		create_unit_arrow_vbo(vbo_array[vo_strategy_unit_arrow], strategy_unit_arrow_vertices);
		glBindVertexArray(vao_array[vo_objective_unit_arrow]);
		create_unit_arrow_vbo(vbo_array[vo_objective_unit_arrow], objective_unit_arrow_vertices);
		glBindVertexArray(vao_array[vo_other_objective_unit_arrow]);
		create_unit_arrow_vbo(vbo_array[vo_other_objective_unit_arrow], other_objective_unit_arrow_vertices);
		glBindVertexArray(vao_array[vo_text_line]);
		create_text_line_vbo(vbo_array[vo_text_line]);
		glBindVertexArray(vao_array[vo_province_text_line]);
		create_text_line_vbo(vbo_array[vo_province_text_line]);
		glBindVertexArray(vao_array[vo_drag_box]);
		create_drag_box_vbo(vbo_array[vo_drag_box]);
		glBindVertexArray(vao_array[vo_selection]);
		create_textured_quad_vbo(vbo_array[vo_selection]);
		glBindVertexArray(vao_array[vo_capital]);
		create_textured_quad_vbo(vbo_array[vo_capital]);
		glBindVertexArray(0);
	}

	void display_data::clear_opengl_objects() {
		assert(loaded_map);
		loaded_map = false;
		/* We don't need to check against 0, since the delete functions already do that for us */
		glDeleteTextures(texture_count, textures);
		glDeleteTextures(texture_count, texture_arrays);
		glDeleteTextures(max_static_meshes * max_static_submeshes, &static_mesh_textures[0][0]);
		glDeleteVertexArrays(vo_count, vao_array);
		glDeleteBuffers(vo_count, vbo_array);
		/* Flags shader for deletion, but doesn't delete them until they're no longer in the rendering context */
		for(const auto& v1 : shaders) { // 3 dimensional array
			for(const auto& v2 : v1) {
				glDeleteProgram(v2);
			}
		}
		//Some graphics drivers will crash if we dont memset to 0
		std::memset(textures, 0, sizeof(textures));
		std::memset(texture_arrays, 0, sizeof(texture_arrays));
		std::memset(static_mesh_textures, 0, sizeof(static_mesh_textures));
		std::memset(vao_array, 0, sizeof(vao_array));
		std::memset(vbo_array, 0, sizeof(vbo_array));
		std::memset(shaders, 0, sizeof(shaders));
	}

	display_data::~display_data() {
		if(loaded_map) { //only clear when map is loaded
			clear_opengl_objects();
		}
	}

	std::optional<simple_fs::file> try_load_shader(simple_fs::directory& root, native_string_view name) {
		auto shader = simple_fs::open_file(root, name);
		if(!bool(shader))
		ogl::notify_user_of_fatal_opengl_error("Unable to open a necessary shader file");
		return shader;
	}

	GLuint create_program(simple_fs::file& vshader_file, simple_fs::file& fshader_file, uint32_t flags, std::string_view sv = "") {
		auto vshader_content = simple_fs::view_contents(vshader_file);
		auto fshader_content = simple_fs::view_contents(fshader_file);
		if(sv.empty()) { //no header
			auto vshader_string = std::string_view(vshader_content.data, vshader_content.file_size);
			auto fshader_string = std::string_view(fshader_content.data, fshader_content.file_size);
			return ogl::create_program(vshader_string, fshader_string, flags);
		} else { //added header
			auto vshader_string = std::string(vshader_content.data, vshader_content.file_size);
			if(!sv.empty()) {
				vshader_string = std::string(sv) + vshader_string;
			}
			auto fshader_string = std::string(fshader_content.data, fshader_content.file_size);
			if(!sv.empty()) {
				fshader_string = std::string(sv) + fshader_string;
			}
			return ogl::create_program(vshader_string, fshader_string, flags);
		}
	}

	void display_data::load_shaders(simple_fs::directory& root) {
		reports::write_debug("Loading map shaders\n");

		// Map shaders
		auto assets_dir = open_directory(root, NATIVE("assets"));
		auto shaders_dir = open_directory(assets_dir, NATIVE("shaders"));

		auto map_vshader = try_load_shader(shaders_dir, NATIVE("map_v.glsl"));
		auto map_far_fshader = try_load_shader(shaders_dir, NATIVE("map_far_f.glsl"));

		auto map_close_fshader = try_load_shader(shaders_dir, NATIVE("map_close_f.glsl"));
		auto screen_vshader = try_load_shader(shaders_dir, NATIVE("screen_v.glsl"));
		auto selection_vshader = try_load_shader(shaders_dir, NATIVE("selection_v.glsl"));
		auto white_color_fshader = try_load_shader(shaders_dir, NATIVE("white_color_f.glsl"));
		auto selection_fshader = try_load_shader(shaders_dir, NATIVE("selection_f.glsl"));

		// Line shaders
		auto line_unit_arrow_vshader = try_load_shader(shaders_dir, NATIVE("line_unit_arrow_v.glsl"));
		auto line_unit_arrow_fshader = try_load_shader(shaders_dir, NATIVE("line_unit_arrow_f.glsl"));
		auto text_line_vshader = try_load_shader(shaders_dir, NATIVE("text_line_v.glsl"));
		auto text_line_fshader = try_load_shader(shaders_dir, NATIVE("text_line_f.glsl"));
		auto tline_vshader = try_load_shader(shaders_dir, NATIVE("textured_line_v.glsl"));
		auto tline_fshader = try_load_shader(shaders_dir, NATIVE("textured_line_f.glsl"));
		auto tlineb_vshader = try_load_shader(shaders_dir, NATIVE("textured_line_b_v.glsl"));
		auto tlineb_fshader = try_load_shader(shaders_dir, NATIVE("textured_line_b_f.glsl"));
		auto model3d_vshader = try_load_shader(shaders_dir, NATIVE("model3d_v.glsl"));
		auto model3d_fshader = try_load_shader(shaders_dir, NATIVE("model3d_f.glsl"));

		std::string_view vs_header =
			"#define HAVE_COLOR_COLORMAP true\n"
			"#define HAVE_WATER_COLORMAP true\n"
			"#define POLITICAL_LIGHTNESS 0.7f\n"
			"#define POLITICAL_TERRAIN_MIX 0.3f\n"
			"#define COLOR_LIGHTNESS 1.5f\n"
			"#define OVERLAY_MIX 1.0f\n"
			"#define HSV_CORRECT false\n";
		auto gfx_dir = open_directory(root, NATIVE("gfx"));
		auto fx_dir = open_directory(gfx_dir, NATIVE("fx"));
		if(auto f = simple_fs::open_file(fx_dir, NATIVE("terrain_2_0.fx")); f) {
			auto contents = simple_fs::view_contents(*f);
			auto str = std::string(contents.data, contents.data + contents.file_size);
			if(str.find("//y1 = ((y1*2.0f + ColorColor))/3.0f;") != std::string::npos) {
				//has colormap
				vs_header =
					"#define HAVE_COLOR_COLORMAP true\n"
					"#define HAVE_WATER_COLORMAP true\n"
					"#define POLITICAL_LIGHTNESS 0.5f\n"
					"#define POLITICAL_TERRAIN_MIX 0.4f\n"
					"#define COLOR_LIGHTNESS 1.5f\n"
					"#define OVERLAY_MIX 1.0f\n"
					"#define HSV_CORRECT false\n";
			}
			if(str.find("//The map is a flat plane") != std::string::npos) {
				//belle cartographie compatibility
				vs_header =
					"#define HAVE_COLOR_COLORMAP true\n"
					"#define HAVE_WATER_COLORMAP false\n"
					"#define POLITICAL_LIGHTNESS 0.5f\n"
					"#define POLITICAL_TERRAIN_MIX 0.5f\n"
					"#define COLOR_LIGHTNESS 1.5f\n"
					"#define OVERLAY_MIX 0.25f\n"
					"#define HSV_CORRECT true\n";
			}
		}

		for(uint32_t j = 0; j < uint8_t(sys::projection_mode::num_of_modes); j++) {
			if(map_vshader && map_far_fshader) {
				shaders[j][shader_far_terrain] = create_program(*map_vshader, *map_far_fshader, j, vs_header);
			}
			if(map_vshader && map_close_fshader) {
				shaders[j][shader_close_terrain] = create_program(*map_vshader, *map_close_fshader, j, vs_header);
			}
			if(tline_vshader && tlineb_fshader) {
				shaders[j][shader_textured_line] = create_program(*tline_vshader, *tline_fshader, j);
			}
			if(tline_vshader && tlineb_fshader) {
				shaders[j][shader_railroad_line] = create_program(*tline_vshader, *tlineb_fshader, j);
			}
			if(tlineb_vshader && tlineb_fshader) {
				shaders[j][shader_borders] = create_program(*tlineb_vshader, *tlineb_fshader, j);
			}
			if(line_unit_arrow_vshader && line_unit_arrow_fshader) {
				shaders[j][shader_line_unit_arrow] = create_program(*line_unit_arrow_vshader, *line_unit_arrow_fshader, j);
			}
			if(text_line_vshader && text_line_fshader) {
				shaders[j][shader_text_line] = create_program(*text_line_vshader, *text_line_fshader, j);
			}
			if(screen_vshader && white_color_fshader) {
				shaders[j][shader_drag_box] = create_program(*screen_vshader, *white_color_fshader, j);
			}
			if(selection_vshader && selection_fshader) {
				shaders[j][shader_selection] = create_program(*selection_vshader, *selection_fshader, j);
			}
			if(model3d_vshader && model3d_fshader) {
				shaders[j][shader_map_standing_object] = create_program(*model3d_vshader, *model3d_fshader, j);
			}
			for(uint32_t i = 0; i < shader_count; i++) {
				if(shaders[j][i] == 0)
					continue;
				shader_uniforms[j][i][uniform_provinces_texture_sampler] = glGetUniformLocation(shaders[j][i], "provinces_texture_sampler");
				shader_uniforms[j][i][uniform_offset] = glGetUniformLocation(shaders[j][i], "offset");
				shader_uniforms[j][i][uniform_aspect_ratio] = glGetUniformLocation(shaders[j][i], "aspect_ratio");
				shader_uniforms[j][i][uniform_zoom] = glGetUniformLocation(shaders[j][i], "zoom");
				shader_uniforms[j][i][uniform_map_size] = glGetUniformLocation(shaders[j][i], "map_size");
				shader_uniforms[j][i][uniform_rotation] = glGetUniformLocation(shaders[j][i], "rotation");
				shader_uniforms[j][i][uniform_gamma] = glGetUniformLocation(shaders[j][i], "gamma");
				shader_uniforms[j][i][uniform_subroutines_index] = glGetUniformLocation(shaders[j][i], "subroutines_index");
				shader_uniforms[j][i][uniform_time] = glGetUniformLocation(shaders[j][i], "time");
				shader_uniforms[j][i][uniform_terrain_texture_sampler] = glGetUniformLocation(shaders[j][i], "terrain_texture_sampler");
				shader_uniforms[j][i][uniform_terrainsheet_texture_sampler] = glGetUniformLocation(shaders[j][i], "terrainsheet_texture_sampler");
				shader_uniforms[j][i][uniform_water_normal] = glGetUniformLocation(shaders[j][i], "water_normal");
				shader_uniforms[j][i][uniform_colormap_water] = glGetUniformLocation(shaders[j][i], "colormap_water");
				shader_uniforms[j][i][uniform_colormap_terrain] = glGetUniformLocation(shaders[j][i], "colormap_terrain");
				shader_uniforms[j][i][uniform_overlay] = glGetUniformLocation(shaders[j][i], "overlay");
				shader_uniforms[j][i][uniform_province_color] = glGetUniformLocation(shaders[j][i], "province_color");
				shader_uniforms[j][i][uniform_colormap_political] = glGetUniformLocation(shaders[j][i], "colormap_political");
				shader_uniforms[j][i][uniform_province_highlight] = glGetUniformLocation(shaders[j][i], "province_highlight");
				shader_uniforms[j][i][uniform_stripes_texture] = glGetUniformLocation(shaders[j][i], "stripes_texture");
				shader_uniforms[j][i][uniform_province_fow] = glGetUniformLocation(shaders[j][i], "province_fow");
				shader_uniforms[j][i][uniform_diag_border_identifier] = glGetUniformLocation(shaders[j][i], "diag_border_identifier");
				shader_uniforms[j][i][uniform_subroutines_index_2] = glGetUniformLocation(shaders[j][i], "subroutines_index_2");
				shader_uniforms[j][i][uniform_line_texture] = glGetUniformLocation(shaders[j][i], "line_texture");
				shader_uniforms[j][i][uniform_texture_sampler] = glGetUniformLocation(shaders[j][i], "texture_sampler");
				shader_uniforms[j][i][uniform_opaque] = glGetUniformLocation(shaders[j][i], "opaque");
				shader_uniforms[j][i][uniform_is_black] = glGetUniformLocation(shaders[j][i], "is_black");
				shader_uniforms[j][i][uniform_border_width] = glGetUniformLocation(shaders[j][i], "border_width");
				shader_uniforms[j][i][uniform_unit_arrow] = glGetUniformLocation(shaders[j][i], "unit_arrow");
				shader_uniforms[j][i][uniform_model_offset] = glGetUniformLocation(shaders[j][i], "model_offset");
				shader_uniforms[j][i][uniform_target_facing] = glGetUniformLocation(shaders[j][i], "target_facing");
				shader_uniforms[j][i][uniform_target_topview_fixup] = glGetUniformLocation(shaders[j][i], "target_topview_fixup");
				shader_uniforms[j][i][uniform_width] = glGetUniformLocation(shaders[j][i], "width");
				shader_uniforms[j][i][uniform_counter_factor] = glGetUniformLocation(shaders[j][i], "counter_factor");
				//model
				shader_uniforms[j][i][uniform_model_position] = glGetUniformLocation(shaders[j][i], "model_position");
				shader_uniforms[j][i][uniform_model_scale] = glGetUniformLocation(shaders[j][i], "model_scale");
				shader_uniforms[j][i][uniform_model_rotation] = glGetUniformLocation(shaders[j][i], "model_rotation");
				shader_uniforms[j][i][uniform_model_scale_rotation] = glGetUniformLocation(shaders[j][i], "model_scale_rotation");
				shader_uniforms[j][i][uniform_model_proj_view] = glGetUniformLocation(shaders[j][i], "model_proj_view");
			}
			//special bone matrix
			if(shaders[j][shader_map_standing_object]) {
				bone_matrices_uniform_array[j] = glGetUniformLocation(shaders[j][shader_map_standing_object], "bones_matrices");
			}
		}
	}

	/*	Obtains the animation matrix corresponding to the given animation bone, at a given time
		@param an The animation object holding the information of the bone id
		@param time_counter The time to play the animation at, you should feed this the global time counter since it
		is mod'ed into the local animation time
		@return The local animation matrix */
	glm::mat4x4 get_animation_bone_matrix(emfx::xsm_animation const& an, float time_counter) {
		auto pos_index = an.get_position_key_index(time_counter);
		auto pos1 = an.get_position_key(pos_index);
		auto pos2 = an.get_position_key(pos_index + 1);
		glm::mat4x4 mt = glm::translate(glm::mat4x4(1.f),
			glm::mix(
				glm::vec3(pos1.value.x, pos1.value.y, pos1.value.z),
				glm::vec3(pos2.value.x, pos2.value.y, pos2.value.z),
				an.get_player_scale_factor(pos1.time, pos2.time, time_counter)
			)
		);
		auto sca_index = an.get_scale_key_index(time_counter);
		auto sca1 = an.get_scale_key(sca_index);
		auto sca2 = an.get_scale_key(sca_index + 1);
		glm::mat4x4 ms = glm::scale(glm::mat4x4(1.f),
			glm::mix(
				glm::vec3(sca1.value.x, sca1.value.y, sca1.value.z),
				glm::vec3(sca2.value.x, sca2.value.y, sca2.value.z),
				an.get_player_scale_factor(sca1.time, sca2.time, time_counter)
			)
		);
		auto rot_index = an.get_rotation_key_index(time_counter);
		auto rot1 = an.get_rotation_key(rot_index);
		auto rot2 = an.get_rotation_key(rot_index + 1);
		glm::mat4x4 mr = glm::toMat4(glm::normalize(
			glm::slerp(
				glm::quat(rot1.value.x, rot1.value.y, rot1.value.z, rot1.value.w),
				glm::quat(rot2.value.x, rot2.value.y, rot2.value.z, rot2.value.w),
				an.get_player_scale_factor(rot1.time, rot2.time, time_counter)
			)
		));
		auto rsc_index = an.get_scale_rotation_key_index(time_counter);
		auto rsc1 = an.get_scale_rotation_key(rsc_index);
		auto rsc2 = an.get_scale_rotation_key(rsc_index + 1);
		glm::mat4x4 mu = glm::toMat4(glm::normalize(
			glm::slerp(
				glm::quat(rsc1.value.x, rsc1.value.y, rsc1.value.z, rsc1.value.w),
				glm::quat(rsc2.value.x, rsc2.value.y, rsc2.value.z, rsc1.value.w),
				an.get_player_scale_factor(rsc1.time, rsc2.time, time_counter)
			)
		));
		return mt * mr * ms;
	}

	void get_hierachical_animation_bone(std::vector<emfx::xsm_animation> const& list, std::array<glm::mat4x4, map::display_data::max_bone_matrices>& matrices, uint32_t start, uint32_t count, uint32_t current, float time_counter, glm::mat4x4 parent_m) {
		auto const node_m = get_animation_bone_matrix(list[current], time_counter);
		auto const global_m = parent_m * node_m;
		for(uint32_t i = start; i < start + count; i++) { //recurse thru all of our children
			if(i != current && list[current].bone_id == list[i].parent_id) {
				get_hierachical_animation_bone(list, matrices, start, count, i, time_counter, global_m);
			}
		}
		matrices[list[current].bone_id] = global_m;
	}

	/*	Finds a given root node, or most importantly, the end of a chain on a given bone hierachy,
		for example the torso, if the arm is given! */
	uint32_t get_root_node(std::vector<emfx::xsm_animation> const& list, uint32_t start, uint32_t count, uint32_t current) {
		for(uint32_t i = start; i < start + count; i++) {
			if(i != current && list[current].parent_id == list[i].bone_id) {
				return get_root_node(list, start, count, i); //our parent
			}
		}
		return current; //ourselves
	}

	float sum_of_bone_chain_length(std::vector<emfx::xsm_animation> const& list, uint32_t start, uint32_t count, uint32_t current, float sum) {
		for(uint32_t i = start; i < start + count; i++) {
			if(i != current && list[current].parent_id == list[i].bone_id) {
				auto const a = glm::vec3(list[current].bone_pose_matrix[3]);
				auto const b = glm::vec3(list[i].bone_pose_matrix[3]);
				return glm::distance(a, b) + sum_of_bone_chain_length(list, start, count, i, sum);
			}
		}
		return sum;
	}

	void display_data::render_models(sys::state& state, std::vector<model_render_command> const& list, float time_counter, sys::projection_mode map_view_mode, float zoom) {
		glBindVertexArray(vao_array[vo_static_mesh]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_static_mesh]);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_DEPTH_BUFFER_BIT);
		glClearDepth(1.f);
		glDepthFunc(GL_LESS);
		glCullFace(GL_FRONT);
		glActiveTexture(GL_TEXTURE0);
		//
		constexpr float animation_zoom_threshold = map::zoom_very_close + 2.f;
		if(zoom > animation_zoom_threshold) { // do animation logic only when close enough
			auto const emfx_size = std::min(map::display_data::max_static_meshes, uint32_t(state.ui_defs.emfx.size()));
			std::vector<std::array<glm::mat4x4, max_bone_matrices>> final_idle_matrices(emfx_size, std::array<glm::mat4x4, max_bone_matrices>{ glm::mat4x4(1.f) });
			std::vector<std::array<glm::mat4x4, max_bone_matrices>> final_move_matrices(emfx_size, std::array<glm::mat4x4, max_bone_matrices>{ glm::mat4x4(1.f) });
			std::vector<std::array<glm::mat4x4, max_bone_matrices>> final_attack_matrices(emfx_size, std::array<glm::mat4x4, max_bone_matrices>{ glm::mat4x4(1.f) });
			std::vector<uint8_t> model_needs_matrix(emfx_size);
			for(auto const& obj : list) {
				model_needs_matrix[obj.emfx.index()] |= (obj.anim == emfx::animation_type::idle) ? 0x80 : 0x00;
				model_needs_matrix[obj.emfx.index()] |= (obj.anim == emfx::animation_type::move) ? 0x40 : 0x00;
				model_needs_matrix[obj.emfx.index()] |= (obj.anim == emfx::animation_type::attack) ? 0x20 : 0x00;
			}
			for(uint32_t i = 0; i < emfx_size; i++) {
				if((model_needs_matrix[i] & 0x80) != 0) {
					auto start = static_mesh_idle_animation_start[i];
					auto count = static_mesh_idle_animation_count[i];
					auto& matrices = final_idle_matrices[i];
					for(uint32_t k = start; k < start + count; k++) {
						auto anim_time = std::fmod(time_counter * (1.f / animations[k].total_anim_time), 1.f);
						get_hierachical_animation_bone(animations, matrices, start, count, k, anim_time, glm::mat4x4(1.f));
					}
					for(uint32_t k = start; k < start + count; k++) {
						matrices[animations[k].bone_id] = matrices[animations[k].bone_id] * glm::inverse(animations[k].bone_bind_pose_matrix);
					}
				}
				if((model_needs_matrix[i] & 0x40) != 0) {
					auto start = static_mesh_move_animation_start[i];
					auto count = static_mesh_move_animation_count[i];
					auto& matrices = final_move_matrices[i];
					for(uint32_t k = start; k < start + count; k++) {
						auto anim_time = std::fmod(time_counter * (1.f / animations[k].total_anim_time), 1.f);
						get_hierachical_animation_bone(animations, matrices, start, count, k, anim_time, glm::mat4x4(1.f));
					}
					for(uint32_t k = start; k < start + count; k++) {
						matrices[animations[k].bone_id] = matrices[animations[k].bone_id] * glm::inverse(animations[k].bone_bind_pose_matrix);
					}
				}
				if((model_needs_matrix[i] & 0x20) != 0) {
					auto start = static_mesh_attack_animation_start[i];
					auto count = static_mesh_attack_animation_count[i];
					auto& matrices = final_attack_matrices[i];
					for(uint32_t k = start; k < start + count; k++) {
						auto anim_time = std::fmod(time_counter * (1.f / animations[k].total_anim_time), 1.f);
						get_hierachical_animation_bone(animations, matrices, start, count, k, anim_time, glm::mat4x4(1.f));
					}
					for(uint32_t k = start; k < start + count; k++) {
						matrices[animations[k].bone_id] = matrices[animations[k].bone_id] * glm::inverse(animations[k].bone_bind_pose_matrix);
					}
				}
			}
			for(const auto& obj : list) {
				auto index = obj.emfx.index();
				if(obj.anim == emfx::animation_type::idle) {
					auto const& final_matrix = final_idle_matrices[index];
					glUniformMatrix4fv(bone_matrices_uniform_array[uint8_t(map_view_mode)], GLsizei(final_matrix.size()), GL_FALSE, (const GLfloat*)final_matrix.data());
				} else if(obj.anim == emfx::animation_type::move) {
					auto const& final_matrix = final_move_matrices[index];
					glUniformMatrix4fv(bone_matrices_uniform_array[uint8_t(map_view_mode)], GLsizei(final_matrix.size()), GL_FALSE, (const GLfloat*)final_matrix.data());
				} else if(obj.anim == emfx::animation_type::attack) {
					auto const& final_matrix = final_attack_matrices[index];
					glUniformMatrix4fv(bone_matrices_uniform_array[uint8_t(map_view_mode)], GLsizei(final_matrix.size()), GL_FALSE, (const GLfloat*)final_matrix.data());
				} else {
					static const std::array<glm::mat4x4, max_bone_matrices> ident_matrix{ glm::mat4x4(1.f) };
					glUniformMatrix4fv(bone_matrices_uniform_array[uint8_t(map_view_mode)], GLsizei(ident_matrix.size()), GL_FALSE, (const GLfloat*)ident_matrix.data());
				}
				glUniform2f(shader_uniforms[uint8_t(map_view_mode)][shader_map_standing_object][uniform_model_offset], obj.pos.x, obj.pos.y);
				glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_map_standing_object][uniform_target_facing], obj.facing);
				//REMOVE -- glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_map_standing_object][uniform_target_topview_fixup], obj.topview_fixup);
				for(uint32_t i = 0; i < static_mesh_starts[index].size(); i++) {
					auto tid = static_mesh_textures[index][i];
					tid = tid ? tid : obj.flag_texid;
					glBindTexture(GL_TEXTURE_2D, tid);
					glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_map_standing_object][uniform_time], time_counter * static_mesh_scrolling_factor[index][i]);
					glDrawArrays(GL_TRIANGLES, static_mesh_starts[index][i], static_mesh_counts[index][i]);
				}
			}
		} else {
			//default matrix -- overriden if zoomed in enough
			static const std::array<glm::mat4x4, max_bone_matrices> ident_matrix{ glm::mat4x4(1.f) };
			glUniformMatrix4fv(bone_matrices_uniform_array[uint8_t(map_view_mode)], GLsizei(ident_matrix.size()), GL_FALSE, (const GLfloat*)ident_matrix.data());
			glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_map_standing_object][uniform_target_facing], 0.f); //not fix rotations
			glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_map_standing_object][uniform_time], time_counter); //dont need to animate scrolling textures
			for(const auto& obj : list) {
				auto index = obj.emfx.index();
				glUniform2f(shader_uniforms[uint8_t(map_view_mode)][shader_map_standing_object][uniform_model_offset], obj.pos.x, obj.pos.y);
				for(uint32_t i = 0; i < static_mesh_starts[index].size(); i++) {
					glBindTexture(GL_TEXTURE_2D, static_mesh_textures[index][i]);
					glDrawArrays(GL_TRIANGLES, static_mesh_starts[index][i], static_mesh_counts[index][i]);
				}
			}
		}
		glDisable(GL_DEPTH_TEST);
	}

	void display_data::update_models(sys::state& state) {
		constexpr float dist_step = 1.77777f;
		model_render_list.clear();
		// Province flags
		province::for_each_land_province(state, [&](dcon::province_id p) {
			for(uint32_t i = 0; i < uint32_t(state.province_definitions.num_allocated_provincial_flags); i++) {
				dcon::provincial_flag_id pfid{ dcon::provincial_flag_id::value_base_t(i) };
				if(state.world.province_get_flag_variables(p, pfid)) {
					auto center = state.world.province_get_mid_point(p);
					model_render_list.emplace_back(model_province_flag[i], center, 0.f, emfx::animation_type::none, 0);
				}
			}
		});
		// Train stations
		province::for_each_land_province(state, [&](dcon::province_id p) {
			auto const level = state.world.province_get_building_level(p, economy::province_building_type::railroad);
			if(level > 0) {
				auto center = state.world.province_get_mid_point(p);
				auto seed_r = p.index() + state.world.province_get_nation_from_province_ownership(p).index() + level;
				auto theta = glm::atan(float(rng::get_random(state, seed_r, seed_r ^ level) % 360) / 180.f - 1.f);
				model_render_list.emplace_back(model_train_station, center, theta, emfx::animation_type::none, 0);
			}
		});
		// Naval bases
		province::for_each_land_province(state, [&](dcon::province_id p) {
			auto units = state.world.province_get_navy_location_as_location(p);
			auto const level = state.world.province_get_building_level(p, economy::province_building_type::naval_base);
			auto p1 = duplicates::get_navy_location(state, p);
			auto p2 = state.world.province_get_mid_point(p);
			auto theta = glm::atan(p2.y - p1.y, p2.x - p1.x);
			if(units.begin() != units.end()) { //full
				model_render_list.emplace_back(model_naval_base_ships[level], p1, theta, emfx::animation_type::none, 0);
			} else { //empty
				model_render_list.emplace_back(model_naval_base[level], p1, theta, emfx::animation_type::none, 0);
			}
		});
		// Fort
		province::for_each_land_province(state, [&](dcon::province_id p) {
			auto const level = state.world.province_get_building_level(p, economy::province_building_type::fort);
			if(level > 0) {
				auto center = state.world.province_get_mid_point(p);
				auto pos = center + glm::vec2(dist_step, -dist_step); //bottom left (from center)
				auto seed_r = p.index() - state.world.province_get_nation_from_province_ownership(p).index() + level;
				auto theta = glm::atan(float(rng::get_random(state, seed_r, seed_r ^ level) % 360) / 180.f - 1.f);
				model_render_list.emplace_back(model_fort[level], pos, theta, emfx::animation_type::none, 0);
			}
		});
		// Factory
		province::for_each_land_province(state, [&](dcon::province_id p) {
			auto factories = state.world.province_get_factory_location_as_province(p);
			if(factories.begin() != factories.end()) {
				auto fac = state.world.factory_location_get_factory(*factories.begin());
				auto level = state.world.factory_get_level(fac);
				auto center = state.world.province_get_mid_point(p);
				auto pos = center + glm::vec2(-dist_step, -dist_step); //bottom right (from center)
				auto seed_r = p.index() + state.world.province_get_nation_from_province_ownership(p).index();
				auto theta = glm::atan(float(rng::get_random(state, seed_r, seed_r ^ level) % 360) / 180.f - 1.f);
				model_render_list.emplace_back(model_factory, pos, theta, emfx::animation_type::none, 0);
			}
		});
		// Construction
		province::for_each_land_province(state, [&](dcon::province_id p) {
			if(state.map_state.visible_provinces[province::to_map_id(p)]) {
				auto lc = state.world.province_get_province_building_construction(p);
				if(lc.begin() != lc.end()) {
					auto center = state.world.province_get_mid_point(p);
					auto pos = center + glm::vec2(dist_step, dist_step); //top left (from center)
					auto seed_r = p.index() + state.world.province_get_nation_from_province_ownership(p).index();
					auto theta = glm::atan(float(rng::get_random(state, seed_r, seed_r ^ int32_t(pos.x)) % 360) / 180.f - 1.f);
					model_render_list.emplace_back(model_construction, pos, float(rng::reduce(seed_r, 180)), emfx::animation_type::idle, 0);
				}
			}
		});
		// Construction [naval]
		province::for_each_land_province(state, [&](dcon::province_id p) {
			if(state.map_state.visible_provinces[province::to_map_id(p)]) {
				auto lc = state.world.province_get_province_naval_construction(p);
				if(lc.begin() != lc.end()) {
					auto p1 = duplicates::get_navy_location(state, p);
					auto p2 = state.world.province_get_mid_point(p);
					auto theta = glm::atan(p2.y - p1.y, p2.x - p1.x);
					model_render_list.emplace_back(model_construction_naval, p1, theta, emfx::animation_type::idle, 0);
				}
			}
		});
		// Construction [military]
		province::for_each_land_province(state, [&](dcon::province_id p) {
			if(state.map_state.visible_provinces[province::to_map_id(p)]) {
				for(const auto pl : state.world.province_get_pop_location(p)) {
					auto lc = pl.get_pop().get_province_land_construction();
					if(lc.begin() != lc.end()) {
						auto center = state.world.province_get_mid_point(p);
						auto pos = center + glm::vec2(-dist_step, -dist_step); //top left (from center)
						auto seed_r = pl.get_pop().id.index();
						auto theta = glm::atan(float(rng::get_random(state, seed_r, seed_r ^ int32_t(pos.x)) % 360) / 180.f - 1.f);
						model_render_list.emplace_back(model_construction_military, pos, float(rng::reduce(seed_r, 180)), emfx::animation_type::idle, 0);
						break;
					}
				}
			}
		});
		// Blockaded
		province::for_each_land_province(state, [&](dcon::province_id p) {
			if(military::province_is_blockaded(state, p)) {
				auto p1 = duplicates::get_navy_location(state, p);
				auto p2 = state.world.province_get_mid_point(p);
				auto theta = glm::atan(p2.y - p1.y, p2.x - p1.x);
				model_render_list.emplace_back(model_blockaded, p1, -theta, emfx::animation_type::none, 0);
			}
		});
		// Siege
		province::for_each_land_province(state, [&](dcon::province_id p) {
			if(state.map_state.visible_provinces[province::to_map_id(p)] && military::province_is_under_siege(state, p)) {
				auto center = state.world.province_get_mid_point(p);
				model_render_list.emplace_back(model_siege, center, 0.f, emfx::animation_type::idle, 0);
			}
		});
		// Render armies
		province::for_each_land_province(state, [&](dcon::province_id p) {
			auto units = state.world.province_get_army_location_as_location(p);
			if(state.map_state.visible_provinces[province::to_map_id(p)] && units.begin() != units.end()) {
				auto p1 = state.world.province_get_mid_point(p);
				auto p2 = p1;
				dcon::emfx_object_id unit_model;
				dcon::unit_type_id unit_type;
				dcon::emfx_object_id moving_model;
				dcon::unit_type_id moving_type;
				dcon::nation_id n;
				for(const auto unit : units) {
					auto path = unit.get_army().get_path();
					if(path.size() > 0) {
						p2 = state.world.province_get_mid_point(path[path.size() - 1]);
					}
					auto gc = unit.get_army().get_controller_from_army_control().get_identity_from_identity_holder().get_graphical_culture();
					if(!unit_model && path.size() == 0) {
						for(const auto sm : unit.get_army().get_army_membership()) {
							auto utid = sm.get_regiment().get_type();
							if(auto model = model_gc_unit[uint8_t(gc)][utid.index()]; model) {
								unit_type = utid;
								unit_model = model;
								n = sm.get_army().get_controller_from_army_control();
								break;
							}
						}
					} else if(!moving_model && path.size() > 0) {
						for(const auto sm : unit.get_army().get_army_membership()) {
							auto utid = sm.get_regiment().get_type();
							if(auto model = model_gc_unit[uint8_t(gc)][utid.index()]; model) {
								moving_type = utid;
								moving_model = model;
								n = sm.get_army().get_controller_from_army_control();
								break;
							}
						}
					}
					if(unit_model && moving_model) {
						break;
					}
				}
				auto lb = state.world.province_get_land_battle_location(p);
				if(lb.begin() != lb.end()) {
					model_render_list.emplace_back(unit_model, glm::vec2(p1.x - dist_step * 2.f, p1.y), 0.f, emfx::animation_type::attack, 0);
					model_render_list.emplace_back(unit_model, glm::vec2(p1.x + dist_step * 2.f, p1.y), -math::pi, emfx::animation_type::attack, 0);
				} else if(unit_model) {
					model_render_list.emplace_back(unit_model, glm::vec2(p1.x, p1.y), math::pi / 2.f, emfx::animation_type::idle, 0);
				}
				if(moving_model) {
					auto theta = glm::atan(p2.y - p1.y, p2.x - p1.x);
					if(p1 == p2) {
						theta = -math::pi / 2.f;
					}
					model_render_list.emplace_back(moving_model, glm::vec2(p1.x + dist_step, p1.y + dist_step), -theta, emfx::animation_type::move, 0);
				}
				if(unit_model || moving_model) {
					n = n ? n : state.world.national_identity_get_nation_from_identity_holder(state.national_definitions.rebel_id);
					auto flag_tex_id = ogl::get_flag_handle(state, state.world.nation_get_identity_from_identity_holder(n), culture::flag_type(state.world.government_type_get_flag(state.world.nation_get_government_type(n))));
					model_render_list.emplace_back(model_flag, glm::vec2(p1.x, p1.y), math::pi / 2.f, emfx::animation_type::idle, flag_tex_id);
				}
			}
		});
		// Render navies
		province::for_each_sea_province(state, [&](dcon::province_id p) {
			auto units = state.world.province_get_navy_location_as_location(p);
			if(state.map_state.visible_provinces[province::to_map_id(p)] && units.begin() != units.end()) {
				auto p1 = state.world.province_get_mid_point(p);
				auto p2 = p1;
				dcon::emfx_object_id unit_model;
				dcon::unit_type_id unit_type;
				dcon::nation_id n;
				bool is_move = false;
				for(const auto unit : units) {
					if(auto path = unit.get_navy().get_path(); path.size() > 0) {
						p2 = state.world.province_get_mid_point(path[path.size() - 1]);
						is_move = true;
					}
					auto gc = unit.get_navy().get_controller_from_navy_control().get_identity_from_identity_holder().get_graphical_culture();
					for(const auto sm : unit.get_navy().get_navy_membership()) {
						auto utid = sm.get_ship().get_type();
						if(auto model = model_gc_unit[0][utid.index()]; model) {
							unit_type = utid;
							unit_model = model;
							n = sm.get_navy().get_controller_from_navy_control();
							break;
						}
					}
					if(unit_model) {
						break;
					}
				}
				auto theta = glm::atan(p2.y - p1.y, p2.x - p1.x);
				if(p1 == p2)
					theta = -math::pi / 2.f;
				auto lb = state.world.province_get_naval_battle_location(p);
				if(lb.begin() != lb.end()) {
					model_render_list.emplace_back(unit_model, glm::vec2(p1.x - dist_step * 2.f, p1.y), -math::pi / 2.f, emfx::animation_type::idle, 0);
					model_render_list.emplace_back(unit_model, glm::vec2(p1.x + dist_step * 2.f, p1.y), -math::pi / 2.f, emfx::animation_type::idle, 0);
				} else {
					model_render_list.emplace_back(unit_model, glm::vec2(p1.x, p1.y), -theta, emfx::animation_type::idle, 0);
					n = n ? n : state.world.national_identity_get_nation_from_identity_holder(state.national_definitions.rebel_id);
					auto flag_tex_id = ogl::get_flag_handle(state, state.world.nation_get_identity_from_identity_holder(n), culture::flag_type(state.world.government_type_get_flag(state.world.nation_get_government_type(n))));
					model_render_list.emplace_back(model_flag_floating, glm::vec2(p1.x, p1.y), math::pi / 2.f, emfx::animation_type::idle, flag_tex_id);
				}
			}
		});
		//remove dead
		for(uint32_t i = 0; i < uint32_t(model_render_list.size()); i++) {
			if(!model_render_list[i].emfx) {
				model_render_list[i] = model_render_list.back();
				model_render_list.pop_back();
				--i;
			}
		}
	}

	void display_data::render(sys::state& state, glm::vec2 screen_size, glm::vec2 offset, float zoom, sys::projection_mode map_view_mode, map_mode::mode active_map_mode, glm::mat3 globe_rotation, float time_counter) {
		if(!loaded_map)
			return;

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if(ogl::msaa_enabled(state)) {
			glBindFramebuffer(GL_FRAMEBUFFER, state.open_gl.msaa_framebuffer);
			glClearColor(0.f, 0.f, 0.f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		} else {
			glClearColor(0.f, 0.f, 0.f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT);
		}

		if(state.ui_state.bg_gfx_id) {
			// Render default background
			glUseProgram(state.open_gl.ui_shader_program);
			glUniform1i(state.open_gl.ui_shader_texture_sampler_uniform, 0);
			glUniform1i(state.open_gl.ui_shader_secondary_texture_sampler_uniform, 1);
			glUniform1f(state.open_gl.ui_shader_screen_width_uniform, float(state.x_size));
			glUniform1f(state.open_gl.ui_shader_screen_height_uniform, float(state.y_size));
			glUniform1f(state.open_gl.ui_shader_gamma_uniform, state.user_settings.gamma);
			glViewport(0, 0, state.x_size, state.y_size);
			auto const& gfx_def = state.ui_defs.gfx[state.ui_state.bg_gfx_id];
			if(gfx_def.primary_texture_handle) {
				auto texid = ogl::get_texture_handle(state, gfx_def.primary_texture_handle, gfx_def.is_partially_transparent());
				if(texid) {
					glBindVertexArray(state.open_gl.global_square_vao);
					glBindBuffer(GL_ARRAY_BUFFER, state.open_gl.global_square_buffer);
					glUniform4f(state.open_gl.ui_shader_d_rect_uniform, 0.f, 0.f, float(state.x_size), float(state.y_size));
					glBindTexture(GL_TEXTURE_2D, texid);
					GLuint subroutines[2] = { ogl::parameters::enabled, ogl::parameters::no_filter };
					glUniform2ui(state.open_gl.ui_shader_subroutines_index_uniform, subroutines[0], subroutines[1]);
					glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
				}
			}
		}

		// Load general shader stuff, used by both land and borders
		auto load_shader = [&](GLuint program) {
			glUseProgram(shaders[uint8_t(map_view_mode)][program]);
			float aspect_ratio = state.map_state.get_aspect_ratio(screen_size, map_view_mode);
			glm::mat4x4 globe_rot4x4(1.f);
			for(uint32_t i = 0; i < 3; i++) {
				globe_rot4x4[i][0] = globe_rotation[i][0];
				globe_rot4x4[i][1] = globe_rotation[i][1];
				globe_rot4x4[i][2] = globe_rotation[i][2];
			}
			glUniformMatrix4fv(shader_uniforms[uint8_t(map_view_mode)][program][uniform_rotation], 1, GL_FALSE, glm::value_ptr(globe_rot4x4));
			float counter_factor = state.map_state.get_counter_factor(state.user_settings.map_counter_factor);
			auto mvp = state.map_state.get_mvp_matrix(map_view_mode, globe_rot4x4, offset, aspect_ratio, counter_factor);
			glUniformMatrix4fv(shader_uniforms[uint8_t(map_view_mode)][program][uniform_model_proj_view], 1, GL_FALSE, glm::value_ptr(mvp));
			glUniform2f(shader_uniforms[uint8_t(map_view_mode)][program][uniform_offset], offset.x, offset.y);
			glUniform1f(shader_uniforms[uint8_t(map_view_mode)][program][uniform_aspect_ratio], aspect_ratio);
			glUniform1f(shader_uniforms[uint8_t(map_view_mode)][program][uniform_zoom], zoom);
			glUniform2f(shader_uniforms[uint8_t(map_view_mode)][program][uniform_map_size], GLfloat(size_x), GLfloat(size_y));
			glUniform1ui(shader_uniforms[uint8_t(map_view_mode)][program][uniform_subroutines_index], GLuint(map_view_mode));
			glUniform1f(shader_uniforms[uint8_t(map_view_mode)][program][uniform_time], time_counter);
			glUniform1f(shader_uniforms[uint8_t(map_view_mode)][program][uniform_counter_factor], counter_factor);
			glUniform1f(shader_uniforms[uint8_t(map_view_mode)][program][uniform_gamma], state.user_settings.gamma);
		};

		if(shaders[uint8_t(map_view_mode)][shader_close_terrain] && active_map_mode == map_mode::mode::terrain || zoom > map::zoom_close) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textures[texture_provinces]);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, textures[texture_terrain]);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D_ARRAY, texture_arrays[texture_array_terrainsheet]);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, textures[texture_water_normal]);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, textures[texture_colormap_water]);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, textures[texture_colormap_terrain]);
			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, textures[texture_overlay]);
			glActiveTexture(GL_TEXTURE8);
			glBindTexture(GL_TEXTURE_2D_ARRAY, texture_arrays[texture_array_province_color]);
			glActiveTexture(GL_TEXTURE9);
			glBindTexture(GL_TEXTURE_2D, textures[texture_colormap_political]);
			glActiveTexture(GL_TEXTURE10);
			glBindTexture(GL_TEXTURE_2D, textures[texture_province_highlight]);
			glActiveTexture(GL_TEXTURE11);
			glBindTexture(GL_TEXTURE_2D, textures[texture_stripes]);
			glActiveTexture(GL_TEXTURE13);
			glBindTexture(GL_TEXTURE_2D, textures[texture_province_fow]);
			glActiveTexture(GL_TEXTURE15);
			glBindTexture(GL_TEXTURE_2D, textures[texture_diag_border_identifier]);
			load_shader(shader_close_terrain);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_close_terrain][uniform_provinces_texture_sampler], 0);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_close_terrain][uniform_terrain_texture_sampler], 1);
			//glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_close_terrain][uniform_unused_texture_2], 2);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_close_terrain][uniform_terrainsheet_texture_sampler], 3);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_close_terrain][uniform_water_normal], 4);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_close_terrain][uniform_colormap_water], 5);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_close_terrain][uniform_colormap_terrain], 6);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_close_terrain][uniform_overlay], 7);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_close_terrain][uniform_province_color], 8);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_close_terrain][uniform_colormap_political], 9);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_close_terrain][uniform_province_highlight], 10);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_close_terrain][uniform_stripes_texture], 11);
			//glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_close_terrain][uniform_unused_texture_12], 12);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_close_terrain][uniform_province_fow], 13);
			//glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_close_terrain][uniform_unused_texture_14], 14);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_close_terrain][uniform_diag_border_identifier], 15);
			{ // Land specific shader uniform
				// get_land()
				GLuint fragment_subroutines = 0;
				if(active_map_mode == map_mode::mode::terrain)
				fragment_subroutines = 0; // get_land_terrain/get_water_terrain()
				else if(zoom > map::zoom_close)
				fragment_subroutines = 1; // get_land_political_close/get_water_terrain()
				glUniform1ui(shader_uniforms[uint8_t(map_view_mode)][shader_close_terrain][uniform_subroutines_index_2], fragment_subroutines);
			}
			if(map_view_mode == sys::projection_mode::flat) {
				glEnable(GL_PRIMITIVE_RESTART);
				glPrimitiveRestartIndex(std::numeric_limits<uint16_t>::max());
				glBindVertexArray(vao_array[vo_land]);
				glDrawElements(GL_TRIANGLE_STRIP, GLsizei(map_indices.size()), GL_UNSIGNED_SHORT, map_indices.data());
				glDisable(GL_PRIMITIVE_RESTART);
			} else {
				glEnable(GL_PRIMITIVE_RESTART);
				glPrimitiveRestartIndex(std::numeric_limits<uint16_t>::max());
				glBindVertexArray(vao_array[vo_land_globe]);
				glDrawElements(GL_TRIANGLE_STRIP, GLsizei(map_globe_indices.size()), GL_UNSIGNED_SHORT, map_globe_indices.data());
				glDisable(GL_PRIMITIVE_RESTART);
			}
		} else if(shaders[uint8_t(map_view_mode)][shader_far_terrain]) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textures[texture_provinces]);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, textures[texture_terrain]);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D_ARRAY, texture_arrays[texture_array_terrainsheet]);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, textures[texture_overlay]);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D_ARRAY, texture_arrays[texture_array_province_color]);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, textures[texture_colormap_political]);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, textures[texture_province_highlight]);
			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, textures[texture_stripes]);
			glActiveTexture(GL_TEXTURE8); //addendum
			glBindTexture(GL_TEXTURE_2D, textures[texture_colormap_water]);
			load_shader(shader_far_terrain);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_far_terrain][uniform_provinces_texture_sampler], 0);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_far_terrain][uniform_terrain_texture_sampler], 1);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_far_terrain][uniform_terrainsheet_texture_sampler], 2);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_far_terrain][uniform_overlay], 3);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_far_terrain][uniform_province_color], 4);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_far_terrain][uniform_colormap_political], 5);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_far_terrain][uniform_province_highlight], 6);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_far_terrain][uniform_stripes_texture], 7);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_far_terrain][uniform_colormap_water], 8); //addendum
			if(map_view_mode == sys::projection_mode::flat) {
				glEnable(GL_PRIMITIVE_RESTART);
				glPrimitiveRestartIndex(std::numeric_limits<uint16_t>::max());
				glBindVertexArray(vao_array[vo_land]);
				glDrawElements(GL_TRIANGLE_STRIP, GLsizei(map_indices.size()), GL_UNSIGNED_SHORT, map_indices.data());
				glDisable(GL_PRIMITIVE_RESTART);
			} else {
				glEnable(GL_PRIMITIVE_RESTART);
				glPrimitiveRestartIndex(std::numeric_limits<uint16_t>::max());
				glBindVertexArray(vao_array[vo_land_globe]);
				glDrawElements(GL_TRIANGLE_STRIP, GLsizei(map_globe_indices.size()), GL_UNSIGNED_SHORT, map_globe_indices.data());
				glDisable(GL_PRIMITIVE_RESTART);
			}
		}

		// Draw the rivers
		if(shaders[uint8_t(map_view_mode)][shader_textured_line] && state.user_settings.rivers_enabled) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textures[texture_river_body]);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, textures[texture_colormap_water]);

			load_shader(shader_textured_line);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_textured_line][uniform_line_texture], 0);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_textured_line][uniform_colormap_water], 1);
			glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_textured_line][uniform_width], 0.00008f);

			glBindVertexArray(vao_array[vo_river]);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_river]);
			glMultiDrawArrays(GL_TRIANGLE_STRIP, river_starts.data(), river_counts.data(), GLsizei(river_starts.size()));
		}

		// Draw the railroads
		if(shaders[uint8_t(map_view_mode)][shader_railroad_line] && state.user_settings.railroads_enabled && zoom > map::zoom_close && !railroad_vertices.empty()) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textures[texture_railroad]);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, textures[texture_colormap_water]);

			load_shader(shader_railroad_line);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_railroad_line][uniform_line_texture], 0);
			glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_railroad_line][uniform_colormap_water], 1);
			glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_railroad_line][uniform_width], 0.0001f);
			glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_railroad_line][uniform_time], 0.f);

			glBindVertexArray(vao_array[vo_railroad]);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_railroad]);
			glMultiDrawArrays(GL_TRIANGLE_STRIP, railroad_starts.data(), railroad_counts.data(), GLsizei(railroad_starts.size()));
		}

		if(state.network_state.save_slock.load(std::memory_order::acquire) == false) {
			// a constant, as to not depend on a scenario/save being reloaded
			/*std::vector<bool> province_on_screen(state.world.province_size(), false);
			for(const auto p : state.world.in_province) {
				static glm::vec2 tmp; //unused variable, used as a temporal for storage
				auto const norm_coord = state.map_state.normalize_map_coord(p.get_mid_point());
				province_on_screen[p.id.index()] = state.map_state.map_to_screen(state, norm_coord, screen_size, tmp);
			}
			for(const auto adj : state.world.in_province_adjacency) {
				auto p1 = adj.get_connected_provinces(0);
				auto p2 = adj.get_connected_provinces(1);
				auto b = province_on_screen[p1.id.index()] || province_on_screen[p2.id.index()];
				province_on_screen[p1.id.index()] = b;
				province_on_screen[p2.id.index()] = b;
			}*/
			std::vector<bool> province_on_screen(state.world.province_size(), true);
			auto const border_is_visible = [&](dcon::province_adjacency_id adj) {
				return true;
				//auto p1 = state.world.province_adjacency_get_connected_provinces(adj, 0);
				//auto p2 = state.world.province_adjacency_get_connected_provinces(adj, 1);
				//return province_on_screen[p1.index()] || province_on_screen[p2.index()];
			};

			if(vao_array[vo_border] && shaders[uint8_t(map_view_mode)][shader_borders]) {
				// Default border parameters
				constexpr float border_type_national = 0.f;
				constexpr float border_type_provincial = 1.f;
				constexpr float border_type_regional = 2.f;
				constexpr float border_type_coastal = 3.f;
				static std::vector<GLint> b_starts; //data sensitive
				static std::vector<GLint> b_counts;
				static uint32_t b_index = 0;
				if(b_starts.size() < borders.size()) {
					b_starts.resize(borders.size());
					b_counts.resize(borders.size());
				}
				// NORMAL BORDERS
				load_shader(shader_borders);
				glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_borders][uniform_time], 0.f); //no scrolling
				glBindVertexArray(vao_array[vo_border]);
				glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_border]);
				//impassible borders
				if(zoom > map::zoom_close) {
					if(zoom > map::zoom_very_close) { // Render province borders
						b_index = 0; //reset index
						for(auto b : borders) {
							if(border_is_visible(b.adj)
							&& (state.world.province_adjacency_get_type(b.adj) & (province::border::non_adjacent_bit | province::border::coastal_bit | province::border::impassible_bit | province::border::national_bit | province::border::state_bit)) == 0) {
								b_starts[b_index] = b.start_index;
								b_counts[b_index] = b.count;
								++b_index;
							}
						}
						if(b_index > 0) {
							glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_borders][uniform_width], 0.0001f); // width
							glActiveTexture(GL_TEXTURE0);
							glBindTexture(GL_TEXTURE_2D, textures[texture_prov_border]);
							glMultiDrawArrays(GL_TRIANGLE_STRIP, b_starts.data(), b_counts.data(), GLsizei(b_index));
						}
					}
					// Render state borders
					b_index = 0;
					for(auto b : borders) {
						if(border_is_visible(b.adj)
						&& (state.world.province_adjacency_get_type(b.adj) & (province::border::non_adjacent_bit | province::border::coastal_bit | province::border::impassible_bit | province::border::national_bit | province::border::state_bit)) == province::border::state_bit) {
							b_starts[b_index] = b.start_index;
							b_counts[b_index] = b.count;
							++b_index;
						}
					}
					if(b_index > 0) {
						glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_borders][uniform_width], 0.0002f); // width
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, textures[texture_state_border]);
						glMultiDrawArrays(GL_TRIANGLE_STRIP, b_starts.data(), b_counts.data(), GLsizei(b_index));
					}
					// Impassable
					b_index = 0;
					for(auto b : borders) {
						if(border_is_visible(b.adj)
						&& (state.world.province_adjacency_get_type(b.adj) & (province::border::non_adjacent_bit | province::border::coastal_bit | province::border::impassible_bit)) == province::border::impassible_bit) {
							b_starts[b_index] = b.start_index;
							b_counts[b_index] = b.count;
							++b_index;
						}
					}
					if(b_index > 0) {
						glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_borders][uniform_width], 0.0003f); // width
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, textures[texture_imp_border]);
						glMultiDrawArrays(GL_TRIANGLE_STRIP, b_starts.data(), b_counts.data(), GLsizei(b_index));
					}
					// national borders
					b_index = 0;
					for(auto b : borders) {
						if(border_is_visible(b.adj)
						&& (state.world.province_adjacency_get_type(b.adj) & (province::border::non_adjacent_bit | province::border::coastal_bit | province::border::national_bit | province::border::impassible_bit)) == province::border::national_bit) {
							b_starts[b_index] = b.start_index;
							b_counts[b_index] = b.count;
							++b_index;
						}
					}
					if(b_index > 0) {
						glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_borders][uniform_width], 0.0003f); // width
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, textures[texture_national_border]);
						glMultiDrawArrays(GL_TRIANGLE_STRIP, b_starts.data(), b_counts.data(), GLsizei(b_index));
					}
				} else {
					// national borders
					b_index = 0;
					for(auto b : borders) {
						if(border_is_visible(b.adj)
						&& (state.world.province_adjacency_get_type(b.adj) & (province::border::non_adjacent_bit | province::border::coastal_bit | province::border::national_bit)) == province::border::national_bit) {
							b_starts[b_index] = b.start_index;
							b_counts[b_index] = b.count;
							++b_index;
						}
					}
					if(b_index > 0) {
						glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_borders][uniform_width], 0.0003f); // width
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, textures[texture_state_border]);
						glMultiDrawArrays(GL_TRIANGLE_STRIP, b_starts.data(), b_counts.data(), GLsizei(b_index));
					}
				}
				if(state.map_state.selected_province || state.current_scene.borders == game_scene::borders_granularity::nation) {
					if(state.current_scene.borders == game_scene::borders_granularity::nation) {
						auto n = state.world.province_get_nation_from_province_ownership(state.map_state.selected_province);
						if(!n)
						n = state.local_player_nation;
						if(n) {
							b_index = 0;
							for(auto b : borders) {
								auto p0 = state.world.province_adjacency_get_connected_provinces(b.adj, 0);
								auto p1 = state.world.province_adjacency_get_connected_provinces(b.adj, 1);
								if((state.world.province_get_nation_from_province_ownership(p0) == n
								|| state.world.province_get_nation_from_province_ownership(p1) == n)
								&& (state.world.province_adjacency_get_type(b.adj) & (province::border::non_adjacent_bit | province::border::coastal_bit | province::border::national_bit)) != 0) {
									b_starts[b_index] = b.start_index;
									b_counts[b_index] = b.count;
									++b_index;
								}
							}
							if(b_index > 0) {
								glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_borders][uniform_width], zoom > map::zoom_close ? 0.0004f : 0.00085f); // width
								glActiveTexture(GL_TEXTURE0);
								glBindTexture(GL_TEXTURE_2D, textures[texture_state_border]);
								glMultiDrawArrays(GL_TRIANGLE_STRIP, b_starts.data(), b_counts.data(), GLsizei(b_index));
							}
						}
					} else if(state.current_scene.borders == game_scene::borders_granularity::state) {
						auto owner = state.world.province_get_nation_from_province_ownership(state.map_state.selected_province);
						if(owner) {
							auto siid = state.world.province_get_state_membership(state.map_state.selected_province);
							//per state
							b_index = 0;
							for(auto b : borders) {
								auto p0 = state.world.province_adjacency_get_connected_provinces(b.adj, 0);
								auto p1 = state.world.province_adjacency_get_connected_provinces(b.adj, 1);
								if((state.world.province_get_state_membership(p0) == siid
								|| state.world.province_get_state_membership(p1) == siid)
								&& (state.world.province_adjacency_get_type(b.adj) & (province::border::non_adjacent_bit | province::border::coastal_bit | province::border::state_bit | province::border::national_bit)) != 0) {
									b_starts[b_index] = b.start_index;
									b_counts[b_index] = b.count;
									++b_index;
								}
							}
							if(b_index > 0) {
								glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_borders][uniform_width], zoom > map::zoom_close ? 0.0004f : 0.00085f); // width
								glActiveTexture(GL_TEXTURE0);
								glBindTexture(GL_TEXTURE_2D, textures[texture_state_border]);
								glMultiDrawArrays(GL_TRIANGLE_STRIP, b_starts.data(), b_counts.data(), GLsizei(b_index));
							}
						}
					} else if(state.current_scene.borders == game_scene::borders_granularity::province) {
						b_index = 0;
						for(auto b : borders) {
							auto p0 = state.world.province_adjacency_get_connected_provinces(b.adj, 0);
							auto p1 = state.world.province_adjacency_get_connected_provinces(b.adj, 1);
							if(p0 == state.map_state.selected_province || p1 == state.map_state.selected_province) {
								b_starts[b_index] = b.start_index;
								b_counts[b_index] = b.count;
								++b_index;
							}
						}
						if(b_index > 0) {
							glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_borders][uniform_width], zoom > map::zoom_close ? 0.0004f : 0.00085f); // width
							glActiveTexture(GL_TEXTURE0);
							glBindTexture(GL_TEXTURE_2D, textures[texture_state_border]);
							glMultiDrawArrays(GL_TRIANGLE_STRIP, b_starts.data(), b_counts.data(), GLsizei(b_index));
						}
					}
				}
				dcon::province_id prov{};
				glm::vec2 map_pos;
				if(!state.ui_state.under_mouse && state.map_state.screen_to_map(glm::vec2(state.mouse_x_position, state.mouse_y_position), screen_size, state.map_state.current_view(state), map_pos)) {
					map_pos *= glm::vec2(float(state.map_state.map_data.size_x), float(state.map_state.map_data.size_y));
					auto idx = int32_t(state.map_state.map_data.size_y - map_pos.y) * int32_t(state.map_state.map_data.size_x) + int32_t(map_pos.x);
					if(0 <= idx && size_t(idx) < state.map_state.map_data.province_id_map.size() && state.map_state.map_data.province_id_map[idx] < province::to_map_id(state.province_definitions.first_sea_province)) {
						auto fat_id = dcon::fatten(state.world, province::from_map_id(state.map_state.map_data.province_id_map[idx]));
						prov = province::from_map_id(state.map_state.map_data.province_id_map[idx]);
						auto owner = state.world.province_get_nation_from_province_ownership(prov);
						if(owner && state.current_scene.borders == game_scene::borders_granularity::nation) {
							//per nation
							b_index = 0;
							for(auto b : borders) {
								auto p0 = state.world.province_adjacency_get_connected_provinces(b.adj, 0);
								auto p1 = state.world.province_adjacency_get_connected_provinces(b.adj, 1);
								if((state.world.province_get_nation_from_province_ownership(p0) == owner
								|| state.world.province_get_nation_from_province_ownership(p1) == owner)
								&& (state.world.province_adjacency_get_type(b.adj) & (province::border::non_adjacent_bit | province::border::coastal_bit | province::border::national_bit)) != 0) {
									b_starts[b_index] = b.start_index;
									b_counts[b_index] = b.count;
									++b_index;
								}
							}
							if(b_index > 0) {
								glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_borders][uniform_width], zoom > map::zoom_close ? 0.0004f : 0.00085f); // width
								glActiveTexture(GL_TEXTURE0);
								glBindTexture(GL_TEXTURE_2D, textures[texture_hover_border]);
								glMultiDrawArrays(GL_TRIANGLE_STRIP, b_starts.data(), b_counts.data(), GLsizei(b_index));
							}
						} else if(owner && state.current_scene.borders == game_scene::borders_granularity::state) {
							auto siid = state.world.province_get_state_membership(prov);
							//per state
							b_index = 0;
							for(auto b : borders) {
								auto p0 = state.world.province_adjacency_get_connected_provinces(b.adj, 0);
								auto p1 = state.world.province_adjacency_get_connected_provinces(b.adj, 1);
								if((state.world.province_get_state_membership(p0) == siid
								|| state.world.province_get_state_membership(p1) == siid)
								&& (state.world.province_adjacency_get_type(b.adj) & (province::border::non_adjacent_bit | province::border::coastal_bit | province::border::state_bit | province::border::national_bit)) != 0) {
									b_starts[b_index] = b.start_index;
									b_counts[b_index] = b.count;
									++b_index;
								}
							}
							if(b_index > 0) {
								glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_borders][uniform_width], zoom > map::zoom_close ? 0.0004f : 0.00085f); // width
								glActiveTexture(GL_TEXTURE0);
								glBindTexture(GL_TEXTURE_2D, textures[texture_hover_border]);
								glMultiDrawArrays(GL_TRIANGLE_STRIP, b_starts.data(), b_counts.data(), GLsizei(b_index));
							}
						} else if(owner && state.current_scene.borders == game_scene::borders_granularity::province) {
							//per province
							b_index = 0;
							for(auto b : borders) {
								auto p0 = state.world.province_adjacency_get_connected_provinces(b.adj, 0);
								auto p1 = state.world.province_adjacency_get_connected_provinces(b.adj, 1);
								if(p0 == prov || p1 == prov) {
									b_starts[b_index] = b.start_index;
									b_counts[b_index] = b.count;
									++b_index;
								}
							}
							if(b_index > 0) {
								glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_borders][uniform_width], zoom > map::zoom_close ? 0.0004f : 0.00085f); // width
								glActiveTexture(GL_TEXTURE0);
								glBindTexture(GL_TEXTURE_2D, textures[texture_hover_border]);
								glMultiDrawArrays(GL_TRIANGLE_STRIP, b_starts.data(), b_counts.data(), GLsizei(b_index));
							}
						}
					}
				}
				// coasts
				if(zoom <= map::zoom_close) {
					glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_borders][uniform_width], 0.0006f); // width
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, textures[texture_coastal_border]);
					glBindVertexArray(vao_array[vo_coastal]);
					glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_coastal]);
					glMultiDrawArrays(GL_TRIANGLE_STRIP, coastal_starts.data(), coastal_counts.data(), GLsizei(coastal_starts.size()));
				} else {
					glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_borders][uniform_width], 0.0004f); // width
					glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_borders][uniform_time], time_counter * 0.25f);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, textures[texture_shoreline]);
					glBindVertexArray(vao_array[vo_coastal]);
					glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_coastal]);
					glMultiDrawArrays(GL_TRIANGLE_STRIP, coastal_starts.data(), coastal_counts.data(), GLsizei(coastal_starts.size()));
				}
			}

			if(shaders[uint8_t(map_view_mode)][shader_selection] && !selection_vertices.empty() && zoom > map::zoom_very_close) { //only render if close enough
				load_shader(shader_selection);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textures[texture_selection]);
				glBindVertexArray(vao_array[vo_selection]);
				glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_selection]);
				glDrawArrays(GL_TRIANGLES, 0, (GLsizei)selection_vertices.size());
			}

			if(shaders[uint8_t(map_view_mode)][shader_selection] && !capital_vertices.empty() && zoom > map::zoom_very_close) { //only render if close enough
				load_shader(shader_selection);
				glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_selection][uniform_time], 0.f);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textures[texture_capital]);
				glBindVertexArray(vao_array[vo_capital]);
				glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_capital]);
				glDrawArrays(GL_TRIANGLES, 0, (GLsizei)capital_vertices.size());
			}

			if(shaders[uint8_t(map_view_mode)][shader_drag_box] && !drag_box_vertices.empty()) {
				glUseProgram(shaders[uint8_t(map_view_mode)][shader_drag_box]);
				glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_drag_box][uniform_gamma], state.user_settings.gamma);
				glBindVertexArray(vao_array[vo_drag_box]);
				glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_drag_box]);
				glDrawArrays(GL_TRIANGLES, 0, (GLsizei)drag_box_vertices.size());
			}

			if(shaders[uint8_t(map_view_mode)][shader_line_unit_arrow] && zoom > map::zoom_close) { //only render if close enough
				if(!unit_arrow_vertices.empty() || !attack_unit_arrow_vertices.empty() || !retreat_unit_arrow_vertices.empty()
				|| !strategy_unit_arrow_vertices.empty() || !objective_unit_arrow_vertices.empty() || !other_objective_unit_arrow_vertices.empty()) {
					load_shader(shader_line_unit_arrow);
					glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_line_unit_arrow][uniform_border_width], 0.0035f); //width
					glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_line_unit_arrow][uniform_unit_arrow], 0);
					if(!unit_arrow_vertices.empty()) {
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, textures[texture_unit_arrow]);
						glBindVertexArray(vao_array[vo_unit_arrow]);
						glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_unit_arrow]);
						glMultiDrawArrays(GL_TRIANGLE_STRIP, unit_arrow_starts.data(), unit_arrow_counts.data(), (GLsizei)unit_arrow_counts.size());
					}
					if(!attack_unit_arrow_vertices.empty()) {
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, textures[texture_attack_unit_arrow]);
						glBindVertexArray(vao_array[vo_attack_unit_arrow]);
						glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_attack_unit_arrow]);
						glMultiDrawArrays(GL_TRIANGLE_STRIP, attack_unit_arrow_starts.data(), attack_unit_arrow_counts.data(), (GLsizei)attack_unit_arrow_counts.size());
					}
					if(!retreat_unit_arrow_vertices.empty()) {
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, textures[texture_retreat_unit_arrow]);
						glBindVertexArray(vao_array[vo_retreat_unit_arrow]);
						glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_retreat_unit_arrow]);
						glMultiDrawArrays(GL_TRIANGLE_STRIP, retreat_unit_arrow_starts.data(), retreat_unit_arrow_counts.data(), (GLsizei)retreat_unit_arrow_counts.size());
					}
					if(!strategy_unit_arrow_vertices.empty()) {
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, textures[texture_strategy_unit_arrow]);
						glBindVertexArray(vao_array[vo_strategy_unit_arrow]);
						glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_strategy_unit_arrow]);
						glMultiDrawArrays(GL_TRIANGLE_STRIP, strategy_unit_arrow_starts.data(), strategy_unit_arrow_counts.data(), (GLsizei)strategy_unit_arrow_counts.size());
					}
					if(!objective_unit_arrow_vertices.empty()) {
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, textures[texture_objective_unit_arrow]);
						glBindVertexArray(vao_array[vo_objective_unit_arrow]);
						glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_objective_unit_arrow]);
						glMultiDrawArrays(GL_TRIANGLE_STRIP, objective_unit_arrow_starts.data(), objective_unit_arrow_counts.data(), (GLsizei)objective_unit_arrow_counts.size());
					}
					if(!other_objective_unit_arrow_vertices.empty()) {
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, textures[texture_other_objective_unit_arrow]);
						glBindVertexArray(vao_array[vo_other_objective_unit_arrow]);
						glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_other_objective_unit_arrow]);
						glMultiDrawArrays(GL_TRIANGLE_STRIP, other_objective_unit_arrow_starts.data(), other_objective_unit_arrow_counts.data(), (GLsizei)retreat_unit_arrow_counts.size());
					}
				}
			}

			if(shaders[uint8_t(map_view_mode)][shader_text_line] && state.user_settings.map_label != sys::map_label_mode::none) {
				auto const& f = state.font_collection.get_font(state, text::font_selection::map_font);
				load_shader(shader_text_line);
				glUniform1i(shader_uniforms[uint8_t(map_view_mode)][shader_text_line][uniform_texture_sampler], 0);
				glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_text_line][uniform_is_black], state.user_settings.black_map_font ? 1.f : 0.f);
				glActiveTexture(GL_TEXTURE0);
				if(zoom < map::zoom_close && !text_line_vertices.empty()) {
					uint32_t index = 0;
					uint32_t last_texture = 0;
					glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_text_line][uniform_opaque], 0.f);
					glBindVertexArray(vao_array[vo_text_line]);
					glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_text_line]);
					for(uint32_t i = 0; i < uint32_t(text_line_texture_per_quad.size()); i++) {
						if(last_texture != text_line_texture_per_quad[i]) {
							if(index > 0) {
								glBindTexture(GL_TEXTURE_2D, last_texture);
								glMultiDrawArrays(GL_TRIANGLES, dyn_text_line_starts.data(), dyn_text_line_counts.data(), GLsizei(index));
							}
							last_texture = text_line_texture_per_quad[i];
							index = 0;
						}
						dyn_text_line_starts[index] = GLint(i * 6);
						dyn_text_line_counts[index] = 6;
						++index;
					}
					if(index > 0) {
						glBindTexture(GL_TEXTURE_2D, last_texture);
						glMultiDrawArrays(GL_TRIANGLES, dyn_text_line_starts.data(), dyn_text_line_counts.data(), GLsizei(index));
					}
				}
				if(zoom >= map::zoom_close && !province_text_line_vertices.empty()) {
					uint32_t index = 0;
					uint32_t last_texture = 0;
					glUniform1f(shader_uniforms[uint8_t(map_view_mode)][shader_text_line][uniform_opaque], 1.f);
					glBindVertexArray(vao_array[vo_province_text_line]);
					glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_province_text_line]);
					for(uint32_t i = 0; i < uint32_t(province_text_line_texture_per_quad.size()); i++) {
						if(last_texture != province_text_line_texture_per_quad[i]) {
							if(index > 0) {
								glBindTexture(GL_TEXTURE_2D, last_texture);
								glMultiDrawArrays(GL_TRIANGLES, dyn_province_text_line_starts.data(), dyn_province_text_line_counts.data(), GLsizei(index));
							}
							last_texture = province_text_line_texture_per_quad[i];
							index = 0;
						}
						dyn_province_text_line_starts[index] = GLint(i * 6);
						dyn_province_text_line_counts[index] = 6;
						++index;
					}
					if(index > 0) {
						glBindTexture(GL_TEXTURE_2D, last_texture);
						glMultiDrawArrays(GL_TRIANGLES, dyn_province_text_line_starts.data(), dyn_province_text_line_counts.data(), GLsizei(index));
					}
				}
			}

			if(shaders[uint8_t(map_view_mode)][shader_map_standing_object] && zoom > map::zoom_close && state.user_settings.render_models) {
				if(model_render_list.size() > 0) {
					// Render standing objects
					load_shader(shader_map_standing_object);
					render_models(state, model_render_list, time_counter, map_view_mode, zoom);
				}
			}
		}
		glBindVertexArray(0);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);

		if(ogl::msaa_enabled(state)) {
			glBindFramebuffer(GL_READ_FRAMEBUFFER, state.open_gl.msaa_framebuffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, state.open_gl.msaa_interbuffer);
			glBlitFramebuffer(0, 0, GLint(screen_size.x), GLint(screen_size.y), 0, 0, GLint(screen_size.x), GLint(screen_size.y), GL_COLOR_BUFFER_BIT, GL_NEAREST);
			// 3. now render quad with scene's visuals as its texture image
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			// draw Screen quad
			glUseProgram(state.open_gl.msaa_shader_program);
			glUniform1f(state.open_gl.msaa_uniform_gaussian_blur, state.user_settings.gaussianblur_level);
			glUniform2f(state.open_gl.msaa_uniform_screen_size, screen_size.x, screen_size.y);
			//
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, state.open_gl.msaa_texture); // use the now resolved color attachment as the quad's texture
			glBindVertexArray(state.open_gl.msaa_vao);
			//glBindBuffer(GL_ARRAY_BUFFER, state.open_gl.msaa_vbo);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}

	GLuint load_province_map(std::vector<uint16_t>& province_index, uint32_t size_x, uint32_t size_y) {
		GLuint texture_handle;
		glGenTextures(1, &texture_handle);
		if(texture_handle) {
			glBindTexture(GL_TEXTURE_2D, texture_handle);
			// Create a texture with only one byte color
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, size_x, size_y, 0, GL_RG, GL_UNSIGNED_BYTE, &province_index[0]);
			ogl::set_gltex_parameters(texture_handle, GL_TEXTURE_2D, GL_NEAREST, GL_CLAMP_TO_EDGE);
		}
		return texture_handle;
	}

	void display_data::gen_prov_color_texture(GLuint texture_handle, std::vector<uint32_t> const& prov_color, uint8_t layers) {
		if(layers == 1) {
			glBindTexture(GL_TEXTURE_2D, texture_handle);
		} else {
			glBindTexture(GL_TEXTURE_2D_ARRAY, texture_handle);
		}
		uint32_t rows = ((uint32_t)prov_color.size()) / 256;
		uint32_t left_on_last_row = ((uint32_t)prov_color.size()) % 256;
		uint32_t x = 0;
		uint32_t y = 0;
		uint32_t width = 256;
		uint32_t height = rows;
		if(layers == 1) {
			glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &prov_color[0]);
		} else {
			// Set the texture data for each layer
			for(int i = 0; i < layers; i++) {
				glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, x, y, i, width, height / layers, 1, GL_RGBA, GL_UNSIGNED_BYTE, &prov_color[i * (prov_color.size() / layers)]);
			}
		}
		x = 0;
		y = rows;
		width = left_on_last_row;
		height = 1;
		// SCHOMBERT: added a conditional to block reading from after the end in the case it is evenly divisible by 256
		// SCHOMBERT: that looks right to me, but I don't fully understand the intent
		if(left_on_last_row > 0 && layers == 1) {
			glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &prov_color[rows * 256]);
		}
	}

	void display_data::set_selected_province(sys::state& state, dcon::province_id prov_id) {
		std::vector<uint32_t> province_highlights(state.world.province_size() + 1, 0);
		state.current_scene.update_highlight_texture(state, province_highlights, prov_id);
		gen_prov_color_texture(textures[texture_province_highlight], province_highlights, 1);
	}

	void display_data::set_province_color(std::vector<uint32_t> const& prov_color) {
		gen_prov_color_texture(texture_arrays[texture_array_province_color], prov_color, 2);
	}

	void add_drag_box_line(std::vector<screen_vertex>& drag_box_vertices, glm::vec2 pos1, glm::vec2 pos2, glm::vec2 size, bool vertical) {
		if(vertical) {
			pos1.y -= size.y;
			pos2.y += size.y;
			size.y = 0;
		} else {
			pos1.x -= size.x;
			pos2.x += size.x;
			size.x = 0;
		}
		drag_box_vertices.emplace_back(pos1.x + size.x, pos1.y - size.y);
		drag_box_vertices.emplace_back(pos1.x - size.x, pos1.y + size.y);
		drag_box_vertices.emplace_back(pos2.x - size.x, pos2.y + size.y);

		drag_box_vertices.emplace_back(pos2.x - size.x, pos2.y + size.y);
		drag_box_vertices.emplace_back(pos2.x + size.x, pos2.y - size.y);
		drag_box_vertices.emplace_back(pos1.x + size.x, pos1.y - size.y);
	}

	void display_data::set_drag_box(bool draw_box, glm::vec2 pos1, glm::vec2 pos2, glm::vec2 pixel_size) {
		drag_box_vertices.clear();
		if(draw_box) {
			// Do not mindlessly update GPU data!
			if(!drag_box_vertices.empty() && pos1.x == pos2.x && pos1.y == pos2.y)
				return;

			if(pos1.x > pos2.x) {
				std::swap(pos1.x, pos2.x);
			}
			if(pos1.y > pos2.y) {
				std::swap(pos1.y, pos2.y);
			}

			glm::vec2 size = pixel_size;
			// Vertical lines
			add_drag_box_line(drag_box_vertices, { pos1.x, pos1.y }, { pos1.x, pos2.y }, size, true);
			add_drag_box_line(drag_box_vertices, { pos2.x, pos1.y }, { pos2.x, pos2.y }, size, true);

			// Horizontal lines
			add_drag_box_line(drag_box_vertices, { pos1.x, pos1.y }, { pos2.x, pos1.y }, size, false);
			add_drag_box_line(drag_box_vertices, { pos1.x, pos2.y }, { pos2.x, pos2.y }, size, false);

			glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_drag_box]);
			assert(!drag_box_vertices.empty());
			glBufferData(GL_ARRAY_BUFFER, sizeof(screen_vertex) * drag_box_vertices.size(), &drag_box_vertices[0], GL_STATIC_DRAW);
		}
	}

	void make_selection_quad(sys::state& state, std::vector<map::textured_screen_vertex>& buffer, glm::vec2 p) {
		glm::vec2 map_size(float(state.map_state.map_data.size_x), float(state.map_state.map_data.size_y));
		glm::vec2 size = glm::vec2(4.f, 4.f);
		auto p1 = (p - size) / map_size;
		auto p2 = (p + size) / map_size;
		assert(p1.x < 1.f || p1.y < 1.f);
		p1.x = std::max(0.f, p1.x);
		p1.y = std::max(0.f, p1.y);
		p2.x = std::min(1.f, p2.x);
		p2.y = std::min(1.f, p2.y);
		// 1--3
		// | /
		// |/
		// 2
		buffer.emplace_back(glm::vec2(p1.x, p1.y), glm::vec2(0.f, 0.f));
		buffer.emplace_back(glm::vec2(p2.x, p1.y), glm::vec2(1.f, 0.f));
		buffer.emplace_back(glm::vec2(p2.x, p2.y), glm::vec2(1.f, 1.f));
		//    3
		//   /|
		//  / |
		// 2--1
		buffer.emplace_back(glm::vec2(p2.x, p2.y), glm::vec2(1.f, 1.f));
		buffer.emplace_back(glm::vec2(p1.x, p2.y), glm::vec2(0.f, 1.f));
		buffer.emplace_back(glm::vec2(p1.x, p1.y), glm::vec2(0.f, 0.f));
	}

	void add_arrow_to_buffer(std::vector<map::curved_line_vertex>& buffer, glm::vec2 start, glm::vec2 end, glm::vec2 prev_normal_dir, glm::vec2 next_normal_dir, float fill_progress, bool end_arrow, float size_x, float size_y) {
		constexpr uint8_t type_unfilled = 0x0;//2;
		constexpr uint8_t type_filled = 0x80;
		constexpr uint8_t type_end = 0xC0;//3;
		glm::vec2 curr_dir = glm::normalize(end - start);
		start /= glm::vec2(size_x, size_y);
		end /= glm::vec2(size_x, size_y);

		if(fill_progress != 0) {
			if(fill_progress < 1.0f) {
				auto pos3 = glm::mix(start, end, fill_progress);
				auto midd_normal_dir = glm::vec2(-curr_dir.y, curr_dir.x);

				buffer.emplace_back(pos3, +midd_normal_dir, -curr_dir, glm::vec2(1.0f, 0.0f), type_filled);//C
				buffer.emplace_back(pos3, -midd_normal_dir, -curr_dir, glm::vec2(1.0f, 1.0f), type_filled);//D
				buffer.emplace_back(pos3, +midd_normal_dir, -curr_dir, glm::vec2(1.0f, 0.0f), type_unfilled);//C
				buffer.emplace_back(pos3, -midd_normal_dir, -curr_dir, glm::vec2(1.0f, 1.0f), type_unfilled);//D

				// Not filled - transition from "filled" to "unfilled"
				buffer.emplace_back(end, +next_normal_dir, -curr_dir, glm::vec2(1.0f, 0.0f), type_unfilled);//C
				buffer.emplace_back(end, -next_normal_dir, -curr_dir, glm::vec2(1.0f, 1.0f), type_unfilled);//D
			} else {
				// Not filled - transition from "filled" to "unfilled"
				buffer.emplace_back(end, +next_normal_dir, -curr_dir, glm::vec2(1.0f, 0.0f), type_filled);//C
				buffer.emplace_back(end, -next_normal_dir, -curr_dir, glm::vec2(1.0f, 1.0f), type_filled);//D
			}
		} else {
			// All unfilled!
			if(buffer.back().type_ == type_filled) {
				buffer.emplace_back(buffer[buffer.size() - 2]);//C
				buffer.emplace_back(buffer[buffer.size() - 2]);//D
				buffer[buffer.size() - 2].type_ = type_unfilled;
				buffer[buffer.size() - 1].type_ = type_unfilled;
			}
			buffer.emplace_back(end, +next_normal_dir, -curr_dir, glm::vec2(1.0f, 0.0f), type_unfilled);//C
			buffer.emplace_back(end, -next_normal_dir, -curr_dir, glm::vec2(1.0f, 1.0f), type_unfilled);//D
		}
		if(end_arrow) {
			if(buffer.back().type_ == type_filled) {
				buffer.emplace_back(buffer[buffer.size() - 2]);//C
				buffer.emplace_back(buffer[buffer.size() - 2]);//D
				buffer[buffer.size() - 2].type_ = type_unfilled;
				buffer[buffer.size() - 1].type_ = type_unfilled;
			}
			buffer.emplace_back(end, +next_normal_dir, -curr_dir, glm::vec2(1.0f, 0.0f), type_end);//C
			buffer.emplace_back(end, -next_normal_dir, -curr_dir, glm::vec2(1.0f, 1.0f), type_end);//D
		}
	}

	constexpr inline uint32_t default_num_b_segments = 12;
	constexpr inline float control_point_length_factor = 0.3f;

	void add_bezier_to_buffer(std::vector<map::curved_line_vertex>& buffer, glm::vec2 start, glm::vec2 end, glm::vec2 start_per, glm::vec2 end_per, float progress, bool last_curve, float size_x, float size_y, uint32_t num_b_segments) {
		auto control_point_length = glm::length(end - start) * control_point_length_factor;

		auto start_control_point = start_per * control_point_length + start;
		auto end_control_point = end_per * control_point_length + end;

		auto bpoint = [=](float t) {
			auto u = 1.0f - t;
			return 0.0f
			+ (u * u * u) * start
			+ (3.0f * u * u * t) * start_control_point
			+ (3.0f * u * t * t) * end_control_point
			+ (t * t * t) * end;
		};

		auto last_normal = glm::vec2(-start_per.y, start_per.x);
	glm::vec2 next_normal{ 0.0f, 0.0f };

		for(uint32_t i = 0; i < num_b_segments - 1; ++i) {
			auto t_start = float(i) / float(num_b_segments);
			auto t_end = float(i + 1) / float(num_b_segments);
			auto t_next = float(i + 2) / float(num_b_segments);

			auto start_point = bpoint(t_start);
			auto end_point = bpoint(t_end);
			auto next_point = bpoint(t_next);

			next_normal = glm::normalize(end_point - start_point) + glm::normalize(end_point - next_point);
			auto temp = glm::normalize(end_point - start_point);
			if(glm::length(next_normal) < 0.00001f) {
				next_normal = glm::normalize(glm::vec2(-temp.y, temp.x));
			} else {
				next_normal = glm::normalize(next_normal);
				if(glm::dot(glm::vec2(-temp.y, temp.x), next_normal) < 0) {
					next_normal = -next_normal;
				}
			}

			if(progress > 0.0f) {
				if(t_end <= progress) { // filled
					add_arrow_to_buffer(buffer, start_point, end_point, last_normal, next_normal, 1.0f, false, size_x, size_y);
				} else if(progress < t_start) { // empty
					add_arrow_to_buffer(buffer, start_point, end_point, last_normal, next_normal, 0.0f, false, size_x, size_y);
				} else {
					auto effective_progress = (progress - t_start) * float(num_b_segments);
					add_arrow_to_buffer(buffer, start_point, end_point, last_normal, next_normal, effective_progress, false, size_x, size_y);
				}
			} else {
				add_arrow_to_buffer(buffer, start_point, end_point, last_normal, next_normal, 0.0f, false, size_x, size_y);
			}

			last_normal = next_normal;
		}
		{
			next_normal = glm::vec2(end_per.y, -end_per.x);
			auto t_start = float(num_b_segments - 1) / float(num_b_segments);
			auto t_end = 1.0f;
			auto start_point = bpoint(t_start);
			auto end_point = bpoint(t_end);

			if(progress > 0.0f) {
				if(t_end <= progress) { // filled
					add_arrow_to_buffer(buffer, start_point, end_point, last_normal, next_normal, 1.0f, last_curve, size_x, size_y);
				} else if(progress < t_start) { // empty
					add_arrow_to_buffer(buffer, start_point, end_point, last_normal, next_normal, 0.0f, last_curve, size_x, size_y);
				} else {
					auto effective_progress = (progress - t_start) * float(num_b_segments);
					add_arrow_to_buffer(buffer, start_point, end_point, last_normal, next_normal, effective_progress, last_curve, size_x, size_y);
				}
			} else {
				add_arrow_to_buffer(buffer, start_point, end_point, last_normal, next_normal, 0.0f, last_curve, size_x, size_y);
			}
		}
	}

	void add_tl_segment_buffer(std::vector<map::textured_line_vertex>& buffer, glm::vec2 start, glm::vec2 end, glm::vec2 next_normal_dir, float size_x, float size_y, float& distance) {
		start /= glm::vec2(size_x, size_y);
		end /= glm::vec2(size_x, size_y);
		auto d = start - end;
		d.x *= 2.0f;
		distance += 0.5f * glm::length(d);
	buffer.emplace_back(textured_line_vertex{ end, +next_normal_dir, 0.0f, distance });//C
	buffer.emplace_back(textured_line_vertex{ end, -next_normal_dir, 1.0f, distance });//D
	}

	void add_tl_bezier_to_buffer(std::vector<map::textured_line_vertex>& buffer, glm::vec2 start, glm::vec2 end, glm::vec2 start_per, glm::vec2 end_per, float progress, bool last_curve, float size_x, float size_y, uint32_t num_b_segments, float& distance) {
		auto control_point_length = glm::length(end - start) * control_point_length_factor;

		auto start_control_point = start_per * control_point_length + start;
		auto end_control_point = end_per * control_point_length + end;

		auto bpoint = [=](float t) {
			auto u = 1.0f - t;
			return 0.0f
			+ (u * u * u) * start
			+ (3.0f * u * u * t) * start_control_point
			+ (3.0f * u * t * t) * end_control_point
			+ (t * t * t) * end;
		};

		auto last_normal = glm::vec2(-start_per.y, start_per.x);
	glm::vec2 next_normal{ 0.0f, 0.0f };

		for(uint32_t i = 0; i < num_b_segments - 1; ++i) {
			auto t_start = float(i) / float(num_b_segments);
			auto t_end = float(i + 1) / float(num_b_segments);
			auto t_next = float(i + 2) / float(num_b_segments);

			auto start_point = bpoint(t_start);
			auto end_point = bpoint(t_end);
			auto next_point = bpoint(t_next);

			next_normal = glm::normalize(end_point - start_point) + glm::normalize(end_point - next_point);
			auto temp = glm::normalize(end_point - start_point);
			if(glm::length(next_normal) < 0.00001f) {
				next_normal = glm::normalize(glm::vec2(-temp.y, temp.x));
			} else {
				next_normal = glm::normalize(next_normal);
				if(glm::dot(glm::vec2(-temp.y, temp.x), next_normal) < 0) {
					next_normal = -next_normal;
				}
			}

			add_tl_segment_buffer(buffer, start_point, end_point, next_normal, size_x, size_y, distance);

			last_normal = next_normal;
		}
		{
			next_normal = glm::vec2(end_per.y, -end_per.x);
			auto t_start = float(num_b_segments - 1) / float(num_b_segments);
			auto t_end = 1.0f;
			auto start_point = bpoint(t_start);
			auto end_point = bpoint(t_end);

			add_tl_segment_buffer(buffer, start_point, end_point, next_normal, size_x, size_y, distance);
		}
	}

	glm::vec2 put_in_local(glm::vec2 new_point, glm::vec2 base_point, float size_x) {
		auto uadjx = std::abs(new_point.x - base_point.x);
		auto ladjx = std::abs(new_point.x - size_x - base_point.x);
		auto radjx = std::abs(new_point.x + size_x - base_point.x);
		if(uadjx < ladjx) {
		return uadjx < radjx ? new_point : glm::vec2{ new_point.x + size_x, new_point.y };
		} else {
		return ladjx < radjx ? glm::vec2{ new_point.x - size_x, new_point.y } : glm::vec2{ new_point.x + size_x, new_point.y };
		}
	}

	void make_navy_path(sys::state& state, std::vector<map::curved_line_vertex>& buffer, dcon::navy_id selected_navy, float size_x, float size_y) {
		auto path = state.world.navy_get_path(selected_navy);
		if(auto ps = path.size(); ps > 0) {
			auto progress = military::fractional_distance_covered(state, selected_navy);

			glm::vec2 current_pos = duplicates::get_navy_location(state, state.world.navy_get_location_from_navy_location(selected_navy));
			glm::vec2 next_pos = put_in_local(duplicates::get_navy_location(state, path[ps - 1]), current_pos, size_x);
			glm::vec2 prev_perpendicular = glm::normalize(next_pos - current_pos);

			auto start_normal = glm::vec2(-prev_perpendicular.y, prev_perpendicular.x);
			auto norm_pos = current_pos / glm::vec2(size_x, size_y);
		
		buffer.emplace_back(norm_pos, +start_normal, glm::vec2{ 0,0 }, glm::vec2(0.0f, 0.0f), progress > 0.0f ? 2.0f : 0.0f);
		buffer.emplace_back(norm_pos, -start_normal, glm::vec2{ 0,0 }, glm::vec2(0.0f, 1.0f), progress > 0.0f ? 2.0f : 0.0f);
			for(auto i = ps; i-- > 0;) {
			glm::vec2 next_perpendicular{ 0.0f, 0.0f };
				next_pos = put_in_local(duplicates::get_navy_location(state, path[i]), current_pos, size_x);

				if(i > 0) {
					glm::vec2 next_next_pos = put_in_local(duplicates::get_navy_location(state, path[i - 1]), next_pos, size_x);
					glm::vec2 a_per = glm::normalize(next_pos - current_pos);
					glm::vec2 b_per = glm::normalize(next_pos - next_next_pos);
					glm::vec2 temp = a_per + b_per;
					if(glm::length(temp) < 0.0001f) {
						next_perpendicular = -a_per;
					} else {
					next_perpendicular = glm::normalize(glm::vec2{ -temp.y, temp.x });
						if(glm::dot(a_per, -next_perpendicular) < glm::dot(a_per, next_perpendicular)) {
							next_perpendicular *= -1.0f;
						}
					}
				} else {
					next_perpendicular = glm::normalize(current_pos - next_pos);
				}

				add_bezier_to_buffer(buffer, current_pos, next_pos, prev_perpendicular, next_perpendicular, i == ps - 1 ? progress : 0.0f, i == 0, size_x, size_y, default_num_b_segments);

				prev_perpendicular = -1.0f * next_perpendicular;
				current_pos = duplicates::get_navy_location(state, path[i]);
			}
		}
	}


	void make_army_path(sys::state& state, std::vector<map::curved_line_vertex>& buffer, dcon::army_id selected_army, float size_x, float size_y) {
		auto path = state.world.army_get_path(selected_army);
		if(auto ps = path.size(); ps > 0) {
			auto progress = military::fractional_distance_covered(state, selected_army);

			glm::vec2 current_pos = duplicates::get_army_location(state, state.world.army_get_location_from_army_location(selected_army));
			glm::vec2 next_pos = put_in_local(duplicates::get_army_location(state, path[ps - 1]), current_pos, size_x);
			glm::vec2 prev_perpendicular = glm::normalize(next_pos - current_pos);

			auto start_normal = glm::vec2(-prev_perpendicular.y, prev_perpendicular.x);
			auto norm_pos = current_pos / glm::vec2(size_x, size_y);

		buffer.emplace_back(norm_pos, +start_normal, glm::vec2{0,0}, glm::vec2(0.0f, 0.0f), progress > 0.0f ? 2.0f : 0.0f);
		buffer.emplace_back(norm_pos, -start_normal, glm::vec2{ 0,0 }, glm::vec2(0.0f, 1.0f), progress > 0.0f ? 2.0f : 0.0f);
			for(auto i = ps; i-- > 0;) {
			glm::vec2 next_perpendicular{ 0.0f, 0.0f };
				next_pos = put_in_local(duplicates::get_army_location(state, path[i]), current_pos, size_x);

				if(i > 0) {
					glm::vec2 next_next_pos = put_in_local(duplicates::get_army_location(state, path[i - 1]), next_pos, size_x);
					glm::vec2 a_per = glm::normalize(next_pos - current_pos);
					glm::vec2 b_per = glm::normalize(next_pos - next_next_pos);
					glm::vec2 temp = a_per + b_per;
					if(glm::length(temp) < 0.00001f) {
						next_perpendicular = -a_per;
					} else {
					next_perpendicular = glm::normalize(glm::vec2{ -temp.y, temp.x });
						if(glm::dot(a_per, -next_perpendicular) < glm::dot(a_per, next_perpendicular)) {
							next_perpendicular *= -1.0f;
						}
					}
				} else {
					next_perpendicular = glm::normalize(current_pos - next_pos);
				}

				add_bezier_to_buffer(buffer, current_pos, next_pos, prev_perpendicular, next_perpendicular, i == ps - 1 ? progress : 0.0f, i == 0, size_x, size_y, default_num_b_segments);

				prev_perpendicular = -1.0f * next_perpendicular;
				current_pos = duplicates::get_army_location(state, path[i]);
			}
		}
	}

	void create_railroad_connection(sys::state& state, std::vector<glm::vec2>& railroad, dcon::province_id p1, dcon::province_id p2) {
		auto const m1 = state.world.province_get_mid_point(p1);
		auto const m2 = state.world.province_get_mid_point(p2);
		railroad.emplace_back(m1);
		//
		auto mid = ((m1 + m2) / glm::vec2(2.f, 2.f));
		const auto rp = rng::get_random(state, p1.index(), p2.index() ^ p1.index());
		const float rf = float(rng::reduce(uint32_t(rp), 8192)) / (8192.f * 0.25f);
		auto const perpendicular = glm::normalize(m2 - m1) * glm::vec2(rf, rf);
		railroad.emplace_back(mid + glm::vec2(-perpendicular.y, perpendicular.x));
	}

	bool get_provinces_part_of_rr_path(sys::state& state, std::vector<bool>& visited_adj, std::vector<bool>& visited_prov, std::vector<dcon::province_id>& provinces, dcon::province_id p) {
		if(state.world.province_get_building_level(p, economy::province_building_type::railroad) == 0)
		return false;
		if(visited_prov[p.index()])
		return false;
		visited_prov[p.index()] = true;
		provinces.push_back(p);

		std::vector<dcon::province_adjacency_id> valid_adj;
		for(const auto adj : state.world.province_get_province_adjacency_as_connected_provinces(p)) {
			auto const pa = adj.get_connected_provinces(adj.get_connected_provinces(0) == p ? 1 : 0);
			if(pa.get_building_level(economy::province_building_type::railroad) == 0
			|| visited_prov[pa.id.index()])
			continue;
			// Do not display railroads if it's a strait OR an impassable land border!
			if((adj.get_type() & province::border::impassible_bit) != 0
			|| (adj.get_type() & province::border::non_adjacent_bit) != 0)
			continue;
			// Don't make earth-wide railroads
			if(std::abs(state.world.province_get_mid_point(p).x - pa.get_mid_point().x) > state.map_state.map_data.size_x / 8.f)
			continue;
			valid_adj.push_back(adj.id);
		}
		pdqsort(valid_adj.begin(), valid_adj.end(), [&](auto const a, auto const b) -> bool {
			auto const ad = state.world.province_adjacency_get_distance(a);
			auto const bd = state.world.province_adjacency_get_distance(b);
			return ad < bd;
		});
		for(const auto a : valid_adj) {
			auto const adj = dcon::fatten(state.world, a);
			auto const pa = adj.get_connected_provinces(adj.get_connected_provinces(0) == p ? 1 : 0);
			if(get_provinces_part_of_rr_path(state, visited_adj, visited_prov, provinces, pa.id)) {
				visited_adj[adj.id.index()] = true;
				break;
			}
		}
		return true;
	}

	void display_data::update_railroad_paths(sys::state& state) {
		// Create paths for the main railroad sections
		std::vector<bool> visited_prov(state.world.province_size() + 1, false);
		std::vector<bool> rr_ends(state.world.province_size() + 1, false);
		std::vector<bool> visited_adj(state.world.province_adjacency_size() + 1, false);
		std::vector<std::vector<glm::vec2>> railroads;
		for(const auto p : state.world.in_province) {
			std::vector<dcon::province_id> provinces;
			if(get_provinces_part_of_rr_path(state, visited_adj, visited_prov, provinces, p)) {
				if(provinces.size() > 1) {
					std::vector<glm::vec2> railroad;
					for(uint32_t i = 0; i < uint32_t(provinces.size() - 1); i++)
					create_railroad_connection(state, railroad, provinces[i], provinces[i + 1]);
					railroad.emplace_back(state.world.province_get_mid_point(provinces.back()));
					assert(!railroad.empty());
					railroads.push_back(railroad);
					rr_ends[provinces.front().index()] = true;
					rr_ends[provinces.back().index()] = true;
				}
			}
		}

		// Populate paths with railroads - only account provinces that have been visited
		// but not the adjacencies
		for(const auto p1 : state.world.in_province) {
			if(visited_prov[p1.id.index()]) {
				auto const p1_level = p1.get_building_level(economy::province_building_type::railroad);
				auto admin_efficiency = province::state_admin_efficiency(state, p1.get_state_membership());
				auto max_adj = std::max<uint32_t>(uint32_t(admin_efficiency * 2.75f), rr_ends[p1.id.index()] ? 3 : 1);
				std::vector<dcon::province_adjacency_id> valid_adj;
				for(const auto adj : p1.get_province_adjacency_as_connected_provinces()) {
					if(max_adj == 0)
					break;
					auto p2 = adj.get_connected_provinces(adj.get_connected_provinces(0) == p1.id ? 1 : 0);
					if(p2.get_building_level(economy::province_building_type::railroad) == 0)
					continue;
					// Do not display railroads if it's a strait OR an impassable land border!
					if((adj.get_type() & province::border::impassible_bit) != 0
					|| (adj.get_type() & province::border::non_adjacent_bit) != 0)
					continue;
					// Don't make earth-wide railroads
					if(std::abs(p1.get_mid_point().x - p2.get_mid_point().x) > state.map_state.map_data.size_x / 8.f)
					continue;
					max_adj--;
					if(visited_adj[adj.id.index()])
					continue;
					if(rr_ends[p1.id.index()] != rr_ends[p2.id.index()]
					&& rr_ends[p1.id.index()] == false)
					continue;
					visited_adj[adj.id.index()] = true;
					valid_adj.push_back(adj.id);
				}
				pdqsort(valid_adj.begin(), valid_adj.end(), [&](auto const a, auto const b) -> bool {
					auto const ad = state.world.province_adjacency_get_distance(a);
					auto const bd = state.world.province_adjacency_get_distance(b);
					return ad < bd;
				});
				for(const auto a : valid_adj) {
					auto const adj = dcon::fatten(state.world, a);
					auto const p2 = adj.get_connected_provinces(adj.get_connected_provinces(0) == p1.id ? 1 : 0);
					//
					std::vector<glm::vec2> railroad;
					create_railroad_connection(state, railroad, p1.id, p2.id);
					railroad.emplace_back(state.world.province_get_mid_point(p2.id));
					assert(!railroad.empty());
					railroads.push_back(railroad);
				}
			}
		}

		railroad_vertices.clear();
		railroad_starts.clear();
		railroad_counts.clear();
		for(const auto& railroad : railroads) {
			railroad_starts.push_back(GLint(railroad_vertices.size()));
			glm::vec2 current_pos = railroad.back();
			glm::vec2 next_pos = put_in_local(railroad[railroad.size() - 2], current_pos, float(size_x));
			glm::vec2 prev_perpendicular = glm::normalize(next_pos - current_pos);
			auto start_normal = glm::vec2(-prev_perpendicular.y, prev_perpendicular.x);
			auto norm_pos = current_pos / glm::vec2(size_x, size_y);
		railroad_vertices.emplace_back(textured_line_vertex{ norm_pos, +start_normal, 0.0f, 0.f });//C
		railroad_vertices.emplace_back(textured_line_vertex{ norm_pos, -start_normal, 1.0f, 0.f });//D
			float distance = 0.0f;
			for(auto i = railroad.size() - 1; i-- > 0;) {
			glm::vec2 next_perpendicular{ 0.0f, 0.0f };
				next_pos = put_in_local(railroad[i], current_pos, float(size_x));
				if(i > 0) {
					glm::vec2 next_next_pos = put_in_local(railroad[i - 1], next_pos, float(size_x));
					glm::vec2 a_per = glm::normalize(next_pos - current_pos);
					glm::vec2 b_per = glm::normalize(next_pos - next_next_pos);
					glm::vec2 temp = a_per + b_per;
					if(glm::length(temp) < 0.00001f) {
						next_perpendicular = -a_per;
					} else {
					next_perpendicular = glm::normalize(glm::vec2{ -temp.y, temp.x });
						if(glm::dot(a_per, -next_perpendicular) < glm::dot(a_per, next_perpendicular))
						next_perpendicular *= -1.0f;
					}
				} else {
					next_perpendicular = glm::normalize(current_pos - next_pos);
				}
				add_tl_bezier_to_buffer(railroad_vertices, current_pos, next_pos, prev_perpendicular, next_perpendicular, 0.0f, false, float(size_x), float(size_y), default_num_b_segments, distance);
				prev_perpendicular = -1.0f * next_perpendicular;
				current_pos = railroad[i];
			}
			railroad_counts.push_back(GLsizei(railroad_vertices.size() - railroad_starts.back()));
			assert(railroad_counts.back() > 1);
		}
		assert(railroad_counts.size() == railroad_starts.size());

		if(!railroad_vertices.empty()) {
			glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_railroad]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(textured_line_vertex) * railroad_vertices.size(), railroad_vertices.data(), GL_STATIC_DRAW);
		}
	}

	void display_data::set_text_lines(sys::state& state, std::vector<text_line_generator_data> const& data) {
		text_line_vertices.clear();
		text_line_texture_per_quad.clear();
		text_line_tagged_vertices.clear();

		const auto map_x_scaling = float(size_x) / float(size_y);
		auto& f = state.font_collection.get_font(state, text::font_selection::map_font);
		if(!f.hb_buf)
			return;

		for(const auto& e : data) {
			// omit invalid, nan or infinite coefficients
			if(!std::isfinite(e.coeff[0]) || !std::isfinite(e.coeff[1]) || !std::isfinite(e.coeff[2]) || !std::isfinite(e.coeff[3]))
				continue;

			bool is_linear = true;
			if((e.coeff[2] != 0) || (e.coeff[3] != 0)) {
				is_linear = false;
			}

			// y = a + bx + cx^2 + dx^3
			// y = mo[0] + mo[1] * x + mo[2] * x * x + mo[3] * x * x * x
			auto poly_fn = [&](float x) {
				return e.coeff[0] + e.coeff[1] * x + e.coeff[2] * x * x + e.coeff[3] * x * x * x;
			};
			auto dpoly_fn = [&](float x) {
				// y = a + 1bx^1 + 1cx^2 + 1dx^3
				// y = 0 + 1bx^0 + 2cx^1 + 3dx^2
				return e.coeff[1] + 2.f * e.coeff[2] * x + 3.f * e.coeff[3] * x * x;
			};

			//cutting box if graph goes outside
			float left = 0.f;
			float right = 1.f;
			if(is_linear) {
				if(e.coeff[1] > 0.01f) {
					left = (-e.coeff[0]) / e.coeff[1];
					right = (1.f - e.coeff[0]) / e.coeff[1];
				} else if(e.coeff[1] < -0.01f) {
					left = (1.f - e.coeff[0]) / e.coeff[1];
					right = (-e.coeff[0]) / e.coeff[1];
				}
			} else {
				while(((poly_fn(left) < 0.f) || (poly_fn(left) > 1.f)) && (left < 1.f)) {
					left += 1.f / 200.f;
				}
				while(((poly_fn(right) < 0.f) || (poly_fn(right) > 1.f)) && (right > 0.f)) {
					right -= 1.f / 200.f;
				}
			}

			left = std::clamp(left, 0.f, 1.f);
			right = std::clamp(right, 0.f, 1.f);
			if(right <= left) {
				continue;
			}

			float result_interval = right - left;
			float center = (right + left) / 2.f;

			glm::vec2 ratio = e.ratio;
			glm::vec2 basis = e.basis;

			auto effective_ratio = ratio.x * map_x_scaling / ratio.y;

			float text_length = f.text_extent(state, e.text, 0, uint32_t(e.text.glyph_info.size()), 1);
			assert(std::isfinite(text_length) && text_length != 0.f);
			float x_step = (result_interval / float(e.text.glyph_info.size() * 32.f));
			float curve_length = 0.f; //width of whole string polynomial
			if(is_linear) {
				float height = poly_fn(right) - poly_fn(left);
				curve_length = 2.f * glm::length(glm::vec2(height * ratio.y, result_interval * ratio.x));
			} else {
				for(float x = left; x <= right; x += x_step) {
					curve_length += 2.f * glm::length(glm::vec2(x_step * ratio.x, (poly_fn(x) - poly_fn(x + x_step)) * ratio.y));
				}
			}

			float size = (curve_length / text_length) * 0.75f; //* 0.66f;
			// typography "golden ratio" steps
			float font_size_index = std::round(5.f * glm::log(size) / glm::log(glm::golden_ratio<float>()));
			if(font_size_index > 5.f) {
				font_size_index = 5.f * std::round(std::min(45.f, font_size_index) / 5.f);
			}
			size = glm::pow(glm::golden_ratio<float>(), font_size_index / 5.f);
			if(size < 15.f) {
				continue;
			}

			auto real_text_size = size / (size_x * 2.0f);
			float letter_spacing_map = std::clamp(0.5f * (curve_length / text_length - size) / 2.f, 0.f, size * 2.f);
			if(state.world.locale_get_prevent_letterspace(state.font_collection.get_current_locale())) {
				letter_spacing_map = 0.f;
			}

			float margin = (curve_length - text_length * (size + letter_spacing_map * 2.f) + letter_spacing_map) / 2.0f;
			float x = left;
			for(float accumulated_length = 0.f; ; x += x_step) {
				auto added_distance = 2.0f * glm::length(glm::vec2(x_step * ratio.x, (poly_fn(x) - poly_fn(x + x_step)) * e.ratio.y));
				if(accumulated_length + added_distance >= margin) {
					x += x_step * (margin - accumulated_length) / added_distance;
					break;
				}
				accumulated_length += added_distance;
			}

			unsigned int glyph_count = static_cast<unsigned int>(e.text.glyph_info.size());
			for(unsigned int i = 0; i < glyph_count; i++) {
				hb_codepoint_t glyphid = e.text.glyph_info[i].codepoint;
				auto gso = f.glyph_positions[glyphid];
				float x_advance = float(e.text.glyph_info[i].x_advance) / (float((1 << 6) * text::magnification_factor));
				float x_offset = float(e.text.glyph_info[i].x_offset) / (float((1 << 6) * text::magnification_factor)) + float(gso.x);
				float y_offset = float(gso.y) - float(e.text.glyph_info[i].y_offset) / (float((1 << 6) * text::magnification_factor));
				if(true) {
					// Add up baseline and kerning offsets
					glm::vec2 glyph_positions{ x_offset / 64.f, -y_offset / 64.f };

					glm::vec2 curr_dir = glm::normalize(glm::vec2(effective_ratio, dpoly_fn(x)));
					glm::vec2 curr_normal_dir = glm::vec2(-curr_dir.y, curr_dir.x);
					curr_dir.x *= 0.5f;
					curr_normal_dir.x *= 0.5f;

					glm::vec2 shader_direction = glm::normalize(glm::vec2(ratio.x, dpoly_fn(x) * ratio.y));

					auto p0 = glm::vec2(x, poly_fn(x)) * ratio + basis;
					p0 /= glm::vec2(size_x, size_y); // Rescale the coordinate to 0-1
					p0 -= (1.5f - 2.f * glyph_positions.y) * curr_normal_dir * real_text_size;
					p0 += (1.0f + 2.f * glyph_positions.x) * curr_dir * real_text_size;

					float step = 1.f / 8.f;
					float tx = float(gso.texture_slot & 7) * step;
					float ty = float((gso.texture_slot & 63) >> 3) * step;

					text_line_tagged_vertices.push_back(e.n);
					text_line_vertices.emplace_back(p0, glm::vec2(-1, 1), shader_direction, glm::vec2(tx, ty), real_text_size);
					text_line_vertices.emplace_back(p0, glm::vec2(-1, -1), shader_direction, glm::vec2(tx, ty + step), real_text_size);
					text_line_vertices.emplace_back(p0, glm::vec2(1, -1), shader_direction, glm::vec2(tx + step, ty + step), real_text_size);
					text_line_vertices.emplace_back(p0, glm::vec2(1, -1), shader_direction, glm::vec2(tx + step, ty + step), real_text_size);
					text_line_vertices.emplace_back(p0, glm::vec2(1, 1), shader_direction, glm::vec2(tx + step, ty), real_text_size);
					text_line_vertices.emplace_back(p0, glm::vec2(-1, 1), shader_direction, glm::vec2(tx, ty), real_text_size);
					text_line_texture_per_quad.emplace_back(f.textures[gso.texture_slot >> 6]);
				}
				float glyph_advance = x_advance * size / 64.f;
				for(float glyph_length = 0.f; ; x += x_step) {
					auto added_distance = 2.f * glm::length(glm::vec2(x_step * ratio.x, (poly_fn(x) - poly_fn(x + x_step)) * ratio.y));
					if(glyph_length + added_distance >= glyph_advance + letter_spacing_map) {
						x += x_step * (glyph_advance + letter_spacing_map - glyph_length) / added_distance;
						break;
					}
					glyph_length += added_distance;
				}
			}
		}
		if(text_line_vertices.size() > 0) {
			for(uint32_t i = 0; i < uint32_t(text_line_texture_per_quad.size()); i++) {
				bool swapped = false;
				for(uint32_t j = 0; j < uint32_t(text_line_texture_per_quad.size() - i - 1); j++) {
					if(text_line_texture_per_quad[j] < text_line_texture_per_quad[j + 1]) {
						std::swap(text_line_texture_per_quad[j], text_line_texture_per_quad[j + 1]);
						std::swap(text_line_tagged_vertices[j], text_line_tagged_vertices[j + 1]);
						map::text_line_vertex tmp[6];
						std::memcpy(tmp, &text_line_vertices[(j + 1) * 6], sizeof(tmp));
						std::memcpy(&text_line_vertices[(j + 1) * 6], &text_line_vertices[j * 6], sizeof(tmp));
						std::memcpy(&text_line_vertices[j * 6], tmp, sizeof(tmp));
						swapped = true;
					}
				}
				if(!swapped) {
					break;
				}
			}
			dyn_text_line_starts.resize(text_line_texture_per_quad.size());
			dyn_text_line_counts.resize(text_line_texture_per_quad.size());
			glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_text_line]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(text_line_vertex) * text_line_vertices.size(), &text_line_vertices[0], GL_DYNAMIC_DRAW);
		}
	}

	void display_data::set_province_text_lines(sys::state& state, std::vector<text_line_generator_data> const& data) {
		province_text_line_vertices.clear();
		province_text_line_texture_per_quad.clear();
		const auto map_x_scaling = float(size_x) / float(size_y);
		auto& f = state.font_collection.get_font(state, text::font_selection::map_font);
		if(!f.hb_buf)
			return;

		for(const auto& e : data) {
			// omit invalid, nan or infinite coefficients
			if(!std::isfinite(e.coeff[0]) || !std::isfinite(e.coeff[1]) || !std::isfinite(e.coeff[2]) || !std::isfinite(e.coeff[3]))
				continue;

			auto effective_ratio = e.ratio.x * map_x_scaling / e.ratio.y;
			float text_length = f.text_extent(state, e.text, 0, uint32_t(e.text.glyph_info.size()), 1);
			if(!std::isfinite(text_length) || text_length == 0.f) {
				continue;
			}

			// y = a + bx + cx^2 + dx^3
			// y = mo[0] + mo[1] * x + mo[2] * x * x + mo[3] * x * x * x
			auto poly_fn = [&](float x) {
				return e.coeff[0] + e.coeff[1] * x + e.coeff[2] * x * x + e.coeff[3] * x * x * x;
			};
			float x_step = (1.f / float(e.text.glyph_info.size() * 16.f));
			float curve_length = 0.f; //width of whole string polynomial
			for(float x = 0.f; x <= 1.f; x += x_step) {
				curve_length += 2.0f * glm::length(glm::vec2(x_step * e.ratio.x, (poly_fn(x) - poly_fn(x + x_step)) * e.ratio.y));
			}

			float size = (curve_length / text_length) * 0.85f;
			auto real_text_size = size / (size_x * 2.0f);
			float margin = (curve_length - text_length * size) / 2.0f;
			float x = 0.f;
			for(float accumulated_length = 0.f; ; x += x_step) {
				auto added_distance = 2.0f * glm::length(glm::vec2(x_step * e.ratio.x, (poly_fn(x) - poly_fn(x + x_step)) * e.ratio.y));
				if(accumulated_length + added_distance >= margin) {
					x += x_step * (margin - accumulated_length) / added_distance;
					break;
				}
				accumulated_length += added_distance;
			}

			unsigned int glyph_count = uint32_t(e.text.glyph_info.size());
			for(unsigned int i = 0; i < glyph_count; i++) {
				hb_codepoint_t glyphid = e.text.glyph_info[i].codepoint;
				auto gso = f.glyph_positions[glyphid];
				float x_advance = float(gso.x_advance);
				float x_offset = float(e.text.glyph_info[i].x_offset) / 4.f + float(gso.x);
				float y_offset = float(gso.y) - float(e.text.glyph_info[i].y_offset) / 4.f;
				if(true) {
					glm::vec2 glyph_positions{ x_offset / 64.f, -y_offset / 64.f };
					auto dpoly_fn = [&](float x) {
						// y = a + 1bx^1 + 1cx^2 + 1dx^3
						// y = 0 + 1bx^0 + 2cx^1 + 3dx^2
						return e.coeff[1] + 2.f * e.coeff[2] * x + 3.f * e.coeff[3] * x * x;
					};

					glm::vec2 curr_dir = glm::normalize(glm::vec2(effective_ratio, dpoly_fn(x)));
					glm::vec2 curr_normal_dir = glm::vec2(-curr_dir.y, curr_dir.x);
					curr_dir.x *= 0.5f;
					curr_normal_dir.x *= 0.5f;

					glm::vec2 shader_direction = glm::normalize(glm::vec2(e.ratio.x, dpoly_fn(x) * e.ratio.y));

					auto p0 = glm::vec2(x, poly_fn(x)) * e.ratio + e.basis;
					p0 /= glm::vec2(size_x, size_y); // Rescale the coordinate to 0-1
					p0 -= (1.5f - 2.f * glyph_positions.y) * curr_normal_dir * real_text_size;
					p0 += (1.0f + 2.f * glyph_positions.x) * curr_dir * real_text_size;

					float step = 1.f / 8.f;
					float tx = float(gso.texture_slot & 7) * step;
					float ty = float((gso.texture_slot & 63) >> 3) * step;

					province_text_line_vertices.emplace_back(p0, glm::vec2(-1, 1), shader_direction, glm::vec2(tx, ty), real_text_size);
					province_text_line_vertices.emplace_back(p0, glm::vec2(-1, -1), shader_direction, glm::vec2(tx, ty + step), real_text_size);
					province_text_line_vertices.emplace_back(p0, glm::vec2(1, -1), shader_direction, glm::vec2(tx + step, ty + step), real_text_size);

					province_text_line_vertices.emplace_back(p0, glm::vec2(1, -1), shader_direction, glm::vec2(tx + step, ty + step), real_text_size);
					province_text_line_vertices.emplace_back(p0, glm::vec2(1, 1), shader_direction, glm::vec2(tx + step, ty), real_text_size);
					province_text_line_vertices.emplace_back(p0, glm::vec2(-1, 1), shader_direction, glm::vec2(tx, ty), real_text_size);
					province_text_line_texture_per_quad.emplace_back(f.textures[gso.texture_slot >> 6]);
				}
				float glyph_advance = x_advance * size / 64.f;
				for(float glyph_length = 0.f; ; x += x_step) {
					auto added_distance = 2.0f * glm::length(glm::vec2(x_step * e.ratio.x, (poly_fn(x) - poly_fn(x + x_step)) * e.ratio.y));
					if(glyph_length + added_distance >= glyph_advance) {
						x += x_step * (glyph_advance - glyph_length) / added_distance;
						break;
					}
					glyph_length += added_distance;
				}
			}
		}
		if(province_text_line_vertices.size() > 0) {
			for(uint32_t i = 0; i < uint32_t(province_text_line_texture_per_quad.size() - 1); i++) {
				bool swapped = false;
				for(uint32_t j = 0; j < uint32_t(province_text_line_texture_per_quad.size() - i - 1); j++) {
					if(province_text_line_texture_per_quad[j] < province_text_line_texture_per_quad[j + 1]) {
						std::swap(province_text_line_texture_per_quad[j], province_text_line_texture_per_quad[j + 1]);
						map::text_line_vertex tmp[6];
						std::memcpy(tmp, &province_text_line_vertices[(j + 1) * 6], sizeof(tmp));
						std::memcpy(&province_text_line_vertices[(j + 1) * 6], &province_text_line_vertices[j * 6], sizeof(tmp));
						std::memcpy(&province_text_line_vertices[j * 6], tmp, sizeof(tmp));
						swapped = true;
					}
				}
				if(!swapped) {
					break;
				}
			}
			dyn_province_text_line_starts.resize(province_text_line_texture_per_quad.size());
			dyn_province_text_line_counts.resize(province_text_line_texture_per_quad.size());
			glBindBuffer(GL_ARRAY_BUFFER, vbo_array[vo_province_text_line]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(text_line_vertex) * province_text_line_vertices.size(), &province_text_line_vertices[0], GL_STATIC_DRAW);
		}
	}

	GLuint load_dds_texture(simple_fs::directory const& dir, native_string_view file_name, int soil_flags = ogl::SOIL_FLAG_TEXTURE_REPEATS) {
		if(auto file = simple_fs::open_file(dir, file_name); file) {
			auto content = simple_fs::view_contents(*file);
			uint32_t size_x, size_y;
			uint8_t const* data = (uint8_t const*)(content.data);
			reports::write_debug("Loading [map] DDS file " + text::native_to_utf8(file_name) + "\n");
			return ogl::SOIL_direct_load_DDS_from_memory(data, content.file_size, size_x, size_y, soil_flags);
		}
		reports::write_debug("Can't load [map] DDS file " + text::native_to_utf8(file_name) + "\n");
		return 0;
	}

	void load_animation(sys::state& state, emfx::xsm_context const& anim_context, uint32_t index, emfx::xac_context const& model_context, emfx::animation_type at) {
		auto root = simple_fs::get_root(state.common_fs);
		auto old_size = state.map_state.map_data.animations.size();
		switch(at) {
		case emfx::animation_type::idle:
			state.map_state.map_data.static_mesh_idle_animation_start[index] = uint32_t(old_size);
			break;
		case emfx::animation_type::move:
			state.map_state.map_data.static_mesh_move_animation_start[index] = uint32_t(old_size);
			break;
		case emfx::animation_type::attack:
			state.map_state.map_data.static_mesh_attack_animation_start[index] = uint32_t(old_size);
			break;
		}
		for(const auto& anim : anim_context.animations) {
			auto it = std::find_if(model_context.nodes.begin(), model_context.nodes.end(), [&](const auto& e) {
				return anim.node == e.name;
			});
			if(it != model_context.nodes.end()) {
				auto t_anim = anim;
				t_anim.parent_id = it->parent_id;
				t_anim.bone_id = int32_t(std::distance(model_context.nodes.begin(), it));
				auto const vp = glm::vec3(it->position.x, it->position.y, it->position.z);
				auto const vs = glm::vec3(it->scale.x, it->scale.y, it->scale.z);
				auto const vq = glm::quat(it->rotation.x, it->rotation.y, it->rotation.z, it->rotation.w);
				t_anim.bone_node_matrix = glm::translate(vp) * glm::toMat4(vq) * glm::scale(vs);
				auto const pp = glm::vec3(t_anim.pose_position.x, t_anim.pose_position.y, t_anim.pose_position.z);
				auto const ps = glm::vec3(t_anim.pose_scale.x, t_anim.pose_scale.y, t_anim.pose_scale.z);
				auto const pq = glm::quat(t_anim.pose_rotation.x, t_anim.pose_rotation.y, t_anim.pose_rotation.z, t_anim.pose_rotation.w);
				t_anim.bone_pose_matrix = glm::translate(pp) * glm::toMat4(pq) * glm::scale(ps);
				auto const ip = glm::vec3(t_anim.bind_pose_position.x, t_anim.bind_pose_position.y, t_anim.bind_pose_position.z);
				auto const is = glm::vec3(t_anim.bind_pose_scale.x, t_anim.bind_pose_scale.y, t_anim.bind_pose_scale.z);
				auto const iq = glm::quat(t_anim.bind_pose_rotation.x, t_anim.bind_pose_rotation.y, t_anim.bind_pose_rotation.z, t_anim.bind_pose_rotation.w);
				t_anim.bone_bind_pose_matrix = glm::translate(ip) * glm::toMat4(iq) * glm::scale(is);
				state.map_state.map_data.animations.push_back(t_anim);
			}
		}
		switch(at) {
		case emfx::animation_type::idle:
			state.map_state.map_data.static_mesh_idle_animation_count[index] = uint32_t(state.map_state.map_data.animations.size() - old_size);
			break;
		case emfx::animation_type::move:
			state.map_state.map_data.static_mesh_move_animation_count[index] = uint32_t(state.map_state.map_data.animations.size() - old_size);
			break;
		case emfx::animation_type::attack:
			state.map_state.map_data.static_mesh_attack_animation_count[index] = uint32_t(state.map_state.map_data.animations.size() - old_size);
			break;
		}
	}

	void load_static_meshes(sys::state& state) {
		reports::write_debug("Loading static meshes\n");

		struct static_mesh_vertex {
			glm::vec3 position_;
			glm::u8vec2 normal_;
			glm::u16vec2 texture_coord_;
			int8_t bone_ids[4] = { -1, -1, -1, -1 };
			float bone_weights[4] = { 0.f, 0.f, 0.f, 0.f };
		};
		std::vector<static_mesh_vertex> static_mesh_vertices;
		auto root = simple_fs::get_root(state.common_fs);
		auto gfx_dir = simple_fs::open_directory(root, NATIVE("gfx"));
		auto gfx_anims = simple_fs::open_directory(gfx_dir, NATIVE("anims"));

		state.map_state.map_data.animations.clear(); //clear animations list
		for(uint32_t i = 0; i < uint32_t(culture::graphical_culture_type::count); i++) {
			state.map_state.map_data.model_gc_unit[i].resize(state.military_definitions.unit_base_definitions.size());
		}
		state.map_state.map_data.static_mesh_counts.resize(display_data::max_static_meshes);
		state.map_state.map_data.static_mesh_starts.resize(display_data::max_static_meshes);
		for(uint32_t i = 0; i < uint32_t(display_data::max_static_meshes); i++) { //reset for reloading
			state.map_state.map_data.static_mesh_counts[i].clear();
			state.map_state.map_data.static_mesh_starts[i].clear();
		}

		//preload all models and all animations
		ankerl::unordered_dense::map<std::string, emfx::xac_context> map_of_models;
		ankerl::unordered_dense::map<std::string, emfx::xsm_context> map_of_animations;
		ankerl::unordered_dense::map<native_string, GLuint> map_of_textures;
		for(uint32_t k = 0; k < display_data::max_static_meshes && k < uint32_t(state.ui_defs.emfx.size()); k++) {
			auto edef = dcon::emfx_object_id(dcon::emfx_object_id::value_base_t(k));
			ui::emfx_object const& emfx_obj = state.ui_defs.emfx[edef];
			{ //models
				auto filename = std::string(state.to_string_view(emfx_obj.actorfile));
				if(map_of_models.find(filename) == map_of_models.end()) {
					if(auto f = simple_fs::open_file(root, text::win1250_to_native(filename)); f) {
						reports::write_debug("Loading XAC: " + filename + "\n");
						parsers::error_handler err(text::native_to_utf8(simple_fs::get_full_name(*f)));
						auto contents = simple_fs::view_contents(*f);
						emfx::xac_context context{};
						emfx::parse_xac(context, contents.data, contents.data + contents.file_size, err);
						emfx::finish(context);
						map_of_models.insert_or_assign(filename, context);
					}
				}
			}
			{ //idle set
				auto filename = std::string(state.to_string_view(emfx_obj.idle));
				if(map_of_animations.find(filename) == map_of_animations.end()) {
					if(auto cf = simple_fs::open_file(root, text::win1250_to_native(filename)); cf) {
						reports::write_debug("Loading XSM: " + filename + "\n");
						parsers::error_handler err(text::native_to_utf8(simple_fs::get_full_name(*cf)));
						auto contents = simple_fs::view_contents(*cf);
						emfx::xsm_context context{};
						emfx::parse_xsm(context, contents.data, contents.data + contents.file_size, err);
						emfx::finish(context);
						map_of_animations.insert_or_assign(filename, context);
					}
				}
			}
			{ //moving set
				auto filename = std::string(state.to_string_view(emfx_obj.move));
				if(map_of_animations.find(filename) == map_of_animations.end()) {
					if(auto cf = simple_fs::open_file(root, text::win1250_to_native(filename)); cf) {
						reports::write_debug("Loading XSM: " + filename + "\n");
						parsers::error_handler err(text::native_to_utf8(simple_fs::get_full_name(*cf)));
						auto contents = simple_fs::view_contents(*cf);
						emfx::xsm_context context{};
						emfx::parse_xsm(context, contents.data, contents.data + contents.file_size, err);
						emfx::finish(context);
						map_of_animations.insert_or_assign(filename, context);
					}
				}
			}
			{ //attacking set
				auto filename = std::string(state.to_string_view(emfx_obj.attack));
				if(map_of_animations.find(filename) == map_of_animations.end()) {
					if(auto cf = simple_fs::open_file(root, text::win1250_to_native(filename)); cf) {
						reports::write_debug("Loading XSM: " + filename + "\n");
						parsers::error_handler err(text::native_to_utf8(simple_fs::get_full_name(*cf)));
						auto contents = simple_fs::view_contents(*cf);
						emfx::xsm_context context{};
						emfx::parse_xsm(context, contents.data, contents.data + contents.file_size, err);
						emfx::finish(context);
						map_of_animations.insert_or_assign(filename, context);
					}
				}
			}
		}

		state.map_state.map_data.model_province_flag.resize(size_t(state.province_definitions.num_allocated_provincial_flags));
		assert(display_data::max_static_meshes >= uint32_t(state.ui_defs.emfx.size()));
		for(uint32_t k = 0; k < display_data::max_static_meshes && k < uint32_t(state.ui_defs.emfx.size()); k++) {
			auto edef = dcon::emfx_object_id(dcon::emfx_object_id::value_base_t(k));
			ui::emfx_object const& emfx_obj = state.ui_defs.emfx[edef];
			//
			auto name = parsers::lowercase_str(state.to_string_view(emfx_obj.name));
			bool is_province_flag_model = false;
			for(uint32_t i = 0; i < uint32_t(state.province_definitions.num_allocated_provincial_flags); i++) {
				dcon::provincial_flag_id pfid{ dcon::provincial_flag_id::value_base_t(i) };
				auto pf_name = text::produce_simple_string(state, state.province_definitions.flag_variable_names[pfid]);
				if(parsers::lowercase_str(pf_name) == name) {
					state.map_state.map_data.model_province_flag[i] = edef;
					is_province_flag_model = true;
					break;
				}
			}
			if(name == "wake") {
				state.map_state.map_data.model_wake = edef;
				for(uint32_t l = 0; l < display_data::max_static_submeshes; l++) {
					state.map_state.map_data.static_mesh_scrolling_factor[k][l] = 0.5f;
				}
			} else if(name == "port_blockade") {
				state.map_state.map_data.model_blockaded = edef;
			} else if(name == "building_factory") {
				state.map_state.map_data.model_factory = edef;
			} else if(name == "building_trainstation") {
				state.map_state.map_data.model_train_station = edef;
			} else if(name == "construction") {
				state.map_state.map_data.model_construction = edef;
			} else if(name == "construction_naval") {
				state.map_state.map_data.model_construction_naval = edef;
			} else if(name == "construction_military") {
				state.map_state.map_data.model_construction_military = edef;
			} else if(name == "siege") {
				state.map_state.map_data.model_siege = edef;
			} else if(name == "flag") {
				state.map_state.map_data.model_flag = edef;
			} else if(name == "flagfloating") {
				state.map_state.map_data.model_flag_floating = edef;
			} else if(name.starts_with("building_naval_base")) {
				uint32_t level = 0;
				if(name.size() > 19) {
					level = name.data()[19] - '0';
				}
				level = std::clamp<uint32_t>(level, 0, 7);
				if(name.ends_with("_ships")) {
					state.map_state.map_data.model_naval_base_ships[level] = edef;
				} else {
					state.map_state.map_data.model_naval_base[level] = edef;
				}
			} else if(name.starts_with("building_fort")) {
				uint32_t level = 1; //yep, starts at 1, lovely
				if(name.size() > 13) {
					level = name.data()[13] - '0';
				}
				level = std::clamp<uint32_t>(level, 0, 7);
				state.map_state.map_data.model_fort[level] = edef;
			} else if(is_province_flag_model) {
				//dont execute the rest
			} else {
				auto t = culture::graphical_culture_type::generic;
				size_t type_pos = std::string::npos;
				if(parsers::has_fixed_prefix_ci(name.data(), name.data() + name.length(), "generic")) {
					type_pos = 7;
				} /* following tokens are dynamic */
#define CT_STRING_ENUM(x) \
	else if(parsers::has_fixed_prefix_ci(name.data(), name.data() + name.length(), #x"gc")) {\
		t = culture::graphical_culture_type::x; type_pos = std::strlen(#x"gc"); }
				CT_STRING_ENUM(african)
				CT_STRING_ENUM(asian)
				CT_STRING_ENUM(british)
				CT_STRING_ENUM(chinese)
				CT_STRING_ENUM(confederate)
				CT_STRING_ENUM(european)
				CT_STRING_ENUM(french)
				CT_STRING_ENUM(indian)
				CT_STRING_ENUM(italian)
				CT_STRING_ENUM(japanese)
				CT_STRING_ENUM(middle_eastern)
				CT_STRING_ENUM(morocco)
				CT_STRING_ENUM(ottoman)
				CT_STRING_ENUM(prussian)
				CT_STRING_ENUM(russian)
				CT_STRING_ENUM(south_american)
				CT_STRING_ENUM(spanish)
				CT_STRING_ENUM(swedish)
				CT_STRING_ENUM(us)
				CT_STRING_ENUM(zulu)
#undef CT_STRING_ENUM
				else if(parsers::has_fixed_prefix_ci(name.data(), name.data() + name.length(), "austriahungarygc")) {
					t = culture::graphical_culture_type::austria_hungary;
					type_pos = 14 + 2;
				}
				reports::write_debug("Loading [" + name + "] as GC model\n");
				if(type_pos != std::string::npos) {
					auto type_name = parsers::lowercase_str(std::string_view(name.data() + type_pos, name.data() + name.length()));
					if(std::isdigit(type_name[type_name.length() - 1])) {
						continue;//type_name.pop_back();
					}
					for(uint32_t i = 0; i < uint32_t(state.military_definitions.unit_base_definitions.size()); i++) {
						auto utid = dcon::unit_type_id(dcon::unit_type_id::value_base_t(i));
						auto const& udef = state.military_definitions.unit_base_definitions[utid];
						auto sprite = parsers::lowercase_str(state.to_string_view(udef.sprite_type));
						if((sprite == "cavalry" && type_name == "mount")
						|| (udef.type == military::unit_type::cavalry)) {
							if(!state.map_state.map_data.model_gc_unit[0][utid.index()]) {
								state.map_state.map_data.model_gc_unit[0][utid.index()] = edef;
							}
						} else if(type_name == sprite) {
							if(!state.map_state.map_data.model_gc_unit[0][utid.index()] || t == culture::graphical_culture_type::generic) {
								state.map_state.map_data.model_gc_unit[0][utid.index()] = edef;
								reports::write_debug("Applying GC#<generic> unit -> " + std::string(state.to_string_view(udef.name)) + "::" + type_name + "(" + name + ")\n");
							}
							if(!state.map_state.map_data.model_gc_unit[uint8_t(t)][utid.index()]) {
								state.map_state.map_data.model_gc_unit[uint8_t(t)][utid.index()] = edef;
								reports::write_debug("Applying GC#" + std::to_string(uint32_t(t)) + " -> " + std::string(state.to_string_view(udef.name)) + "::" + type_name + "(" + name + ")\n");
							}
							break;
						}
					}
				}
			}

			auto actorfile = std::string(state.to_string_view(emfx_obj.actorfile));
			if(auto it1 = map_of_models.find(actorfile); it1 != map_of_models.end()) {
				auto const& context = it1->second;
				if(auto it = map_of_animations.find(std::string(state.to_string_view(emfx_obj.idle))); it != map_of_animations.end()) {
					load_animation(state, it->second, k, context, emfx::animation_type::idle);
				}
				if(auto it = map_of_animations.find(std::string(state.to_string_view(emfx_obj.move))); it != map_of_animations.end()) {
					load_animation(state, it->second, k, context, emfx::animation_type::move);
				}
				if(auto it = map_of_animations.find(std::string(state.to_string_view(emfx_obj.attack))); it != map_of_animations.end()) {
					load_animation(state, it->second, k, context, emfx::animation_type::attack);
				}
				uint32_t node_index = 0;
				for(auto const& node : context.nodes) {
					if(node.name == "polySurface95" || node.name == "polySurface97") {
						node_index++;
						continue;
					}
					int32_t mesh_index = 0;
					for(auto const& mesh : node.meshes) {
						bool is_collision = node.collision_mesh == mesh_index || node.name == "pCube1";
						if(is_collision) {
							mesh_index++;
							continue;
						}
						uint32_t vertex_offset = 0;
						for(auto const& sub : mesh.submeshes) {
							auto old_size = static_mesh_vertices.size();
							bool has_invalid_bone_ids = false;
							for(uint32_t i = 0; i < uint32_t(sub.indices.size()); i += 3) {
								static_mesh_vertex triangle_vertices[3];
								for(uint32_t j = 0; j < 3; j++) {
									static_mesh_vertex& smv = triangle_vertices[j];
									auto const index = sub.indices[i + j] + vertex_offset;
									auto const vv = mesh.vertices[index % mesh.vertices.size()];
									auto const vn = mesh.normals.empty() || index >= mesh.normals.size()
										? emfx::xac_vector3f{ vv.x, vv.y, vv.z }
										: mesh.normals[index];
									auto const vt = mesh.texcoords.empty() || index >= mesh.texcoords.size()
										? emfx::xac_vector2f{ vv.x, vv.y }
										: mesh.texcoords[index];
									smv.position_ = glm::vec3(vv.x, vv.y, vv.z);
									smv.normal_ = glm::u8vec3(vn.x * 255.f, vn.y * 255.f, vn.z * 255.f);
									smv.texture_coord_ = glm::u16vec2(vt.x * 65535.f / 4.f, vt.y * 65535.f / 4.f);
									//
									auto const inf_index = mesh.influence_indices[index];
									assert(inf_index < mesh.influence_starts.size() && inf_index < mesh.influence_starts.size());
									uint32_t i_start = mesh.influence_starts[inf_index];
									uint32_t i_count = mesh.influence_counts[inf_index];
									assert(i_count <= std::extent_v<decltype(smv.bone_ids)>);
									for(uint32_t l = 0; l < std::extent_v<decltype(smv.bone_ids)>; l++) {
										smv.bone_ids[l] = -1;
										smv.bone_weights[l] = 0.f;
									}
									uint32_t added_count = 0;
									for(uint32_t l = i_start; l < i_start + i_count; l++) {
										auto influence = mesh.influences[l];
										if(influence.bone_id >= int32_t(context.nodes.size())
										|| influence.bone_id >= int32_t(state.map_state.map_data.max_bone_matrices)) {
											influence.bone_id = -1;
										}
										if(influence.bone_id != -1) {
											smv.bone_ids[added_count] = int8_t(influence.bone_id);
											smv.bone_weights[added_count] = influence.weight;
											++added_count;
										}
									}
								}
								for(const auto& smv : triangle_vertices) {
									static_mesh_vertex tmp = smv;
									tmp.position_ *= (emfx_obj.scale > 0.f ? emfx_obj.scale : 1.f) * 0.85f; //nudge for good scales
									static_mesh_vertices.push_back(tmp);
								}
							}
							vertex_offset += sub.num_vertices;
							auto submesh_index = state.map_state.map_data.static_mesh_starts[k].size();
							assert(submesh_index < state.map_state.map_data.max_static_submeshes);
							// This is how most models fallback to find their textures...
							if(context.materials.size() > 0) {
								auto const& mat = context.materials[sub.material_id];
								auto const& layer = get_diffuse_layer(mat);
								if(!layer.texture.empty() && layer.texture != "unionjacksquare") {
									native_string fname = text::win1250_to_native(layer.texture) + NATIVE(".dds");
									GLuint texid;
									if(auto it = map_of_textures.find(fname); it != map_of_textures.end()) {
										texid = it->second;
									} else {
										texid = load_dds_texture(gfx_anims, fname, 0);
										ogl::set_gltex_parameters(texid, GL_TEXTURE_2D, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT);
										map_of_textures.insert_or_assign(fname, texid);
									}
									if(parsers::is_fixed_token_ci(layer.texture.data(), layer.texture.data() + layer.texture.length(), "smoke")) {
										state.map_state.map_data.static_mesh_scrolling_factor[k][submesh_index] = 1.f;
									} else if(parsers::is_fixed_token_ci(layer.texture.data(), layer.texture.data() + layer.texture.length(), "texanim")) {
										state.map_state.map_data.static_mesh_scrolling_factor[k][submesh_index] = 1.f;
									}
									state.map_state.map_data.static_mesh_textures[k][submesh_index] = texid;
								}
							}
							state.map_state.map_data.static_mesh_submesh_node_index[k][submesh_index] = node_index;
							state.map_state.map_data.static_mesh_starts[k].push_back(GLint(old_size));
							state.map_state.map_data.static_mesh_counts[k].push_back(GLsizei(static_mesh_vertices.size() - old_size));
						}
						mesh_index++;
					}
					node_index++;
				}
			}
		}

		// Fill missing models with generic (slot 0)
		for(uint32_t j = 0; j < uint32_t(state.military_definitions.unit_base_definitions.size()); j++) {
			for(uint32_t i = 1; i < uint32_t(culture::graphical_culture_type::count); i++) {
				if(!state.map_state.map_data.model_gc_unit[i][j]) {
					state.map_state.map_data.model_gc_unit[i][j] = state.map_state.map_data.model_gc_unit[0][j];
				}
			}
			if(!state.map_state.map_data.model_gc_unit[0][j]) {
				auto utid = dcon::unit_type_id(dcon::unit_type_id::value_base_t(j));
				auto const& udef = state.military_definitions.unit_base_definitions[utid];
				reports::write_debug("Unit model missing for: " + std::to_string(j) + "#" + std::string(state.to_string_view(udef.name)) + "\n");
			}
		}

		if(!static_mesh_vertices.empty()) {
			glBindBuffer(GL_ARRAY_BUFFER, state.map_state.map_data.vbo_array[state.map_state.map_data.vo_static_mesh]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(static_mesh_vertex) * static_mesh_vertices.size(), &static_mesh_vertices[0], GL_STATIC_DRAW);
		}
		glBindVertexArray(state.map_state.map_data.vao_array[state.map_state.map_data.vo_static_mesh]);
		glBindBuffer(GL_ARRAY_BUFFER, state.map_state.map_data.vbo_array[state.map_state.map_data.vo_static_mesh]);
		//glBindVertexBuffer(0, state.map_state.map_data.vbo_array[state.map_state.map_data.vo_static_mesh], 0, sizeof(static_mesh_vertex)); // Bind the VBO to 0 of the VAO
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(static_mesh_vertex), (const void*)offsetof(static_mesh_vertex, position_)); // Set up vertex attribute format for the position
		glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(static_mesh_vertex), (const void*)offsetof(static_mesh_vertex, normal_)); // Set up vertex attribute format for the normal direction
		glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(static_mesh_vertex), (const void*)offsetof(static_mesh_vertex, texture_coord_)); // Set up vertex attribute format for the texture coordinates
		glVertexAttribIPointer(3, 4, GL_BYTE, sizeof(static_mesh_vertex), (const void*)offsetof(static_mesh_vertex, bone_ids));
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(static_mesh_vertex), (const void*)offsetof(static_mesh_vertex, bone_weights));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glBindVertexArray(0);
	}

	void display_data::load_map(sys::state& state) {
		reports::write_debug("Loading map\n");
		assert(!loaded_map);
		loaded_map = true;

		auto root = simple_fs::get_root(state.common_fs);
		glGenVertexArrays(vo_count, vao_array);
		glGenBuffers(vo_count, vbo_array);
		load_shaders(root);
		load_static_meshes(state);
		create_meshes();

		auto assets_dir = simple_fs::open_directory(root, NATIVE("assets"));
		auto map_dir = simple_fs::open_directory(root, NATIVE("map"));
		auto map_terrain_dir = simple_fs::open_directory(map_dir, NATIVE("terrain"));
		auto gfx_dir = simple_fs::open_directory(root, NATIVE("gfx"));
		auto map_items_dir = simple_fs::open_directory(gfx_dir, NATIVE("mapitems"));
		auto gfx_anims_dir = simple_fs::open_directory(gfx_dir, NATIVE("anims"));
		auto test_dir = simple_fs::open_directory(gfx_dir, NATIVE("test"));

		glGenTextures(1, &textures[texture_diag_border_identifier]);
		glBindTexture(GL_TEXTURE_2D, textures[texture_diag_border_identifier]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, size_x, size_y, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, diagonal_borders.data());
		ogl::set_gltex_parameters(textures[texture_diag_border_identifier], GL_TEXTURE_2D, GL_NEAREST, GL_CLAMP_TO_EDGE);

		textures[texture_terrain] = ogl::make_gl_texture(&terrain_id_map[0], size_x, size_y, 1);
		ogl::set_gltex_parameters(textures[texture_terrain], GL_TEXTURE_2D, GL_NEAREST, GL_CLAMP_TO_EDGE);

		textures[texture_provinces] = load_province_map(province_id_map, size_x, size_y);
		auto texturesheet = open_file(map_terrain_dir, NATIVE("texturesheet.tga"));
		if(!texturesheet) {
			texturesheet = open_file(map_terrain_dir, NATIVE("texturesheet.dds"));
		}
		if(texturesheet) {
			texture_arrays[texture_array_terrainsheet] = ogl::load_texture_array_from_file(*texturesheet, 8, 8);
		}

		textures[texture_water_normal] = load_dds_texture(map_terrain_dir, NATIVE("sea_normal.dds"));
		textures[texture_colormap_water] = load_dds_texture(map_terrain_dir, NATIVE("colormap_water.dds"));

		textures[texture_colormap_terrain] = load_dds_texture(map_terrain_dir, NATIVE("colormap.dds"));
		textures[texture_colormap_political] = load_dds_texture(map_terrain_dir, NATIVE("colormap_political.dds"));
		textures[texture_overlay] = load_dds_texture(map_terrain_dir, NATIVE("map_overlay_tile.dds"));
		textures[texture_stripes] = load_dds_texture(map_terrain_dir, NATIVE("stripes.dds"));

		textures[texture_shoreline] = load_dds_texture(assets_dir, NATIVE("shoreline.dds"));
		ogl::set_gltex_parameters(textures[texture_shoreline], GL_TEXTURE_2D, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT);

		textures[texture_river_body] = load_dds_texture(assets_dir, NATIVE("river.dds"));
		ogl::set_gltex_parameters(textures[texture_river_body], GL_TEXTURE_2D, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT);

		textures[texture_national_border] = load_dds_texture(assets_dir, NATIVE("nat_border.dds"));
		ogl::set_gltex_parameters(textures[texture_national_border], GL_TEXTURE_2D, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT);

		textures[texture_state_border] = load_dds_texture(assets_dir, NATIVE("state_border.dds"));
		ogl::set_gltex_parameters(textures[texture_state_border], GL_TEXTURE_2D, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT);

		textures[texture_prov_border] = load_dds_texture(assets_dir, NATIVE("prov_border.dds"));
		ogl::set_gltex_parameters(textures[texture_prov_border], GL_TEXTURE_2D, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT);

		textures[texture_imp_border] = load_dds_texture(assets_dir, NATIVE("imp_border.dds"));
		ogl::set_gltex_parameters(textures[texture_imp_border], GL_TEXTURE_2D, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT);

		textures[texture_coastal_border] = load_dds_texture(assets_dir, NATIVE("coastborder.dds"));
		ogl::set_gltex_parameters(textures[texture_coastal_border], GL_TEXTURE_2D, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT);

		textures[texture_railroad] = load_dds_texture(gfx_anims_dir, NATIVE("railroad.dds"));
		ogl::set_gltex_parameters(textures[texture_railroad], GL_TEXTURE_2D, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT);

		textures[texture_unit_arrow] = ogl::make_gl_texture(map_items_dir, NATIVE("movearrow.tga"));
		ogl::set_gltex_parameters(textures[texture_unit_arrow], GL_TEXTURE_2D, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE);

		textures[texture_attack_unit_arrow] = ogl::make_gl_texture(map_items_dir, NATIVE("attackarrow.tga"));
		ogl::set_gltex_parameters(textures[texture_attack_unit_arrow], GL_TEXTURE_2D, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE);

		textures[texture_retreat_unit_arrow] = ogl::make_gl_texture(map_items_dir, NATIVE("retreatarrow.tga"));
		ogl::set_gltex_parameters(textures[texture_retreat_unit_arrow], GL_TEXTURE_2D, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE);

		textures[texture_strategy_unit_arrow] = ogl::make_gl_texture(map_items_dir, NATIVE("stratarrow.tga"));
		ogl::set_gltex_parameters(textures[texture_strategy_unit_arrow], GL_TEXTURE_2D, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE);

		textures[texture_objective_unit_arrow] = ogl::make_gl_texture(map_items_dir, NATIVE("objectivearrow.tga"));
		ogl::set_gltex_parameters(textures[texture_objective_unit_arrow], GL_TEXTURE_2D, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE);

		textures[texture_other_objective_unit_arrow] = ogl::make_gl_texture(map_items_dir, NATIVE("otherobjectivearrow.tga"));
		ogl::set_gltex_parameters(textures[texture_other_objective_unit_arrow], GL_TEXTURE_2D, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE);

		textures[texture_hover_border] = load_dds_texture(assets_dir, NATIVE("hover_border.dds"));
		ogl::set_gltex_parameters(textures[texture_hover_border], GL_TEXTURE_2D, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT);

		textures[texture_selection] = load_dds_texture(test_dir, NATIVE("selection.dds"));
		ogl::set_gltex_parameters(textures[texture_selection], GL_TEXTURE_2D, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE);
		textures[texture_capital] = load_dds_texture(map_items_dir, NATIVE("building_capital.dds"));
		ogl::set_gltex_parameters(textures[texture_capital], GL_TEXTURE_2D, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE);

		// Get the province_color handle
		// province_color is an array of 2 textures, one for province and the other for stripes
		glGenTextures(1, &texture_arrays[texture_array_province_color]);
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture_arrays[texture_array_province_color]);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 256, 256, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		ogl::set_gltex_parameters(texture_arrays[texture_array_province_color], GL_TEXTURE_2D_ARRAY, GL_NEAREST, GL_CLAMP_TO_EDGE);

		// Get the province_highlight handle
		glGenTextures(1, &textures[texture_province_highlight]);
		glBindTexture(GL_TEXTURE_2D, textures[texture_province_highlight]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		ogl::set_gltex_parameters(textures[texture_province_highlight], GL_TEXTURE_2D, GL_NEAREST, GL_CLAMP_TO_EDGE);

		// Get the province_fow handle
		glGenTextures(1, &textures[texture_province_fow]);
		glBindTexture(GL_TEXTURE_2D, textures[texture_province_fow]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		ogl::set_gltex_parameters(textures[texture_province_fow], GL_TEXTURE_2D, GL_NEAREST, GL_CLAMP_TO_EDGE);

		reports::write_debug("Generating province color and highlight textures\n");
		uint32_t province_size = state.world.province_size() + 1;
		province_size += 256 - province_size % 256;

		std::vector<uint32_t> test_highlight(province_size);
		gen_prov_color_texture(textures[texture_province_highlight], test_highlight, 1);
		for(uint32_t i = 0; i < test_highlight.size(); ++i) {
			test_highlight[i] = 255;
		}
		std::vector<uint32_t> test_color(province_size * 4);
		for(uint32_t i = 0; i < test_color.size(); ++i) {
			test_color[i] = 255;
		}
		set_province_color(test_color);

		reports::write_debug("Finished map loading\n");
	}

} // namespace map
