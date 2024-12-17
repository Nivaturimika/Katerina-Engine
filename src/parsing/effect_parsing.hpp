#pragma once
#include "nations.hpp"
#include "parsers.hpp"
#include "script_constants.hpp"
#include "system_state.hpp"
#include "trigger_parsing.hpp"
#include "text.hpp"
#include "triggers.hpp"

namespace economy {
	dcon::modifier_id get_province_selector_modifier(sys::state& state);
	dcon::modifier_id get_province_immigrator_modifier(sys::state& state);
}

namespace parsers {

	struct effect_building_context {
		scenario_building_context& outer_context;
		std::vector<uint16_t> compiled_effect;
		size_t limit_position = 0;
		bool effect_is_for_event = false;

		trigger::slot_contents main_slot = trigger::slot_contents::empty;
		trigger::slot_contents this_slot = trigger::slot_contents::empty;
		trigger::slot_contents from_slot = trigger::slot_contents::empty;

		effect_building_context(scenario_building_context& outer_context, trigger::slot_contents main_slot,
			trigger::slot_contents this_slot, trigger::slot_contents from_slot)
			: outer_context(outer_context), main_slot(main_slot), this_slot(this_slot), from_slot(from_slot) { }

		void add_float_to_payload(float f) {
			union {
				struct {
					uint16_t low;
					uint16_t high;
				} v;
				float f;
			} pack_float;
			pack_float.f = f;
			compiled_effect.push_back(pack_float.v.low);
			compiled_effect.push_back(pack_float.v.high);
		}

		void add_int32_t_to_payload(int32_t i) {
			union {
				struct {
					uint16_t low;
					uint16_t high;
				} v;
				int32_t i;
			} pack_int;
			pack_int.i = i;
			compiled_effect.push_back(pack_int.v.low);
			compiled_effect.push_back(pack_int.v.high);
		}
	};

	struct ef_trigger_revolt {
		dcon::culture_id culture_;
		dcon::religion_id religion_;
		dcon::ideology_id ideology_;
		dcon::rebel_type_id type_;
		void culture(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void religion(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void ideology(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void type(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void finish(effect_building_context&) { }
	};

	struct ef_diplomatic_influence {
		std::string_view who;
		int32_t value = 0;
		void finish(effect_building_context&) { }
	};
	struct ef_relation {
		std::string_view who;
		int32_t value = 0;
		void finish(effect_building_context&) { }
	};
	struct ef_add_province_modifier {
		dcon::modifier_id name_;
		int32_t duration = 0;
		void name(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_modifiers.find(std::string(value));
				it != context.outer_context.map_of_modifiers.end()) {
				name_ = it->second;
			} else {
				err.accumulated_errors += "Invalid modifier " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void finish(effect_building_context&) { }
	};
	struct ef_add_country_modifier {
		dcon::modifier_id name_;
		int32_t duration = 0;
		void name(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_modifiers.find(std::string(value));
				it != context.outer_context.map_of_modifiers.end()) {
				name_ = it->second;
			} else {
				err.accumulated_errors += "Invalid modifier " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void finish(effect_building_context&) { }
	};
	struct ef_casus_belli {
		dcon::cb_type_id type_;
		std::string_view target;
		int32_t months = 0;
		void type(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_cb_types.find(std::string(value));
				it != context.outer_context.map_of_cb_types.end()) {
				type_ = it->second.id;
			} else {
				err.accumulated_errors += "Invalid cb type " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void finish(effect_building_context&) { }
	};
	struct ef_add_casus_belli {
		dcon::cb_type_id type_;
		std::string_view target;
		int32_t months = 0;
		void type(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_cb_types.find(std::string(value));
				it != context.outer_context.map_of_cb_types.end()) {
				type_ = it->second.id;
			} else {
				err.accumulated_errors += "Invalid cb type " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void finish(effect_building_context&) { }
	};
	struct ef_remove_casus_belli {
		dcon::cb_type_id type_;
		std::string_view target;
		void type(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_cb_types.find(std::string(value));
				it != context.outer_context.map_of_cb_types.end()) {
				type_ = it->second.id;
			} else {
				err.accumulated_errors += "Invalid cb type " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void finish(effect_building_context&) { }
	};
	struct ef_this_remove_casus_belli {
		dcon::cb_type_id type_;
		std::string_view target;
		void type(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_cb_types.find(std::string(value));
				it != context.outer_context.map_of_cb_types.end()) {
				type_ = it->second.id;
			} else {
				err.accumulated_errors += "Invalid cb type " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void finish(effect_building_context&) { }
	};
	struct ef_wargoal {
		dcon::cb_type_id casus_belli_;
		dcon::national_identity_id country_;
		bool target_country_is_this = false;
		bool target_country_is_from = false;
		dcon::province_id state_province_id_;
		bool special_end_wargoal = false;
		bool special_call_ally_wargoal = false;

		void country(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
			if(is_this(value)) {
				target_country_is_this = true;
			} else if(is_from(value)) {
				target_country_is_from = true;
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					country_ = it->second;
				} else {
					err.accumulated_errors += "wargoal given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors += "wargoal given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		}
		void casus_belli(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
			if(is_fixed_token_ci(value.data(), value.data() + value.length(), "test_end_war")) {
				special_end_wargoal = true;
			} else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "call_allies_cb")) {
				special_call_ally_wargoal = true;
			} else if(auto it = context.outer_context.map_of_cb_types.find(std::string(value));
				it != context.outer_context.map_of_cb_types.end()) {
				casus_belli_ = it->second.id;
			} else {
				err.accumulated_errors +=
					"Invalid cb type " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void state_province_id(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context) {
			if(0 <= value && size_t(value) < context.outer_context.original_id_to_prov_id_map.size()) {
				state_province_id_ = context.outer_context.original_id_to_prov_id_map[value];
			} else {
				err.accumulated_errors += "wargoal given an invalid province id (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			}
		}
		void finish(effect_building_context&) { }
	};
	struct ef_war {
		bool call_ally = true;
		std::string_view target;
		ef_wargoal defender_goal;
		ef_wargoal attacker_goal;
		void finish(effect_building_context&) { }
	};
	struct ef_country_event {
		int32_t days = -1;
		dcon::national_event_id id_;
		void id(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context);
		void finish(effect_building_context&) { }
	};
	struct ef_province_event {
		int32_t days = -1;
		dcon::provincial_event_id id_;
		void id(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context);
		void finish(effect_building_context&) { }
	};
	struct ef_sub_unit {
		std::string_view value;
		dcon::unit_type_id type_;
		void type(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_unit_types.find(std::string(v));
				it != context.outer_context.map_of_unit_types.end()) {
				type_ = it->second;
			} else {
				err.accumulated_errors += "Invalid unit type " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void finish(effect_building_context&) { }
	};
	struct ef_set_variable {
		float value = 0.0f;
		dcon::national_variable_id which_;
		void which(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
			which_ = context.outer_context.get_national_variable(std::string(v));
		}
		void finish(effect_building_context&) { }
	};
	struct ef_change_variable {
		float value = 0.0f;
		dcon::national_variable_id which_;
		void which(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
			which_ = context.outer_context.get_national_variable(std::string(v));
		}
		void finish(effect_building_context&) { }
	};
	struct ef_ideology {
		float factor = 0.0f;
		dcon::ideology_id value_;
		void value(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_ideologies.find(std::string(v));
				it != context.outer_context.map_of_ideologies.end()) {
				value_ = it->second.id;
			} else {
				err.accumulated_errors += "Invalid ideology " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void finish(effect_building_context&) { }
	};
	struct ef_dominant_issue {
		float factor = 0.0f;
		dcon::issue_option_id value_;
		void value(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_ioptions.find(std::string(v)); it != context.outer_context.map_of_ioptions.end()) {
				value_ = it->second.id;
			} else {
				err.accumulated_errors += "Invalid issue option " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void finish(effect_building_context&) { }
	};
	struct ef_upper_house {
		float value = 0.0f;
		dcon::ideology_id ideology_;
		void ideology(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_ideologies.find(std::string(v));
				it != context.outer_context.map_of_ideologies.end()) {
				ideology_ = it->second.id;
			} else {
				err.accumulated_errors += "Invalid ideology " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void finish(effect_building_context&) { }
	};
	struct ef_scaled_militancy {
		float factor = 0.0f;
		float unemployment = 0.0f;
		dcon::ideology_id ideology_;
		dcon::issue_option_id issue_;
		void ideology(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_ideologies.find(std::string(v));
				it != context.outer_context.map_of_ideologies.end()) {
				ideology_ = it->second.id;
			} else {
				err.accumulated_errors += "Invalid ideology " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void issue(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_ioptions.find(std::string(v)); it != context.outer_context.map_of_ioptions.end()) {
				issue_ = it->second.id;
			} else {
				err.accumulated_errors += "Invalid issue option " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void finish(effect_building_context&) { }
	};
	struct ef_scaled_consciousness {
		float factor = 0.0f;
		float unemployment = 0.0f;
		dcon::ideology_id ideology_;
		dcon::issue_option_id issue_;
		void ideology(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_ideologies.find(std::string(v));
				it != context.outer_context.map_of_ideologies.end()) {
				ideology_ = it->second.id;
			} else {
				err.accumulated_errors += "Invalid ideology " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void issue(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_ioptions.find(std::string(v)); it != context.outer_context.map_of_ioptions.end()) {
				issue_ = it->second.id;
			} else {
				err.accumulated_errors += "Invalid issue option " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void finish(effect_building_context&) { }
	};
	struct ef_define_general {
		std::string_view name;
		dcon::leader_trait_id background_;
		dcon::leader_trait_id personality_;
		void background(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_leader_traits.find(std::string(v));
				it != context.outer_context.map_of_leader_traits.end()) {
				background_ = it->second;
			} else {
				err.accumulated_errors += "Invalid leader trait " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void personality(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_leader_traits.find(std::string(v));
				it != context.outer_context.map_of_leader_traits.end()) {
				personality_ = it->second;
			} else {
				err.accumulated_errors += "Invalid leader trait " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void finish(effect_building_context&) { }
	};
	struct ef_define_admiral {
		std::string_view name;
		dcon::leader_trait_id background_;
		dcon::leader_trait_id personality_;
		void background(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_leader_traits.find(std::string(v));
				it != context.outer_context.map_of_leader_traits.end()) {
				background_ = it->second;
			} else {
				err.accumulated_errors += "Invalid leader trait " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void personality(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_leader_traits.find(std::string(v));
				it != context.outer_context.map_of_leader_traits.end()) {
				personality_ = it->second;
			} else {
				err.accumulated_errors += "Invalid leader trait " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void finish(effect_building_context&) { }
	};
	struct ef_add_war_goal {
		dcon::cb_type_id casus_belli_;
		void casus_belli(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_cb_types.find(std::string(v)); it != context.outer_context.map_of_cb_types.end()) {
				casus_belli_ = it->second.id;
			} else {
				err.accumulated_errors += "Invalid cb type " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void finish(effect_building_context&) { }
	};
	struct ef_move_issue_percentage {
		float value = 0.0f;
		dcon::issue_option_id from_;
		dcon::issue_option_id to_;
		void from(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_ioptions.find(std::string(v)); it != context.outer_context.map_of_ioptions.end()) {
				from_ = it->second.id;
			} else {
				err.accumulated_errors += "Invalid issue option " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void to(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_ioptions.find(std::string(v)); it != context.outer_context.map_of_ioptions.end()) {
				to_ = it->second.id;
			} else {
				err.accumulated_errors += "Invalid issue option " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void finish(effect_building_context&) { }
	};
	struct ef_party_loyalty {
		float loyalty_value = 0.0f;
		dcon::province_id province_id_;
		dcon::ideology_id ideology_;
		void ideology(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
			if(auto it = context.outer_context.map_of_ideologies.find(std::string(v));
				it != context.outer_context.map_of_ideologies.end()) {
				ideology_ = it->second.id;
			} else {
				err.accumulated_errors += "Invalid ideology " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
		void province_id(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context) {
			if(0 <= value && size_t(value) < context.outer_context.original_id_to_prov_id_map.size()) {
				province_id_ = context.outer_context.original_id_to_prov_id_map[value];
			} else {
				err.accumulated_errors += "party_loyalty given an invalid province id (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			}
		}
		void finish(effect_building_context&) { }
	};
	struct ef_change_party_name {
		dcon::ideology_id ideology_;
		dcon::text_key name_;
		void ideology(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context);
		void name(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
			name_ = text::find_or_add_key(context.outer_context.state, v, false);
		}
		void finish(effect_building_context&) { }
	};
	struct ef_change_party_position {
		dcon::ideology_id ideology_;
		dcon::issue_option_id opt_;
		void ideology(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context);
		void position(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context);
		void finish(effect_building_context&) { }
	};

	struct ef_add_or_create_pop {
		float amount;
		dcon::pop_type_id pop_type_;
		dcon::culture_id culture_;
		dcon::religion_id religion_;
		void type(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context);
		void culture(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context);
		void religion(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context);
		void finish(effect_building_context&) { }
	};

	struct ef_trigger_crisis {
		bool overwrite;
		sys::crisis_type type_;
		std::string liberation_tag;
		std::string colony;
		void type(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context);
		void finish(effect_building_context&) { }
	};
	struct ef_build_railway_in_capital {
		bool limit_to_world_greatest_level = false;
		bool in_whole_capital_state = false;
		void finish(effect_building_context&) { }
	};
	struct ef_build_fort_in_capital {
		bool limit_to_world_greatest_level = false;
		bool in_whole_capital_state = false;
		void finish(effect_building_context&) { }
	};
	struct ef_build_bank_in_capital {
		bool limit_to_world_greatest_level = false;
		bool in_whole_capital_state = false;
		void finish(effect_building_context&) { }
	};
	struct ef_build_university_in_capital {
		bool limit_to_world_greatest_level = false;
		bool in_whole_capital_state = false;
		void finish(effect_building_context&) { }
	};

	struct ef_random_list {
		int32_t chances_sum = 0;
		void any_group(std::string_view label, int32_t value, error_handler& err, int32_t line, effect_building_context& context) {
			chances_sum += value;
		}
		void finish(effect_building_context&) { }
	};

	struct ef_clear_news_scopes {
		std::string type;
		dcon::trigger_key limit;
		void finish(effect_building_context&) { }
	};

	struct effect_body {
		int32_t chance = 0;
		int32_t loop_iterations = 0;
		void finish(effect_building_context&) { }
		void capital(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context);
		void add_core(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void remove_core(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void change_region_name(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void trade_goods(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void add_accepted_culture(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void primary_culture(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void remove_accepted_culture(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void life_rating(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context);
		void religion(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void is_slave(association_type t, bool value, error_handler& err, int32_t line, effect_building_context& context);
		void research_points(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context);
		void tech_school(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void government(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void treasury(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context);
		void war_exhaustion(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context);
		void prestige(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context);
		void change_tag(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void change_tag_no_core_switch(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void set_country_flag(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void clr_country_flag(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void set_province_flag(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void clr_province_flag(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void remove_crisis(association_type t, bool value, error_handler& err, int32_t line, effect_building_context& context);
		void trigger_crisis(ef_trigger_crisis value, error_handler& err, int32_t line, effect_building_context& context);
		void country_event(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context);
		void province_event(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context);
		void military_access(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void badboy(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context);
		void secede_province(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void inherit(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void annex_to(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void release(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void change_controller(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void infrastructure(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context);
		void fort(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context);
		void naval_base(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context);
		void bank(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context);
		void university(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context);
		void province_selector(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context);
		void province_immigrator(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context);
		void money(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context);
		void leadership(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context);
		void create_vassal(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void end_military_access(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void leave_alliance(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void end_war(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void enable_ideology(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void ruling_party_ideology(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void plurality(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context);
		void remove_province_modifier(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void remove_country_modifier(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void create_alliance(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void release_vassal(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void change_province_name(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void enable_canal(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context);
		void set_global_flag(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void clr_global_flag(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void nationalvalue(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void civilized(association_type t, bool value, error_handler& err, int32_t line, effect_building_context& context);
		void election(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void social_reform(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void political_reform(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void add_tax_relative_income(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context);
		void neutrality(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void reduce_pop(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context);
		void move_pop(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context);
		void pop_type(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void years_of_research(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context);
		void prestige_factor(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context);
		void military_reform(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void economic_reform(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void remove_random_military_reforms(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context);
		void remove_random_economic_reforms(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context);
		void add_crime(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void nationalize(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void build_factory_in_capital_state(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void activate_technology(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void great_wars_enabled(association_type t, bool value, error_handler& err, int32_t line, effect_building_context& context);
		void world_wars_enabled(association_type t, bool value, error_handler& err, int32_t line, effect_building_context& context);
		void assimilate(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void literacy(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context);
		void add_crisis_interest(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void flashpoint_tension(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context);
		void add_crisis_temperature(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context);
		void consciousness(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context);
		void militancy(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context);
		void rgo_size(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context);
		void add_province_modifier(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void add_country_modifier(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void trigger_revolt(ef_trigger_revolt const& value, error_handler& err, int32_t line, effect_building_context& context) ;
		void diplomatic_influence(ef_diplomatic_influence const& value, error_handler& err, int32_t line, effect_building_context& context);
		void relation(ef_relation const& value, error_handler& err, int32_t line, effect_building_context& context);
		void add_province_modifier(ef_add_province_modifier const& value, error_handler& err, int32_t line, effect_building_context& context);
		void add_country_modifier(ef_add_country_modifier const& value, error_handler& err, int32_t line, effect_building_context& context);
		void casus_belli(ef_casus_belli const& value, error_handler& err, int32_t line, effect_building_context& context);
		void add_casus_belli(ef_add_casus_belli const& value, error_handler& err, int32_t line, effect_building_context& context);
		void remove_casus_belli(ef_remove_casus_belli const& value, error_handler& err, int32_t line, effect_building_context& context);
		void this_remove_casus_belli(ef_this_remove_casus_belli const& value, error_handler& err, int32_t line, effect_building_context& context);
		void war(ef_war const& value, error_handler& err, int32_t line, effect_building_context& context);
		void country_event(ef_country_event const& value, error_handler& err, int32_t line, effect_building_context& context);
		void province_event(ef_province_event const& value, error_handler& err, int32_t line, effect_building_context& context);
		void sub_unit(ef_sub_unit const& value, error_handler& err, int32_t line, effect_building_context& context);
		void set_variable(ef_set_variable const& value, error_handler& err, int32_t line, effect_building_context& context);
		void change_variable(ef_change_variable const& value, error_handler& err, int32_t line, effect_building_context& context);
		void increment_variable(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void decrement_variable(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void set_variable_to_zero(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void change_terrain(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void masquerade_as_nation(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void change_party_name(ef_change_party_name const& value, error_handler& err, int32_t line, effect_building_context& context);
		void change_party_position(ef_change_party_position const& value, error_handler& err, int32_t line, effect_building_context& context);
		void ideology(ef_ideology const& value, error_handler& err, int32_t line, effect_building_context& context);
		void dominant_issue(ef_dominant_issue const& value, error_handler& err, int32_t line, effect_building_context& context);
		void upper_house(ef_upper_house const& value, error_handler& err, int32_t line, effect_building_context& context);
		void scaled_militancy(ef_scaled_militancy const& value, error_handler& err, int32_t line, effect_building_context& context);
		void scaled_consciousness(ef_scaled_consciousness const& value, error_handler& err, int32_t line, effect_building_context& context);
		void define_general(ef_define_general const& value, error_handler& err, int32_t line, effect_building_context& context);
		void define_admiral(ef_define_admiral const& value, error_handler& err, int32_t line, effect_building_context& context);
		void kill_leader(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void add_war_goal(ef_add_war_goal const& value, error_handler& err, int32_t line, effect_building_context& context);
		void move_issue_percentage(ef_move_issue_percentage const& value, error_handler& err, int32_t line, effect_building_context& context);
		void party_loyalty(ef_party_loyalty const& value, error_handler& err, int32_t line, effect_building_context& context);
		void build_railway_in_capital(ef_build_railway_in_capital const& value, error_handler& err, int32_t line, effect_building_context& context);
		void build_bank_in_capital(ef_build_bank_in_capital const& value, error_handler& err, int32_t line, effect_building_context& context);
		void build_university_in_capital(ef_build_university_in_capital const& value, error_handler& err, int32_t line, effect_building_context& context);
		void build_fort_in_capital(ef_build_fort_in_capital const& value, error_handler& err, int32_t line, effect_building_context& context);
		void change_pop_size(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context);
		void add_or_create_pop(ef_add_or_create_pop const& value, error_handler& err, int32_t line, effect_building_context& context);
		void any_value(std::string_view label, association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context);
		void limit(dcon::trigger_key lim, error_handler& err, int32_t line, effect_building_context& context) {
			context.compiled_effect[context.limit_position] = trigger::payload(lim).value;
		}
	};

	struct ef_scope_random_by_modifier : public effect_body {
		dcon::value_modifier_key chance_modifier;
		void finish(effect_building_context&) { }
	};
	void ef_random_by_modifier(token_generator& gen, error_handler& err, effect_building_context& context);

	struct event_option : public effect_body {
		dcon::value_modifier_key ai_chance;
		dcon::text_key name_;
		void name(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
			name_ = text::find_or_add_key(context.outer_context.state, value, false);
		}
	};

	dcon::value_modifier_key make_option_ai_chance(token_generator& gen, error_handler& err, effect_building_context& context);
	dcon::trigger_key ef_limit(token_generator& gen, error_handler& err, effect_building_context& context);

	void ef_scope_if(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_else_if(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_else(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_hidden_tooltip(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_any_neighbor_province(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_any_neighbor_country(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_random_neighbor_country(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_any_country(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_any_existing_country_except_scoped(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_any_defined_country(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_random_country(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_random_neighbor_province(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_random_empty_neighbor_province(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_any_greater_power(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_poor_strata(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_middle_strata(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_rich_strata(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_random_pop(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_random_owned(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_any_owned(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_all_core(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_any_state(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_random_state(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_any_pop(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_owner(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_controller(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_location(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_country(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_capital_scope(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_this(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_from(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_sea_zone(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_cultural_union(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_overlord(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_sphere_owner(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_independence(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_flashpoint_tag_scope(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_crisis_state_scope(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_state_scope(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_random(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_random_list(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_variable(std::string_view label, token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_any_substate(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_loop_bounded(token_generator& gen, error_handler& err, effect_building_context& context);
	dcon::value_modifier_key read_chance_modifier(token_generator& gen, error_handler& err, effect_building_context& context);
	int32_t add_to_random_list(std::string_view label, token_generator& gen, error_handler& err, effect_building_context& context);

	// missing
	void ef_scope_from_bounce(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_this_bounce(token_generator& gen, error_handler& err, effect_building_context& context);
	void lambda_country_event(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_random_greater_power(token_generator& gen, error_handler& err, effect_building_context& context);
	void lambda_province_event(token_generator& gen, error_handler& err, effect_building_context& context);
	void ef_scope_any_empty_neighbor_province(token_generator& gen, error_handler& err, effect_building_context& context);

	dcon::effect_key make_effect(token_generator& gen, error_handler& err, effect_building_context& context);
	int32_t simplify_effect(uint16_t* source);

} // namespace parsers
