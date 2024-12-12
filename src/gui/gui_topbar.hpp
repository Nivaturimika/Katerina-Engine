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
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
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

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
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
		int32_t get_icon_frame(sys::state& state, dcon::nation_id nation_id) noexcept override;
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
	};

	class topbar_rebels_icon : public button_element_base {
	public:
		void on_update(sys::state& state) noexcept override;
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
	};

	class topbar_colony_icon : public standard_nation_button {
		uint32_t index = 0;
		dcon::province_id get_state_def_province(sys::state& state, dcon::state_definition_id sdef) noexcept;
	public:
		void button_action(sys::state& state) noexcept override;
		int32_t get_icon_frame(sys::state& state, dcon::nation_id nation_id) noexcept override;
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
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
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
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
			}
			return message_result::unseen;
		}
	};

} // namespace ui
