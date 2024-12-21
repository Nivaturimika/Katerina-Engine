#pragma once

#include "economy_factory.hpp"
#include "gui_element_types.hpp"
#include "gui_common_elements.hpp"

namespace ui {
	class factory_prod_subsidise_all_button : public tinted_button_element_base {
	public:
		void button_action(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class factory_prod_unsubsidise_all_button : public tinted_button_element_base {
	public:
		void button_action(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class factory_select_all_button : public button_element_base {
	public:
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class factory_deselect_all_button : public button_element_base {
		public:
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class factory_show_empty_states_button : public button_element_base {
	public:
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::tooltip;
		}
		void on_update(sys::state& state) noexcept override;
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	template<production_sort_order Sort>
	class factory_production_sort : public button_element_base {
		void button_action(sys::state& state) noexcept override {
			send(state, parent, Sort);
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			switch(Sort) {
			case production_sort_order::name:
				text::add_line(state, contents, "sort_by_name");
				break;
			case production_sort_order::factories:
				text::add_line(state, contents, "sort_by_factories");
				break;
			case production_sort_order::infrastructure:
				text::add_line(state, contents, "sort_by_infrastructure");
				break;
			case production_sort_order::focus:
				text::add_line(state, contents, "sort_by_focus");
				break;
			case production_sort_order::owners:
				text::add_line(state, contents, "sort_by_capitalists");
				break;
			case production_sort_order::primary_workers:
			case production_sort_order::secondary_workers:
				text::add_line(state, contents, "sort_by_unemployed_workers");
				break;
			default:
				break;
			}
		}
	};

	class factory_buttons_window : public window_element_base {
	public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
	};

	class production_investment_country_select : public button_element_base {
	public:
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
	};

	class production_investment_country_info : public listbox_row_element_base<dcon::nation_id> {
	private:
		flag_button* country_flag = nullptr;
	public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
		message_result get(sys::state& state, Cyto::Any& payload) noexcept override;
	};

	class production_country_listbox : public listbox_element_base<production_investment_country_info, dcon::nation_id> {
	protected:
		std::string_view get_row_element_name() override {
			return "investment_country_entry";
		}
	public:
		void on_update(sys::state& state) noexcept override;
	};

	class production_sort_nation_gp_flag : public nation_gp_flag {
	public:
		void button_action(sys::state& state) noexcept override;
	};

	class production_sort_my_nation_flag : public flag_button {
	public:
		dcon::national_identity_id get_current_nation(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
	};

	class invest_brow_window : public window_element_base {
		production_country_listbox* country_listbox = nullptr;
		country_filter_setting filter = country_filter_setting{};
		country_sort_setting sort = country_sort_setting{};
	public:
		void on_create(sys::state& state) noexcept override;
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
		message_result get(sys::state& state, Cyto::Any& payload) noexcept override;
	};

	class pop_sort_buttons_window : public window_element_base {
		xy_pair sort_template_offset{};
	public:
		void on_create(sys::state& state) noexcept override;
	};

	struct commodity_filter_query_data {
		dcon::commodity_id cid;
		bool filter;
	};
	struct commodity_filter_toggle_data : public element_selection_wrapper<dcon::commodity_id> { };

	class commodity_filter_button : public button_element_base {
	public:
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class commodity_filter_enabled_image : public image_element_base {
	public:
		void on_update(sys::state& state) noexcept override;
	};

	class commodity_filter_item : public window_element_base {
	public:
		dcon::commodity_id content{};
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
		message_result get(sys::state& state, Cyto::Any& payload) noexcept override;
	};

	class commodity_filters_window : public window_element_base {
		public:
		void on_create(sys::state& state) noexcept override;
	};
}

#include "gui_projects_window.hpp"
#include "gui_build_factory_window.hpp"
#include "gui_project_investment_window.hpp"
#include "gui_foreign_investment_window.hpp"
#include "economy_factory_templates.hpp"
#include "gui_trigger_tooltips.hpp"
#include "province_templates.hpp"

namespace ui {
	struct production_selection_wrapper {
		dcon::state_instance_id data{};
		bool is_build = false;
		xy_pair focus_pos{ 0, 0 };
	};

	class factory_employment_image : public image_element_base {
	public:
		void on_update(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class factory_priority_button : public button_element_base {
	public:
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class factory_upgrade_button : public button_element_base {
	public:
		void on_update(sys::state& state) noexcept override;
		void button_right_action(sys::state& state) noexcept override;
		void button_shift_action(sys::state& state) noexcept override;
		void button_shift_right_action(sys::state& state) noexcept override;
		void button_ctrl_action(sys::state& state) noexcept override;
		void button_ctrl_right_action(sys::state& state) noexcept override;
		void button_ctrl_shift_action(sys::state& state) noexcept override;
		void button_ctrl_shift_right_action(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		void render(sys::state& state, int32_t x, int32_t y) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class factory_reopen_button : public button_element_base {
	public:
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		void on_create(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class factory_subsidise_button : public button_element_base {
	public:
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class factory_delete_button : public button_element_base {
	public:
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class factory_close_and_delete_button : public button_element_base {
	public:
		bool visible = true;
		void on_update(sys::state& state) noexcept override;
		message_result test_mouse(sys::state& state, int32_t x, int32_t y, mouse_probe_type type) noexcept override;
		void button_action(sys::state& state) noexcept override;
		void render(sys::state& state, int32_t x, int32_t y) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	struct state_factory_slot {
		dcon::factory_id id;
		std::variant<std::monostate, economy_factory::upgraded_factory, economy_factory::new_factory> activity;
	};

	struct production_factory_slot_data : public state_factory_slot {
		size_t index = 0;
	};

	class factory_build_progress_bar : public progress_bar {
	public:
		void on_update(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class factory_upgrade_progress_bar : public progress_bar {
	public:
		void on_update(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class normal_factory_background : public opaque_element_base {
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class factory_input_icon : public image_element_base {
	public:
		dcon::commodity_id com{};
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return com ? tooltip_behavior::variable_tooltip : tooltip_behavior::no_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
		void render(sys::state& state, int32_t x, int32_t y) noexcept override;
	};

	class factory_cancel_new_const_button : public button_element_base {
		public:
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};
	class factory_cancel_upgrade_button : public button_element_base {
		public:
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class production_factory_output : public commodity_image {
	public:
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class production_factory_info : public window_element_base {
		factory_input_icon* input_icons[economy::commodity_set::set_size] = { nullptr };
		image_element_base* input_lack_icons[economy::commodity_set::set_size] = { nullptr };
		std::vector<element_base*> factory_elements;
		std::vector<element_base*> upgrade_elements;
		std::vector<element_base*> build_elements;
		std::vector<element_base*> closed_elements;
		dcon::commodity_id output_commodity;

	public:
		uint8_t index = 0; // from 0 to int32_t(state.defines.factories_per_state)
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
		void on_update(sys::state& state) noexcept override;
		message_result get(sys::state& state, Cyto::Any& payload) noexcept override;
	};

	class production_factory_info_bounds_window : public window_element_base {
		std::vector<element_base*> infos;
		std::vector<state_factory_slot> factories;

		bool get_filter(sys::state& state, dcon::commodity_id cid) ;
	public:
		void on_create(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
		message_result get(sys::state& state, Cyto::Any& payload) noexcept override;
	};

	class province_build_new_factory : public button_element_base {
		public:
		void on_create(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class production_build_new_factory : public button_element_base {
		public:
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class production_national_focus_button : public button_element_base {
		int32_t get_icon_frame(sys::state& state) noexcept;
	public:
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		void button_right_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class state_infrastructure : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override;
	};

	class production_factory_pop_amount : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class production_factory_pop_window : public window_element_base {
	public:
		dcon::pop_type_id pt;
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
		message_result get(sys::state& state, Cyto::Any& payload) noexcept override;
	};

	class production_state_info : public listbox_row_element_base<dcon::state_instance_id> {
	public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
		void on_create(sys::state& state) noexcept override;
	};

	void populate_production_states_list(sys::state& state, std::vector<dcon::state_instance_id>& row_contents, dcon::nation_id n, bool show_empty, production_sort_order sort_order);

	struct production_foreign_invest_target {
		dcon::nation_id n;
	};

	class production_state_invest_listbox : public listbox_element_base<production_state_info, dcon::state_instance_id> {
	protected:
		std::string_view get_row_element_name() override {
			return "state_info";
		}
	public:
		production_sort_order sort_order = production_sort_order::name;
		void on_update(sys::state& state) noexcept override;
		message_result get(sys::state& state, Cyto::Any& payload) noexcept override;
	};

	class production_state_listbox : public listbox_element_base<production_state_info, dcon::state_instance_id> {
		protected:
		std::string_view get_row_element_name() override {
			return "state_info";
		}

		public:
		production_sort_order sort_order = production_sort_order::name;
		void on_update(sys::state& state) noexcept override;
	};

	class production_goods_category_name : public window_element_base {
		simple_text_element_base* goods_cat_name = nullptr;

		std::string_view get_commodity_group_name(sys::state& state, dcon::commodity_group_id g) {
			return state.to_string_view(state.commodity_group_names[g]);
		}

	public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
		message_result set(sys::state& state, Cyto::Any& payload) noexcept override;
	};

	class commodity_primary_worker_amount : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override;
	};

	class commodity_secondary_worker_amount : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override;
	};

	class commodity_player_production_text : public simple_text_element_base {
		public:
		void on_update(sys::state& state) noexcept override;
	};

	class production_good_info : public window_element_base {
		commodity_player_production_text* good_output_total = nullptr;
		image_element_base* good_not_producing_overlay = nullptr;

		public:
		dcon::commodity_id commodity_id;

		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
		void on_update(sys::state& state) noexcept override;
		message_result get(sys::state& state, Cyto::Any& payload) noexcept override;
	};

	class production_factories_tab_button : public generic_tab_button<production_window_tab> {
	public:
		void on_create(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
	};

	class production_projects_tab_button : public generic_tab_button<production_window_tab> {
	public:
		void on_create(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
	};

	struct open_investment_nation {
		dcon::nation_id id;
	};

	class production_window : public generic_tabbed_window<production_window_tab> {
		bool show_empty_states = true;
		std::unique_ptr<bool[]> show_output_commodity;

		production_state_listbox* state_listbox = nullptr;
		production_state_invest_listbox* state_listbox_invest = nullptr;
		element_base* nf_win = nullptr;
		element_base* build_win = nullptr;
		element_base* project_window = nullptr;
		production_foreign_investment_window* foreign_invest_win = nullptr;

		dcon::commodity_group_id curr_commodity_group{};
		dcon::state_instance_id focus_state{};
		dcon::nation_id foreign_nation{};
		xy_pair base_commodity_offset{33, 50};
		xy_pair commodity_offset{33, 50};

		std::vector<element_base*> factory_elements;
		std::vector<element_base*> investment_brow_elements;
		std::vector<element_base*> project_elements;
		std::vector<element_base*> good_elements;
		std::vector<element_base*> investment_nation;
		std::vector<bool> commodity_filters;
		bool open_foreign_invest = false;
		void set_visible_vector_elements(sys::state& state, std::vector<element_base*>& elements, bool v) noexcept;
		void hide_sub_windows(sys::state& state) noexcept ;
	public:
		void on_create(sys::state& state) noexcept override;
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
		message_result get(sys::state& state, Cyto::Any& payload) noexcept override;
	};

	void open_foreign_investment(sys::state& state, dcon::nation_id n);
	void open_build_factory(sys::state& state, dcon::state_instance_id st);
} // namespace ui
