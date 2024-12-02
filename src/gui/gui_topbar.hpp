#pragma once

#include "demographics.hpp"
#include "gui_element_types.hpp"
#include "gui_production_window.hpp"
#include "gui_diplomacy_window.hpp"
#include "gui_technology_window.hpp"
#include "gui_politics_window.hpp"
#include "gui_budget_window.hpp"
#include "gui_trade_window.hpp"
#include "gui_population_window.hpp"
#include "gui_military_window.hpp"
#include "gui_chat_window.hpp"
#include "gui_common_elements.hpp"
#include "gui_diplomacy_request_topbar.hpp"
#include "nations.hpp"
#include "politics.hpp"
#include "rebels.hpp"
#include "system_state.hpp"
#include "text.hpp"
#include "gui_event.hpp"
#include "pdqsort.h"

namespace ui {

	class topbar_nation_name : public generic_name_text<dcon::nation_id> {
		public:
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			if(state.cheat_data.show_province_id_tooltip) {
				auto box = text::open_layout_box(contents);
				text::add_to_layout_box(state, contents, box, std::string_view("Nation ID:"));
				text::add_space_to_layout_box(state, contents, box);
				text::add_to_layout_box(state, contents, box, std::to_string(state.local_player_nation.value));
				text::close_layout_box(contents, box);
			}
		}
	};

	class topbar_nation_prestige_text : public nation_prestige_text {
	public:
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class topbar_nation_industry_score_text : public nation_industry_score_text {
	public:
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class topbar_nation_military_score_text : public nation_military_score_text {
	public:
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class topbar_nation_colonial_power_text : public nation_colonial_power_text {
	public:
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class topbar_budget_line_graph : public line_graph {
	public:
		topbar_budget_line_graph() : line_graph(32) { }
		void on_create(sys::state& state) noexcept override {
			line_graph::on_create(state);
			on_update(state);
		}
		void on_update(sys::state& state) noexcept override {
			std::vector<float> datapoints(size_t(32));
			for(size_t i = 0; i < state.player_data_cache.treasury_record.size(); ++i) {
				datapoints[i] = state.player_data_cache.treasury_record[(state.ui_date.value + 1 + i) % 32] - state.player_data_cache.treasury_record[(state.ui_date.value + 0 + i) % 32];
			}
			datapoints[datapoints.size() - 1] = state.player_data_cache.treasury_record[(state.ui_date.value + 1 + 31) % 32] - state.player_data_cache.treasury_record[(state.ui_date.value + 0 + 31) % 32];
			datapoints[0] = datapoints[1]; // otherwise you will store the difference between two non-consecutive days here
			set_data_points(state, datapoints);
		}
	};

	class topbar_nation_literacy_text : public expanded_hitbox_text {
	public:
		void on_create(sys::state& state) noexcept override {
			top_left_extension = ui::xy_pair{ 25, 3 };
			bottom_right_extension = ui::xy_pair{ -10, 2 };
			expanded_hitbox_text::on_create(state);
		}

		void on_update(sys::state& state) noexcept override {
			auto n = retrieve<dcon::nation_id>(state, parent);
			auto literacy = state.world.nation_get_demographics(n, demographics::literacy);
			auto total_pop = std::max(1.0f, state.world.nation_get_demographics(n, demographics::total));
			set_text(state, "?Y" + text::format_percentage(literacy / total_pop, 1));
		}

		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class topbar_nation_infamy_text : public expanded_hitbox_text {
	public:
		void on_create(sys::state& state) noexcept override {
			top_left_extension = ui::xy_pair{ 25, 3 };
			bottom_right_extension = ui::xy_pair{ 0, 2 };
			expanded_hitbox_text::on_create(state);
		}
		void on_update(sys::state& state) noexcept override {
			set_text(state, "?Y" + text::format_float(state.world.nation_get_infamy(retrieve<dcon::nation_id>(state, parent)), 2));
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class topbar_nation_population_text : public multiline_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class topbar_treasury_text : public multiline_text_element_base {
		std::array<float, size_t(budget_slider_target::target_count)> values;
		std::array<float, size_t(budget_slider_target::target_count)> multipliers;
	public:
		void on_update(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class topbar_nation_focus_allocation_text : public expanded_hitbox_text {
	public:
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void on_create(sys::state& state) noexcept override {
			top_left_extension = ui::xy_pair{ 25, 3 };
			bottom_right_extension = ui::xy_pair{ 0, 2 };
			expanded_hitbox_text::on_create(state);
		}

		void on_update(sys::state& state) noexcept override {
			auto nation_id = retrieve<dcon::nation_id>(state, parent);
			auto available = nations::max_national_focuses(state, nation_id);
			auto in_use = nations::national_focuses_in_use(state, nation_id);
			if(in_use < available) {
				set_text(state, "?R" + text::format_ratio(in_use, available));
			} else {
				set_text(state, "?Y" + text::format_ratio(in_use, available));
			}
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class topbar_nation_militancy_text : public expanded_hitbox_text {
	public:
		void on_create(sys::state& state) noexcept override {
			top_left_extension = ui::xy_pair{ 25, 3 };
			bottom_right_extension = ui::xy_pair{ 0, 2 };
			expanded_hitbox_text::on_create(state);
		}
		void on_update(sys::state& state) noexcept override {
			auto nation_id = retrieve<dcon::nation_id>(state, parent);
			auto militancy = state.world.nation_get_demographics(nation_id, demographics::militancy);
			auto total_pop = state.world.nation_get_demographics(nation_id, demographics::total);
			set_text(state, "?Y" + text::format_float(total_pop == 0.f ? 0.f : militancy / total_pop));
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class topbar_nation_consciousness_text : public expanded_hitbox_text {
	public:
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void on_create(sys::state& state) noexcept override {
			top_left_extension = ui::xy_pair{ 25, 3 };
			bottom_right_extension = ui::xy_pair{ 0, 2 };
			expanded_hitbox_text::on_create(state);
		}

		void on_update(sys::state& state) noexcept override {
			auto nation_id = retrieve<dcon::nation_id>(state, parent);
			auto militancy = state.world.nation_get_demographics(nation_id, demographics::consciousness);
			auto total_pop = state.world.nation_get_demographics(nation_id, demographics::total);
			set_text(state, "?Y" + text::format_float(total_pop == 0.f ? 0.f : militancy / total_pop));
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class topbar_nation_diplomatic_points_text : public expanded_hitbox_text {
	public:
		void on_create(sys::state& state) noexcept override {
		top_left_extension = ui::xy_pair{ 25, 3 };
		bottom_right_extension = ui::xy_pair{ 0, 2 };
			expanded_hitbox_text::on_create(state);
		}

		void on_update(sys::state& state) noexcept override {
			auto points = nations::diplomatic_points(state, retrieve<dcon::nation_id>(state, parent));
			set_text(state, "?Y" + text::format_float(points, 1));
		}

		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class topbar_nation_brigade_allocation_text : public nation_brigade_allocation_text {
	public:
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			auto nation_id = retrieve<dcon::nation_id>(state, parent);
			auto box = text::open_layout_box(contents, 0);
			text::substitution_map sub;
			text::add_to_substitution_map(sub, text::variable_type::curr, state.world.nation_get_active_regiments(nation_id));
			text::add_to_substitution_map(sub, text::variable_type::max, state.world.nation_get_recruitable_regiments(nation_id));
			text::localised_format_box(state, contents, box, std::string_view("topbar_army_tooltip"), sub);
			text::close_layout_box(contents, box);
		}
	};

	class topbar_nation_navy_allocation_text : public nation_navy_allocation_text {
	public:
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class topbar_nation_mobilization_size_text : public nation_mobilization_size_text {
		public:
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class topbar_nation_leadership_points_text : public nation_leadership_points_text {
	private:
		float getResearchPointsFromPop(sys::state& state, dcon::pop_type_id pop, dcon::nation_id n) {
			/*
			Now imagine that Rock Hudson is standing at the top of the water slide hurling Nintendo consoles down the water slide.
			If it weren't for the ladders, which allow the water to pass through but not the Nintendo consoles,
			the Nintendo consoles could hit someone in the wave pool on the head, in which case the water park could get sued.
			*/
			// auto sum =  (fat_pop.get_research_points() * ((state.world.nation_get_demographics(n, demographics::to_key(state,
			// fat_pop)) / state.world.nation_get_demographics(n, demographics::total)) / fat_pop.get_research_optimum() ));
			auto sum = ((state.world.nation_get_demographics(n, demographics::to_key(state, pop)) /
			state.world.nation_get_demographics(n, demographics::total)) /
			state.world.pop_type_get_research_optimum(state.culture_definitions.officers));
			return sum;
		}

	public:
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class background_image : public opaque_element_base {
	public:
		void render(sys::state& state, int32_t x, int32_t y) noexcept override {
			base_data.size.x = int16_t(ui_width(state));
			base_data.size.y = int16_t(ui_height(state));
			opaque_element_base::render(state, x, y);
			//Put it far away!
			if(base_data.position.x >= 256 || base_data.position.y >= 256) {
				base_data.position.x = int16_t(8192 * 2);
				base_data.position.y = int16_t(8192 * 2);
			}
		}
	};
	class topbar_budget_tab_button : public checkbox_button {
		public:
		void button_action(sys::state& state) noexcept override {
			if(state.ui_state.budget_subwindow && state.ui_state.budget_subwindow->is_visible()) {
				state.ui_state.budget_subwindow->set_visible(state, false);
				return;
			}
			state.open_budget();
		}
		bool is_active(sys::state& state) noexcept override {
			return state.ui_state.topbar_subwindow
				&& state.ui_state.topbar_subwindow == state.ui_state.budget_subwindow
				&& state.ui_state.topbar_subwindow->is_visible();
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			text::add_line(state, contents, "alice_topbar_tab_1");
		}
		sound::audio_instance& get_click_sound(sys::state& state) noexcept override {
			return sound::get_tab_budget_sound(state);
		}
	};
	class topbar_trade_tab_button : public checkbox_button {
		public:
		void button_action(sys::state& state) noexcept override {
			if(state.ui_state.trade_subwindow && state.ui_state.trade_subwindow->is_visible()) {
				state.ui_state.trade_subwindow->set_visible(state, false);
				return;
			}
			state.open_trade();
		}
		bool is_active(sys::state& state) noexcept override {
			return state.ui_state.topbar_subwindow
				&& state.ui_state.topbar_subwindow == state.ui_state.trade_subwindow
				&& state.ui_state.topbar_subwindow->is_visible();
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			text::add_line(state, contents, "alice_topbar_tab_1");
		}
		sound::audio_instance& get_click_sound(sys::state& state) noexcept override {
			return sound::get_tab_budget_sound(state); // for now we reuse the budget tab sound
		}
	};
	class topbar_politics_tab_button : public checkbox_button {
		public:
		void button_action(sys::state& state) noexcept override {
			if(state.ui_state.politics_subwindow && state.ui_state.politics_subwindow->is_visible()) {
				state.ui_state.politics_subwindow->set_visible(state, false);
				return;
			}
			state.open_politics();
		}
		bool is_active(sys::state& state) noexcept override {
			return state.ui_state.topbar_subwindow
				&& state.ui_state.topbar_subwindow == state.ui_state.politics_subwindow
				&& state.ui_state.topbar_subwindow->is_visible();
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			text::add_line(state, contents, "alice_topbar_tab_1");
		}
		sound::audio_instance& get_click_sound(sys::state& state) noexcept override {
			return sound::get_tab_politics_sound(state);
		}
	};
	class topbar_diplomacy_tab_button : public checkbox_button {
	public:
		void button_action(sys::state& state) noexcept override {
			if(state.ui_state.diplomacy_subwindow && state.ui_state.diplomacy_subwindow->is_visible()) {
				state.ui_state.diplomacy_subwindow->set_visible(state, false);
				return;
			}
			state.open_diplomacy(dcon::nation_id{ });
		}
		bool is_active(sys::state& state) noexcept override {
			return state.ui_state.topbar_subwindow
				&& state.ui_state.topbar_subwindow == state.ui_state.diplomacy_subwindow
				&& state.ui_state.topbar_subwindow->is_visible();
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			text::add_line(state, contents, "alice_topbar_tab_1");
		}
		sound::audio_instance& get_click_sound(sys::state& state) noexcept override {
			return sound::get_tab_diplomacy_sound(state);
		}
	};
	class topbar_military_tab_button : public checkbox_button {
	public:
		void button_action(sys::state& state) noexcept override {
			if(state.ui_state.military_subwindow && state.ui_state.military_subwindow->is_visible()) {
				state.ui_state.military_subwindow->set_visible(state, false);
				return;
			}
			state.open_military();
		}
		bool is_active(sys::state& state) noexcept override {
			return state.ui_state.topbar_subwindow
				&& state.ui_state.topbar_subwindow == state.ui_state.military_subwindow
				&& state.ui_state.topbar_subwindow->is_visible();
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			text::add_line(state, contents, "alice_topbar_tab_1");
		}
		sound::audio_instance& get_click_sound(sys::state& state) noexcept override {
			return sound::get_tab_military_sound(state);
		}
	};
	class topbar_production_tab_button : public checkbox_button {
		public:
		void button_action(sys::state& state) noexcept override {
			if(state.ui_state.production_subwindow && state.ui_state.production_subwindow->is_visible()) {
				state.ui_state.production_subwindow->set_visible(state, false);
				return;
			}
			state.open_production();
		}
		bool is_active(sys::state& state) noexcept override {
			return state.ui_state.topbar_subwindow
				&& state.ui_state.topbar_subwindow == state.ui_state.production_subwindow
				&& state.ui_state.topbar_subwindow->is_visible();
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			text::add_line(state, contents, "alice_topbar_tab_1");
		}
		sound::audio_instance& get_click_sound(sys::state& state) noexcept override {
			return sound::get_tab_production_sound(state);
		}
	};
	class topbar_technology_tab_button : public checkbox_button {
		public:
		void button_action(sys::state& state) noexcept override {
			if(state.ui_state.technology_subwindow && state.ui_state.technology_subwindow->is_visible()) {
				state.ui_state.technology_subwindow->set_visible(state, false);
				return;
			}
			state.open_technology();
		}
		bool is_active(sys::state& state) noexcept override {
			return state.ui_state.topbar_subwindow
				&& state.ui_state.topbar_subwindow == state.ui_state.technology_subwindow
				&& state.ui_state.topbar_subwindow->is_visible();
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			text::add_line(state, contents, "alice_topbar_tab_1");
		}
		sound::audio_instance& get_click_sound(sys::state& state) noexcept override {
			return sound::get_tab_technology_sound(state);
		}
	};

	class topbar_population_view_button : public checkbox_button {
	public:
		sound::audio_instance& get_click_sound(sys::state& state) noexcept override {
			return sound::get_tab_population_sound(state);
		}
		void button_action(sys::state& state) noexcept override {
			if(state.ui_state.population_subwindow && state.ui_state.population_subwindow->is_visible()) {
				state.ui_state.population_subwindow->set_visible(state, false);
				return;
			}
			state.open_population();
		}
		bool is_active(sys::state& state) noexcept override {
			return state.ui_state.topbar_subwindow
				&& state.ui_state.topbar_subwindow == state.ui_state.population_subwindow
				&& state.ui_state.topbar_subwindow->is_visible();
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			text::add_line(state, contents, "alice_topbar_tab_2");
		}
	};

	class topbar_date_text : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			set_text(state, text::date_to_string(state, state.current_date));
		}
	};

	class topbar_pause_button : public button_element_base {
		public:
		void on_create(sys::state& state) noexcept override {
			button_element_base::on_create(state);
			if(state.network_mode == sys::network_mode_type::single_player) {
				base_data.data.button.shortcut = sys::virtual_key::SPACE;
			}
		}
		sound::audio_instance& get_click_sound(sys::state& state) noexcept override {
			if(state.actual_game_speed <= 0) {
				return sound::get_unpause_sound(state);
			}
			return sound::get_pause_sound(state);
		}
		void button_action(sys::state& state) noexcept override {
			if(state.network_mode == sys::network_mode_type::single_player) {
				if(state.actual_game_speed <= 0) {
					state.actual_game_speed = state.ui_state.held_game_speed;
				} else {
					state.ui_state.held_game_speed = state.actual_game_speed.load();
					state.actual_game_speed = 0;
				}
			} else {
				command::notify_pause_game(state, state.local_player_nation);
			}
		}
		void on_update(sys::state& state) noexcept override {
			disabled = state.internally_paused || state.ui_pause.load(std::memory_order::acquire);
			if(state.network_mode == sys::network_mode_type::single_player) {
				disabled = disabled || ui::events_pause_test(state);
			}
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class topbar_speedup_button : public button_element_base {
		public:
		void on_create(sys::state& state) noexcept override {
			button_element_base::on_create(state);
			base_data.data.button.shortcut = sys::virtual_key::ADD;
			disabled = state.network_mode == sys::network_mode_type::client;
		}

		void button_action(sys::state& state) noexcept override {
			if(state.actual_game_speed > 0) {
				state.actual_game_speed = std::min(5, state.actual_game_speed.load() + 1);
			} else {
				state.ui_state.held_game_speed = std::min(5, state.ui_state.held_game_speed + 1);
			}
		}

		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			auto box = text::open_layout_box(contents, 0);
			text::localised_format_box(state, contents, box, std::string_view("topbar_inc_speed"));
			text::close_layout_box(contents, box);
		}
	};

	class topbar_speeddown_button : public button_element_base {
		public:
		void on_create(sys::state& state) noexcept override {
			button_element_base::on_create(state);
			base_data.data.button.shortcut = sys::virtual_key::SUBTRACT;
			disabled = state.network_mode == sys::network_mode_type::client;
		}

		void button_action(sys::state& state) noexcept override {
			if(state.actual_game_speed > 0) {
				state.actual_game_speed = std::max(1, state.actual_game_speed.load() - 1);
			} else {
				state.ui_state.held_game_speed = std::max(1, state.ui_state.held_game_speed - 1);
			}
		}

		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			auto box = text::open_layout_box(contents, 0);
			text::localised_format_box(state, contents, box, std::string_view("topbar_dec_speed"));
			text::close_layout_box(contents, box);
		}
	};

	class topbar_speed_indicator : public topbar_pause_button {
		public:
		void render(sys::state& state, int32_t x, int32_t y) noexcept override {
			if(state.network_mode == sys::network_mode_type::single_player || state.network_mode == sys::network_mode_type::host) {
				if(state.internally_paused || state.ui_pause.load(std::memory_order::acquire)) {
					frame = 0;
				} else {
					frame = state.actual_game_speed;
				}
			} else {
				frame = state.actual_game_speed;
			}
			topbar_pause_button::render(state, x, y);
		}
	};

	class topbar_losing_gp_status_icon : public standard_nation_icon {
		public:
		int32_t get_icon_frame(sys::state& state, dcon::nation_id nation_id) noexcept override {
			return int32_t(!(nations::is_great_power(state, nation_id) &&
										 state.world.nation_get_rank(nation_id) > uint16_t(state.defines.great_nations_count)));
		}

		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			if(!nations::is_great_power(state, state.local_player_nation)) {
				text::add_line(state, contents, "countryalert_no_gpstatus");
			} else if(state.world.nation_get_rank(state.local_player_nation) > uint16_t(state.defines.great_nations_count)) {
				text::add_line(state, contents, "we_are_losing_gp_status");
				auto box = text::open_layout_box(contents);
			text::substitution_map sub{};
				text::add_to_substitution_map(sub, text::variable_type::x, int32_t(state.defines.great_nations_count));
				for(const auto gp : state.great_nations) {
					if(gp.nation == state.local_player_nation) {
						text::add_to_substitution_map(sub, text::variable_type::date, gp.last_greatness + int32_t(state.defines.greatness_days));
						break;
					}
				}
				text::localised_format_box(state, contents, box, "alice_gp_status_regain_expiration", sub);
				text::close_layout_box(contents, box);
			} else if(state.world.nation_get_rank(state.local_player_nation) <= uint16_t(state.defines.great_nations_count)) {
				text::add_line(state, contents, "countryalert_no_loosinggpstatus");
			}
		}
	};

	class topbar_at_peace_text : public standard_nation_text {
		public:
		std::string get_text(sys::state& state, dcon::nation_id nation_id) noexcept override {
			return text::produce_simple_string(state, "atpeace");
		}
	};

	class topbar_building_factories_icon : public standard_nation_icon {
		public:
		int32_t get_icon_frame(sys::state& state, dcon::nation_id nation_id) noexcept override {
			return int32_t(!economy_factory::nation_is_constructing_factories(state, nation_id));
		}

		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			if(parent) {
			Cyto::Any payload = dcon::nation_id{};
				parent->impl_get(state, payload);
				auto nation_id = any_cast<dcon::nation_id>(payload);

				auto box = text::open_layout_box(contents, 0);
				if(!economy_factory::nation_is_constructing_factories(state, nation_id)) {
				text::localised_format_box(state, contents, box, "countryalert_no_isbuildingfactories", text::substitution_map{});
				} else if(economy_factory::nation_is_constructing_factories(state, nation_id)) {
				text::localised_format_box(state, contents, box, "countryalert_isbuildingfactories", text::substitution_map{});
					auto nation_fat_id = dcon::fatten(state.world, nation_id);
					nation_fat_id.for_each_state_building_construction([&](dcon::state_building_construction_id building_slim_id) {
						auto building_fat_id = dcon::fatten(state.world, building_slim_id);
						auto stateName = building_fat_id.get_state().get_definition().get_name();
						auto factoryType = building_fat_id.get_type().get_name();

						text::add_line_break_to_layout_box(state, contents, box);
						text::add_to_layout_box(state, contents, box, stateName);
						text::add_space_to_layout_box(state, contents, box);
						text::add_to_layout_box(state, contents, box, factoryType);
					});
				}
				text::close_layout_box(contents, box);
			}
		}
	};

	class topbar_closed_factories_icon : public standard_nation_icon {
	public:
		int32_t get_icon_frame(sys::state& state, dcon::nation_id nation_id) noexcept override {
			return int32_t(!economy_factory::nation_has_closed_factories(state, nation_id));
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class topbar_budget_warning : public image_element_base {
	public:
		bool visible = false;
		void on_update(sys::state& state) noexcept override {
			visible = (state.world.nation_get_spending_level(state.local_player_nation) < 1.0f);
		}
		void render(sys::state& state, int32_t x, int32_t y) noexcept override {
			if(visible)
				image_element_base::render(state, x, y);
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			if(visible) {
				text::add_line(state, contents, "topbar_budget_warning");
			}
		}
	};

	class topbar_unemployment_icon : public image_element_base {
	public:
		void on_update(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class topbar_available_reforms_icon : public standard_nation_button {
	public:
		int32_t get_icon_frame(sys::state& state, dcon::nation_id nation_id) noexcept override {
			return int32_t(!nations::has_reform_available(state, nation_id));
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
		void button_action(sys::state& state) noexcept override {
			if(state.ui_state.politics_subwindow && state.ui_state.politics_subwindow->is_visible()) {
				state.ui_state.politics_subwindow->set_visible(state, false);
				return;
			}
			state.open_politics();

			Cyto::Any defs = Cyto::make_any<politics_window_tab>(politics_window_tab::reforms);
			state.ui_state.politics_subwindow->impl_get(state, defs);
		}
	};

	class topbar_available_decisions_icon : public standard_nation_button {
	public:
		int32_t get_icon_frame(sys::state& state, dcon::nation_id nation_id) noexcept override {
			return int32_t(!nations::has_decision_available(state, nation_id));
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
		void button_action(sys::state& state) noexcept override {
			if(state.ui_state.politics_subwindow && state.ui_state.politics_subwindow->is_visible()) {
				state.ui_state.politics_subwindow->set_visible(state, false);
				return;
			}
			state.open_politics();

			Cyto::Any defs = Cyto::make_any<politics_window_tab>(politics_window_tab::decisions);
			state.ui_state.politics_subwindow->impl_get(state, defs);
		}
	};

	class topbar_ongoing_election_icon : public standard_nation_icon {
		public:
		int32_t get_icon_frame(sys::state& state, dcon::nation_id nation_id) noexcept override {
			return int32_t(!politics::is_election_ongoing(state, nation_id));
		}

		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			auto nation_id = retrieve<dcon::nation_id>(state, parent);
			auto box = text::open_layout_box(contents, 0);
			if(politics::has_elections(state, nation_id)) {
				if(!politics::is_election_ongoing(state, nation_id)) {
					text::localised_format_box(state, contents, box, std::string_view("countryalert_no_isinelection"),
						text::substitution_map{});
				} else if(politics::is_election_ongoing(state, nation_id)) {
					text::substitution_map sub;
					text::add_to_substitution_map(sub, text::variable_type::date, dcon::fatten(state.world, nation_id).get_election_ends());
					text::localised_format_box(state, contents, box, std::string_view("countryalert_isinelection"), sub);
				}
			} else {
				text::localised_format_box(state, contents, box, std::string_view("term_for_life"));
			}
			text::close_layout_box(contents, box);
		}
	};

	class topbar_rebels_icon : public button_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			for(auto rf : state.world.nation_get_rebellion_within(state.local_player_nation)) {
				auto org = rf.get_rebels().get_organization();
				if(org >= 0.01f) {
					frame = 0;
					return;
				}
			}
			frame = 1;
		}

		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			bool showed_title = false;

			for(auto rf : state.world.nation_get_rebellion_within(state.local_player_nation)) {
				auto org = rf.get_rebels().get_organization();
				if(org >= 0.01f) {
					if(!showed_title) {
						text::add_line(state, contents, "countryalert_haverebels");
						text::add_line_break_to_layout(state, contents);
						showed_title = true;
					}
					auto rebelname = rebel::rebel_name(state, rf.get_rebels());
					auto rebelsize = rf.get_rebels().get_possible_regiments();

					text::add_line(state, contents, "topbar_faction",
					text::variable_type::name, std::string_view{ rebelname },
					text::variable_type::strength, text::pretty_integer{ rebelsize },
					text::variable_type::org, text::fp_percentage{org});
				}
			}
		}

		void button_action(sys::state& state) noexcept override {
			if(state.ui_state.politics_subwindow && state.ui_state.politics_subwindow->is_visible()) {
				state.ui_state.politics_subwindow->set_visible(state, false);
				return;
			}
			state.open_politics();

			Cyto::Any defs = Cyto::make_any<politics_window_tab>(politics_window_tab::movements);
			state.ui_state.politics_subwindow->impl_get(state, defs);
		}
	};

	class topbar_colony_icon : public standard_nation_button {
		uint32_t index = 0;

		dcon::province_id get_state_def_province(sys::state& state, dcon::state_definition_id sdef) noexcept {
			for(auto const p : state.world.state_definition_get_abstract_state_membership(sdef)) {
				if(!p.get_province().get_nation_from_province_ownership()) {
					return p.get_province();
				}
			}
		return dcon::province_id{};
		}
		public:
		void button_action(sys::state& state) noexcept override {
			std::vector<dcon::province_id> provinces;
			auto nation_id = retrieve<dcon::nation_id>(state, parent);
			auto nation_fat_id = dcon::fatten(state.world, nation_id);
			for(auto si : state.world.nation_get_state_ownership(nation_id)) {
				if(province::can_integrate_colony(state, si.get_state())) {
					provinces.push_back(si.get_state().get_capital());
				}
			}

			state.world.for_each_state_definition([&](dcon::state_definition_id sdef) {
				if(province::can_start_colony(state, nation_id, sdef)) {
					dcon::province_id province;
					for(auto p : state.world.state_definition_get_abstract_state_membership(sdef)) {
						if(!p.get_province().get_nation_from_province_ownership()) {
							province = p.get_province().id;
							break;
						}
					}
					if(province) {
						provinces.push_back(province);
					}
				}
			});

			nation_fat_id.for_each_colonization([&](dcon::colonization_id colony) {
				auto sdef = state.world.colonization_get_state(colony);
				if(state.world.state_definition_get_colonization_stage(sdef) == 3) { //make protectorate
					provinces.push_back(get_state_def_province(state, sdef));
				} else if(province::can_invest_in_colony(state, nation_id, sdef)) { //invest
					provinces.push_back(get_state_def_province(state, sdef));
				} else { //losing rase
					auto lvl = state.world.colonization_get_level(colony);
					for(auto cols : state.world.state_definition_get_colonization(sdef)) {
						if(lvl < cols.get_level()) {
							provinces.push_back(get_state_def_province(state, sdef));
							break;
						}
					}
				}
			});
			if(!provinces.empty()) {
				index++;
				if(index >= uint32_t(provinces.size())) {
					index = 0;
				}
				if(auto prov = provinces[index]; prov && prov.value < state.province_definitions.first_sea_province.value) {
					sound::play_interface_sound(state, sound::get_click_sound(state), state.user_settings.interface_volume * state.user_settings.master_volume);
					state.map_state.set_selected_province(prov);
					game_scene::open_province_window(state, prov);
					if(state.map_state.get_zoom() < map::zoom_very_close)
					state.map_state.zoom = map::zoom_very_close;
					state.map_state.center_map_on_province(state, prov);
				}
			}
		}

		int32_t get_icon_frame(sys::state& state, dcon::nation_id nation_id) noexcept override {
			bool any_integratable = false;
			for(auto si : state.world.nation_get_state_ownership(nation_id)) {
				if(province::can_integrate_colony(state, si.get_state())) {
					any_integratable = true;
					break;
				}
			}
			state.world.for_each_state_definition([&](dcon::state_definition_id sdef) {
				if(province::can_start_colony(state, nation_id, sdef)) {
					any_integratable = true;
				}
			});
			if(nations::can_expand_colony(state, nation_id) || any_integratable) {
				return 0;
			} else if(nations::is_losing_colonial_race(state, nation_id)) {
				return 1;
			} else {
				return 2;
			}
		}
		// TODO - when the player clicks on the colony icon and there are colonies to expand then we want to teleport their camera to the
		// colony's position & open the prov window

		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			auto nation_id = retrieve<dcon::nation_id>(state, parent);
			auto nation_fat_id = dcon::fatten(state.world, nation_id);

			bool is_empty = true;

			for(auto si : state.world.nation_get_state_ownership(nation_id)) {
				if(province::can_integrate_colony(state, si.get_state())) {
					text::add_line(state, contents, "countryalert_colonialgood_state", text::variable_type::region, si.get_state().id);
					is_empty = false;
				}
			}

			state.world.for_each_state_definition([&](dcon::state_definition_id sdef) {
				if(province::can_start_colony(state, nation_id, sdef)) {
					text::add_line(state, contents, "alice_countryalert_colonialgood_start", text::variable_type::region, sdef);
					is_empty = false;
				}
			});

			nation_fat_id.for_each_colonization([&](dcon::colonization_id colony) {
				auto sdef = state.world.colonization_get_state(colony);
				if(state.world.state_definition_get_colonization_stage(sdef) == 3) {
					text::add_line(state, contents, "countryalert_colonialgood_colony", text::variable_type::region, sdef);
					is_empty = false;
				} else if(province::can_invest_in_colony(state, nation_id, sdef)) {
					text::add_line(state, contents, "countryalert_colonialgood_invest", text::variable_type::region, sdef);
					is_empty = false;
				}
				auto lvl = state.world.colonization_get_level(colony);
				for(auto cols : state.world.state_definition_get_colonization(sdef)) {
					if(lvl < cols.get_level()) {
						text::add_line(state, contents, "countryalert_colonialbad_influence", text::variable_type::region, sdef);
						is_empty = false;
					}
				}
			});

			if(is_empty) {
				text::add_line(state, contents, "countryalert_no_colonial");
			}
		}
	};

	class topbar_crisis_icon : public standard_nation_icon {
	public:
		int32_t get_icon_frame(sys::state& state, dcon::nation_id nation_id) noexcept override {
			if(state.current_crisis == sys::crisis_type::none) {
				return 2;
			} else if(state.crisis_temperature > 0.8f) {
				return 1;
			} else {
				return 0;
			}
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class topbar_sphere_icon : public button_element_base {
	public:
		std::vector<dcon::nation_id> visited_nations;
		void button_action(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override {
			auto n = retrieve<dcon::nation_id>(state, parent);
			frame = 1;
			if(nations::sphereing_progress_is_possible(state, n)) {
				frame = 0;
			} else if(rebel::sphere_member_has_ongoing_revolt(state, n)) {
				frame = 2;
			}
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class topbar_nation_daily_research_points_text : public expanded_hitbox_text {
		public:
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void on_create(sys::state& state) noexcept override {
		top_left_extension = ui::xy_pair{ 25, 3 };
		bottom_right_extension = ui::xy_pair{ 0, 2 };
			expanded_hitbox_text::on_create(state);
		}

		void on_update(sys::state& state) noexcept override {
			auto points = nations::daily_research_points(state, retrieve<dcon::nation_id>(state, parent));
			set_text(state, "?Y" + text::format_float(points, 2));
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			auto nation_id = retrieve<dcon::nation_id>(state, parent);
			auto fat = dcon::fatten(state.world, nation_id);

			auto tech_id = nations::current_research(state, nation_id);

			auto total_pop = state.world.nation_get_demographics(nation_id, demographics::total);
			for(auto pt : state.world.in_pop_type) {
				auto rp = state.world.pop_type_get_research_points(pt);
				if(rp > 0 && state.world.nation_get_demographics(nation_id, demographics::to_key(state, pt)) > 0.0f) {
					auto amount = rp * std::min(1.0f, state.world.nation_get_demographics(nation_id, demographics::to_key(state, pt)) / (total_pop * state.world.pop_type_get_research_optimum(pt)));

					text::substitution_map sub1;
					text::add_to_substitution_map(sub1, text::variable_type::poptype, state.world.pop_type_get_name(pt));
				text::add_to_substitution_map(sub1, text::variable_type::value, text::fp_two_places{ amount });
					text::add_to_substitution_map(sub1, text::variable_type::fraction,
						text::fp_two_places{ (state.world.nation_get_demographics(nation_id, demographics::to_key(state, pt)) / total_pop) * 100 });
					text::add_to_substitution_map(sub1, text::variable_type::optimal,
					text::fp_two_places{ (state.world.pop_type_get_research_optimum(pt) * 100) });

					auto box = text::open_layout_box(contents, 0);
					text::localised_format_box(state, contents, box, std::string_view("tech_daily_researchpoints_tooltip"), sub1);
					text::close_layout_box(contents, box);
				}
			}
			active_modifiers_description(state, contents, nation_id, 0, sys::national_mod_offsets::research_points, true);
			text::add_line_break_to_layout(state, contents);

			active_modifiers_description(state, contents, nation_id, 0, sys::national_mod_offsets::research_points_modifier, true);
			text::add_line_break_to_layout(state, contents);

			if(!bool(tech_id)) {
				auto box2 = text::open_layout_box(contents, 0);
				text::localised_single_sub_box(state, contents, box2, std::string_view("rp_accumulated"), text::variable_type::val,
					text::fp_one_place{fat.get_research_points()});

				text::close_layout_box(contents, box2);
			}
		}
	};

	class topbar_commodity_xport_icon : public image_element_base {
	public:
		uint8_t slot = 0;
		dcon::commodity_id commodity_id{};
		float amount = 0.f;

		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			auto box = text::open_layout_box(contents, 0);
			text::add_to_layout_box(state, contents, box,
			text::produce_simple_string(state, state.world.commodity_get_name(commodity_id)), text::text_color::white);
			text::add_to_layout_box(state, contents, box, std::string_view(":"), text::text_color::white);
			text::add_space_to_layout_box(state, contents, box);
			text::add_to_layout_box(state, contents, box, text::format_float(amount, 2), text::text_color::white);
			text::close_layout_box(contents, box);
		}
	};

	class news_page_date : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			set_text(state, text::date_to_string(state, state.news_definitions.last_print));
		}
	};
	template<size_t size, size_t index>
	class news_article_picture : public image_element_base {
		dcon::gfx_object_id no_news_image;
	public:
		bool get_horizontal_flip(sys::state& state) noexcept override {
			return false; //never flip
		}
		void on_create(sys::state& state) noexcept override {
			image_element_base::on_create(state);
		}
		void on_update(sys::state& state) noexcept override {
			base_data.data.image.gfx_object = no_news_image;
			if constexpr(size == news::news_size_huge) {
				base_data.data.image.gfx_object = state.news_definitions.large_articles[index].picture;
			} else if constexpr(size == news::news_size_medium) {
				base_data.data.image.gfx_object = state.news_definitions.medium_articles[index].picture;
			} else if constexpr(size == news::news_size_small) {
				base_data.data.image.gfx_object = state.news_definitions.small_articles[index].picture;
			}
		}
		void render(sys::state& state, int32_t x, int32_t y) noexcept override {
			if(auto gid = base_data.data.image.gfx_object; gid) {
				auto& gfx_def = state.ui_defs.gfx[gid];
				auto tid = ogl::get_texture_handle(state, gfx_def.primary_texture_handle, gfx_def.is_partially_transparent());
				if(!tid) {
					tid = ogl::get_texture_handle(state, ui::definitions::no_news_image, false);
				}
				ogl::render_textured_rect(state, get_color_modification(this == state.ui_state.under_mouse, disabled, interactable),
					float(x), float(y), float(base_data.size.x), float(base_data.size.y),
					tid, base_data.get_rotation(), gfx_def.is_vertically_flipped(), get_horizontal_flip(state));
			} else {
				auto tid = ogl::get_texture_handle(state, ui::definitions::no_news_image, false);
				ogl::render_textured_rect(state, get_color_modification(this == state.ui_state.under_mouse, disabled, interactable),
					float(x), float(y), float(base_data.size.x), float(base_data.size.y),
					tid, base_data.get_rotation(), false, get_horizontal_flip(state));
			}
		}
	};
	template<size_t size, size_t index>
	class news_article_title : public multiline_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			news::news_scope scope;
			if constexpr(size == news::news_size_huge) {
				scope = state.news_definitions.large_articles[index];
			} else if constexpr(size == news::news_size_medium) {
				scope = state.news_definitions.medium_articles[index];
			} else if constexpr(size == news::news_size_small) {
				scope = state.news_definitions.small_articles[index];
			}
			auto contents = text::create_endless_layout(state, internal_layout,
				text::layout_parameters{ 0, 0, int16_t(base_data.size.x), int16_t(base_data.size.y),
				base_data.data.text.font_handle, 0, text::alignment::left,
				black_text ? text::text_color::black : text::text_color::white, false
			});
			auto box = text::open_layout_box(contents);
			text::substitution_map sub{};
			text::add_to_substitution_map(sub, text::variable_type::string_0_0, scope.strings[0][0]);
			text::add_to_substitution_map(sub, text::variable_type::string_0_1, scope.strings[0][1]);
			text::add_to_substitution_map(sub, text::variable_type::string_0_2, scope.strings[0][2]);
			text::add_to_substitution_map(sub, text::variable_type::string_0_3, scope.strings[0][3]);
			text::add_to_substitution_map(sub, text::variable_type::string_0_4, scope.strings[0][4]);
			text::add_to_substitution_map(sub, text::variable_type::value_int_0_0, scope.values[0][0]);
			text::add_to_substitution_map(sub, text::variable_type::value_int_0_1, scope.values[0][1]);
			text::add_to_substitution_map(sub, text::variable_type::value_int_0_2, scope.values[0][2]);
			text::add_to_substitution_map(sub, text::variable_type::value_int_0_3, scope.values[0][3]);
			text::add_to_substitution_map(sub, text::variable_type::date_long_0, scope.dates[0][0]);
			text::add_to_substitution_map(sub, text::variable_type::date_long_1, scope.dates[0][1]);
			text::add_to_substitution_map(sub, text::variable_type::date_short_0, scope.dates[0][0]);
			text::add_to_substitution_map(sub, text::variable_type::tag0_0, scope.tags[0][0]);
			text::add_to_substitution_map(sub, text::variable_type::tag_0_0, scope.tags[0][0]);
			text::add_to_substitution_map(sub, text::variable_type::tag_0_0_, scope.tags[0][0]);
			text::add_to_substitution_map(sub, text::variable_type::tag_0_0_adj, state.world.national_identity_get_adjective(scope.tags[0][0]));
			text::add_to_substitution_map(sub, text::variable_type::tag_0_1, scope.tags[0][1]);
			text::add_to_substitution_map(sub, text::variable_type::tag_0_1_adj, state.world.national_identity_get_adjective(scope.tags[0][1]));
			text::add_to_substitution_map(sub, text::variable_type::tag_0_2, scope.tags[0][2]);
			text::add_to_substitution_map(sub, text::variable_type::tag_0_2_adj, state.world.national_identity_get_adjective(scope.tags[0][2]));
			text::add_to_substitution_map(sub, text::variable_type::tag_0_3, scope.tags[0][3]);
			text::add_to_substitution_map(sub, text::variable_type::tag_0_3_adj, state.world.national_identity_get_adjective(scope.tags[0][3]));
			text::add_to_layout_box(state, contents, box, scope.title, sub);
			text::close_layout_box(contents, box);
		}
	};
	template<size_t size, size_t index>
	class news_article_desc : public scrollable_text {
	public:
		void on_update(sys::state& state) noexcept override {
			news::news_scope scope;
			if constexpr(size == news::news_size_huge) {
				scope = state.news_definitions.large_articles[index];
			} else if constexpr(size == news::news_size_medium) {
				scope = state.news_definitions.medium_articles[index];
			} else if constexpr(size == news::news_size_small) {
				scope = state.news_definitions.small_articles[index];
			}
			auto contents = text::create_endless_layout(state, delegate->internal_layout,
				text::layout_parameters{ 0, 0, int16_t(base_data.size.x), int16_t(base_data.size.y),
				delegate->base_data.data.text.font_handle, 0, text::alignment::left,
				delegate->black_text ? text::text_color::black : text::text_color::white, false
			});
			auto box = text::open_layout_box(contents);
			text::substitution_map sub{};
			text::add_to_substitution_map(sub, text::variable_type::string_0_0, scope.strings[0][0]);
			text::add_to_substitution_map(sub, text::variable_type::string_0_1, scope.strings[0][1]);
			text::add_to_substitution_map(sub, text::variable_type::string_0_2, scope.strings[0][2]);
			text::add_to_substitution_map(sub, text::variable_type::string_0_3, scope.strings[0][3]);
			text::add_to_substitution_map(sub, text::variable_type::string_0_4, scope.strings[0][4]);
			text::add_to_substitution_map(sub, text::variable_type::value_int_0_0, scope.values[0][0]);
			text::add_to_substitution_map(sub, text::variable_type::value_int_0_1, scope.values[0][1]);
			text::add_to_substitution_map(sub, text::variable_type::value_int_0_2, scope.values[0][2]);
			text::add_to_substitution_map(sub, text::variable_type::value_int_0_3, scope.values[0][3]);
			text::add_to_substitution_map(sub, text::variable_type::date_long_0, scope.dates[0][0]);
			text::add_to_substitution_map(sub, text::variable_type::date_long_1, scope.dates[0][1]);
			text::add_to_substitution_map(sub, text::variable_type::date_short_0, scope.dates[0][0]);
			text::add_to_substitution_map(sub, text::variable_type::tag0_0, scope.tags[0][0]);
			text::add_to_substitution_map(sub, text::variable_type::tag_0_0, scope.tags[0][0]);
			text::add_to_substitution_map(sub, text::variable_type::tag_0_0_, scope.tags[0][0]);
			text::add_to_substitution_map(sub, text::variable_type::tag_0_0_adj, state.world.national_identity_get_adjective(scope.tags[0][0]));
			text::add_to_substitution_map(sub, text::variable_type::tag_0_1, scope.tags[0][1]);
			text::add_to_substitution_map(sub, text::variable_type::tag_0_1_adj, state.world.national_identity_get_adjective(scope.tags[0][1]));
			text::add_to_substitution_map(sub, text::variable_type::tag_0_2, scope.tags[0][2]);
			text::add_to_substitution_map(sub, text::variable_type::tag_0_2_adj, state.world.national_identity_get_adjective(scope.tags[0][2]));
			text::add_to_substitution_map(sub, text::variable_type::tag_0_3, scope.tags[0][3]);
			text::add_to_substitution_map(sub, text::variable_type::tag_0_3_adj, state.world.national_identity_get_adjective(scope.tags[0][3]));
			text::add_to_layout_box(state, contents, box, scope.desc, sub);
			text::close_layout_box(contents, box);
		}
	};
	template<size_t size, size_t index>
	class news_article_window : public window_element_base {
	public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "image") {
				return make_element_by_type<news_article_picture<size, index>>(state, id);
			} else if(name == "title") {
				return make_element_by_type<news_article_title<size, index>>(state, id);
			} else if(name == "desc") {
				return make_element_by_type<news_article_desc<size, index>>(state, id);
			} else {
				return nullptr;
			}
		}
	};
	class news_page_window : public window_element_base {
	public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "window_bg") {
				return make_element_by_type<draggable_target>(state, id);
			} else if(name == "close_button") {
				return make_element_by_type<generic_close_button>(state, id);
			} else if(name == "date") {
				return make_element_by_type<news_page_date>(state, id);
			} else if(name == "article_main") {
				return make_element_by_type<news_article_window<news::news_size_huge, 0>>(state, id);
			} else if(name == "article_medium_1") {
				return make_element_by_type<news_article_window<news::news_size_medium, 0>>(state, id);
			} else if(name == "article_medium_2") {
				return make_element_by_type<news_article_window<news::news_size_medium, 1>>(state, id);
			} else if(name == "article_small_1") {
				return make_element_by_type<news_article_window<news::news_size_small, 0>>(state, id);
			} else if(name == "article_small_2") {
				return make_element_by_type<news_article_window<news::news_size_small, 1>>(state, id);
			} else if(name == "article_small_3") {
				return make_element_by_type<news_article_window<news::news_size_small, 2>>(state, id);
			} else if(name == "article_small_4") {
				return make_element_by_type<news_article_window<news::news_size_small, 3>>(state, id);
			} else if(name == "article_small_5") {
				return make_element_by_type<news_article_window<news::news_size_small, 4>>(state, id);
			} else {
				return nullptr;
			}
		}
	};
	class news_open_button : public button_element_base {
	public:
		void button_action(sys::state& state) noexcept override {
			state.ui_state.news_page_window->set_visible(state, !state.ui_state.news_page_window->is_visible());
		}
	};
	class news_open_label : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			set_text(state, "X");
		}
	};
	class news_icon_window : public window_element_base {
	public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "button") {
				return make_element_by_type<news_open_button>(state, id);
			} else if(name == "label") {
				return make_element_by_type<news_open_label>(state, id);
			} else {
				return nullptr;
			}
		}
	};

	class topbar_window : public window_element_base {
	private:
		dcon::nation_id current_nation{};
		std::vector<topbar_commodity_xport_icon*> import_icons;
		std::vector<topbar_commodity_xport_icon*> export_icons;
		std::vector<topbar_commodity_xport_icon*> produced_icons;
		simple_text_element_base* atpeacetext = nullptr;
		element_base* background_pic = nullptr;
	public:
		void on_create(sys::state& state) noexcept override;
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
		void on_update(sys::state& state) noexcept override;
		void render(sys::state& state, int32_t x, int32_t y) noexcept override {
			if(background_pic) {
				background_pic->set_visible(state, state.ui_state.topbar_subwindow && state.ui_state.topbar_subwindow->is_visible());
			}
			window_element_base::render(state, x, y);
		}
		message_result get(sys::state& state, Cyto::Any& payload) noexcept override {
			if(payload.holds_type<dcon::nation_id>()) {
				payload.emplace<dcon::nation_id>(state.local_player_nation);
				return message_result::consumed;
			} else {
				return message_result::unseen;
			}
		}
	};

} // namespace ui
