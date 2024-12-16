#include "system_state.hpp"
#include "dcon_generated.hpp"
#include "demographics.hpp"
#include "gui_element_types.hpp"
#include "gui_element_templates.hpp"
#include "gui_graphics.hpp"
#include "text.hpp"
#include "parsers.hpp"

#include "gui_common_elements.hpp"
#include "gui_diplomacy_actions_window.hpp"
#include "gui_pick_wargoal_window.hpp"
#include "gui_peace_window.hpp"
#include "gui_crisis_window.hpp"
#include "gui_diplomacy_window.hpp"

namespace ui {
	int32_t calculate_partial_score(sys::state& state, dcon::nation_id target, dcon::cb_type_id id, dcon::state_definition_id state_def, dcon::national_identity_id second_nation) {
		int32_t cost = -1;

		auto war = military::find_war_between(state, state.local_player_nation, target);
		if(!military::cb_requires_selection_of_a_state(state, id) && !military::cb_requires_selection_of_a_liberatable_tag(state, id) && !military::cb_requires_selection_of_a_valid_nation(state, id)) {

		cost = military::peace_cost(state, military::find_war_between(state, state.local_player_nation, target), id, state.local_player_nation, target, dcon::nation_id{}, dcon::state_definition_id{}, dcon::national_identity_id{});
		} else if(military::cb_requires_selection_of_a_state(state, id)) {

			if(state_def) {
				if(military::cb_requires_selection_of_a_liberatable_tag(state, id)) {
					if(!second_nation) {
						return -1;
					}
				} else if(military::cb_requires_selection_of_a_valid_nation(state, id)) {
					if(!second_nation) {
						return -1;
					}
				}

				cost = 0;

				// for each state ...
				if(war) {
					auto is_attacker = military::is_attacker(state, war, state.local_player_nation);
					for(auto si : state.world.in_state_instance) {
						if(si.get_definition() == state_def) {
							auto wr = military::get_role(state, war, si.get_nation_from_state_ownership());
							if((is_attacker && wr == military::war_role::defender) || (!is_attacker && wr == military::war_role::attacker)) {
								if(military::cb_requires_selection_of_a_liberatable_tag(state, id)) {
								cost += military::peace_cost(state, war, id, state.local_player_nation, si.get_nation_from_state_ownership(), dcon::nation_id{}, state_def, second_nation);
								} else if(military::cb_requires_selection_of_a_valid_nation(state, id)) {
								cost += military::peace_cost(state, war, id, state.local_player_nation, si.get_nation_from_state_ownership(), state.world.national_identity_get_nation_from_identity_holder(second_nation), state_def, dcon::national_identity_id{});
								} else {
								cost += military::peace_cost(state, war, id, state.local_player_nation, si.get_nation_from_state_ownership(), dcon::nation_id{}, state_def, dcon::national_identity_id{});
								}
							}
						}
					}
				} else {
					for(auto si : state.world.in_state_instance) {
						if(si.get_definition() == state_def) {
							auto n = si.get_nation_from_state_ownership();
							auto no = n.get_overlord_as_subject().get_ruler();
							if(n == target || no == target) {
								if(military::cb_requires_selection_of_a_liberatable_tag(state, id)) {
								cost += military::peace_cost(state, dcon::war_id{}, id, state.local_player_nation, si.get_nation_from_state_ownership(), dcon::nation_id{}, state_def, second_nation);
								} else if(military::cb_requires_selection_of_a_valid_nation(state, id)) {
								cost += military::peace_cost(state, dcon::war_id{}, id, state.local_player_nation, si.get_nation_from_state_ownership(), state.world.national_identity_get_nation_from_identity_holder(second_nation), state_def, dcon::national_identity_id{});
								} else {
								cost += military::peace_cost(state, dcon::war_id{}, id, state.local_player_nation, si.get_nation_from_state_ownership(), dcon::nation_id{}, state_def, dcon::national_identity_id{});
								}
							}
						}
					}
				}
			}
		} else if(military::cb_requires_selection_of_a_liberatable_tag(state, id)) {
			if(second_nation) {
			cost = military::peace_cost(state, military::find_war_between(state, state.local_player_nation, target), id, state.local_player_nation, target, dcon::nation_id{}, dcon::state_definition_id{}, second_nation);
			}
		} else if(military::cb_requires_selection_of_a_valid_nation(state, id)) {
			if(second_nation) {
			cost = military::peace_cost(state, military::find_war_between(state, state.local_player_nation, target), id, state.local_player_nation, target, state.world.national_identity_get_nation_from_identity_holder(second_nation), dcon::state_definition_id{}, dcon::national_identity_id{});
			}
		}
		return cost;
	}

	void explain_influence(sys::state& state, dcon::nation_id target, text::columnar_layout& contents) {
		int32_t total_influence_shares = 0;
		auto n = fatten(state.world, state.local_player_nation);

		for(auto rel : state.world.nation_get_gp_relationship_as_great_power(state.local_player_nation)) {
			if(nations::can_accumulate_influence_with(state, state.local_player_nation, rel.get_influence_target(), rel)) {
				switch(rel.get_status() & nations::influence::priority_mask) {
				case nations::influence::priority_one:
					total_influence_shares += 1;
					break;
				case nations::influence::priority_two:
					total_influence_shares += 2;
					break;
				case nations::influence::priority_three:
					total_influence_shares += 3;
					break;
				default:
				case nations::influence::priority_zero:
					break;
				}
			}
		}

		auto rel = fatten(state.world, state.world.get_gp_relationship_by_gp_influence_pair(target, state.local_player_nation));

		if((state.world.gp_relationship_get_status(rel) & nations::influence::is_banned) != 0) {
			text::add_line(state, contents, "influence_explain_1");
			return;
		}
		if(military::has_truce_with(state, state.local_player_nation, target)) {
			text::add_line(state, contents, "influence_explain_2");
			return;
		}
		if(military::are_at_war(state, state.local_player_nation, target)) {
			text::add_line(state, contents, "influence_explain_3");
			return;
		}

		if(total_influence_shares > 0) {
			float total_gain = state.defines.base_greatpower_daily_influence
			* (1.0f + n.get_modifier_values(sys::national_mod_offsets::influence_modifier))
			* (1.0f + n.get_modifier_values(sys::national_mod_offsets::influence));
			float gp_score = n.get_industrial_score() + n.get_military_score() + nations::prestige_score(state, n);
			float base_shares = nations::get_base_shares(state, rel, total_gain, total_influence_shares);

			float total_fi = nations::get_foreign_investment(state, n);
			auto gp_invest = state.world.unilateral_relationship_get_foreign_investment(
			state.world.get_unilateral_relationship_by_unilateral_pair(target, n));

			float discredit_factor = (rel.get_status() & nations::influence::is_discredited) != 0
				? state.defines.discredit_influence_gain_factor
				: 0.0f;
			float neighbor_factor = bool(state.world.get_nation_adjacency_by_nation_adjacency_pair(n, target))
				? state.defines.neighbour_bonus_influence_percent
				: 0.0f;
			float sphere_neighbor_factor = nations::has_sphere_neighbour(state, n, target)
				? state.defines.sphere_neighbour_bonus_influence_percent
				: 0.0f;
			float continent_factor =
				n.get_capital().get_continent() != state.world.nation_get_capital(target).get_continent()
				? state.defines.other_continent_bonus_influence_percent
				: 0.0f;
			float puppet_factor = dcon::fatten(state.world, state.world.nation_get_overlord_as_subject(target)).get_ruler() == n
				? state.defines.puppet_bonus_influence_percent
				: 0.0f;
			float relationship_factor =
			state.world.diplomatic_relation_get_value(state.world.get_diplomatic_relation_by_diplomatic_pair(n, target)) / state.defines.relation_influence_modifier;

			float investment_factor =
				total_fi > 0.0f
				? state.defines.investment_influence_defense * gp_invest / total_fi
				: 0.0f;
			float pop_factor =
				state.world.nation_get_demographics(target, demographics::total) > state.defines.large_population_limit
				? state.defines.large_population_influence_penalty * state.world.nation_get_demographics(target, demographics::total) / state.defines.large_population_influence_penalty_chunk
				: 0.0f;
			float score_factor =
				gp_score > 0.0f
				? std::max(1.0f - (state.world.nation_get_industrial_score(target) + state.world.nation_get_military_score(target) + nations::prestige_score(state, target)) / gp_score, 0.0f)
				: 0.0f;

			float total_multiplier = 1.0f + discredit_factor + neighbor_factor + sphere_neighbor_factor + continent_factor + puppet_factor + relationship_factor + investment_factor + pop_factor + score_factor;

			auto gain_amount = std::max(0.0f, base_shares * total_multiplier);
			text::add_line(state, contents, "remove_diplomacy_dailyinflulence_gain", text::variable_type::num, text::fp_two_places{ gain_amount }, text::variable_type::country, target);
			text::add_line_break_to_layout(state, contents);
			text::add_line(state, contents, "influence_explain_4", text::variable_type::x, text::fp_two_places{ total_gain });
			text::add_line(state, contents, "influence_explain_5", text::variable_type::x, text::fp_two_places{ state.defines.base_greatpower_daily_influence }, text::variable_type::y, text::fp_percentage{ 1.0f + n.get_modifier_values(sys::national_mod_offsets::influence_modifier) }, text::variable_type::val, text::fp_percentage{ 1.0f + n.get_modifier_values(sys::national_mod_offsets::influence) });
			text::add_line(state, contents, "influence_explain_6", text::variable_type::x, text::fp_percentage{ 1.0f + n.get_modifier_values(sys::national_mod_offsets::influence_modifier) });
			active_modifiers_description(state, contents, n, 15, sys::national_mod_offsets::influence_modifier, false);
			text::add_line(state, contents, "influence_explain_7", text::variable_type::x, text::fp_percentage{ 1.0f + n.get_modifier_values(sys::national_mod_offsets::influence) });
			active_modifiers_description(state, contents, n, 15, sys::national_mod_offsets::influence, false);
			text::add_line_break_to_layout(state, contents);
			text::add_line(state, contents, "influence_explain_8", text::variable_type::x, text::fp_two_places{ base_shares });
			if(discredit_factor != 0 || neighbor_factor != 0 || sphere_neighbor_factor != 0 || continent_factor != 0 || puppet_factor != 0 || relationship_factor != 0 || investment_factor != 0 || pop_factor != 0 || score_factor != 0) {
				text::add_line(state, contents, "influence_explain_9");
				if(discredit_factor != 0) {
					text::add_line(state, contents, "influence_explain_10", text::variable_type::x, text::fp_two_places{ discredit_factor }, 15);
				}
				if(neighbor_factor != 0) {
					text::add_line(state, contents, "influence_explain_11", text::variable_type::x, text::fp_two_places{ neighbor_factor }, 15);
				}
				if(sphere_neighbor_factor != 0) {
					text::add_line(state, contents, "influence_explain_12", text::variable_type::x, text::fp_two_places{ sphere_neighbor_factor }, 15);
				}
				if(continent_factor != 0) {
					text::add_line(state, contents, "influence_explain_13", text::variable_type::x, text::fp_two_places{ continent_factor }, 15);
				}
				if(puppet_factor != 0) {
					text::add_line(state, contents, "influence_explain_14", text::variable_type::x, text::fp_two_places{ puppet_factor }, 15);
				}
				if(relationship_factor != 0) {
					text::add_line(state, contents, "influence_explain_15", text::variable_type::x, text::fp_two_places{ relationship_factor }, 15);
				}
				if(investment_factor != 0) {
					text::add_line(state, contents, "influence_explain_16", text::variable_type::x, text::fp_two_places{ investment_factor }, 15);
				}
				if(pop_factor != 0) {
					text::add_line(state, contents, "influence_explain_17", text::variable_type::x, text::fp_two_places{ pop_factor }, 15);
				}
				if(score_factor != 0) {
					text::add_line(state, contents, "influence_explain_18", text::variable_type::x, text::fp_two_places{ score_factor }, 15);
				}
			}
		}
	}

	void diplomacy_priority_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto nation_id = retrieve<dcon::nation_id>(state, parent);
		if(!nations::is_great_power(state, state.local_player_nation)) {
			text::add_line(state, contents, "diplomacy_cannot_set_prio");
		} else if(nations::is_great_power(state, nation_id)) {
			text::add_line(state, contents, "diplomacy_cannot_set_prio_gp");
		} else {
			explain_influence(state, nation_id, contents);
		}
		auto box = text::open_layout_box(contents, 0);
		text::add_divider_to_layout_box(state, contents, box);
		text::localised_format_box(state, contents, box, std::string_view("diplomacy_set_prio_desc"));
		text::close_layout_box(contents, box);
		active_modifiers_description(state, contents, state.local_player_nation, 0, sys::national_mod_offsets::influence, false);
	}

	void wargoal_icon::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto wg = retrieve<dcon::wargoal_id>(state, parent);
		auto cb = state.world.wargoal_get_type(wg);
		text::add_line(state, contents, state.world.cb_type_get_name(cb));
		text::add_line_break_to_layout(state, contents);
		text::add_line(state, contents, "war_goal_1", text::variable_type::x, state.world.wargoal_get_added_by(wg));
		text::add_line(state, contents, "war_goal_2", text::variable_type::x, state.world.wargoal_get_target_nation(wg));
		if(state.world.wargoal_get_associated_state(wg)) {
			text::add_line(state, contents, "war_goal_3", text::variable_type::x, state.world.wargoal_get_associated_state(wg));
		}
		if(state.world.wargoal_get_associated_tag(wg)) {
			text::add_line(state, contents, "war_goal_10", text::variable_type::x, state.world.wargoal_get_associated_tag(wg));
		} else if(state.world.wargoal_get_secondary_nation(wg)) {
			text::add_line(state, contents, "war_goal_4", text::variable_type::x, state.world.wargoal_get_secondary_nation(wg));
		}
		if(state.world.wargoal_get_ticking_war_score(wg) != 0) {
			text::add_line(state, contents, "war_goal_5", text::variable_type::x, text::fp_one_place{state.world.wargoal_get_ticking_war_score(wg)});
			{
				auto box = text::open_layout_box(contents);
				text::substitution_map sub{};
				text::add_to_substitution_map(sub, text::variable_type::x, text::fp_percentage{ state.defines.tws_fulfilled_idle_space });
				text::add_to_substitution_map(sub, text::variable_type::y, text::fp_two_places{ state.defines.tws_fulfilled_speed });
				text::localised_format_box(state, contents, box, "war_goal_6", sub);
				text::close_layout_box(contents, box);
			}
			{
				auto box = text::open_layout_box(contents);
				text::substitution_map sub{};
				text::add_to_substitution_map(sub, text::variable_type::x, text::pretty_integer{ int32_t(state.defines.tws_grace_period_days) });
				text::add_to_substitution_map(sub, text::variable_type::y, text::fp_two_places{ state.defines.tws_not_fulfilled_speed });
				text::localised_format_box(state, contents, box, "war_goal_7", sub);
				text::close_layout_box(contents, box);
			}
			auto const start_date = state.world.war_get_start_date(state.world.wargoal_get_war_from_wargoals_attached(wg));
			auto const end_date = start_date + int32_t(state.defines.tws_grace_period_days);
			auto box = text::open_layout_box(contents);
			text::substitution_map sub{};
			text::add_to_substitution_map(sub, text::variable_type::x, start_date);
			text::add_to_substitution_map(sub, text::variable_type::y, end_date);
			if(end_date <= state.current_date) {
				text::localised_format_box(state, contents, box, "war_goal_9", sub);
			} else {
				text::localised_format_box(state, contents, box, "war_goal_8", sub);
			}
			text::close_layout_box(contents, box);
		}
	}

	void war_score_progress_bar::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto war = retrieve<dcon::war_id>(state, parent);
		text::add_line(state, contents, "war_score_1", text::variable_type::x, text::fp_one_place{military::primary_warscore_from_occupation(state, war)});
		text::add_line(state, contents, "war_score_2", text::variable_type::x, text::fp_one_place{military::primary_warscore_from_battles(state, war)});
		text::add_line(state, contents, "war_score_3", text::variable_type::x, text::fp_one_place{military::primary_warscore_from_war_goals(state, war)});
		text::add_line(state, contents, "war_score_4", text::variable_type::x, text::fp_one_place{ military::primary_warscore_from_blockades(state, war) });
		//if(state.cheat_data.show_province_id_tooltip) {
		//	text::add_line_with_condition(state, contents, "ai_defender_will_surrender", ai::ai_will_accept_alliance(state, , dcon::nation_id from));
		//}
	}
}
