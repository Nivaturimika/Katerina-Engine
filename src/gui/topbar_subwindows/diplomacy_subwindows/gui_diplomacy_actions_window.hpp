#pragma once

#include "gui_element_types.hpp"
#include "military.hpp"
#include "economy_estimations.hpp"
#include "ai.hpp"

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
		static std::string_view get_title_key(diplomacy_action v) noexcept {
			switch(v) {
			case diplomacy_action::ally:
				return "alliancetitle";
			case diplomacy_action::cancel_ally:
				return "cancelalliancetitle";
			case diplomacy_action::call_ally:
				return "callallytitle";
			case diplomacy_action::declare_war:
				return "wartitle";
			case diplomacy_action::military_access:
				return "askmilitaryaccesstitle";
			case diplomacy_action::cancel_military_access:
				return "cancelaskmilitaryaccesstitle";
			case diplomacy_action::give_military_access:
				return "givemilitaryaccesstitle";
			case diplomacy_action::cancel_give_military_access:
				return "cancelgivemilitaryaccesstitle";
			case diplomacy_action::increase_relations:
				return "increaserelationtitle";
			case diplomacy_action::decrease_relations:
				return "decreaserelationtitle";
			case diplomacy_action::war_subsidies:
				return "warsubsidiestitle";
			case diplomacy_action::cancel_war_subsidies:
				return "cancel_warsubsidiestitle";
			case diplomacy_action::increase_opinion:
				return "increaseopiniontitle";
			case diplomacy_action::add_to_sphere:
				return "addtospheretitle";
			case diplomacy_action::remove_from_sphere:
				return "removefromspheretitle";
			case diplomacy_action::justify_war:
				return "make_cbtitle";
			case diplomacy_action::command_units:
				return "give_unit_commandtitle";
			case diplomacy_action::cancel_command_units:
				return "cancel_unit_commandtitle";
			case diplomacy_action::make_peace:
				return "make_peacetitle";
			default:
				return "";
			}
		}

		public:
		void on_update(sys::state& state) noexcept override {
			set_text(state, text::produce_simple_string(state, get_title_key(content)));
		}
	};
	class diplomacy_action_dialog_description_text : public generic_settable_element<multiline_text_element_base, diplomacy_action> {
		static std::string_view get_title_key(diplomacy_action v) noexcept {
			switch(v) {
				case diplomacy_action::ally:
				return "alliancenewdesc";
				case diplomacy_action::cancel_ally:
				return "breakalliancenewdesc";
				case diplomacy_action::call_ally:
				return "callally_desc";
				case diplomacy_action::declare_war:
				return "war_desc";
				case diplomacy_action::military_access:
				return "askmilitaryaccess_desc";
				case diplomacy_action::cancel_military_access:
				return "cancelaskmilitaryaccess_desc";
				case diplomacy_action::give_military_access:
				return "givemilitaryaccess_desc";
				case diplomacy_action::cancel_give_military_access:
				return "cancelgivemilitaryaccess_desc";
				case diplomacy_action::increase_relations:
				return "increaserelation_desc";
				case diplomacy_action::decrease_relations:
				return "decreaserelation_desc";
				case diplomacy_action::war_subsidies:
				return "warsubsidies_desc";
				case diplomacy_action::cancel_war_subsidies:
				return "cancel_warsubsidies_desc";
				case diplomacy_action::increase_opinion:
				return "increaseopinion_desc";
				case diplomacy_action::add_to_sphere:
				return "addtosphere_desc";
				case diplomacy_action::remove_from_sphere:
				return "removefromsphere_desc";
				case diplomacy_action::justify_war:
				return "make_cb_desc";
				case diplomacy_action::command_units:
				return "give_unit_command_desc";
				case diplomacy_action::cancel_command_units:
				return "cancel_unit_command_desc";
				case diplomacy_action::make_peace:
				return "make_peace_desc";
				default:
				return "";
			}
		}

		public:
		void on_update(sys::state& state) noexcept override {
			auto contents = text::create_endless_layout(state, internal_layout,
				text::layout_parameters{ 0, 0, static_cast<int16_t>(base_data.size.x), static_cast<int16_t>(base_data.size.y),
						base_data.data.text.font_handle, 0, text::alignment::left, text::text_color::white, false });
			auto box = text::open_layout_box(contents);
			text::localised_format_box(state, contents, box, get_title_key(content));
			text::close_layout_box(contents, box);
		}
	};

	class diplomacy_action_dialog_agree_button : public generic_settable_element<button_element_base, diplomacy_action> {
		bool get_can_perform(sys::state& state) noexcept {
			auto target = retrieve<dcon::nation_id>(state, parent);
			switch(content) {
				case diplomacy_action::ally:
				return command::can_ask_for_alliance(state, state.local_player_nation, target);
				case diplomacy_action::cancel_ally:
				return command::can_cancel_alliance(state, state.local_player_nation, target);
				case diplomacy_action::call_ally:
				{
					for(auto wp : dcon::fatten(state.world, state.local_player_nation).get_war_participant())
					if(command::can_call_to_arms(state, state.local_player_nation, target, dcon::fatten(state.world, wp).get_war().id))
					return true;
					return false;
				}
				case diplomacy_action::declare_war:
				return false;
				case diplomacy_action::military_access:
				return command::can_ask_for_access(state, state.local_player_nation, target);
				case diplomacy_action::cancel_military_access:
				return command::can_cancel_military_access(state, state.local_player_nation, target);
				case diplomacy_action::give_military_access:
				return false;
				case diplomacy_action::cancel_give_military_access:
				return command::can_cancel_given_military_access(state, state.local_player_nation, target);
				case diplomacy_action::increase_relations:
				return command::can_increase_relations(state, state.local_player_nation, target);
				case diplomacy_action::decrease_relations:
				return command::can_decrease_relations(state, state.local_player_nation, target);
				case diplomacy_action::war_subsidies:
				return command::can_give_war_subsidies(state, state.local_player_nation, target);
				case diplomacy_action::cancel_war_subsidies:
				return command::can_cancel_war_subsidies(state, state.local_player_nation, target);
				case diplomacy_action::increase_opinion:
				return command::can_increase_opinion(state, state.local_player_nation, target);
				case diplomacy_action::add_to_sphere:
				return command::can_add_to_sphere(state, state.local_player_nation, target);
				case diplomacy_action::remove_from_sphere:
				return command::can_remove_from_sphere(state, state.local_player_nation, target, state.world.nation_get_in_sphere_of(target));
				case diplomacy_action::justify_war:
				return false;
				case diplomacy_action::command_units:
				return false;
				case diplomacy_action::cancel_command_units:
				return false;
				case diplomacy_action::make_peace:
				return false;
				case diplomacy_action::crisis_backdown:
				return false;
				case diplomacy_action::crisis_support:
				return false;
				case diplomacy_action::add_wargoal:
				return false;
				case diplomacy_action::state_transfer:
				return false;
			}
			return false;
		}

		public:
		void on_create(sys::state& state) noexcept override {
			base_data.data.button.txt = state.lookup_key("agree");
			button_element_base::on_create(state);
		}

		void on_update(sys::state& state) noexcept override {
			disabled = !get_can_perform(state);
		}

		void button_action(sys::state& state) noexcept override {
			auto target = retrieve<dcon::nation_id>(state, parent);
			switch(content) {
				case diplomacy_action::ally:
				command::ask_for_alliance(state, state.local_player_nation, target);
				break;
				case diplomacy_action::cancel_ally:
				command::cancel_alliance(state, state.local_player_nation, target);
				break;
				case diplomacy_action::call_ally:
				for(auto wp : dcon::fatten(state.world, state.local_player_nation).get_war_participant())
				command::call_to_arms(state, state.local_player_nation, target, dcon::fatten(state.world, wp).get_war().id);
				break;
				case diplomacy_action::declare_war:
				break;
				case diplomacy_action::military_access:
				command::ask_for_military_access(state, state.local_player_nation, target);
				break;
				case diplomacy_action::cancel_military_access:
				command::cancel_military_access(state, state.local_player_nation, target);
				break;
				case diplomacy_action::give_military_access:
				break;
				case diplomacy_action::cancel_give_military_access:
				command::cancel_given_military_access(state, state.local_player_nation, target);
				break;
				case diplomacy_action::increase_relations:
				command::increase_relations(state, state.local_player_nation, target);
				break;
				case diplomacy_action::decrease_relations:
				command::decrease_relations(state, state.local_player_nation, target);
				break;
				case diplomacy_action::war_subsidies:
				command::give_war_subsidies(state, state.local_player_nation, target);
				break;
				case diplomacy_action::cancel_war_subsidies:
				command::cancel_war_subsidies(state, state.local_player_nation, target);
				break;
				case diplomacy_action::increase_opinion:
				command::increase_opinion(state, state.local_player_nation, target);
				break;
				case diplomacy_action::add_to_sphere:
				command::add_to_sphere(state, state.local_player_nation, target);
				break;
				case diplomacy_action::remove_from_sphere:
				command::remove_from_sphere(state, state.local_player_nation, target, state.world.nation_get_in_sphere_of(target));
				break;
				case diplomacy_action::justify_war:
				break;
				case diplomacy_action::command_units:
				break;
				case diplomacy_action::cancel_command_units:
				break;
				case diplomacy_action::make_peace:
				break;
				case diplomacy_action::crisis_backdown:
				break;
				case diplomacy_action::crisis_support:
				break;
				case diplomacy_action::add_wargoal:
				break;
				case diplomacy_action::state_transfer:
				break;
			}
			if(parent) {
				parent->set_visible(state, false);
			}
		}
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
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "title") {
				return make_element_by_type<diplomacy_action_dialog_title_text>(state, id);
			} else if(name == "description") {
				return make_element_by_type<diplomacy_action_dialog_description_text>(state, id);
			} else if(name == "chance_string") {
				return make_element_by_type<simple_text_element_base>(state, id);
			} else if(name == "agreebutton") {
				return make_element_by_type<diplomacy_action_dialog_agree_button>(state, id);
			} else if(name == "declinebutton") {
				return make_element_by_type<diplomacy_action_dialog_decline_button>(state, id);
			} else if(name == "leftshield") {
				return make_element_by_type<nation_player_flag>(state, id);
			} else if(name == "rightshield") {
				return make_element_by_type<flag_button>(state, id);
			} else if(name == "background") {
				auto ptr = make_element_by_type<draggable_target>(state, id);
				ptr->base_data.size = base_data.size;
				return ptr;
			} else {
				return nullptr;
			}
		}
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
			auto n = nations::get_nth_great_power(state, uint16_t(retrieve<gp_choice_num>(state, parent).value));
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

		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "country_name") {
				return make_element_by_type<select_gp_name>(state, id);
			} else if(name == "country_flag") {
				return make_element_by_type<select_gp_choice_button>(state, id);
			} else if(name == "country_selected") {
				return make_element_by_type<select_gp_selection_icon>(state, id);
			} else if(name == "country_discredited") {
				return make_element_by_type<select_gp_discredited>(state, id);
			} else if(name == "country_banned_embassy") {
				return make_element_by_type<select_gp_banned>(state, id);
			} else if(name == "country_opinion") {
				return make_element_by_type<select_gp_opinion_detail>(state, id);
			} else if(name == "country_influence") {
				return make_element_by_type<select_gp_influence_detail>(state, id);
			} else {
				return nullptr;
			}
		}

		message_result get(sys::state& state, Cyto::Any& payload) noexcept override {
			if(payload.holds_type<gp_choice_num>()) {
			payload.emplace<gp_choice_num>(gp_choice_num{ rank });
				return message_result::consumed;
			}
			return window_element_base::get(state, payload);
		}
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
		void on_create(sys::state& state) noexcept override {
			base_data.data.button.txt = state.lookup_key("ok");
			button_element_base::on_create(state);
		}

		void on_update(sys::state& state) noexcept override {
			auto gp = nations::get_nth_great_power(state, uint16_t(retrieve<gp_choice_get_selection>(state, parent).value));
			auto target = retrieve<dcon::nation_id>(state, parent);

			switch(retrieve<gp_choice_actions>(state, parent)) {
				case gp_choice_actions::discredit:
				disabled = !command::can_discredit_advisors(state, state.local_player_nation, target, gp);
				break;
				case gp_choice_actions::expel_advisors:
				disabled = !command::can_expel_advisors(state, state.local_player_nation, target, gp);
				break;
				case gp_choice_actions::ban_embassy:
				disabled = !command::can_ban_embassy(state, state.local_player_nation, target, gp);
				break;
				case gp_choice_actions::decrease_opinion:
				disabled = !command::can_decrease_opinion(state, state.local_player_nation, target, gp);
				break;
			}
		}
		void button_action(sys::state& state) noexcept override {
			auto gp = nations::get_nth_great_power(state, uint16_t(retrieve<gp_choice_get_selection>(state, parent).value));
			auto target = retrieve<dcon::nation_id>(state, parent);

			switch(retrieve<gp_choice_actions>(state, parent)) {
				case gp_choice_actions::discredit:
				command::discredit_advisors(state, state.local_player_nation, target, gp);
				break;
				case gp_choice_actions::expel_advisors:
				command::expel_advisors(state, state.local_player_nation, target, gp);
				break;
				case gp_choice_actions::ban_embassy:
				command::ban_embassy(state, state.local_player_nation, target, gp);
				break;
				case gp_choice_actions::decrease_opinion:
				command::decrease_opinion(state, state.local_player_nation, target, gp);
				break;
			}
			parent->set_visible(state, false);
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			auto source = state.local_player_nation;
			auto target = retrieve<dcon::nation_id>(state, parent);
			auto affected_gp = nations::get_nth_great_power(state, uint16_t(retrieve<gp_choice_get_selection>(state, parent).value));

			auto rel = state.world.get_gp_relationship_by_gp_influence_pair(target, state.local_player_nation);
			text::add_line_with_condition(state, contents, "iaction_explain_2", (state.world.gp_relationship_get_status(rel) & nations::influence::is_banned) == 0);
			switch(retrieve<gp_choice_actions>(state, parent)) {
			case gp_choice_actions::discredit:
			{
				auto clevel = (nations::influence::level_mask & state.world.gp_relationship_get_status(rel));
				text::add_line_with_condition(state, contents, "discredit_explain_2", clevel != nations::influence::level_hostile);
				auto orel = state.world.get_gp_relationship_by_gp_influence_pair(target, affected_gp);
				text::add_line_with_condition(state, contents, "discredit_explain_3", (state.world.gp_relationship_get_status(orel) & nations::influence::is_banned) == 0);
				text::add_line_with_condition(state, contents, "iaction_explain_4", nations::influence::is_influence_level_greater_or_equal(clevel, nations::influence::get_level(state, affected_gp, target)));
				if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::discredit_advisors)].limit; k) {
					text::add_line_break_to_layout(state, contents);
					ui::trigger_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), trigger::to_generic(affected_gp));
				}
				if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::discredit_advisors)].effect; k) {
					auto const r_lo = uint32_t(source.value);
					auto const r_hi = uint32_t(source.index() ^ (target.index() << 4));
					text::add_line_break_to_layout(state, contents);
					ui::effect_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), trigger::to_generic(affected_gp), r_lo, r_hi);
				}
				break;
			}
			case gp_choice_actions::expel_advisors:
			{
				auto clevel = (nations::influence::level_mask & state.world.gp_relationship_get_status(rel));
				text::add_line_with_condition(state, contents, "expel_explain_2", clevel != nations::influence::level_hostile && clevel != nations::influence::level_opposed);
				text::add_line_with_condition(state, contents, "iaction_explain_4", nations::influence::is_influence_level_greater_or_equal(clevel, nations::influence::get_level(state, affected_gp, target)));
				if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::expel_advisors)].limit; k) {
					text::add_line_break_to_layout(state, contents);
					ui::trigger_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), trigger::to_generic(affected_gp));
				}
				if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::expel_advisors)].effect; k) {
					auto const r_lo = uint32_t(source.value);
					auto const r_hi = uint32_t(source.index() ^ (target.index() << 4));
					text::add_line_break_to_layout(state, contents);
					ui::effect_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), trigger::to_generic(affected_gp), r_lo, r_hi);
				}
				break;
			}
			case gp_choice_actions::ban_embassy:
			{
				auto clevel = (nations::influence::level_mask & state.world.gp_relationship_get_status(rel));
				text::add_line_with_condition(state, contents, "ban_explain_2", clevel == nations::influence::level_friendly || clevel == nations::influence::level_in_sphere);
				text::add_line_with_condition(state, contents, "iaction_explain_5", nations::influence::is_influence_level_greater_or_equal(clevel, nations::influence::get_level(state, affected_gp, target)));
				if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::ban_embassy)].limit; k) {
					text::add_line_break_to_layout(state, contents);
					ui::trigger_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), trigger::to_generic(affected_gp));
				}
				if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::ban_embassy)].effect; k) {
					auto const r_lo = uint32_t(source.value);
					auto const r_hi = uint32_t(source.index() ^ (target.index() << 4));
					text::add_line_break_to_layout(state, contents);
					ui::effect_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), trigger::to_generic(affected_gp), r_lo, r_hi);
				}
				break;
			}
			case gp_choice_actions::decrease_opinion:
			{
				auto clevel = (nations::influence::level_mask & state.world.gp_relationship_get_status(rel));
				text::add_line_with_condition(state, contents, "dec_op_explain_3", clevel != nations::influence::level_hostile);
				if((nations::influence::level_mask & state.world.gp_relationship_get_status(state.world.get_gp_relationship_by_gp_influence_pair(target, affected_gp))) == nations::influence::level_hostile) {
					text::add_line_with_condition(state, contents, "dec_op_explain_4", false);
				}
				text::add_line_with_condition(state, contents, "iaction_explain_5", nations::influence::is_influence_level_greater_or_equal(clevel, nations::influence::get_level(state, affected_gp, target)));
				if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::decrease_opinion)].limit; k) {
					text::add_line_break_to_layout(state, contents);
					ui::trigger_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), trigger::to_generic(affected_gp));
				}
				if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::decrease_opinion)].effect; k) {
					auto const r_lo = uint32_t(source.value);
					auto const r_hi = uint32_t(source.index() ^ (target.index() << 4));
					text::add_line_break_to_layout(state, contents);
					ui::effect_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), trigger::to_generic(affected_gp), r_lo, r_hi);
				}
				break;
			}
			}
		}
	};

	class diplomacy_gp_action_dialog_window : public window_element_base {
		public:
		int32_t selected_gp = 0;
	dcon::nation_id action_target{ 0 };
		gp_choice_actions current_action = gp_choice_actions::discredit;

		void on_create(sys::state& state) noexcept override {
			window_element_base::on_create(state);

			xy_pair gp_base_select_offset =
			state.ui_defs.gui[state.ui_state.defs_by_name.find(state.lookup_key("gpselectoptionpos"))->second.definition].position;
			xy_pair gp_select_offset = gp_base_select_offset;
			for(uint8_t i = 0; i < uint8_t(state.defines.great_nations_count); i++) {
				auto ptr = make_element_by_type<diplomacy_action_gp_dialog_select_window>(state,
					state.ui_state.defs_by_name.find(state.lookup_key("gpselectoption"))->second.definition);
				ptr->base_data.position = gp_select_offset;
				ptr->rank = i;
				// Arrange in columns of 2 elements each...
				gp_select_offset.y += ptr->base_data.size.y;
				if(i != 0 && i % 2 == 1) {
					gp_select_offset.x += ptr->base_data.size.x;
					gp_select_offset.y = gp_base_select_offset.y;
				}
				add_child_to_front(std::move(ptr));
			}
		}

		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "title") {
				return make_element_by_type<gp_action_choice_title>(state, id);
			} else if(name == "description") {
				return make_element_by_type<gp_action_choice_desc>(state, id);
			} else if(name == "agreebutton") {
				return make_element_by_type<gp_action_choice_accept_button>(state, id);
			} else if(name == "declinebutton") {
				return make_element_by_type<gp_action_choice_cancel_button>(state, id);
			} else if(name == "leftshield") {
				return make_element_by_type<nation_player_flag>(state, id);
			} else if(name == "rightshield") {
				return make_element_by_type<flag_button>(state, id);
			} else if(name == "background") {
				auto ptr = make_element_by_type<draggable_target>(state, id);
				ptr->base_data.size = base_data.size;
				return ptr;
			} else {
				return nullptr;
			}
		}

		message_result get(sys::state& state, Cyto::Any& payload) noexcept override {
			if(payload.holds_type<gp_choice_select>()) {
				selected_gp = any_cast<gp_choice_select>(payload).value;
				impl_on_update(state);
				return message_result::consumed;
			} else if(payload.holds_type<dcon::nation_id>()) {
				payload.emplace<dcon::nation_id>(action_target);
				return message_result::consumed;
			} else if(payload.holds_type<gp_choice_get_selection>()) {
			payload.emplace<gp_choice_get_selection>(gp_choice_get_selection{ selected_gp });
				return message_result::consumed;
			} else if(payload.holds_type<gp_choice_actions>()) {
				payload.emplace<gp_choice_actions>(current_action);
				return message_result::consumed;
			}
			return window_element_base::get(state, payload);
		}
	};

	class diplomacy_action_state_transfer_button : public button_element_base {
		public:
		void on_create(sys::state& state) noexcept override {
			base_data.data.button.txt = state.lookup_key("state_transfer_button");
			button_element_base::on_create(state);
		}

		void on_update(sys::state& state) noexcept override {
			auto content = retrieve<dcon::nation_id>(state, parent);
			disabled = true;
			for(const auto s : state.world.nation_get_state_ownership(content)) {
				if(command::can_state_transfer(state, state.local_player_nation, content, s.get_state().get_definition())) {
					disabled = false;
					break;
				}
			}
		}

		void button_action(sys::state& state) noexcept override {
			auto content = retrieve<dcon::nation_id>(state, parent);

			sys::state_selection_data seldata;
			seldata.single_state_select = true;
			for(const auto s : state.world.nation_get_state_ownership(state.local_player_nation)) {
				if(command::can_state_transfer(state, state.local_player_nation, content, s.get_state().get_definition())) {
					seldata.selectable_states.push_back(s.get_state().get_definition());
				}
			}
			seldata.on_select = [this, content](sys::state& state, dcon::state_definition_id sdef) {
				command::state_transfer(state, state.local_player_nation, content, sdef);
				impl_on_update(state);
			};
			seldata.on_cancel = [this](sys::state& state) {
				impl_on_update(state);
			};
			state.start_state_selection(seldata);
		}

		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			auto target = retrieve<dcon::nation_id>(state, parent);
			auto source = state.local_player_nation;

			text::add_line(state, contents, "state_transfer_desc");
			if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::state_transfer)].limit; k) {
				text::add_line_break_to_layout(state, contents);
				ui::trigger_description(state, contents, k, -1, trigger::to_generic(source), trigger::to_generic(target));
			}
			if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::state_transfer)].effect; k) {
				auto const r_lo = uint32_t(source.value);
				auto const r_hi = uint32_t(source.index() ^ (target.index() << 4));
				text::add_line_break_to_layout(state, contents);
				ui::effect_description(state, contents, k, -1, trigger::to_generic(source), trigger::to_generic(target), r_lo, r_hi);
			}
		}
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
