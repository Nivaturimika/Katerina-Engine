#include "parsers_declarations.hpp"
#include "system_state.hpp"
#include "container_types.hpp"
#include "parsers.hpp"

namespace parsers {
	game_rules_action_context::game_rules_action_context(scenario_building_context& outer_context) : outer_context(outer_context) {

	}

	void game_rule_action::type(association_type, std::string_view v, error_handler& err, int32_t line, game_rules_action_context& context) {
		if(false) { /* ... */ }
#define CT_STRING_ENUM(X) else if(is_fixed_token_ci(v.data(), v.data() + v.length(), #X)) { context.action_type = sys::static_game_rule::X; }
		CT_STRING_ENUM(state_transfer)
		CT_STRING_ENUM(start_research)
		CT_STRING_ENUM(increase_relations)
		CT_STRING_ENUM(decrease_relations)
		CT_STRING_ENUM(factory_construction)
		CT_STRING_ENUM(unit_construction)
		CT_STRING_ENUM(delete_factory)
		CT_STRING_ENUM(release_vassal)
		CT_STRING_ENUM(start_election)
		CT_STRING_ENUM(change_influence_priority)
		CT_STRING_ENUM(discredit_advisors)
		CT_STRING_ENUM(expel_advisors)
		CT_STRING_ENUM(ban_embassy)
		CT_STRING_ENUM(increase_opinion)
		CT_STRING_ENUM(decrease_opinion)
		CT_STRING_ENUM(add_to_sphere)
		CT_STRING_ENUM(remove_from_sphere)
		CT_STRING_ENUM(intervene_in_war)
		CT_STRING_ENUM(civilize)
		CT_STRING_ENUM(become_interested_in_crisis)
		CT_STRING_ENUM(change_stockpile_settings)
		CT_STRING_ENUM(fabricate_cb)
		CT_STRING_ENUM(give_military_access)
		CT_STRING_ENUM(ask_military_access)
		CT_STRING_ENUM(release_subject)
		CT_STRING_ENUM(add_war_goal)
		CT_STRING_ENUM(declare_war)
		CT_STRING_ENUM(land_rally_point)
		CT_STRING_ENUM(naval_rally_point)
		CT_STRING_ENUM(release_and_play_as)
		CT_STRING_ENUM(release_and_make_vassal)
		CT_STRING_ENUM(on_colonial_migration)
		CT_STRING_ENUM(on_immigration)
		CT_STRING_ENUM(on_internal_migration)
#undef CT_STRING_ENUM
	}

	void game_rule_action::finish(game_rules_action_context& context) {
		if(context.action_type != sys::static_game_rule::count) {
			auto& gr = context.outer_context.state.national_definitions.static_game_rules[uint8_t(context.action_type)];
			gr.limit = limit;
			gr.effect = effect;
		}
	}

	game_rule_action make_game_rule_action(token_generator& gen, error_handler& err, scenario_building_context& context) {
		game_rules_action_context a_context{ context };
		return parse_game_rule_action(gen, err, a_context);
	}

	dcon::trigger_key make_game_rule_trigger(token_generator& gen, error_handler& err, game_rules_action_context& context) {
		trigger_building_context t_context{ context.outer_context, trigger::slot_contents::empty, trigger::slot_contents::empty, trigger::slot_contents::empty };
		if(context.action_type == sys::static_game_rule::state_transfer) {
			t_context.main_slot = trigger::slot_contents::state;
			t_context.this_slot = trigger::slot_contents::nation;
			t_context.from_slot = trigger::slot_contents::nation;
		} else if(context.action_type == sys::static_game_rule::factory_construction
			|| context.action_type == sys::static_game_rule::unit_construction
			|| context.action_type == sys::static_game_rule::land_rally_point
			|| context.action_type == sys::static_game_rule::naval_rally_point) {
			t_context.main_slot = trigger::slot_contents::nation;
			t_context.this_slot = trigger::slot_contents::nation;
			t_context.from_slot = trigger::slot_contents::province;
		} else if(context.action_type == sys::static_game_rule::start_research
			|| context.action_type == sys::static_game_rule::delete_factory
			|| context.action_type == sys::static_game_rule::start_election
			|| context.action_type == sys::static_game_rule::change_influence_priority
			|| context.action_type == sys::static_game_rule::add_to_sphere
			|| context.action_type == sys::static_game_rule::civilize
			|| context.action_type == sys::static_game_rule::become_interested_in_crisis
			|| context.action_type == sys::static_game_rule::change_stockpile_settings) {
			t_context.main_slot = trigger::slot_contents::nation;
			t_context.this_slot = trigger::slot_contents::nation;
		} else {
			t_context.main_slot = trigger::slot_contents::nation;
			t_context.this_slot = trigger::slot_contents::nation;
			t_context.from_slot = trigger::slot_contents::nation;
		}
		return make_trigger(gen, err, t_context);
	}

	dcon::effect_key make_game_rule_effect(token_generator& gen, error_handler& err, game_rules_action_context& context) {
		effect_building_context t_context{ context.outer_context, trigger::slot_contents::empty, trigger::slot_contents::empty, trigger::slot_contents::empty };
		if(context.action_type == sys::static_game_rule::state_transfer) {
			t_context.main_slot = trigger::slot_contents::state;
			t_context.this_slot = trigger::slot_contents::nation;
			t_context.from_slot = trigger::slot_contents::nation;
		} else if(context.action_type == sys::static_game_rule::factory_construction
			|| context.action_type == sys::static_game_rule::unit_construction) {
			t_context.main_slot = trigger::slot_contents::nation;
			t_context.this_slot = trigger::slot_contents::nation;
			t_context.from_slot = trigger::slot_contents::province;
		} else if(context.action_type == sys::static_game_rule::start_research
			|| context.action_type == sys::static_game_rule::delete_factory
			|| context.action_type == sys::static_game_rule::start_election
			|| context.action_type == sys::static_game_rule::change_influence_priority
			|| context.action_type == sys::static_game_rule::add_to_sphere
			|| context.action_type == sys::static_game_rule::civilize
			|| context.action_type == sys::static_game_rule::become_interested_in_crisis
			|| context.action_type == sys::static_game_rule::change_stockpile_settings) {
			t_context.main_slot = trigger::slot_contents::nation;
			t_context.this_slot = trigger::slot_contents::nation;
		} else {
			t_context.main_slot = trigger::slot_contents::nation;
			t_context.this_slot = trigger::slot_contents::nation;
			t_context.from_slot = trigger::slot_contents::nation;
		}
		return make_effect(gen, err, t_context);
	}

	void game_rules_file::action(game_rule_action value, error_handler& err, int32_t line, scenario_building_context&) {

	}

	void game_rules_file::finish(scenario_building_context&) {

	}
}
