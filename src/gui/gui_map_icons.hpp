 #pragma once

#include "dcon_generated.hpp"
#include "gui_element_types.hpp"
#include "gui_graphics.hpp"
#include "province.hpp"
#include "text.hpp"
#include "unit_tooltip.hpp"
#include "gui_land_combat.hpp"
#include "gui_naval_combat.hpp"
#include "gui_unit_grid_box.hpp"
#include "map_state.hpp"

namespace ui {

	inline constexpr float big_counter_cutoff = 15.0f;
	inline constexpr float prov_details_cutoff = 18.0f;

	struct toggle_unit_grid {
		bool with_shift;
	};

	struct top_display_parameters {
		float top_left_value = 0.0f;
		float top_right_value = 0.0f;
		float top_left_org_value = 0.0f;
		float top_right_org_value = 0.0f;
		float battle_progress = 0.0f;
		dcon::nation_id top_left_nation;
		dcon::nation_id top_right_nation;
		dcon::rebel_faction_id top_left_rebel;
		dcon::rebel_faction_id top_right_rebel;
		int8_t top_left_status = 0;
		int8_t top_dig_in = -1;
		int8_t top_right_dig_in = -1;
		int8_t right_frames = 0;
		int8_t colors_used = 0;
		int8_t common_unit_1 = -1;
		int8_t common_unit_2 = -1;
		std::array<outline_color, 5> colors;
		bool is_army = false;
		float attacker_casualties = 0.0f;
		float defender_casualties = 0.0f;
		bool player_involved_battle = false;
		bool player_is_attacker = false;
	};

	class prov_map_br_overlay : public image_element_base {
		public:
		void on_create(sys::state& state) noexcept override {
			image_element_base::on_create(state);
			frame = 1;
		}
	};

	enum class unit_counter_position_type : uint8_t {
		land,
		land_move,
		port,
	};

	bool filter_unit_for_position_type(sys::state& state, unit_counter_position_type pos, dcon::army_id a) {
		auto fat_id = dcon::fatten(state.world, a);
		switch(pos) {
		case unit_counter_position_type::port:
		case unit_counter_position_type::land:
			return (fat_id.get_path().size() == 0 || fat_id.get_battle_from_army_battle_participation())
				&& !fat_id.get_navy_from_army_transport();
		case unit_counter_position_type::land_move:
			return (fat_id.get_path().size() > 0 && !fat_id.get_battle_from_army_battle_participation());
		}
	}
	bool filter_unit_for_position_type(sys::state& state, unit_counter_position_type pos, dcon::navy_id a) {
		auto fat_id = dcon::fatten(state.world, a);
		switch(pos) {
		case unit_counter_position_type::port:
		case unit_counter_position_type::land:
		default:
			return fat_id.get_path().size() == 0 || fat_id.get_battle_from_navy_battle_participation();
		case unit_counter_position_type::land_move:
			return (fat_id.get_path().size() > 0 && !fat_id.get_battle_from_navy_battle_participation());
		}
		return true;
	}

	template<unit_counter_position_type A>
	class unit_counter_org_bar : public vertical_progress_bar {
	public:
		void on_update(sys::state& state) noexcept override {
			float total = 0.f;
			float value = 0.f;
			auto prov = retrieve<dcon::province_id>(state, parent);
			if(bool(retrieve<dcon::army_id>(state, parent))) {
				for(const auto al : state.world.province_get_army_location_as_location(prov)) {
					if(filter_unit_for_position_type(state, A, al.get_army())) {
						for(const auto memb : al.get_army().get_army_membership()) {
							value += memb.get_regiment().get_org();
							total += 1.f;
						}
					}
				}
			}
			if(bool(retrieve<dcon::navy_id>(state, parent))) {
				for(const auto al : state.world.province_get_navy_location_as_location(prov)) {
					if(filter_unit_for_position_type(state, A, al.get_navy())) {
						for(const auto memb : al.get_navy().get_navy_membership()) {
							value += memb.get_ship().get_org();
							total += 1.f;
						}
					}
				}
			}
			progress = (total == 0.f) ? 0.f : value / total;
		}
	};

	class unit_counter_flag : public flag_button2 {
		public:
		mouse_probe impl_probe_mouse(sys::state& state, int32_t x, int32_t y, mouse_probe_type type) noexcept override {
			if(type == mouse_probe_type::tooltip)
				return flag_button2::impl_probe_mouse(state, x, y, type);
			return mouse_probe{ nullptr, ui::xy_pair{} };
		}
	};

	template<unit_counter_position_type A>
	class unit_counter_strength : public simple_text_element_base {
		void on_update(sys::state& state) noexcept override {
			auto prov = retrieve<dcon::province_id>(state, parent);
			float value = 0.f;
			bool has_navy = bool(retrieve<dcon::navy_id>(state, parent));
			if(!has_navy) {
				for(const auto al : state.world.province_get_army_location_as_location(prov)) {
					if(filter_unit_for_position_type(state, A, al.get_army())) {
						for(const auto memb : al.get_army().get_army_membership()) {
							value += memb.get_regiment().get_strength() * 3.f;
						}
					}
				}
			} else {
				for(const auto al : state.world.province_get_navy_location_as_location(prov)) {
					if(filter_unit_for_position_type(state, A, al.get_navy())) {
						for(const auto memb : al.get_navy().get_navy_membership()) {
							value += memb.get_ship().get_strength();
						}
					}
				}
			}
			set_text(state, text::prettify(int64_t(value)));
		}
	};

	class unit_counter_attrition : public image_element_base {
		bool visible = false;
	public:
		void on_update(sys::state& state) noexcept override {
			visible = false;
			auto n = retrieve<dcon::navy_id>(state, parent);
			if(n && military::will_recieve_attrition(state, n)) {
				visible = true;
			}
			auto a = retrieve<dcon::army_id>(state, parent);
			if(a && military::will_recieve_attrition(state, a)) {
				visible = true;
			}
		}
		void impl_render(sys::state& state, int32_t x, int32_t y) noexcept override {
			if(visible)
				image_element_base::impl_render(state, x, y);
		}
	};

	class unit_counter_color_bg : public image_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			auto prov = retrieve<dcon::province_id>(state, parent);
			frame = 0;
			auto n = retrieve<dcon::navy_id>(state, parent);
			if(n) {
				if(state.world.navy_get_controller_from_navy_control(n) != state.local_player_nation) {
					frame = 2; //gray
					if(military::are_at_war(state, state.world.navy_get_controller_from_navy_control(n), state.local_player_nation)) {
						frame = 1; //red
					} else if(military::are_allied_in_war(state, state.world.navy_get_controller_from_navy_control(n), state.local_player_nation)) {
						frame = 0; //green
					}
				}
				return;
			}
			auto a = retrieve<dcon::army_id>(state, parent);
			if(state.world.army_get_controller_from_army_control(a) != state.local_player_nation) {
				frame = 2; //gray
				if(military::are_at_war(state, state.world.army_get_controller_from_army_control(a), state.local_player_nation)
				|| state.world.army_get_controller_from_army_rebel_control(a)) {
					frame = 1; //red
				} else if(military::are_allied_in_war(state, state.world.army_get_controller_from_army_control(a), state.local_player_nation)) {
					frame = 0; //green
				} else if(state.world.army_get_is_retreating(a) || state.world.army_get_black_flag(a)) {
					frame = 2; //gray
				}
			}
		}
	};

	template<unit_counter_position_type A>
	class unit_counter_bg : public button_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			auto prov = retrieve<dcon::province_id>(state, parent);
			auto n = retrieve<dcon::navy_id>(state, parent);
			frame = n ? 1 : 0;
		}

		void button_action(sys::state& state) noexcept override {
			auto prov = retrieve<dcon::province_id>(state, parent);
			state.selected_armies.clear();
			state.selected_navies.clear();
			if(bool(retrieve<dcon::navy_id>(state, parent))) {
				for(const auto al : state.world.province_get_navy_location_as_location(prov)) {
					if(al.get_navy().get_controller_from_navy_control() == state.local_player_nation) {
						if(filter_unit_for_position_type(state, A, al.get_navy())) {
							state.select(al.get_navy());
						}
					}
				}
			}
			if(state.selected_navies.empty() && bool(retrieve<dcon::army_id>(state, parent))) {
				for(const auto al : state.world.province_get_army_location_as_location(prov)) {
					if(!al.get_army().get_navy_from_army_transport()
					&& al.get_army().get_controller_from_army_control() == state.local_player_nation) {
						if(filter_unit_for_position_type(state, A, al.get_army())) {
							state.select(al.get_army());
						}
					}
				}
			}
		}

		void button_shift_action(sys::state& state) noexcept override {
			auto prov = retrieve<dcon::province_id>(state, parent);
			if(bool(retrieve<dcon::navy_id>(state, parent))) {
				state.selected_armies.clear();
				for(const auto al : state.world.province_get_navy_location_as_location(prov)) {
					if(al.get_navy().get_controller_from_navy_control() == state.local_player_nation) {
						if(filter_unit_for_position_type(state, A, al.get_navy())) {
							state.select(al.get_navy());
						}
					}
				}
			}
			if(bool(retrieve<dcon::army_id>(state, parent))) {
				state.selected_navies.clear();
				for(const auto al : state.world.province_get_army_location_as_location(prov)) {
					if(al.get_army().get_controller_from_army_control() == state.local_player_nation) {
						if(filter_unit_for_position_type(state, A, al.get_army())) {
							state.select(al.get_army());
						}
					}
				}
			}
		}

		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			populate_unit_tooltip(state, contents, retrieve<dcon::province_id>(state, parent));
		}
	};

	template<unit_counter_position_type A>
	class unit_counter_window : public window_element_base {
	public:
		bool visible = true;
		bool populated = false;
		dcon::province_id prov;
		dcon::army_id army;
		dcon::navy_id navy;

		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "unit_panel_bg") {
				return make_element_by_type<unit_counter_bg<A>>(state, id);
			} else if(name == "unit_panel_color") {
				return make_element_by_type<unit_counter_color_bg>(state, id);
			} else if(name == "unit_strength") {
				return make_element_by_type<unit_counter_strength<A>>(state, id);
			} else if(name == "unit_panel_org_bar") {
				return make_element_by_type<unit_counter_org_bar<A>>(state, id);
			} else if(name == "unit_panel_country_flag") {
				auto ptr = make_element_by_type<unit_counter_flag>(state, id);
				ptr->base_data.position.y -= 1; //nudge
				return ptr;
			} else if(name == "unit_panel_attr") {
				return make_element_by_type<unit_counter_attrition>(state, id);
			} else {
				return nullptr;
			}
		}

		void impl_on_update(sys::state& state) noexcept override {
			on_update(state);
			if(populated) {
				for(auto& c : children) {
					if(c->is_visible()) {
						c->impl_on_update(state);
					}
				}
			}
		}

		void on_update(sys::state& state) noexcept override {
			if constexpr(A == unit_counter_position_type::port) { //port
				army = dcon::army_id{};
				navy = dcon::navy_id{};
				for(auto al : state.world.province_get_navy_location_as_location(prov)) {
					if(al.get_navy()) {
						navy = al.get_navy();
						if(al.get_navy().get_controller_from_navy_control() == state.local_player_nation)
							break;
					}
				}
			} else if constexpr(A == unit_counter_position_type::land_move) { //moving units
				army = dcon::army_id{};
				for(auto al : state.world.province_get_army_location_as_location(prov)) {
					if(!al.get_army().get_navy_from_army_transport()
					&& al.get_army()
					&& al.get_army().get_path().size() > 0
					&& !al.get_army().get_battle_from_army_battle_participation()) {
						army = al.get_army();
						if(al.get_army().get_controller_from_army_control() == state.local_player_nation)
							break;
					}
				}
				navy = dcon::navy_id{};
				if(prov.index() >= state.province_definitions.first_sea_province.index()) {
					for(auto al : state.world.province_get_navy_location_as_location(prov)) {
						if(al.get_navy()
						&& al.get_navy().get_path().size() > 0
						&& !al.get_navy().get_battle_from_navy_battle_participation()) {
							navy = al.get_navy();
							if(al.get_navy().get_controller_from_navy_control() == state.local_player_nation)
								break;
						}
					}
				}
			} else { //units
				army = dcon::army_id{};
				for(auto al : state.world.province_get_army_location_as_location(prov)) {
					if(!al.get_army().get_navy_from_army_transport()
					&& al.get_army()
					&& al.get_army().get_path().size() == 0) {
						army = al.get_army();
						if(al.get_army().get_controller_from_army_control() == state.local_player_nation)
							break;
					}
				}
				navy = dcon::navy_id{};
				if(prov.index() >= state.province_definitions.first_sea_province.index()) {
					for(auto al : state.world.province_get_navy_location_as_location(prov)) {
						if(al.get_navy()
						&& al.get_navy().get_path().size() == 0) {
							navy = al.get_navy();
							if(al.get_navy().get_controller_from_navy_control() == state.local_player_nation)
								break;
						}
					}
				}
			}
			populated = bool(army) || bool(navy);
		}

		void impl_render(sys::state& state, int32_t x, int32_t y) noexcept override {
			if(populated) {
				glm::vec2 map_pos;
				if constexpr(A == unit_counter_position_type::port) {
					glm::vec2 map_size = glm::vec2(state.map_state.map_data.size_x, state.map_state.map_data.size_y);
					glm::vec2 v = map::get_port_location(state, prov) / map_size;
					v.y -= 2.5f / float(state.map_state.map_data.size_y);
					map_pos = glm::vec2(v.x, 1.f - v.y);
				} else if constexpr(A == unit_counter_position_type::land_move) { //moving units
					auto path = army ? state.world.army_get_path(army) : state.world.navy_get_path(navy);
					if(path.size() > 0) {
						auto dp = state.world.province_get_mid_point(path[path.size() - 1]);
						auto mp = state.world.province_get_mid_point(prov);
						auto theta = glm::atan(dp.x - mp.x, dp.y - mp.y);
						mp.x += 2.f * glm::sin(theta);
						mp.y += 2.f * glm::cos(theta);
						map_pos = state.map_state.normalize_map_coord(mp);
					} else {
						visible = false;
						return;
					}
				} else {
					auto mid_point = state.world.province_get_mid_point(prov);
					map_pos = state.map_state.normalize_map_coord(mid_point);
				}
				auto screen_size = glm::vec2{ float(state.x_size / state.user_settings.ui_scale), float(state.y_size / state.user_settings.ui_scale) };
				glm::vec2 screen_pos;
				if(!state.map_state.map_to_screen(state, map_pos, screen_size, screen_pos)) {
					visible = false;
					return;
				}
				if(!state.map_state.visible_provinces[province::to_map_id(prov)]) {
					visible = false;
					return;
				}
				visible = true;
				auto new_position = xy_pair{ int16_t(screen_pos.x), int16_t(screen_pos.y) };
				new_position.x += 7 - base_data.size.x / 2;
				new_position.y -= 22;
				base_data.position = new_position;
				base_data.flags &= ~ui::element_data::orientation_mask; //position upperleft
				window_element_base::impl_render(state, new_position.x, new_position.y);
			}
		}

		message_result get(sys::state& state, Cyto::Any& payload) noexcept override {
			if(payload.holds_type<dcon::province_id>()) {
				payload.emplace<dcon::province_id>(prov);
				return message_result::consumed;
			} else if(payload.holds_type<dcon::army_id>()) {
				payload.emplace<dcon::army_id>(army);
				return message_result::consumed;
			} else if(payload.holds_type<dcon::navy_id>()) {
				payload.emplace<dcon::navy_id>(navy);
				return message_result::consumed;
			} else if(payload.holds_type<dcon::rebel_faction_id>()) {
				payload.emplace<dcon::rebel_faction_id>(state.world.army_get_controller_from_army_rebel_control(army));
				return message_result::consumed;
			} else if(payload.holds_type<dcon::nation_id>()) {
				if(navy) {
					payload.emplace<dcon::nation_id>(state.world.navy_get_controller_from_navy_control(navy));
				} else {
					payload.emplace<dcon::nation_id>(state.world.army_get_controller_from_army_control(army));
				}
				return message_result::consumed;
			}
			return message_result::unseen;
		}

		mouse_probe impl_probe_mouse(sys::state& state, int32_t x, int32_t y, mouse_probe_type type) noexcept override {
			if(visible && populated)
				return window_element_base::impl_probe_mouse(state, x, y, type);
			return mouse_probe{ nullptr, ui::xy_pair{} };
		}
	};


	class siege_counter_progress : public progress_bar {
		public:
		void on_update(sys::state& state) noexcept override {
			auto prov = retrieve<dcon::province_id>(state, parent);
			progress = state.world.province_get_siege_progress(prov);
		}
	};

	class siege_counter_window : public window_element_base {
		public:
		bool visible = true;
		bool populated = false;
		dcon::province_id prov;
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "siege_progress_bar") {
				auto ptr = make_element_by_type<siege_counter_progress>(state, id);
				ptr->base_data.position.y -= 1;
				return ptr;
			} else {
				return nullptr;
			}
		}

		void impl_on_update(sys::state& state) noexcept override {
			on_update(state);
			if(!populated)
			return;

			for(auto& c : children) {
				if(c->is_visible()) {
					c->impl_on_update(state);
				}
			}
		}

		void on_update(sys::state& state) noexcept override {
			populated = state.world.province_get_siege_progress(prov) > 0.f;
		}

		void impl_render(sys::state& state, int32_t x, int32_t y) noexcept override {
			if(populated) {
				auto mid_point = state.world.province_get_mid_point(prov);
				auto map_pos = state.map_state.normalize_map_coord(mid_point);
			auto screen_size = glm::vec2{ float(state.x_size / state.user_settings.ui_scale), float(state.y_size / state.user_settings.ui_scale) };
				glm::vec2 screen_pos;
				if(!state.map_state.map_to_screen(state, map_pos, screen_size, screen_pos)) {
					visible = false;
					return;
				}
				if(!state.map_state.visible_provinces[province::to_map_id(prov)]) {
					visible = false;
					return;
				}
				visible = true;
			auto new_position = xy_pair{ int16_t(screen_pos.x), int16_t(screen_pos.y) };
				new_position.x += 7 - base_data.size.x / 2;
				new_position.y += -4;
				base_data.position = new_position;
				window_element_base::impl_render(state, new_position.x, new_position.y);
			}
		}
		message_result get(sys::state& state, Cyto::Any& payload) noexcept override {
			if(payload.holds_type<dcon::province_id>()) {
				payload.emplace<dcon::province_id>(prov);
				return message_result::consumed;
			}
			return message_result::unseen;
		}
		mouse_probe impl_probe_mouse(sys::state& state, int32_t x, int32_t y, mouse_probe_type type) noexcept override {
			if(visible && populated)
			return window_element_base::impl_probe_mouse(state, x, y, type);
		return mouse_probe{ nullptr, ui::xy_pair{} };
		}
	};

	class battle_panel_bg : public button_element_base {
		public:
		void button_action(sys::state& state) noexcept override {
			auto lbattle = retrieve<dcon::land_battle_id>(state, parent);
			auto nbattle = retrieve<dcon::naval_battle_id>(state, parent);
			if(lbattle || nbattle) {
				game_scene::deselect_units(state);
			state.map_state.set_selected_province(dcon::province_id{});
			game_scene::open_province_window(state, dcon::province_id{});
				if(state.ui_state.army_status_window) {
					state.ui_state.army_status_window->set_visible(state, false);
				}
				if(state.ui_state.navy_status_window) {
					state.ui_state.navy_status_window->set_visible(state, false);
				}
				if(state.ui_state.multi_unit_selection_window) {
					state.ui_state.multi_unit_selection_window->set_visible(state, false);
				}
				if(state.ui_state.army_reorg_window) {
					state.ui_state.army_reorg_window->set_visible(state, false);
				}
				if(state.ui_state.navy_reorg_window) {
					state.ui_state.navy_reorg_window->set_visible(state, false);
				}
			} else {
				return;
			}
			//
			if(lbattle) {
				if(!state.ui_state.army_combat_window) {
					auto new_elm = ui::make_element_by_type<ui::land_combat_window>(state, "alice_land_combat");
					state.ui_state.army_combat_window = new_elm.get();
					state.ui_state.root->add_child_to_front(std::move(new_elm));
				}
				land_combat_window* win = static_cast<land_combat_window*>(state.ui_state.army_combat_window);
				win->battle = lbattle;
				//
				if(state.ui_state.army_combat_window->is_visible()) {
					state.ui_state.army_combat_window->impl_on_update(state);
				} else {
					state.ui_state.army_combat_window->set_visible(state, true);
					if(state.ui_state.naval_combat_window) {
						state.ui_state.naval_combat_window->set_visible(state, false);
					}
				}
			} else if(nbattle) {
				if(!state.ui_state.naval_combat_window) {
					auto new_elm = ui::make_element_by_type<ui::naval_combat_window>(state, "alice_naval_combat");
					state.ui_state.naval_combat_window = new_elm.get();
					state.ui_state.root->add_child_to_front(std::move(new_elm));
				}
				naval_combat_window* win = static_cast<naval_combat_window*>(state.ui_state.naval_combat_window);
				win->battle = nbattle;
				//
				if(state.ui_state.naval_combat_window->is_visible()) {
					state.ui_state.naval_combat_window->impl_on_update(state);
				} else {
					state.ui_state.naval_combat_window->set_visible(state, true);
					if(state.ui_state.army_combat_window) {
						state.ui_state.army_combat_window->set_visible(state, false);
					}
				}
			}
		}
	};

	template<bool IsAttacker>
	class battle_counter_strength : public simple_text_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			auto lbattle = retrieve<dcon::land_battle_id>(state, parent);
			if(lbattle) {
				float value = 0.f;
				auto w = state.world.land_battle_get_war_from_land_battle_in_war(lbattle);
				auto is_attacker = state.world.land_battle_get_war_attacker_is_attacker(lbattle);
				for(const auto a : state.world.land_battle_get_army_battle_participation(lbattle)) {
					auto const controller = a.get_army().get_army_control().get_controller();
					auto const role = military::get_role(state, w, controller);
					if((role == military::war_role::attacker && (is_attacker == IsAttacker))
					|| (role == military::war_role::defender && !(is_attacker == IsAttacker))
					|| (!w && IsAttacker == bool(controller))) {
						for(const auto memb : a.get_army().get_army_membership()) {
							value += memb.get_regiment().get_strength() * 3.f;
						}
					}
				}
				set_text(state, text::prettify(int64_t(value)));
			} else {
				auto nbattle = retrieve<dcon::naval_battle_id>(state, parent);
				float value = 0.f;
				auto w = state.world.naval_battle_get_war_from_naval_battle_in_war(nbattle);
				auto is_attacker = state.world.naval_battle_get_war_attacker_is_attacker(nbattle);
				for(const auto a : state.world.naval_battle_get_navy_battle_participation(nbattle)) {
					auto const role = military::get_role(state, w, a.get_navy().get_navy_control().get_controller());
					if((role == military::war_role::attacker && (is_attacker == IsAttacker))
					|| (role == military::war_role::defender && !(is_attacker == IsAttacker))) {
						for(const auto memb : a.get_navy().get_navy_membership()) {
							value += memb.get_ship().get_strength();
						}
					}
				}
				set_text(state, text::prettify(int64_t(value)));
			}
		}
	};

	template<bool IsAttacker>
	class battle_counter_org_bar : public vertical_progress_bar {
		public:
		void on_update(sys::state& state) noexcept override {
			float total = 0.f;
			float value = 0.f;
			//
			auto lbattle = retrieve<dcon::land_battle_id>(state, parent);
			if(lbattle) {
				auto w = state.world.land_battle_get_war_from_land_battle_in_war(lbattle);
				auto is_attacker = state.world.land_battle_get_war_attacker_is_attacker(lbattle);
				for(const auto a : state.world.land_battle_get_army_battle_participation(lbattle)) {
					auto const controller = a.get_army().get_army_control().get_controller();
					auto const role = military::get_role(state, w, controller);
					if((role == military::war_role::attacker && (is_attacker == IsAttacker))
					|| (role == military::war_role::defender && !(is_attacker == IsAttacker))
					|| (!w && IsAttacker == bool(controller))) {
						for(const auto memb : a.get_army().get_army_membership()) {
							value += memb.get_regiment().get_org();
							total += 1.f;
						}
					}
				}
			} else {
				auto nbattle = retrieve<dcon::naval_battle_id>(state, parent);
				auto w = state.world.naval_battle_get_war_from_naval_battle_in_war(nbattle);
				auto is_attacker = state.world.naval_battle_get_war_attacker_is_attacker(nbattle);
				for(const auto a : state.world.naval_battle_get_navy_battle_participation(nbattle)) {
					auto const role = military::get_role(state, w, a.get_navy().get_navy_control().get_controller());
					if((role == military::war_role::attacker && (is_attacker == IsAttacker))
					|| (role == military::war_role::defender && !(is_attacker == IsAttacker))) {
						for(const auto memb : a.get_navy().get_navy_membership()) {
							value += memb.get_ship().get_org();
							total += 1.f;
						}
					}
				}
			}
			progress = (total == 0.f) ? 0.f : value / total;
		}
	};

	template<bool IsAttacker>
	class battle_counter_flag : public flag_button {
		public:
		dcon::national_identity_id get_current_nation(sys::state& state) noexcept override {
			auto lbattle = retrieve<dcon::land_battle_id>(state, parent);
			if(lbattle) {
				dcon::nation_id n = IsAttacker
				? military::get_land_battle_lead_attacker(state, lbattle)
				: military::get_land_battle_lead_defender(state, lbattle);
				return state.world.nation_get_identity_from_identity_holder(n);
			}
			auto nbattle = retrieve<dcon::naval_battle_id>(state, parent);
			if(nbattle) {
				dcon::nation_id n = IsAttacker
				? military::get_naval_battle_lead_attacker(state, nbattle)
				: military::get_naval_battle_lead_defender(state, nbattle);
				return state.world.nation_get_identity_from_identity_holder(n);
			}
			return state.national_definitions.rebel_id;
		}
		mouse_probe impl_probe_mouse(sys::state& state, int32_t x, int32_t y, mouse_probe_type type) noexcept override {
			if(type == mouse_probe_type::tooltip)
			return flag_button::impl_probe_mouse(state, x, y, type);
		return mouse_probe{ nullptr, ui::xy_pair{} };
		}
	};

	class battle_counter_window : public window_element_base {
		public:
		bool visible = true;
		bool populated = false;
		dcon::province_id prov;
		dcon::land_battle_id land_battle;
		dcon::naval_battle_id naval_battle;

		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "combat_panel_bg") {
				return make_element_by_type<battle_panel_bg>(state, id);
			} else if(name == "unit_strengthl") {
				return make_element_by_type<battle_counter_strength<false>>(state, id);
			} else if(name == "unit_strengthr") {
				return make_element_by_type<battle_counter_strength<true>>(state, id);
			} else if(name == "combat_country_flagl") {
				auto ptr = make_element_by_type<battle_counter_flag<false>>(state, id);
				ptr->base_data.position.y -= 1; //nudge
				return ptr;
			} else if(name == "combat_country_flagr") {
				auto ptr = make_element_by_type<battle_counter_flag<true>>(state, id);
				ptr->base_data.position.y -= 1; //nudge
				return ptr;
			} else if(name == "unit_panel_org_barl") {
				return make_element_by_type<battle_counter_org_bar<false>>(state, id);
			} else if(name == "unit_panel_org_barr") {
				return make_element_by_type<battle_counter_org_bar<true>>(state, id);
			} else {
				return nullptr;
			}
		}

		void impl_on_update(sys::state& state) noexcept override {
			on_update(state);
			if(!populated)
			return;

			for(auto& c : children) {
				if(c->is_visible()) {
					c->impl_on_update(state);
				}
			}
		}

		void on_update(sys::state& state) noexcept override {
		land_battle = dcon::land_battle_id{ };
			for(const auto lb : state.world.province_get_land_battle_location(prov)) {
				land_battle = lb.get_battle();
			}
		naval_battle = dcon::naval_battle_id{ };
			for(const auto lb : state.world.province_get_naval_battle_location(prov)) {
				naval_battle = lb.get_battle();
			}
			populated = bool(land_battle) || bool(naval_battle);
		}

		void impl_render(sys::state& state, int32_t x, int32_t y) noexcept override {
			if(populated) {
				auto mid_point = state.world.province_get_mid_point(prov);
				auto map_pos = state.map_state.normalize_map_coord(mid_point);
			auto screen_size = glm::vec2{ float(state.x_size / state.user_settings.ui_scale), float(state.y_size / state.user_settings.ui_scale) };
				glm::vec2 screen_pos;
				if(!state.map_state.map_to_screen(state, map_pos, screen_size, screen_pos)) {
					visible = false;
					return;
				}
				if(!state.map_state.visible_provinces[province::to_map_id(prov)]) {
					visible = false;
					return;
				}
				visible = true;
			auto new_position = xy_pair{ int16_t(screen_pos.x), int16_t(screen_pos.y) };
				new_position.x += 7 - base_data.size.x / 2; //114/2 = 57
				new_position.y -= 24;
				base_data.position = new_position;
				base_data.flags &= ~ui::element_data::orientation_mask; //position upperleft
				window_element_base::impl_render(state, new_position.x, new_position.y);
			}
		}
		message_result get(sys::state& state, Cyto::Any& payload) noexcept override {
			if(payload.holds_type<dcon::province_id>()) {
				payload.emplace<dcon::province_id>(prov);
				return message_result::consumed;
			} else if(payload.holds_type<dcon::land_battle_id>()) {
				payload.emplace<dcon::land_battle_id>(land_battle);
				return message_result::consumed;
			} else if(payload.holds_type<dcon::naval_battle_id>()) {
				payload.emplace<dcon::naval_battle_id>(naval_battle);
				return message_result::consumed;
			}
			return message_result::unseen;
		}
		mouse_probe impl_probe_mouse(sys::state& state, int32_t x, int32_t y, mouse_probe_type type) noexcept override {
			if(visible && populated)
			return window_element_base::impl_probe_mouse(state, x, y, type);
		return mouse_probe{ nullptr, ui::xy_pair{} };
		}
	};

	class mobilization_progress_bar : public vertical_progress_bar {
		public:
		void on_update(sys::state& state) noexcept override {
			auto prov = retrieve<dcon::province_id>(state, parent);
			int32_t remaining = 0;
			for(const auto p : state.world.nation_get_mobilization_schedule(state.local_player_nation)) {
				if(p.where == prov) {
					remaining++;
				}
			}
			int32_t total = military::mobilized_regiments_possible_from_province(state, prov);
			progress = (total == 0) ? 0 : float(remaining) / float(total);
		}
	};

	class mobilization_units_left : public simple_text_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			auto prov = retrieve<dcon::province_id>(state, parent);
			int32_t remaining = 0;
			for(const auto p : state.world.nation_get_mobilization_schedule(state.local_player_nation)) {
				if(p.where == prov) {
					remaining++;
				}
			}
			set_text(state, text::prettify(remaining));
		}
	};

	class mobilization_counter_window : public window_element_base {
		public:
		bool visible = true;
		bool populated = false;
		dcon::province_id prov;
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "mobilization_progress_bar") {
				return make_element_by_type<mobilization_progress_bar>(state, id);
			} else if(name == "units_left") {
				return make_element_by_type<mobilization_units_left>(state, id);
			} else {
				return nullptr;
			}
		}

		void impl_on_update(sys::state& state) noexcept override {
			on_update(state);
			if(!populated)
			return;

			for(auto& c : children) {
				if(c->is_visible()) {
					c->impl_on_update(state);
				}
			}
		}

		void on_update(sys::state& state) noexcept override {
			populated = false;
			if(state.map_state.active_map_mode == map_mode::mode::recruitment) {
				for(const auto p : state.world.nation_get_mobilization_schedule(state.local_player_nation)) {
					if(p.where == prov) {
						populated = true;
						break;
					}
				}
			}
		}

		void impl_render(sys::state& state, int32_t x, int32_t y) noexcept override {
			if(populated) {
				auto mid_point = state.world.province_get_mid_point(prov);
				auto map_pos = state.map_state.normalize_map_coord(mid_point);
			auto screen_size = glm::vec2{ float(state.x_size / state.user_settings.ui_scale), float(state.y_size / state.user_settings.ui_scale) };
				glm::vec2 screen_pos;
				if(!state.map_state.map_to_screen(state, map_pos, screen_size, screen_pos)) {
					visible = false;
					return;
				}
				if(!state.map_state.visible_provinces[province::to_map_id(prov)]) {
					visible = false;
					return;
				}
				visible = true;
			auto new_position = xy_pair{ int16_t(screen_pos.x), int16_t(screen_pos.y) };
				new_position.x += 7 - base_data.size.x / 2; //114/2 = 57
				new_position.y -= 24 * 3;
				base_data.position = new_position;
				base_data.flags &= ~ui::element_data::orientation_mask; //position upperleft
				window_element_base::impl_render(state, new_position.x, new_position.y);
			}
		}
		message_result get(sys::state& state, Cyto::Any& payload) noexcept override {
			if(payload.holds_type<dcon::province_id>()) {
				payload.emplace<dcon::province_id>(prov);
				return message_result::consumed;
			}
			return message_result::unseen;
		}
		mouse_probe impl_probe_mouse(sys::state& state, int32_t x, int32_t y, mouse_probe_type type) noexcept override {
			if(visible && populated)
			return window_element_base::impl_probe_mouse(state, x, y, type);
		return mouse_probe{ nullptr, ui::xy_pair{} };
		}
	};

	class colonization_counter_progress : public simple_text_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			auto c = retrieve<dcon::colonization_id>(state, parent);
			auto level = state.world.colonization_get_level(c);
			set_text(state, std::to_string(level));
		}
	};
	class colonization_counter_flag : public flag_button {
		public:
		dcon::national_identity_id get_current_nation(sys::state& state) noexcept override {
			auto c = retrieve<dcon::colonization_id>(state, parent);
			return state.world.nation_get_identity_from_identity_holder(state.world.colonization_get_colonizer(c));
		}
		mouse_probe impl_probe_mouse(sys::state& state, int32_t x, int32_t y, mouse_probe_type type) noexcept override {
			if(type == mouse_probe_type::tooltip)
			return flag_button::impl_probe_mouse(state, x, y, type);
		return mouse_probe{ nullptr, ui::xy_pair{} };
		}
	};
	class colonization_counter_window : public window_element_base {
		public:
		bool visible = true;
		bool populated = false;
		dcon::province_id prov;
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "colonization_progress") {
				return make_element_by_type<colonization_counter_progress>(state, id);
			} else if(name == "colonization_country_flag") {
				return make_element_by_type<colonization_counter_flag>(state, id);
			} else {
				return nullptr;
			}
		}

		void impl_on_update(sys::state& state) noexcept override {
			on_update(state);
			if(!populated)
			return;

			for(auto& c : children) {
				if(c->is_visible()) {
					c->impl_on_update(state);
				}
			}
		}

		void on_update(sys::state& state) noexcept override {
			populated = bool(retrieve<dcon::colonization_id>(state, parent));

		}

		void impl_render(sys::state& state, int32_t x, int32_t y) noexcept override {
			if(populated) {
				auto mid_point = state.world.province_get_mid_point(prov);
				auto map_pos = state.map_state.normalize_map_coord(mid_point);
			auto screen_size = glm::vec2{ float(state.x_size / state.user_settings.ui_scale), float(state.y_size / state.user_settings.ui_scale) };
				glm::vec2 screen_pos;
				if(!state.map_state.map_to_screen(state, map_pos, screen_size, screen_pos)) {
					visible = false;
					return;
				}
				if(!state.map_state.visible_provinces[province::to_map_id(prov)]) {
					visible = false;
					return;
				}
				visible = true;
			auto new_position = xy_pair{ int16_t(screen_pos.x), int16_t(screen_pos.y) };
				new_position.x += 7 - base_data.size.x / 2; //114/2 = 57
				new_position.y -= 24 * 3;
				base_data.position = new_position;
				base_data.flags &= ~ui::element_data::orientation_mask; //position upperleft
				window_element_base::impl_render(state, new_position.x, new_position.y);
			}
		}
		message_result get(sys::state& state, Cyto::Any& payload) noexcept override {
			if(payload.holds_type<dcon::province_id>()) {
				payload.emplace<dcon::province_id>(prov);
				return message_result::consumed;
			} else if(payload.holds_type<dcon::colonization_id>()) {
				auto const sdef = state.world.province_get_state_from_abstract_state_membership(prov);
				auto cr = state.world.state_definition_get_colonization(sdef);
				if(cr.begin() != cr.end()) {
					payload.emplace<dcon::colonization_id>(*cr.begin());
				}
				return message_result::consumed;
			}
			return message_result::unseen;
		}
		mouse_probe impl_probe_mouse(sys::state& state, int32_t x, int32_t y, mouse_probe_type type) noexcept override {
			if(visible && populated)
			return window_element_base::impl_probe_mouse(state, x, y, type);
		return mouse_probe{ nullptr, ui::xy_pair{} };
		}
	};

	class map_pv_rail_dots : public image_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			auto p = retrieve<dcon::province_id>(state, parent);
			frame = 6 - state.world.province_get_building_level(p, economy::province_building_type::railroad);
		}
	};
	class map_pv_fort_dots : public image_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			auto p = retrieve<dcon::province_id>(state, parent);
			frame = 6 - state.world.province_get_building_level(p, economy::province_building_type::fort);
		}
	};

	class map_pv_bank : public image_element_base {
		public:
		sys::date last_update;
		char cached_level = 0;

		void render(sys::state& state, int32_t x, int32_t y) noexcept override {
			if(last_update != state.ui_date) {
				cached_level = '0' + state.world.province_get_building_level(retrieve<dcon::province_id>(state, parent), economy::province_building_type::bank);
				last_update = state.ui_date;
			}
			image_element_base::render(state, x, y);
		ogl::color3f color{ 0.f, 0.f, 0.f };
			//ogl::render_text(state, &cached_level, 1, ogl::color_modification::none, float(x + 16 + 1.0f), float(y + 1.0f), color, 1);
		}
	};

	class map_pv_university : public image_element_base {
		public:
		sys::date last_update;
		char cached_level = 0;

		void render(sys::state& state, int32_t x, int32_t y) noexcept override {
			if(last_update != state.ui_date) {
				cached_level = '0' + state.world.province_get_building_level(retrieve<dcon::province_id>(state, parent), economy::province_building_type::university);
				last_update = state.ui_date;
			}
			image_element_base::render(state, x, y);
		ogl::color3f color{ 0.f, 0.f, 0.f };
			//ogl::render_text(state, &cached_level, 1, ogl::color_modification::none, float(x + 16 + 1.0f), float(y + 1.0f), color, 1);
		}
	};

	class province_details_container : public window_element_base {
		public:
		dcon::province_id prov;
		sys::date last_update;
		bool visible = false;
	
		element_base* capital_icon = nullptr;
		element_base* rails_icon = nullptr;
		element_base* rails_dots = nullptr;
		element_base* fort_icon = nullptr;
		element_base* fort_dots = nullptr;
		element_base* bank_icon = nullptr;
		element_base* unv_icon = nullptr;

		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "capital_icon") {
				auto ptr = make_element_by_type<image_element_base>(state, id);
				capital_icon = ptr.get();
				return ptr;
			} else if(name == "rail_icon") {
				auto ptr = make_element_by_type<image_element_base>(state, id);
				rails_icon = ptr.get();
				return ptr;
			} else if(name == "rail_dots") {
				auto ptr = make_element_by_type<map_pv_rail_dots>(state, id);
				rails_dots = ptr.get();
				return ptr;
			} else if(name == "fort_icon") {
				auto ptr = make_element_by_type<image_element_base>(state, id);
				fort_icon = ptr.get();
				return ptr;
			} else if(name == "fort_dots") {
				auto ptr = make_element_by_type<map_pv_fort_dots>(state, id);
				fort_dots = ptr.get();
				return ptr;
			} else if(name == "bank_icon") {
				auto ptr = make_element_by_type<map_pv_bank>(state, id);
				bank_icon = ptr.get();
				return ptr;
			} else if(name == "university_icon") {
				auto ptr = make_element_by_type<map_pv_university>(state, id);
				unv_icon = ptr.get();
				return ptr;
			} else {
				return nullptr;
			}
		}

		void impl_on_update(sys::state& state) noexcept override {
			if(!visible)
			return;
			if(last_update && state.ui_date == last_update)
			return;

			last_update = state.ui_date;
			on_update(state);

			for(auto& c : children) {
				if(c->is_visible()) {
					c->impl_on_update(state);
				}
			}
		}

		void on_update(sys::state& state) noexcept override {
			int32_t rows = 0;
			if(state.world.nation_get_capital(state.world.province_get_nation_from_province_ownership(prov)) == prov) {
				capital_icon->set_visible(state, true);
				++rows;
			} else {
				capital_icon->set_visible(state, false);
			}
			if(state.world.province_get_building_level(prov, economy::province_building_type::railroad) != 0) {
				++rows;
				rails_icon->set_visible(state, true);
				rails_dots->set_visible(state, true);
			} else {
				rails_icon->set_visible(state, false);
				rails_dots->set_visible(state, false);
			}
			if(state.world.province_get_building_level(prov, economy::province_building_type::fort) != 0) {
				++rows;
				fort_icon->set_visible(state, true);
				fort_dots->set_visible(state, true);
			} else {
				fort_icon->set_visible(state, false);
				fort_dots->set_visible(state, false);
			}
			if((state.economy_definitions.building_definitions[uint32_t(economy::province_building_type::university)].defined && state.world.province_get_building_level(prov, economy::province_building_type::university) != 0)
			|| (state.economy_definitions.building_definitions[uint32_t(economy::province_building_type::bank)].defined && state.world.province_get_building_level(prov, economy::province_building_type::bank) != 0)) {
				++rows;
			} else {
				bank_icon->set_visible(state, false);
				unv_icon->set_visible(state, false);
			}

			auto top = (-16 * rows) / 2;
			if(state.world.nation_get_capital(state.world.province_get_nation_from_province_ownership(prov)) == prov) {
				capital_icon->base_data.position.y = int16_t(top - 2);
				capital_icon->base_data.position.x = int16_t(-10);
				top += 16;
			}
			if(state.world.province_get_building_level(prov, economy::province_building_type::railroad) != 0) {
				rails_icon->base_data.position.y = int16_t(top );
				rails_dots->base_data.position.y = int16_t(top);
				int32_t total_width = 18 + 2 + 3 + 4 * state.world.province_get_building_level(prov, economy::province_building_type::railroad);
				rails_icon->base_data.position.x = int16_t(-total_width / 2);
				rails_dots->base_data.position.x = int16_t(20 -total_width / 2);
				top += 16;
			}
			if(state.world.province_get_building_level(prov, economy::province_building_type::fort) != 0) {
				fort_icon->base_data.position.y = int16_t(top);
				fort_dots->base_data.position.y = int16_t(top);
				int32_t total_width = 18 + 2 + 3 + 4 * state.world.province_get_building_level(prov, economy::province_building_type::fort);
				fort_icon->base_data.position.x = int16_t(-total_width / 2);
				fort_dots->base_data.position.x = int16_t(20 - total_width / 2);
				top += 16;
			}
			if((state.economy_definitions.building_definitions[uint32_t(economy::province_building_type::university)].defined && state.world.province_get_building_level(prov, economy::province_building_type::university) != 0)
			|| (state.economy_definitions.building_definitions[uint32_t(economy::province_building_type::bank)].defined && state.world.province_get_building_level(prov, economy::province_building_type::bank) != 0)) {


				if((state.economy_definitions.building_definitions[uint32_t(economy::province_building_type::university)].defined && state.world.province_get_building_level(prov, economy::province_building_type::university) != 0)
				&& (state.economy_definitions.building_definitions[uint32_t(economy::province_building_type::bank)].defined && state.world.province_get_building_level(prov, economy::province_building_type::bank) != 0)) {

					unv_icon->base_data.position.y = int16_t(top);
					unv_icon->base_data.position.x = int16_t(0);
					bank_icon->base_data.position.y = int16_t(top);
					bank_icon->base_data.position.x = int16_t(-32);
					bank_icon->set_visible(state, true);
					unv_icon->set_visible(state, true);
				} else if(state.economy_definitions.building_definitions[uint32_t(economy::province_building_type::university)].defined && state.world.province_get_building_level(prov, economy::province_building_type::university) != 0) {

					unv_icon->base_data.position.y = int16_t(top);
					unv_icon->base_data.position.x = int16_t(-16);
					bank_icon->set_visible(state, false);
					unv_icon->set_visible(state, true);
				} else if(state.economy_definitions.building_definitions[uint32_t(economy::province_building_type::bank)].defined && state.world.province_get_building_level(prov, economy::province_building_type::bank) != 0) {

					bank_icon->base_data.position.y = int16_t(top);
					bank_icon->base_data.position.x = int16_t(-16);
					bank_icon->set_visible(state, true);
					unv_icon->set_visible(state, false);
				}
			}
		}

		void impl_render(sys::state& state, int32_t x, int32_t y) noexcept override {
			auto mid_point = state.world.province_get_mid_point(prov);
			auto map_pos = state.map_state.normalize_map_coord(mid_point);
			auto screen_size = glm::vec2{ float(state.x_size / state.user_settings.ui_scale), float(state.y_size / state.user_settings.ui_scale) };
			glm::vec2 screen_pos;

			if(!state.map_state.map_to_screen(state, map_pos, screen_size, screen_pos)) {
				visible = false;
				return;
			}

			if(screen_pos.x < -32 || screen_pos.y < -32 || screen_pos.x > state.ui_state.root->base_data.size.x + 32 || screen_pos.y > state.ui_state.root->base_data.size.y + 32) {
				visible = false;
				return;
			}
			if(visible == false) {
				visible = true;
				impl_on_update(state);
			}

			auto new_position = xy_pair{ int16_t(screen_pos.x), int16_t(screen_pos.y) };
			base_data.position = new_position;
			window_element_base::impl_render(state, new_position.x, new_position.y);
		}

		message_result get(sys::state& state, Cyto::Any& payload) noexcept override {
			if(payload.holds_type<dcon::province_id>()) {
				payload.emplace<dcon::province_id>(prov);
				return message_result::consumed;
			}
			return message_result::unseen;
		}

		mouse_probe impl_probe_mouse(sys::state& state, int32_t x, int32_t y, mouse_probe_type type) noexcept override {
			return mouse_probe{ nullptr, ui::xy_pair{} };
		}
	};

	class rgo_icon : public image_element_base {
		public:
	dcon::province_id content{};
		void impl_render(sys::state& state, int32_t x, int32_t y) noexcept override {
			auto mid_point = state.world.province_get_mid_point(content);
			auto map_pos = state.map_state.normalize_map_coord(mid_point);
		auto screen_size = glm::vec2{float(state.x_size / state.user_settings.ui_scale), float(state.y_size / state.user_settings.ui_scale)};
			glm::vec2 screen_pos;
			if(!state.map_state.map_to_screen(state, map_pos, screen_size, screen_pos))
			return;
		auto new_position = xy_pair{int16_t(screen_pos.x - base_data.size.x / 2), int16_t(screen_pos.y - base_data.size.y / 2)};
			image_element_base::base_data.position = new_position;
			image_element_base::impl_render(state, new_position.x, new_position.y);
		}
		void on_update(sys::state& state) noexcept override {
			auto cid = state.world.province_get_rgo(content).id;
			frame = int32_t(state.world.commodity_get_icon(cid));
		}
	};
} // namespace ui
