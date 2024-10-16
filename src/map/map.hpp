#pragma once

#include "glad.h"
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include "map_modes.hpp"
#include "opengl_wrapper.hpp"
#include "xac.hpp"

namespace sys {
	struct state;
};
namespace parsers {
	struct scenario_building_context;
};

namespace map {

	struct map_vertex {
		map_vertex(float x, float y)
		: position_(uint16_t(x * 65535.f), uint16_t(y * 65535.f))
		{
		}
		glm::u16vec2 position_;
	};
	struct screen_vertex {
		screen_vertex(float x, float y)
		: position_(uint16_t(x * 65535.f), uint16_t(y * 65535.f))
		{
		}
		glm::u16vec2 position_;
	};

	struct textured_screen_vertex {
		textured_screen_vertex(glm::vec2 position, glm::vec2 texcoord)
		: position_(uint16_t(position.x * 65535.f), uint16_t(position.y * 65535.f)),
		texcoord_(uint16_t(texcoord.x * 65535.f), uint16_t(texcoord.y * 65535.f))
		{
		}
		glm::u16vec2 position_;
		glm::u16vec2 texcoord_;
	};

	struct curved_line_vertex {
		curved_line_vertex() {
		}
		curved_line_vertex(glm::vec2 position, glm::vec2 normal_direction, glm::vec2 direction, glm::vec2 texture_coord, float type)
		: position_(uint16_t(position.x * 65535.f), uint16_t(position.y * 65535.f)),
		normal_direction_(uint16_t(normal_direction.x * 32767.f), uint16_t(normal_direction.y * 32767.f)),
		direction_(uint16_t(direction.x * 32767.f), uint16_t(direction.y * 32767.f)),
		texture_coord_(uint16_t(texture_coord.x * 65535.f), uint16_t(texture_coord.y * 65535.f)),
		type_(uint8_t(type))
		{
		}
		glm::u16vec2 position_;
		glm::i16vec2 normal_direction_;
		glm::i16vec2 direction_;
		glm::u16vec2 texture_coord_;
		uint8_t type_ = 0;
	};

	struct textured_line_vertex {
	textured_line_vertex() { }
		textured_line_vertex(glm::vec2 position, glm::vec2 normal_direction, float texture_coord, float distance)
		: position_(uint16_t(position.x * 65535.f), uint16_t(position.y * 65535.f)),
		normal_direction_(uint16_t(normal_direction.x * 32767.f), uint16_t(normal_direction.y * 32767.f)),
		texture_coord_(uint8_t(texture_coord * 255.f)),
		distance_(uint16_t(distance * 65535.f))
		{
		}
		glm::u16vec2 position_;
		glm::i16vec2 normal_direction_;
		uint8_t texture_coord_ = 0;
		uint16_t distance_ = 0;
	};

	struct textured_line_vertex_b {
		textured_line_vertex_b() {
		}
		textured_line_vertex_b(glm::vec2 position, glm::vec2 previous_point, glm::vec2 next_point, float texture_coord, float distance)
		: position_(uint16_t(position.x * 65535.f), uint16_t(position.y * 65535.f)),
		previous_point_(uint16_t(previous_point.x * 32767.f), uint16_t(previous_point.y * 32767.f)),
		next_point_(uint16_t(next_point.x * 32767.f), uint16_t(next_point.y * 32767.f)),
		texture_coord_(uint8_t(texture_coord * 255.f)),
		distance_(uint16_t(distance * 65535.f))
		{
		}
		glm::u16vec2 position_;
		glm::i16vec2 previous_point_;
		glm::i16vec2 next_point_;
		uint8_t texture_coord_ = 0;
		uint16_t distance_ = 0;
	};

	struct text_line_vertex {
		text_line_vertex() {
		}
		text_line_vertex(glm::vec2 position, glm::vec2 normal_direction, glm::vec2 direction, glm::vec2 texture_coord, float thickness)
		/*: position_(uint16_t(position.x * 65535.f), uint16_t(position.y * 65535.f)),
		normal_direction_(uint16_t(normal_direction.x * 32767.f), uint16_t(normal_direction.y * 32767.f)),
		direction_(uint16_t(direction.x * 32767.f), uint16_t(direction.y * 32767.f)),
		texture_coord_(uint16_t(texture_coord.x * 65535.f), uint16_t(texture_coord.y * 65535.f)),
		thickness_(thickness)*/
		: position_(position),
		normal_direction_(normal_direction),
		direction_(direction),
		texture_coord_(texture_coord),
		thickness_(thickness)
		{
		}
		glm::vec2 position_;
		glm::vec2 normal_direction_;
		glm::vec2 direction_;
		glm::vec2 texture_coord_;
		float thickness_ = 0.f;
	};

	struct text_line_generator_data {
		text_line_generator_data() { };
		text_line_generator_data(text::stored_glyphs&& text_, glm::vec4 coeff_, glm::vec2 basis_, glm::vec2 ratio_, dcon::nation_id n_) : text(std::move(text_)), coeff{ coeff_ }, basis{ basis_ }, ratio{ ratio_ }, n{ n_ } { };
		text::stored_glyphs text;
		glm::vec4 coeff;
		glm::vec2 basis;
		glm::vec2 ratio;
		dcon::nation_id n;
	};

	struct border {
		int start_index = 0;
		int count = 0;
		dcon::province_adjacency_id adj;
		uint16_t padding = 0;
	};

	struct model_render_command {
		dcon::emfx_object_id emfx;
		glm::vec2 pos = glm::vec2(0.f, 0.f);
		float facing = 0.f;
		emfx::animation_type anim;
	};

	class display_data {
		bool loaded_map = false;
		public:
	display_data(){};
		~display_data();

		// Called to load the terrain and province map data
		void load_map_data(parsers::scenario_building_context& context);
		// Called to load the map. Will load the texture and shaders from disk
		void load_map(sys::state& state);
		void clear_opengl_objects();

		void render(sys::state& state, glm::vec2 screen_size, glm::vec2 offset, float zoom, sys::projection_mode map_view_mode, map_mode::mode active_map_mode, glm::mat3 globe_rotation, float time_counter);
		void update_models(sys::state& state);
		void update_borders(sys::state& state);
		void update_fog_of_war(sys::state& state);
		void set_selected_province(sys::state& state, dcon::province_id province_id);
		void set_province_color(std::vector<uint32_t> const& prov_color);
		void set_drag_box(bool draw_box, glm::vec2 pos1, glm::vec2 pos2, glm::vec2 pixel_size);
		void update_railroad_paths(sys::state& state);
		void set_text_lines(sys::state& state, std::vector<text_line_generator_data> const& data);
		void set_province_text_lines(sys::state& state, std::vector<text_line_generator_data> const& data);

		std::vector<model_render_command> model_render_list;
		std::vector<border> borders;
		std::vector<textured_line_vertex_b> border_vertices;
		std::vector<textured_line_vertex> river_vertices;
		std::vector<GLint> river_starts;
		std::vector<GLsizei> river_counts;
		std::vector<textured_line_vertex> railroad_vertices;
		std::vector<GLint> railroad_starts;
		std::vector<GLsizei> railroad_counts;
		std::vector<textured_line_vertex_b> coastal_vertices;
		std::vector<GLint> coastal_starts;
		std::vector<GLsizei> coastal_counts;
		std::vector<std::vector<GLint>> static_mesh_starts;
		std::vector<std::vector<GLsizei>> static_mesh_counts;
		//
		std::vector<GLint> dyn_text_line_starts;
		std::vector<GLint> dyn_text_line_counts;
		std::vector<GLint> dyn_province_text_line_starts;
		std::vector<GLint> dyn_province_text_line_counts;
		//
		std::vector<curved_line_vertex> unit_arrow_vertices;
		std::vector<GLint> unit_arrow_starts;
		std::vector<GLsizei> unit_arrow_counts;
		//
		std::vector<curved_line_vertex> attack_unit_arrow_vertices;
		std::vector<GLint> attack_unit_arrow_starts;
		std::vector<GLsizei> attack_unit_arrow_counts;
		//
		std::vector<curved_line_vertex> retreat_unit_arrow_vertices;
		std::vector<GLint> retreat_unit_arrow_starts;
		std::vector<GLsizei> retreat_unit_arrow_counts;
		//
		std::vector<curved_line_vertex> strategy_unit_arrow_vertices;
		std::vector<GLint> strategy_unit_arrow_starts;
		std::vector<GLsizei> strategy_unit_arrow_counts;
		//
		std::vector<curved_line_vertex> objective_unit_arrow_vertices;
		std::vector<GLint> objective_unit_arrow_starts;
		std::vector<GLsizei> objective_unit_arrow_counts;
		//
		std::vector<curved_line_vertex> other_objective_unit_arrow_vertices;
		std::vector<GLint> other_objective_unit_arrow_starts;
		std::vector<GLsizei> other_objective_unit_arrow_counts;
		/* Country/Region labels */
		std::vector<GLuint> text_line_texture_per_quad;
		std::vector<text_line_vertex> text_line_vertices;
		std::vector<dcon::nation_id> text_line_tagged_vertices;
		/* Province labels */
		std::vector<GLuint> province_text_line_texture_per_quad;
		std::vector<text_line_vertex> province_text_line_vertices;
		/* Flat map billboards */
		std::vector<screen_vertex> drag_box_vertices;
		std::vector<textured_screen_vertex> selection_vertices;
		std::vector<textured_screen_vertex> capital_vertices;
		//
		std::vector<uint8_t> terrain_id_map;
		std::vector<uint8_t> median_terrain_type;
		std::vector<uint8_t> diagonal_borders;
		// map pixel -> province id
		std::vector<uint16_t> province_id_map;
		std::vector<uint16_t> map_indices;
		std::vector<uint16_t> map_globe_indices;

		uint32_t size_x;
		uint32_t size_y;

		// Meshes
		static constexpr uint32_t vo_land = 0;
		static constexpr uint32_t vo_border = 1;
		static constexpr uint32_t vo_river = 2;
		static constexpr uint32_t vo_unit_arrow = 3;
		static constexpr uint32_t vo_text_line = 4;
		static constexpr uint32_t vo_drag_box = 5;
		static constexpr uint32_t vo_coastal = 6;
		static constexpr uint32_t vo_railroad = 7;
		static constexpr uint32_t vo_static_mesh = 8;
		static constexpr uint32_t vo_province_text_line = 9;
		static constexpr uint32_t vo_attack_unit_arrow = 10;
		static constexpr uint32_t vo_retreat_unit_arrow = 11;
		static constexpr uint32_t vo_strategy_unit_arrow = 12;
		static constexpr uint32_t vo_objective_unit_arrow = 13;
		static constexpr uint32_t vo_other_objective_unit_arrow = 14;
		static constexpr uint32_t vo_selection = 15;
		static constexpr uint32_t vo_land_globe = 16;
		static constexpr uint32_t vo_capital = 17;
		static constexpr uint32_t vo_count = 18;
	GLuint vao_array[vo_count] = { 0 };
	GLuint vbo_array[vo_count] = { 0 };
		// Textures
		static constexpr uint32_t texture_provinces = 0;
		static constexpr uint32_t texture_terrain = 1;
		static constexpr uint32_t texture_water_normal = 2;
		static constexpr uint32_t texture_colormap_water = 3;
		static constexpr uint32_t texture_colormap_terrain = 4;
		static constexpr uint32_t texture_colormap_political = 5;
		static constexpr uint32_t texture_overlay = 6;
		static constexpr uint32_t texture_province_highlight = 7;
		static constexpr uint32_t texture_stripes = 8;
		static constexpr uint32_t texture_river_body = 9;
		static constexpr uint32_t texture_national_border = 10;
		static constexpr uint32_t texture_state_border = 11;
		static constexpr uint32_t texture_prov_border = 12;
		static constexpr uint32_t texture_imp_border = 13;
		static constexpr uint32_t texture_unit_arrow = 14;
		static constexpr uint32_t texture_province_fow = 15;
		static constexpr uint32_t texture_coastal_border = 16;
		static constexpr uint32_t texture_diag_border_identifier = 17;
		static constexpr uint32_t texture_railroad = 18;
		static constexpr uint32_t texture_attack_unit_arrow = 19;
		static constexpr uint32_t texture_retreat_unit_arrow = 20;
		static constexpr uint32_t texture_strategy_unit_arrow = 21;
		static constexpr uint32_t texture_objective_unit_arrow = 22;
		static constexpr uint32_t texture_other_objective_unit_arrow = 23;
		static constexpr uint32_t texture_hover_border = 24;
		static constexpr uint32_t texture_shoreline = 25;
		static constexpr uint32_t texture_selection = 26;
		static constexpr uint32_t texture_capital = 27;
		static constexpr uint32_t texture_count = 28;
	GLuint textures[texture_count] = { 0 };
		// Texture Array
		static constexpr uint32_t texture_array_terrainsheet = 0;
		static constexpr uint32_t texture_array_province_color = 1;
		static constexpr uint32_t texture_array_count = 2;
	GLuint texture_arrays[texture_array_count] = { 0 };
		// Shaders
		static constexpr uint32_t shader_far_terrain = 0;
		static constexpr uint32_t shader_close_terrain = 1;
		static constexpr uint32_t shader_line_border = 2;
		static constexpr uint32_t shader_textured_line = 3;
		static constexpr uint32_t shader_line_unit_arrow = 4;
		static constexpr uint32_t shader_text_line = 5;
		static constexpr uint32_t shader_drag_box = 6;
		static constexpr uint32_t shader_borders = 7;
		static constexpr uint32_t shader_railroad_line = 8;
		static constexpr uint32_t shader_map_standing_object = 9;
		static constexpr uint32_t shader_selection = 10;
		static constexpr uint32_t shader_count = 11;
	GLuint shaders[uint8_t(sys::projection_mode::num_of_modes)][shader_count] = { 0 };

		static constexpr uint32_t uniform_offset = 0;
		static constexpr uint32_t uniform_aspect_ratio = 1;
		static constexpr uint32_t uniform_zoom = 2;
		static constexpr uint32_t uniform_map_size = 3;
		static constexpr uint32_t uniform_rotation = 4;
		static constexpr uint32_t uniform_gamma = 5;
		static constexpr uint32_t uniform_subroutines_index = 6;
		static constexpr uint32_t uniform_time = 7;
		//
		static constexpr uint32_t uniform_provinces_texture_sampler = 8;
		static constexpr uint32_t uniform_terrain_texture_sampler = 9;
		static constexpr uint32_t uniform_terrainsheet_texture_sampler = 10;
		static constexpr uint32_t uniform_water_normal = 11;
		static constexpr uint32_t uniform_colormap_water = 12;
		static constexpr uint32_t uniform_colormap_terrain = 13;
		static constexpr uint32_t uniform_overlay = 14;
		static constexpr uint32_t uniform_province_color = 15;
		static constexpr uint32_t uniform_colormap_political = 16;
		static constexpr uint32_t uniform_province_highlight = 17;
		static constexpr uint32_t uniform_stripes_texture = 18;
		static constexpr uint32_t uniform_province_fow = 19;
		static constexpr uint32_t uniform_diag_border_identifier = 20;
		static constexpr uint32_t uniform_width = 21;
		static constexpr uint32_t uniform_subroutines_index_2 = 22;
		static constexpr uint32_t uniform_line_texture = 23;
		static constexpr uint32_t uniform_texture_sampler = 24;
		static constexpr uint32_t uniform_opaque = 25;
		static constexpr uint32_t uniform_is_black = 26;
		static constexpr uint32_t uniform_border_width = 27;
		static constexpr uint32_t uniform_unit_arrow = 28;
		static constexpr uint32_t uniform_model_offset = 29;
		static constexpr uint32_t uniform_target_facing = 30;
		static constexpr uint32_t uniform_target_topview_fixup = 31;
		static constexpr uint32_t uniform_counter_factor = 32;
		static constexpr uint32_t uniform_model_position = 33;
		static constexpr uint32_t uniform_model_scale = 34;
		static constexpr uint32_t uniform_model_rotation = 35;
		static constexpr uint32_t uniform_model_scale_rotation = 36;
		static constexpr uint32_t uniform_model_proj_view = 37;
		static constexpr uint32_t uniform_count = 38;
	GLuint shader_uniforms[uint8_t(sys::projection_mode::num_of_modes)][shader_count][uniform_count] = { };

		static constexpr uint32_t max_bone_matrices = 100;
	GLuint bone_matrices_uniform_array[uint8_t(sys::projection_mode::num_of_modes)] = { };

		// models: Textures for static meshes
		static constexpr uint32_t max_static_meshes = 0x140;
		static constexpr uint32_t max_static_submeshes = 16;
	GLuint static_mesh_textures[max_static_meshes][max_static_submeshes] = { };
	float static_mesh_scrolling_factor[max_static_meshes][max_static_submeshes] = { };
	uint32_t static_mesh_submesh_node_index[max_static_meshes][max_static_submeshes] = { };

	uint32_t static_mesh_idle_animation_start[max_static_meshes] = { };
	uint32_t static_mesh_idle_animation_count[max_static_meshes] = { };
	uint32_t static_mesh_move_animation_start[max_static_meshes] = { };
	uint32_t static_mesh_move_animation_count[max_static_meshes] = { };
	uint32_t static_mesh_attack_animation_start[max_static_meshes] = { };
	uint32_t static_mesh_attack_animation_count[max_static_meshes] = { };

		dcon::emfx_object_id model_wake;
		dcon::emfx_object_id model_blockaded;
		dcon::emfx_object_id model_naval_base[7];
		dcon::emfx_object_id model_naval_base_ships[7];
		dcon::emfx_object_id model_fort[7];
		dcon::emfx_object_id model_factory;
		dcon::emfx_object_id model_train_station;
		dcon::emfx_object_id model_construction;
		dcon::emfx_object_id model_construction_naval;
		dcon::emfx_object_id model_construction_military;
		dcon::emfx_object_id model_siege;
		dcon::emfx_object_id model_flag;
		dcon::emfx_object_id model_flag_floating;
		std::vector<dcon::emfx_object_id> model_province_flag;
		std::vector<dcon::emfx_object_id> model_gc_unit[uint8_t(culture::graphical_culture_type::count)];
		std::vector<emfx::xsm_animation> animations;

		void load_border_data(parsers::scenario_building_context& context);
		void create_border_ogl_objects();
		void load_province_data(parsers::scenario_building_context& context, ogl::image& image);
		void load_provinces_mid_point(parsers::scenario_building_context& context);
		void load_terrain_data(parsers::scenario_building_context& context);
		void load_median_terrain_type(parsers::scenario_building_context& context);

		uint16_t safe_get_province(int32_t x, int32_t y);
		void make_coastal_borders(sys::state& state, std::vector<bool>& visited);
		void make_borders(sys::state& state, std::vector<bool>& visited);

		void render_models(sys::state& state, std::vector<model_render_command> const& list, float time_counter, sys::projection_mode map_view_mode, float zoom);
		void load_shaders(simple_fs::directory& root);
		void create_meshes();
		void gen_prov_color_texture(GLuint texture_handle, std::vector<uint32_t> const& prov_color, uint8_t layers = 1);

		void create_curved_river_vertices(parsers::scenario_building_context& context, std::vector<uint8_t> const& river_data, std::vector<uint8_t> const& terrain_data);
	};

	void load_river_crossings(parsers::scenario_building_context& context, std::vector<uint8_t> const& river_data, glm::ivec2 map_size);

	void make_navy_path(sys::state& state, std::vector<map::curved_line_vertex>& buffer, dcon::navy_id selected_navy, float size_x, float size_y);
	void make_army_path(sys::state& state, std::vector<map::curved_line_vertex>& buffer, dcon::army_id selected_army, float size_x, float size_y);
	void make_selection_quad(sys::state& state, std::vector<map::textured_screen_vertex>& buffer, glm::vec2 p);
	glm::vec2 put_in_local(glm::vec2 new_point, glm::vec2 base_point, float size_x);
	void add_bezier_to_buffer(std::vector<map::curved_line_vertex>& buffer, glm::vec2 start, glm::vec2 end, glm::vec2 start_per, glm::vec2 end_per, float progress, bool last_curve, float size_x, float size_y, uint32_t num_b_segments);
	void add_tl_bezier_to_buffer(std::vector<map::textured_line_vertex>& buffer, glm::vec2 start, glm::vec2 end, glm::vec2 start_per, glm::vec2 end_per, float progress, bool last_curve, float size_x, float size_y, uint32_t num_b_segments, float& distance);

} // namespace map
