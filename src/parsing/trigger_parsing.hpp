#pragma once

#include <stdint.h>
#include <optional>
#include <string_view>
#include <string>

#include "constants.hpp"
#include "parsers.hpp"
#include "script_constants.hpp"
#include "nations.hpp"
#include "container_types.hpp"

namespace economy {
	dcon::modifier_id get_province_selector_modifier(sys::state& state);
	dcon::modifier_id get_province_immigrator_modifier(sys::state& state);
}

namespace parsers {

	struct trigger_building_context {
		scenario_building_context& outer_context;
		std::vector<uint16_t> compiled_trigger;

		float factor = 0.0f;

		trigger::slot_contents main_slot = trigger::slot_contents::empty;
		trigger::slot_contents this_slot = trigger::slot_contents::empty;
		trigger::slot_contents from_slot = trigger::slot_contents::empty;

		trigger_building_context(scenario_building_context& outer_context, trigger::slot_contents main_slot,
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

			compiled_trigger.push_back(pack_float.v.low);
			compiled_trigger.push_back(pack_float.v.high);
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

			compiled_trigger.push_back(pack_int.v.low);
			compiled_trigger.push_back(pack_int.v.high);
		}
	};

	struct tr_diplomatic_influence {
		std::string_view who;
		int32_t value_ = 0;
		association_type a;
		void value(association_type t, int32_t v, error_handler& err, int32_t line, trigger_building_context& context) {
			a = t;
			value_ = v;
		}
		void finish(trigger_building_context&) { }
	};

	struct tr_party_loyalty {
		std::string_view ideology;
		float value_ = 0.0f;
		association_type a;
		int32_t province_id = 0;
		void value(association_type t, float v, error_handler& err, int32_t line, trigger_building_context& context) {
			a = t;
			value_ = v;
		}
	void finish(trigger_building_context&) { }
	};

	struct tr_unemployment_by_type {
		std::string_view type;
		float value_ = 0.0f;
		association_type a;
		void value(association_type t, float v, error_handler& err, int32_t line, trigger_building_context& context) {
			a = t;
			value_ = v;
		}
	void finish(trigger_building_context&) { }
	};

	struct tr_upper_house {
		std::string_view ideology;
		float value_ = 0.0f;
		association_type a;
		void value(association_type t, float v, error_handler& err, int32_t line, trigger_building_context& context) {
			a = t;
			value_ = v;
		}
	void finish(trigger_building_context&) { }
	};

	struct tr_check_variable {
		std::string_view which;
		float value_ = 0.0f;
		association_type a;
		void value(association_type t, float v, error_handler& err, int32_t line, trigger_building_context& context) {
			a = t;
			value_ = v;
		}
	void finish(trigger_building_context&) { }
	};

	struct tr_relation {
		std::string_view who;
		int32_t value_ = 0;
		association_type a;
		void value(association_type t, int32_t v, error_handler& err, int32_t line, trigger_building_context& context) {
			a = t;
			value_ = v;
		}
	void finish(trigger_building_context&) { }
	};

	struct tr_pop_unemployment {
		std::string_view type;
		float value_ = 0.0f;
		association_type a;
		void value(association_type t, float v, error_handler& err, int32_t line, trigger_building_context& context) {
			a = t;
			value_ = v;
		}
	void finish(trigger_building_context&) { }
	};

	struct tr_can_build_in_province {
		std::string_view building;
		bool limit_to_world_greatest_level = false;
	void finish(trigger_building_context&) { }
	};

	struct tr_can_build_fort_in_capital {
		bool in_whole_capital_state = false;
		bool limit_to_world_greatest_level = false;
	void finish(trigger_building_context&) { }
	};

	struct tr_can_build_railway_in_capital {
		bool in_whole_capital_state = false;
		bool limit_to_world_greatest_level = false;
	void finish(trigger_building_context&) { }
	};

	struct tr_work_available {
		std::vector<dcon::pop_type_id> pop_type_list;
		void worker(association_type, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
			if(auto it = context.outer_context.map_of_poptypes.find(std::string(value));
				it != context.outer_context.map_of_poptypes.end()) {
				pop_type_list.push_back(it->second);
			} else {
				err.accumulated_errors +=
					std::string(value) + " is not a valid pop type name (" + err.file_name + " line " + std::to_string(line) + ")\n";
			}
		}
	void finish(trigger_building_context&) { }
	};

	struct tr_news_data_parameters {
		std::vector<std::string> list;
		void free_value(std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
			list.push_back(std::string(value));
		}
		void finish(trigger_building_context&) { }
	};
	struct tr_party_name {
		dcon::ideology_id ideology_;
		dcon::text_key name_;
		void ideology(association_type t, std::string_view v, error_handler& err, int32_t line, trigger_building_context& context);
		void name(association_type t, std::string_view v, error_handler& err, int32_t line, trigger_building_context& context);
		void finish(trigger_building_context&) { }
	};
	struct tr_party_position {
		dcon::ideology_id ideology_;
		dcon::issue_option_id opt_;
		void ideology(association_type t, std::string_view v, error_handler& err, int32_t line, trigger_building_context& context);
		void position(association_type t, std::string_view v, error_handler& err, int32_t line, trigger_building_context& context);
		void finish(trigger_building_context&) { }
	};

	inline bool is_from(std::string_view value) {
		return is_fixed_token_ci(value.data(), value.data() + value.length(), "from");
	}
	inline bool is_this(std::string_view value) {
		return is_fixed_token_ci(value.data(), value.data() + value.length(), "this");
	}
	inline bool is_reb(std::string_view value) {
		return is_fixed_token_ci(value.data(), value.data() + value.length(), "reb");
	}

	struct trigger_body {
		void finish(trigger_building_context&) { }
		void factor(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
			context.factor = value;
		}
		void ai(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void year(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_canal_enabled(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void month(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void great_wars_enabled(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void world_wars_enabled(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void crisis_exist(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_liberation_crisis(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_claim_crisis(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void port(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void involved_in_crisis(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_cultural_sphere(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void social_movement(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void political_movement(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void rich_tax_above_poor(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_substate(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_flashpoint(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_disarmed(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void can_nationalize(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void part_of_sphere(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void constructing_cb_discovered(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void colonial_nation(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_capital(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void election(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void always(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_releasable_vassal(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void someone_can_form_union_tag(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_state_capital(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_factories(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_empty_adjacent_province(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void minorities(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void culture_has_union_tag(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_colonial(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_greater_power(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void can_create_vassals(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_recently_lost_war(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_mobilised(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void crime_higher_than_education(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void civilized(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void rank(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void crisis_temperature(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_recent_imigration(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void province_control_days(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void num_of_substates(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void num_of_vassals_no_substates(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void number_of_states(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void war_score(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void flashpoint_tension(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void life_needs(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void everyday_needs(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void luxury_needs(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void social_movement_strength(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void political_movement_strength(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void total_num_of_ports(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void agree_with_ruling_party(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void constructing_cb_progress(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void civilization_progress(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void rich_strata_life_needs(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void rich_strata_everyday_needs(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void rich_strata_luxury_needs(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void middle_strata_life_needs(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void middle_strata_everyday_needs(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void middle_strata_luxury_needs(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void poor_strata_life_needs(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void poor_strata_everyday_needs(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void poor_strata_luxury_needs(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void revanchism(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void poor_strata_militancy(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void middle_strata_militancy(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void rich_strata_militancy(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void consciousness(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void literacy(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void militancy(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void military_spending(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void administration_spending(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void education_spending(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void national_provinces_occupied(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void social_spending(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void brigades_compare(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void rich_tax(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void middle_tax(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void poor_tax(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void mobilisation_size(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void province_id(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void invention(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void big_producer(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void strata(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void life_rating(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_empty_adjacent_state(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void state_id(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void cash_reserves(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void unemployment(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_slave(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_independant(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_national_minority(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void government(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void constructing_cb_type(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void can_build_factory_in_capital_state(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void capital(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void tech_school(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void primary_culture(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void total_sunk_by_us(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_crime(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void accepted_culture(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void pop_majority_religion(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void pop_majority_culture(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void pop_majority_issue(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void pop_majority_ideology(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void trade_goods_in_state(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void culture(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_pop_culture(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_pop_religion(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void culture_group(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void religion(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void terrain(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void trade_goods(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_secondary_power(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_faction(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_unclaimed_cores(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void have_core_in(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_cultural_union(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void can_build_factory(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void war(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void war_exhaustion(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void blockade(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void owns(association_type a, std::string_view v, error_handler& err, int32_t line, trigger_building_context& context);
		void controls(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_core(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void num_of_revolts(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void revolt_percentage(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void num_of_cities(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void num_of_ports(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void num_of_allies(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void num_of_vassals(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void owned_by(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void exists(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_country_flag(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_province_flag(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_global_flag(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void continent(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void casus_belli(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void military_access(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void prestige(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void badboy(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_building(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void empty(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_blockaded(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_country_modifier(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_province_modifier(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void nationalvalue(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void region(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void tag(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void stronger_army_than(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void neighbour(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void country_units_in_state(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void units_in_province(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void war_with(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void unit_in_battle(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void unit_has_leader(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_national_focus(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void total_amount_of_divisions(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void money(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void lost_national(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_vassal(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void ruling_party_ideology(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void ruling_party(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_leader(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_ideology_enabled(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void political_reform_want(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void social_reform_want(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void total_amount_of_ships(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void plurality(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void corruption(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_state_religion(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_primary_culture(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_accepted_culture(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_coastal(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void in_sphere(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void produces(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_pop_type(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void total_pops(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void average_militancy(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void average_consciousness(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_next_reform(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void rebel_power_fraction(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void recruited_percentage(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
		void has_culture_core(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void nationalism(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_overseas(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void controlled_by_rebels(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context);
		void controlled_by(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void truce_with(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_sphere_leader_of(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void constructing_cb(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void vassal_of(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void substate_of(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_our_vassal(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void this_culture_union(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void alliance_with(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void in_default(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void industrial_score(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void military_score(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void test(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void is_possible_vassal(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void diplomatic_influence(tr_diplomatic_influence const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void pop_unemployment(tr_pop_unemployment const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void relation(tr_relation const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void check_variable(tr_check_variable const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void upper_house(tr_upper_house const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void unemployment_by_type(tr_unemployment_by_type const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void party_loyalty(tr_party_loyalty const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void can_build_in_province(tr_can_build_in_province const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void can_build_railway_in_capital(tr_can_build_railway_in_capital const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void can_build_fort_in_capital(tr_can_build_fort_in_capital const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void work_available(tr_work_available const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void tags_eq(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void strings_eq(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void values_eq(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void dates_eq(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void tags_greater(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void strings_greater(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void values_greater(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void dates_greater(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void tags_match(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void strings_match(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void values_match(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void dates_match(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void tags_contains(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void strings_contains(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void values_contains(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void dates_contains(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void length_greater(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void news_printing_count(association_type, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
		void party_name(tr_party_name const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void party_position(tr_party_position const& value, error_handler& err, int32_t line, trigger_building_context& context);
		void any_value(std::string_view label, association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
	};

	void tr_scope_and(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_or(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_not(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_any_neighbor_province(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_any_neighbor_country(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_war_countries(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_all_war_countries(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_any_greater_power(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_any_owned_province(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_any_core(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_all_core(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_any_state(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_any_substate(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_any_sphere_member(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_any_pop(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_owner(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_controller(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_location(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_country(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_capital_scope(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_this(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_from(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_sea_zone(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_cultural_union(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_overlord(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_sphere_owner(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_independence(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_flashpoint_tag_scope(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_crisis_state_scope(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_crisis_attacker_scope(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_crisis_defender_scope(token_generator& gen, error_handler& err, trigger_building_context& context); 
	void tr_state_scope(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_variable(std::string_view name, token_generator& gen, error_handler& err, trigger_building_context& context);
	//extensions
	void tr_scope_any_country(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_all_state(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_all_substate(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_all_sphere_member(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_all_pop(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_all_greater_power(token_generator& gen, error_handler& err, trigger_building_context& context);
	void tr_scope_every_country(token_generator& gen, error_handler& err, trigger_building_context& context);

	void invert_trigger(uint16_t* source);
	bool scope_is_empty(uint16_t const* source);
	bool scope_has_single_member(uint16_t const* source);
	int32_t simplify_trigger(uint16_t* source);
	dcon::trigger_key make_trigger(token_generator& gen, error_handler& err, trigger_building_context& context);

	struct value_modifier_definition {
		std::optional<float> factor;
		float base = 0.0f;
		void months(association_type, float value, error_handler& err, int32_t line, trigger_building_context& context) {
			factor = value * 30.0f;
		}
		void years(association_type, float value, error_handler& err, int32_t line, trigger_building_context& context) {
			factor = value * 365.0f;
		}
		void group(value_modifier_definition const& value, error_handler& err, int32_t line, trigger_building_context& context) { }
		void finish(trigger_building_context&) { }
	};

	void make_value_modifier_segment(token_generator& gen, error_handler& err, trigger_building_context& context);
	dcon::value_modifier_key make_value_modifier(token_generator& gen, error_handler& err, trigger_building_context& context);

	struct scripted_trigger_file {
		void finish(scenario_building_context const&) { }
	};

	struct stored_condition : public trigger_body {
		void main_parameter(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void this_parameter(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
		void from_parameter(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context);
	};

	void make_stored_trigger(std::string_view tag, token_generator& gen, error_handler& err, scenario_building_context& context);

} // namespace parsers
