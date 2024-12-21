#pragma once

#include "gui_element_types.hpp"
#include "gui_common_elements.hpp"

namespace ui {
	enum class diplomacy_action : uint8_t {
		ally,
		cancel_ally,
		call_ally,
		military_access,
		cancel_military_access,
		give_military_access,
		cancel_give_military_access,
		increase_relations,
		decrease_relations,
		war_subsidies,
		cancel_war_subsidies,
		declare_war,
		command_units,
		cancel_command_units,
		increase_opinion,
		add_to_sphere,
		remove_from_sphere,
		justify_war,
		make_peace,
		crisis_backdown,
		crisis_support,
		add_wargoal,
		state_transfer,
	};

	enum class gp_choice_actions {
		discredit,
		expel_advisors,
		ban_embassy,
		decrease_opinion
	};

	struct trigger_gp_choice {
		gp_choice_actions action = gp_choice_actions::discredit;
	};


	class diplomacy_action_discredit_button : public button_element_base {
		public:
		void on_create(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class diplomacy_action_expel_advisors_button : public button_element_base {
		public:
		void on_create(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class diplomacy_action_ban_embassy_button : public button_element_base {
		public:
		void on_create(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};


	class diplomacy_action_decrease_opinion_button : public button_element_base {
		public:
		void on_create(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};


	class diplomacy_action_ally_button : public button_element_base {
		public:
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class diplomacy_action_call_ally_button : public button_element_base {
		public:
		void on_create(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class diplomacy_action_military_access_button : public button_element_base {
		public:
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class diplomacy_action_give_military_access_button : public button_element_base {
		public:
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class diplomacy_action_increase_relations_button : public button_element_base {
		public:
		void on_create(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;

		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class diplomacy_action_decrease_relations_button : public button_element_base {
		public:
		void on_create(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;

		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class diplomacy_action_war_subisides_button : public button_element_base {
		public:
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class diplomacy_action_declare_war_button : public button_element_base {
		public:
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class diplomacy_action_release_subject_button : public button_element_base {
		public:
		void on_create(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class diplomacy_action_increase_opinion_button : public button_element_base {
		public:
		void on_create(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class diplomacy_action_add_to_sphere_button : public button_element_base {
		public:
		void on_create(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class diplomacy_action_remove_from_sphere_button : public button_element_base {
		public:
		void on_create(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class diplomacy_action_justify_war_button : public button_element_base {
		bool has_any_usable_cb(sys::state& state, dcon::nation_id source, dcon::nation_id target) noexcept {
			auto res = false;
			state.world.for_each_cb_type([&](dcon::cb_type_id cb) {
				if(command::can_fabricate_cb(state, source, target, cb)) {
					res = true;
				}
			});
			return res;
		}
	public:
		void on_create(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class diplomacy_action_dialog_title_text : public generic_settable_element<simple_text_element_base, diplomacy_action> {
		static std::string_view get_title_key(diplomacy_action v) noexcept;
	public:
		void on_update(sys::state& state) noexcept override {
			set_text(state, text::produce_simple_string(state, get_title_key(content)));
		}
	};
	class diplomacy_action_dialog_description_text : public generic_settable_element<multiline_text_element_base, diplomacy_action> {
		static std::string_view get_title_key(diplomacy_action v) noexcept;
	public:
		void on_update(sys::state& state) noexcept override;
	};

	class diplomacy_action_dialog_agree_button : public generic_settable_element<button_element_base, diplomacy_action> {
		bool get_can_perform(sys::state& state) noexcept;
	public:
		void on_create(sys::state& state) noexcept override {
			base_data.data.button.txt = state.lookup_key("agree");
			button_element_base::on_create(state);
		}
		void on_update(sys::state& state) noexcept override {
			disabled = !get_can_perform(state);
		}
		void button_action(sys::state& state) noexcept override;
	};
	class diplomacy_action_dialog_decline_button : public generic_close_button {
		public:
		void on_create(sys::state& state) noexcept override {
			base_data.data.button.txt = state.lookup_key("decline");
			button_element_base::on_create(state);
		}
	};

	class diplomacy_action_dialog_window : public window_element_base {
	public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
	};

	struct gp_choice_num {
		int32_t value = 0;
	};

	struct gp_choice_select {
		int32_t value = 0;
	};

	struct gp_choice_get_selection {
		int32_t value = 0;
	};

	struct select_gp_choice_button : public flag_button {
	public:
		dcon::national_identity_id get_current_nation(sys::state& state) noexcept override {
			auto n = nations::get_nth_great_power(state, uint16_t(retrieve<gp_choice_num>(state, parent).value));
			return state.world.nation_get_identity_from_identity_holder(n);
		}
		void button_action(sys::state& state) noexcept override {
			auto this_selection = retrieve<gp_choice_num>(state, parent);
			send(state, parent, gp_choice_select{ this_selection.value });
		}
	};

	class select_gp_name : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			auto const n = nations::get_nth_great_power(state, uint16_t(retrieve<gp_choice_num>(state, parent).value));
			set_text(state, text::produce_simple_string(state, text::get_name(state, n)));
		}
	};

	struct select_gp_selection_icon : public image_element_base {
	public:
		bool show = false;
		void on_update(sys::state& state) noexcept override {
			show = retrieve<gp_choice_num>(state, parent).value == retrieve<gp_choice_get_selection>(state, parent).value;
		}
		void render(sys::state& state, int32_t x, int32_t y) noexcept override {
			if(show) {
				image_element_base::render(state, x, y);
			}
		}
	};

	class select_gp_banned : public image_element_base {
		public:
		bool show = false;
		void on_update(sys::state& state) noexcept override {
			auto gp = nations::get_nth_great_power(state, uint16_t(retrieve<gp_choice_num>(state, parent).value));
			auto target = retrieve<dcon::nation_id>(state, parent);
			show = (state.world.gp_relationship_get_status(state.world.get_gp_relationship_by_gp_influence_pair(target, gp)) & nations::influence::is_banned) != 0;
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return show ? tooltip_behavior::variable_tooltip : tooltip_behavior::no_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			auto gp = nations::get_nth_great_power(state, uint16_t(retrieve<gp_choice_num>(state, parent).value));
			auto target = retrieve<dcon::nation_id>(state, parent);
			auto rel = state.world.get_gp_relationship_by_gp_influence_pair(target, gp);

			text::add_line(state, contents, "dp_inf_tooltip_ban", text::variable_type::x, state.world.gp_relationship_get_penalty_expires_date(rel));
		}
		void render(sys::state& state, int32_t x, int32_t y) noexcept override {
			if(show)
			image_element_base::render(state, x, y);
		}
	};

	class select_gp_discredited : public image_element_base {
		public:
		bool show = false;
		void on_update(sys::state& state) noexcept override {
			auto gp = nations::get_nth_great_power(state, uint16_t(retrieve<gp_choice_num>(state, parent).value));
			auto target = retrieve<dcon::nation_id>(state, parent);
			show = (state.world.gp_relationship_get_status(state.world.get_gp_relationship_by_gp_influence_pair(target, gp)) & nations::influence::is_discredited) != 0;
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return show ? tooltip_behavior::variable_tooltip : tooltip_behavior::no_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			auto gp = nations::get_nth_great_power(state, uint16_t(retrieve<gp_choice_num>(state, parent).value));
			auto target = retrieve<dcon::nation_id>(state, parent);
			auto rel = state.world.get_gp_relationship_by_gp_influence_pair(target, gp);

			text::add_line(state, contents, "dp_inf_tooltip_discredit", text::variable_type::x, state.world.gp_relationship_get_penalty_expires_date(rel));
		}
		void render(sys::state& state, int32_t x, int32_t y) noexcept override {
			if(show)
			image_element_base::render(state, x, y);
		}
	};

	class select_gp_opinion_detail : public simple_text_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			auto gp = nations::get_nth_great_power(state, uint16_t(retrieve<gp_choice_num>(state, parent).value));
			auto target = retrieve<dcon::nation_id>(state, parent);

			set_text(state, text::get_influence_level_name(state, state.world.gp_relationship_get_status(state.world.get_gp_relationship_by_gp_influence_pair(target, gp))));
		}
	};
	class select_gp_influence_detail : public simple_text_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			auto gp = nations::get_nth_great_power(state, uint16_t(retrieve<gp_choice_num>(state, parent).value));
			auto target = retrieve<dcon::nation_id>(state, parent);

			set_text(state, text::format_float(state.world.gp_relationship_get_influence(state.world.get_gp_relationship_by_gp_influence_pair(target, gp)), 1));
		}
	};

	class diplomacy_action_gp_dialog_select_window : public window_element_base {
	public:
		uint8_t rank = 0;
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
		message_result get(sys::state& state, Cyto::Any& payload) noexcept override;
	};


	class gp_action_choice_title : public generic_settable_element<simple_text_element_base, diplomacy_action> {
		public:
		void on_update(sys::state& state) noexcept override {
			switch(retrieve<gp_choice_actions>(state, parent)) {
			case gp_choice_actions::discredit:
				set_text(state, text::produce_simple_string(state, "discredittitle"));
				break;
			case gp_choice_actions::expel_advisors:
				set_text(state, text::produce_simple_string(state, "expeladvisorstitle"));
				break;
			case gp_choice_actions::ban_embassy:
				set_text(state, text::produce_simple_string(state, "banembassytitle"));
				break;
			case gp_choice_actions::decrease_opinion:
				set_text(state, text::produce_simple_string(state, "decreaseopiniontitle"));
				break;
			default:
				break;
			}

		}
	};

	class gp_action_choice_desc : public generic_settable_element<simple_text_element_base, diplomacy_action> {
		public:
		void on_update(sys::state& state) noexcept override {
			text::substitution_map m;
			switch(retrieve<gp_choice_actions>(state, parent)) {
			case gp_choice_actions::discredit:
				text::add_to_substitution_map(m, text::variable_type::days, int64_t(state.defines.discredit_days));
				set_text(state, text::resolve_string_substitution(state, "discredit_desc", m));
				break;
			case gp_choice_actions::expel_advisors:
				set_text(state, text::produce_simple_string(state, "expeladvisors_desc"));
				break;
			case gp_choice_actions::ban_embassy:
				text::add_to_substitution_map(m, text::variable_type::days, int64_t(state.defines.banembassy_days));
				set_text(state, text::resolve_string_substitution(state, "banembassy_desc", m));
				break;
			case gp_choice_actions::decrease_opinion:
				set_text(state, text::produce_simple_string(state, "decreaseopinion_desc"));
				break;
			default:
				break;
			}
		}
	};

	class gp_action_choice_cancel_button : public generic_close_button {
		public:
		void on_create(sys::state& state) noexcept override {
			base_data.data.button.txt = state.lookup_key("cancel");
			button_element_base::on_create(state);
		}
	};

	class gp_action_choice_accept_button : public button_element_base {
		public:
		void on_create(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	class diplomacy_gp_action_dialog_window : public window_element_base {
	public:
		int32_t selected_gp = 0;
		dcon::nation_id action_target{ 0 };
		gp_choice_actions current_action = gp_choice_actions::discredit;

		void on_create(sys::state& state) noexcept override;
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
		message_result get(sys::state& state, Cyto::Any& payload) noexcept override;
	};

	class diplomacy_action_state_transfer_button : public button_element_base {
		public:
		void on_create(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
	};

	template<typename T>
	class diplomacy_action_window : public window_element_base {
		public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "action_option") {
				return make_element_by_type<T>(state, id);
			} else {
				return nullptr;
			}
		}
	};

} // namespace ui
