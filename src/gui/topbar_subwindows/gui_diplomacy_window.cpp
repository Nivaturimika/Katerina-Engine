#include "system_state.hpp"
#include "dcon_generated.hpp"
#include "demographics.hpp"
#include "gui_element_types.hpp"
#include "gui_element_templates.hpp"
#include "gui_graphics.hpp"
#include "text.hpp"
#include "parsers.hpp"
#include "military.hpp"
#include "economy_estimations.hpp"
#include "ai.hpp"

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




//class diplomacy_action_discredit_button : public button_element_base {
//	public:
	void diplomacy_action_discredit_button::on_create(sys::state& state) noexcept {
		base_data.data.button.txt = state.lookup_key("discredit_button");
		button_element_base::on_create(state);
	}

	void diplomacy_action_discredit_button::on_update(sys::state& state) noexcept {
		auto target = retrieve<dcon::nation_id>(state, parent);

		disabled = false;

		if(!state.world.nation_get_is_great_power(state.local_player_nation) || state.world.nation_get_is_great_power(target))
			disabled = true;

		auto rel = state.world.get_gp_relationship_by_gp_influence_pair(target, state.local_player_nation);
		if(state.world.gp_relationship_get_influence(rel) < state.defines.discredit_influence_cost)
			disabled = true;

		if((state.world.gp_relationship_get_status(rel) & nations::influence::is_banned) != 0)
			disabled = true;

		if(military::are_at_war(state, state.local_player_nation, target))
			disabled = true;

		auto clevel = (nations::influence::level_mask & state.world.gp_relationship_get_status(rel));
		if(clevel == nations::influence::level_hostile)
			disabled = true;
	}

	void diplomacy_action_discredit_button::button_action(sys::state& state) noexcept {
		send(state, parent, trigger_gp_choice{ gp_choice_actions::discredit });
	}

	void diplomacy_action_discredit_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto source = state.local_player_nation;
		auto target = retrieve<dcon::nation_id>(state, parent);

		text::add_line(state, contents, "discredit_desc", text::variable_type::days, int64_t(state.defines.discredit_days));
		text::add_line_break_to_layout(state, contents);
		auto rel = state.world.get_gp_relationship_by_gp_influence_pair(target, state.local_player_nation);
		text::add_line_with_condition(state, contents, "iaction_explain_2", (state.world.gp_relationship_get_status(rel) & nations::influence::is_banned) == 0);
		auto clevel = (nations::influence::level_mask & state.world.gp_relationship_get_status(rel));
		text::add_line_with_condition(state, contents, "discredit_explain_2", clevel != nations::influence::level_hostile);

		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::discredit_advisors)].limit; k) {
			text::add_line_break_to_layout(state, contents);
			ui::trigger_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), -1);
		}
		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::discredit_advisors)].effect; k) {
			auto const r_lo = uint32_t(source.value);
			auto const r_hi = uint32_t(source.index() ^ (target.index() << 4));
			text::add_line_break_to_layout(state, contents);
			ui::effect_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), -1, r_lo, r_hi);
		}
	}
// 	};

//class diplomacy_action_expel_advisors_button : public button_element_base {
//	public:
	void diplomacy_action_expel_advisors_button::on_create(sys::state& state) noexcept {
		base_data.data.button.txt = state.lookup_key("expeladvisors_button");
		button_element_base::on_create(state);
	}

	void diplomacy_action_expel_advisors_button::on_update(sys::state& state) noexcept {
		auto target = retrieve<dcon::nation_id>(state, parent);

		disabled = false;

		if(!state.world.nation_get_is_great_power(state.local_player_nation) || state.world.nation_get_is_great_power(target))
		disabled = true;

		auto rel = state.world.get_gp_relationship_by_gp_influence_pair(target, state.local_player_nation);
		if(state.world.gp_relationship_get_influence(rel) < state.defines.expeladvisors_influence_cost)
		disabled = true;

		if((state.world.gp_relationship_get_status(rel) & nations::influence::is_banned) != 0)
		disabled = true;

		if(military::are_at_war(state, state.local_player_nation, target))
		disabled = true;

		auto clevel = (nations::influence::level_mask & state.world.gp_relationship_get_status(rel));
		if(clevel == nations::influence::level_hostile || clevel == nations::influence::level_opposed)
		disabled = true;
	}

	void diplomacy_action_expel_advisors_button::button_action(sys::state& state) noexcept {
	send(state, parent, trigger_gp_choice{ gp_choice_actions::expel_advisors });
	}

	void diplomacy_action_expel_advisors_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto source = state.local_player_nation;
		auto target = retrieve<dcon::nation_id>(state, parent);

		text::add_line(state, contents, "expeladvisors_desc");
		text::add_line_break_to_layout(state, contents);
		auto rel = state.world.get_gp_relationship_by_gp_influence_pair(target, state.local_player_nation);
		text::add_line_with_condition(state, contents, "iaction_explain_2", (state.world.gp_relationship_get_status(rel) & nations::influence::is_banned) == 0);
		auto clevel = (nations::influence::level_mask & state.world.gp_relationship_get_status(rel));
		text::add_line_with_condition(state, contents, "expel_explain_2", clevel != nations::influence::level_hostile && clevel != nations::influence::level_opposed);

		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::expel_advisors)].limit; k) {
			text::add_line_break_to_layout(state, contents);
			ui::trigger_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), -1);
		}
		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::expel_advisors)].effect; k) {
			auto const r_lo = uint32_t(source.value);
			auto const r_hi = uint32_t(source.index() ^ (target.index() << 4));
			text::add_line_break_to_layout(state, contents);
			ui::effect_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), -1, r_lo, r_hi);
		}
	}
// 	};

//class diplomacy_action_ban_embassy_button : public button_element_base {
//	public:
	void diplomacy_action_ban_embassy_button::on_create(sys::state& state) noexcept {
		base_data.data.button.txt = state.lookup_key("banembassy_button");
		button_element_base::on_create(state);
	}

	void diplomacy_action_ban_embassy_button::on_update(sys::state& state) noexcept {
		auto target = retrieve<dcon::nation_id>(state, parent);

		disabled = false;

		if(!state.world.nation_get_is_great_power(state.local_player_nation) || state.world.nation_get_is_great_power(target))
		disabled = true;

		auto rel = state.world.get_gp_relationship_by_gp_influence_pair(target, state.local_player_nation);
		if(state.world.gp_relationship_get_influence(rel) < state.defines.banembassy_influence_cost)
		disabled = true;

		if((state.world.gp_relationship_get_status(rel) & nations::influence::is_banned) != 0)
		disabled = true;

		if(military::are_at_war(state, state.local_player_nation, target))
		disabled = true;

		auto clevel = (nations::influence::level_mask & state.world.gp_relationship_get_status(rel));
		if(clevel != nations::influence::level_friendly && clevel != nations::influence::level_in_sphere)
		disabled = true;
	}

	void diplomacy_action_ban_embassy_button::button_action(sys::state& state) noexcept {
	send(state, parent, trigger_gp_choice{ gp_choice_actions::ban_embassy });
	}

	void diplomacy_action_ban_embassy_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto source = state.local_player_nation;
		auto target = retrieve<dcon::nation_id>(state, parent);

		text::add_line(state, contents, "banembassy_desc", text::variable_type::days, int64_t(state.defines.banembassy_days));
		text::add_line_break_to_layout(state, contents);
		auto rel = state.world.get_gp_relationship_by_gp_influence_pair(target, state.local_player_nation);
		text::add_line_with_condition(state, contents, "iaction_explain_2", (state.world.gp_relationship_get_status(rel) & nations::influence::is_banned) == 0);
		auto clevel = (nations::influence::level_mask & state.world.gp_relationship_get_status(rel));
		text::add_line_with_condition(state, contents, "ban_explain_2", clevel == nations::influence::level_friendly || clevel == nations::influence::level_in_sphere);

		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::ban_embassy)].limit; k) {
			text::add_line_break_to_layout(state, contents);
			ui::trigger_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), -1);
		}
		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::ban_embassy)].effect; k) {
			auto const r_lo = uint32_t(source.value);
			auto const r_hi = uint32_t(source.index() ^ (target.index() << 4));
			text::add_line_break_to_layout(state, contents);
			ui::effect_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), -1, r_lo, r_hi);
		}
	}
// 	};


//class diplomacy_action_decrease_opinion_button : public button_element_base {
//	public:
	void diplomacy_action_decrease_opinion_button::on_create(sys::state& state) noexcept {
		base_data.data.button.txt = state.lookup_key("decreaseopinion_button");
		button_element_base::on_create(state);
	}

	void diplomacy_action_decrease_opinion_button::on_update(sys::state& state) noexcept {
		auto target = retrieve<dcon::nation_id>(state, parent);

		disabled = false;

		if(!state.world.nation_get_is_great_power(state.local_player_nation) || state.world.nation_get_is_great_power(target))
		disabled = true;

		auto rel = state.world.get_gp_relationship_by_gp_influence_pair(target, state.local_player_nation);
		if(state.world.gp_relationship_get_influence(rel) < state.defines.decreaseopinion_influence_cost)
		disabled = true;

		if((state.world.gp_relationship_get_status(rel) & nations::influence::is_banned) != 0)
		disabled = true;

		if(military::are_at_war(state, state.local_player_nation, target))
		disabled = true;

		auto clevel = (nations::influence::level_mask & state.world.gp_relationship_get_status(rel));
		if(clevel == nations::influence::level_hostile)
		disabled = true;
	}

	void diplomacy_action_decrease_opinion_button::button_action(sys::state& state) noexcept {
	send(state, parent, trigger_gp_choice{ gp_choice_actions::decrease_opinion });
	}

	void diplomacy_action_decrease_opinion_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto source = state.local_player_nation;
		auto target = retrieve<dcon::nation_id>(state, parent);

		text::add_line(state, contents, "decreaseopinion_desc");
		text::add_line_break_to_layout(state, contents);
		auto rel = state.world.get_gp_relationship_by_gp_influence_pair(target, state.local_player_nation);
		text::add_line_with_condition(state, contents, "iaction_explain_2", (state.world.gp_relationship_get_status(rel) & nations::influence::is_banned) == 0);
		auto clevel = (nations::influence::level_mask & state.world.gp_relationship_get_status(rel));
		text::add_line_with_condition(state, contents, "dec_op_explain_3", clevel != nations::influence::level_hostile);

		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::decrease_opinion)].limit; k) {
			text::add_line_break_to_layout(state, contents);
			ui::trigger_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), -1);
		}
		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::decrease_opinion)].effect; k) {
			auto const r_lo = uint32_t(source.value);
			auto const r_hi = uint32_t(source.index() ^ (target.index() << 4));
			text::add_line_break_to_layout(state, contents);
			ui::effect_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), -1, r_lo, r_hi);
		}
	}
// 	};


//class diplomacy_action_ally_button : public button_element_base {
//	public:
	void diplomacy_action_ally_button::on_update(sys::state& state) noexcept {
		auto const content = retrieve<dcon::nation_id>(state, parent);
		if(nations::are_allied(state, content, state.local_player_nation)) {
			set_button_text(state, text::produce_simple_string(state, "cancelalliance_button"));
			disabled = !command::can_cancel_alliance(state, state.local_player_nation, content);
		} else {
			set_button_text(state, text::produce_simple_string(state, "alliance_button"));

			diplomatic_message::message m;
			m.type = diplomatic_message::type::alliance_request;
			m.from = state.local_player_nation;
			m.to = content;
			disabled = !command::can_ask_for_alliance(state, state.local_player_nation, content) || (!state.world.nation_get_is_player_controlled(content) && !diplomatic_message::ai_will_accept(state, m));
		}
	}

	void diplomacy_action_ally_button::button_action(sys::state& state) noexcept {
		auto content = retrieve<dcon::nation_id>(state, parent);
		if(nations::are_allied(state, content, state.local_player_nation)) {
			command::cancel_alliance(state, state.local_player_nation, content);
		} else {
			command::ask_for_alliance(state, state.local_player_nation, content);
		}
	}

	void diplomacy_action_ally_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto content = retrieve<dcon::nation_id>(state, parent);
		if(nations::are_allied(state, content, state.local_player_nation)) {
			text::add_line(state, contents, "cancelalliance_desc");
			text::add_line_break_to_layout(state, contents);

			if(state.local_player_nation == content) {
				text::add_line_with_condition(state, contents, "cancel_ally_explain_1", false);
			}
			if(state.defines.cancelalliance_diplomatic_cost > 0) {
				text::add_line_with_condition(state, contents, "cancel_ally_explain_2", state.world.nation_get_diplomatic_points(state.local_player_nation) >= state.defines.cancelalliance_diplomatic_cost, text::variable_type::x, int64_t(state.defines.cancelalliance_diplomatic_cost));
			}

			auto rel = state.world.get_diplomatic_relation_by_diplomatic_pair(content, state.local_player_nation);
			text::add_line_with_condition(state, contents, "cancel_ally_explain_3", state.world.diplomatic_relation_get_are_allied(rel));
			//text::add_line_with_condition(state, contents, "cancel_ally_explain_4", !military::are_allied_in_war(state, state.local_player_nation, content));

			auto ol = state.world.nation_get_overlord_as_subject(state.local_player_nation);
			text::add_line_with_condition(state, contents, "cancel_ally_explain_5", state.world.overlord_get_ruler(ol) != content);
		} else {
			auto asker = state.local_player_nation;
			auto target = content;

			text::add_line(state, contents, "alliance_desc");
			text::add_line_break_to_layout(state, contents);

			if(asker == target)
			text::add_line_with_condition(state, contents, "ally_explain_1", false);

			if(state.defines.alliance_diplomatic_cost > 0) {
				text::add_line_with_condition(state, contents, "ally_explain_2", state.world.nation_get_diplomatic_points(state.local_player_nation) >= state.defines.alliance_diplomatic_cost, text::variable_type::x, int64_t(state.defines.alliance_diplomatic_cost));
			}
			auto rel = state.world.get_diplomatic_relation_by_diplomatic_pair(content, state.local_player_nation);
			text::add_line_with_condition(state, contents, "ally_explain_3", state.world.diplomatic_relation_get_are_allied(rel) == false);
			text::add_line_with_condition(state, contents, "ally_explain_4", !state.world.nation_get_is_great_power(asker) || !state.world.nation_get_is_great_power(target) || state.current_crisis == sys::crisis_type::none);

			auto ol = state.world.nation_get_overlord_as_subject(asker);
			text::add_line_with_condition(state, contents, "ally_explain_5", !state.world.overlord_get_ruler(ol));
			auto ol2 = state.world.nation_get_overlord_as_subject(target);
			text::add_line_with_condition(state, contents, "ally_explain_8", !state.world.overlord_get_ruler(ol2));

			text::add_line_with_condition(state, contents, "ally_explain_6", !military::are_at_war(state, asker, target));

			if(!state.world.nation_get_is_player_controlled(content)) {
				diplomatic_message::message m;
				m.type = diplomatic_message::type::alliance_request;
				m.from = state.local_player_nation;
				m.to = content;

				text::add_line_with_condition(state, contents, "ally_explain_7", diplomatic_message::ai_will_accept(state, m));
				ai::explain_ai_alliance_reasons(state, content, contents, 15);
			}
		}

	}
// 	};

//class diplomacy_action_call_ally_button : public button_element_base {
//	public:
	void diplomacy_action_call_ally_button::on_create(sys::state& state) noexcept {
		base_data.data.button.txt = state.lookup_key("callally_button");
		button_element_base::on_create(state);
	}

	void diplomacy_action_call_ally_button::on_update(sys::state& state) noexcept {
		auto content = retrieve<dcon::nation_id>(state, parent);

		disabled = true;
		for(auto war_par : state.world.nation_get_war_participant(state.local_player_nation)) {
			if(command::can_call_to_arms(state, state.local_player_nation, content, war_par.get_war())) {

				if(!state.world.nation_get_is_player_controlled(content)) {
					diplomatic_message::message m;
					m.type = diplomatic_message::type::call_ally_request;
					m.from = state.local_player_nation;
					m.to = content;
					m.data.war = war_par.get_war();
					if(diplomatic_message::ai_will_accept(state, m)) {
						disabled = false;
						break;
					}
				} else {
					disabled = false;
					break;
				}
			}
		}
	}

	void diplomacy_action_call_ally_button::button_action(sys::state& state) noexcept {
		auto const asker = state.local_player_nation;
		auto const target = retrieve<dcon::nation_id>(state, parent);
		for(auto war_par : state.world.nation_get_war_participant(asker)) {
			if(command::can_call_to_arms(state, state.local_player_nation, target, war_par.get_war())) {
				if(!state.world.nation_get_is_player_controlled(target)) {
					diplomatic_message::message m;
					m.type = diplomatic_message::type::call_ally_request;
					m.from = state.local_player_nation;
					m.to = target;
					m.data.war = war_par.get_war();
					if(diplomatic_message::ai_will_accept(state, m)) {
						command::call_to_arms(state, asker, target, war_par.get_war());
					}
				} else {
					command::call_to_arms(state, asker, target, war_par.get_war());
				}
			}
		}
	}

	void diplomacy_action_call_ally_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto asker = state.local_player_nation;
		auto target = retrieve<dcon::nation_id>(state, parent);

		text::add_line(state, contents, "remove_callally_desc");
		text::add_line_break_to_layout(state, contents);

		if(state.defines.callally_diplomatic_cost > 0) {
			text::add_line_with_condition(state, contents, "call_ally_explain_2", state.world.nation_get_diplomatic_points(asker) >= state.defines.callally_diplomatic_cost, text::variable_type::x, int64_t(state.defines.callally_diplomatic_cost));
		}
		text::add_line_with_condition(state, contents, "call_ally_explain_1", nations::are_allied(state, asker, target));
		text::add_line_with_condition(state, contents, "call_ally_explain_3", state.world.nation_get_is_at_war(asker));

		bool possible_war = false;
		bool that_ai_will_accept = false;

		for(auto war_par : state.world.nation_get_war_participant(asker)) {
			if(!military::is_civil_war(state, war_par.get_war())
			&& military::standard_war_joining_is_possible(state, war_par.get_war(), target, military::is_attacker(state, war_par.get_war(), asker))
			&& (!war_par.get_war().get_is_crisis_war() || state.military_definitions.great_wars_enabled)) {

				possible_war = true;
				if(!state.world.nation_get_is_player_controlled(target)) {
					diplomatic_message::message m;
					m.type = diplomatic_message::type::call_ally_request;
					m.from = state.local_player_nation;
					m.to = target;
					m.data.war = war_par.get_war();
					if(diplomatic_message::ai_will_accept(state, m)) {
						that_ai_will_accept = true;
					}
				}
			}
		}
		text::add_line_with_condition(state, contents, "call_ally_explain_4", possible_war);
		if(!state.world.nation_get_is_player_controlled(target)) {
			text::add_line_with_condition(state, contents, "call_ally_explain_5", that_ai_will_accept);
		}

	}
// 	};

//class diplomacy_action_military_access_button : public button_element_base {
//	public:
	void diplomacy_action_military_access_button::on_update(sys::state& state) noexcept {
		auto target = retrieve<dcon::nation_id>(state, parent);
		auto urid = state.world.get_unilateral_relationship_by_unilateral_pair(target, state.local_player_nation);

		if(state.world.unilateral_relationship_get_military_access(urid)) {
			disabled = !command::can_cancel_military_access(state, state.local_player_nation, target);
			set_button_text(state, text::produce_simple_string(state, "cancelaskmilitaryaccess_button"));
		} else {
			if(command::can_ask_for_access(state, state.local_player_nation, target)) {
				if(!state.world.nation_get_is_player_controlled(target)) {
					diplomatic_message::message m;
					m.type = diplomatic_message::type::access_request;
					m.from = state.local_player_nation;
					m.to = target;
					if(diplomatic_message::ai_will_accept(state, m)) {
						disabled = false;
					} else {
						disabled = true;
					}
				} else {
					disabled = false;
				}
			} else {
				disabled = true;
			}
			set_button_text(state, text::produce_simple_string(state, "askmilitaryaccess_button"));
		}
	}

	void diplomacy_action_military_access_button::button_action(sys::state& state) noexcept {
		auto target = retrieve<dcon::nation_id>(state, parent);
		auto urid = state.world.get_unilateral_relationship_by_unilateral_pair(target, state.local_player_nation);
		if(state.world.unilateral_relationship_get_military_access(urid)) {
			command::cancel_military_access(state, state.local_player_nation, target);
		} else {
			command::ask_for_military_access(state, state.local_player_nation, target);
		}
	}

	void diplomacy_action_military_access_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto target = retrieve<dcon::nation_id>(state, parent);
		auto urid = state.world.get_unilateral_relationship_by_unilateral_pair(target, state.local_player_nation);

		if(state.world.unilateral_relationship_get_military_access(urid)) {
			text::add_line(state, contents, "cancelaskmilitaryaccess_desc");
			text::add_line_break_to_layout(state, contents);

			if(state.defines.cancelaskmilaccess_diplomatic_cost > 0) {
				text::add_line_with_condition(state, contents, "cancel_access_explain_1", state.world.nation_get_diplomatic_points(state.local_player_nation) >= state.defines.cancelaskmilaccess_diplomatic_cost, text::variable_type::x, int64_t(state.defines.cancelaskmilaccess_diplomatic_cost));
			}
		} else {
			text::add_line(state, contents, "askmilitaryaccess_desc");
			text::add_line_break_to_layout(state, contents);

			if(target == state.local_player_nation) {
				text::add_line_with_condition(state, contents, "ask_access_explain_1", false);
			}
			if(state.defines.askmilaccess_diplomatic_cost > 0) {
				text::add_line_with_condition(state, contents, "ask_access_explain_2", state.world.nation_get_diplomatic_points(state.local_player_nation) >= state.defines.askmilaccess_diplomatic_cost, text::variable_type::x, int64_t(state.defines.askmilaccess_diplomatic_cost));
			}
			if(!state.world.nation_get_is_player_controlled(target)) {
				diplomatic_message::message m;
				m.type = diplomatic_message::type::access_request;
				m.from = state.local_player_nation;
				m.to = target;
				text::add_line_with_condition(state, contents, "ask_access_explain_3", diplomatic_message::ai_will_accept(state, m));

				ai::explain_ai_access_reasons(state, target, contents, 15);
			}
			text::add_line_with_condition(state, contents, "ask_access_explain_4", !military::are_at_war(state, state.local_player_nation, target));
		}

	}
// 	};

//class diplomacy_action_give_military_access_button : public button_element_base {
//	public:
	void diplomacy_action_give_military_access_button::on_update(sys::state& state) noexcept {
		auto target = retrieve<dcon::nation_id>(state, parent);
		auto urid = state.world.get_unilateral_relationship_by_unilateral_pair(state.local_player_nation, target);

		if(state.world.unilateral_relationship_get_military_access(urid)) {
			set_button_text(state, text::produce_simple_string(state, "cancelgivemilitaryaccess_button"));
			disabled = !command::can_cancel_given_military_access(state, state.local_player_nation, target);
		} else {
			set_button_text(state, text::produce_simple_string(state, "givemilitaryaccess_button"));
			disabled = !command::can_give_military_access(state, state.local_player_nation, target);
		}
	}

	void diplomacy_action_give_military_access_button::button_action(sys::state& state) noexcept {
		auto target = retrieve<dcon::nation_id>(state, parent);
		auto urid = state.world.get_unilateral_relationship_by_unilateral_pair(state.local_player_nation, target);

		if(state.world.unilateral_relationship_get_military_access(urid)) {
			command::cancel_given_military_access(state, state.local_player_nation, target);
		} else {
			command::give_military_access(state, state.local_player_nation, target);
		}
	}

	void diplomacy_action_give_military_access_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto target = retrieve<dcon::nation_id>(state, parent);
		auto urid = state.world.get_unilateral_relationship_by_unilateral_pair(state.local_player_nation, target);

		if(state.world.unilateral_relationship_get_military_access(urid)) {
			text::add_line(state, contents, "cancelgivemilitaryaccess_desc");
			text::add_line_break_to_layout(state, contents);

			if(state.defines.cancelgivemilaccess_diplomatic_cost > 0) {
				text::add_line_with_condition(state, contents, "cancel_given_access_explain_1", state.world.nation_get_diplomatic_points(state.local_player_nation) >= state.defines.cancelgivemilaccess_diplomatic_cost, text::variable_type::x, int64_t(state.defines.cancelgivemilaccess_diplomatic_cost));
			}
		} else {
			text::add_line(state, contents, "givemilitaryaccess_desc");
			text::add_line_break_to_layout(state, contents);

			if(target == state.local_player_nation) {
				text::add_line_with_condition(state, contents, "give_access_explain_1", false);
			}
			if(state.defines.givemilaccess_diplomatic_cost > 0) {
				text::add_line_with_condition(state, contents, "give_access_explain_2", state.world.nation_get_diplomatic_points(state.local_player_nation) >= state.defines.givemilaccess_diplomatic_cost, text::variable_type::x, int64_t(state.defines.givemilaccess_diplomatic_cost));
			}
			text::add_line_with_condition(state, contents, "give_access_explain_3", !military::are_at_war(state, state.local_player_nation, target));
		}

	}
// 	};

//class diplomacy_action_increase_relations_button : public button_element_base {
//	public:
	void diplomacy_action_increase_relations_button::on_create(sys::state& state) noexcept {
		base_data.data.button.txt = state.lookup_key("increaserelation_button");
		button_element_base::on_create(state);
	}

	void diplomacy_action_increase_relations_button::on_update(sys::state& state) noexcept {
		auto content = retrieve<dcon::nation_id>(state, parent);
		disabled = !command::can_increase_relations(state, state.local_player_nation, content);
	}

	void diplomacy_action_increase_relations_button::button_action(sys::state& state) noexcept {
		command::increase_relations(state, state.local_player_nation, retrieve<dcon::nation_id>(state, parent));
	}

	void diplomacy_action_increase_relations_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto source = state.local_player_nation;
		auto target = retrieve<dcon::nation_id>(state, parent);

		text::add_line(state, contents, "increase_rel_explain_1", text::variable_type::x, int64_t(state.defines.increaserelation_relation_on_accept));
		text::add_line_break_to_layout(state, contents);
		text::add_line_with_condition(state, contents, "increase_rel_explain_3", state.world.nation_get_diplomatic_points(state.local_player_nation) >= state.defines.increaserelation_diplomatic_cost, text::variable_type::x, int64_t(state.defines.increaserelation_diplomatic_cost));

		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::increase_relations)].limit; k) {
			text::add_line_break_to_layout(state, contents);
			ui::trigger_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(source), trigger::to_generic(target));
		}
		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::increase_relations)].effect; k) {
			auto const r_lo = uint32_t(source.value);
			auto const r_hi = uint32_t(source.index() ^ (target.index() << 4));
			text::add_line_break_to_layout(state, contents);
			ui::effect_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(source), trigger::to_generic(target), r_lo, r_hi);
		}
	}
// 	};

//class diplomacy_action_decrease_relations_button : public button_element_base {
//	public:
	void diplomacy_action_decrease_relations_button::on_create(sys::state& state) noexcept {
		base_data.data.button.txt = state.lookup_key("decreaserelation_button");
		button_element_base::on_create(state);
	}

	void diplomacy_action_decrease_relations_button::on_update(sys::state& state) noexcept {
		auto content = retrieve<dcon::nation_id>(state, parent);
		disabled = !command::can_decrease_relations(state, state.local_player_nation, content);
	}

	void diplomacy_action_decrease_relations_button::button_action(sys::state& state) noexcept {
		command::decrease_relations(state, state.local_player_nation, retrieve<dcon::nation_id>(state, parent));
	}

	void diplomacy_action_decrease_relations_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto source = state.local_player_nation;
		auto target = retrieve<dcon::nation_id>(state, parent);

		text::add_line(state, contents, "decrease_rel_explain_1", text::variable_type::x, int64_t(state.defines.decreaserelation_relation_on_accept));
		text::add_line_break_to_layout(state, contents);
		text::add_line_with_condition(state, contents, "increase_rel_explain_3", state.world.nation_get_diplomatic_points(state.local_player_nation) >= state.defines.decreaserelation_diplomatic_cost, text::variable_type::x, int64_t(state.defines.decreaserelation_diplomatic_cost));

		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::decrease_relations)].limit; k) {
			text::add_line_break_to_layout(state, contents);
			ui::trigger_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(source), trigger::to_generic(target));
		}
		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::decrease_relations)].effect; k) {
			auto const r_lo = uint32_t(source.value);
			auto const r_hi = uint32_t(source.index() ^ (target.index() << 4));
			text::add_line_break_to_layout(state, contents);
			ui::effect_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(source), trigger::to_generic(target), r_lo, r_hi);
		}
	}
// 	};

//class diplomacy_action_war_subisides_button : public button_element_base {
//	public:
	void diplomacy_action_war_subisides_button::on_update(sys::state& state) noexcept {
		auto target = retrieve<dcon::nation_id>(state, parent);
		auto rel = state.world.get_unilateral_relationship_by_unilateral_pair(target, state.local_player_nation);
		bool subsidies = state.world.unilateral_relationship_get_war_subsidies(rel);
		set_button_text(state, text::produce_simple_string(state, subsidies ? "cancel_warsubsidies_button" : "warsubsidies_button"));
		disabled = subsidies
		? !command::can_cancel_war_subsidies(state, state.local_player_nation, target)
		: !command::can_give_war_subsidies(state, state.local_player_nation, target);
	}

	void diplomacy_action_war_subisides_button::button_action(sys::state& state) noexcept {
		auto target = retrieve<dcon::nation_id>(state, parent);
		auto rel = state.world.get_unilateral_relationship_by_unilateral_pair(target, state.local_player_nation);
		bool subsidies = state.world.unilateral_relationship_get_war_subsidies(rel);
		if(subsidies) {
			command::cancel_war_subsidies(state, state.local_player_nation, target);
		} else {
			command::give_war_subsidies(state, state.local_player_nation, target);
		}
	}

	void diplomacy_action_war_subisides_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto target = retrieve<dcon::nation_id>(state, parent);
		auto rel = state.world.get_unilateral_relationship_by_unilateral_pair(target, state.local_player_nation);
		bool subsidies = state.world.unilateral_relationship_get_war_subsidies(rel);

		if(subsidies) {
			text::add_line(state, contents, "cancel_w_sub_explain_1", text::variable_type::x, text::fp_currency{ economy_estimations::estimate_war_subsidies(state, target) });
			if(state.defines.cancelwarsubsidy_diplomatic_cost > 0) {
				text::add_line_break_to_layout(state, contents);
				text::add_line_with_condition(state, contents, "cancel_w_sub_explain_2", state.world.nation_get_diplomatic_points(state.local_player_nation) >= state.defines.cancelwarsubsidy_diplomatic_cost, text::variable_type::x, int16_t(state.defines.cancelwarsubsidy_diplomatic_cost));
			}
		} else {
			text::add_line(state, contents, "warsubsidies_desc", text::variable_type::money, text::fp_one_place{ economy_estimations::estimate_war_subsidies(state, target) });
			text::add_line_break_to_layout(state, contents);

			if(state.local_player_nation == target) {
				text::add_line_with_condition(state, contents, "w_sub_explain_1", false);
			}
			if(state.defines.warsubsidy_diplomatic_cost > 0) {
				text::add_line_with_condition(state, contents, "w_sub_explain_2", state.world.nation_get_diplomatic_points(state.local_player_nation) >= state.defines.warsubsidy_diplomatic_cost, text::variable_type::x, int16_t(state.defines.warsubsidy_diplomatic_cost));
			}
			text::add_line_with_condition(state, contents, "w_sub_explain_3", !military::are_at_war(state, state.local_player_nation, target));
			text::add_line_with_condition(state, contents, "w_sub_explain_4", state.world.nation_get_is_at_war(target));
		}
	}
// 	};

//class diplomacy_action_declare_war_button : public button_element_base {
//	public:
	void diplomacy_action_declare_war_button::on_update(sys::state& state) noexcept {
		dcon::nation_id content = retrieve<dcon::nation_id>(state, parent);

		if(military::are_at_war(state, state.local_player_nation, content)) {
			disabled = !command::can_start_peace_offer(state, state.local_player_nation, content, military::find_war_between(state, state.local_player_nation, content), true);
			set_button_text(state, text::produce_simple_string(state, "peace_button"));
		} else {
			disabled = state.local_player_nation == content || !military::can_use_cb_against(state, state.local_player_nation, content) || state.world.nation_get_diplomatic_points(state.local_player_nation) < state.defines.declarewar_diplomatic_cost || military::are_in_common_war(state, state.local_player_nation, content);
			set_button_text(state, text::produce_simple_string(state, "war_button"));
		}
	}

	void diplomacy_action_declare_war_button::button_action(sys::state& state) noexcept {
		if(parent) {
			dcon::nation_id content = retrieve<dcon::nation_id>(state, parent);
			Cyto::Any ac_payload = military::are_at_war(state, state.local_player_nation, content) ? diplomacy_action::make_peace : diplomacy_action::declare_war;
			parent->impl_get(state, ac_payload);
		}
	}

	void diplomacy_action_declare_war_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto target = retrieve<dcon::nation_id>(state, parent);
		auto source = state.local_player_nation;
		if(military::are_at_war(state, state.local_player_nation, target)) {
			text::add_line(state, contents, "peace_description");
			text::add_line_break_to_layout(state, contents);
			{
				auto ol = state.world.nation_get_overlord_as_subject(source);
				text::add_line_with_condition(state, contents, "peace_explain_1", !state.world.overlord_get_ruler(ol));
			}
			{
				auto ol = state.world.nation_get_overlord_as_subject(target);
				text::add_line_with_condition(state, contents, "peace_explain_2", !state.world.overlord_get_ruler(ol));
			}
			auto war = military::find_war_between(state, source, target);
			if(state.world.war_get_is_crisis_war(war)) {
				bool both_wl = (state.world.war_get_primary_attacker(war) == source && state.world.war_get_primary_defender(war) == target) || (state.world.war_get_primary_attacker(war) == target || state.world.war_get_primary_defender(war) == source);
				text::add_line_with_condition(state, contents, "peace_explain_4", both_wl);
			} else {
				bool one_is_wl = [&]() {
					auto const role = military::get_role(state, war, target);
					if(state.world.war_get_primary_attacker(war) == source) {
						return role == military::war_role::defender;
					} else if(state.world.war_get_primary_defender(war) == source) {
						return role == military::war_role::attacker;
					} else if(state.world.war_get_primary_attacker(war) == target) {
						return role == military::war_role::defender;
					} else if(state.world.war_get_primary_defender(war) == target) {
						return role == military::war_role::attacker;
					}
					return false;
				}();
				text::add_line_with_condition(state, contents, "peace_explain_3", one_is_wl);
			}
			text::add_line_with_condition(state, contents, "peace_explain_5", !(state.world.nation_get_peace_offer_from_pending_peace_offer(source)));
		} else {
			text::add_line(state, contents, "act_wardesc");
			text::add_line_break_to_layout(state, contents);
			if(state.defines.declarewar_diplomatic_cost > 0.f) {
				text::add_line_with_condition(state, contents, "war_explain_3", state.world.nation_get_diplomatic_points(state.local_player_nation) >= state.defines.declarewar_diplomatic_cost, text::variable_type::x, int64_t(state.defines.declarewar_diplomatic_cost));
			}
			if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::declare_war)].limit; k) {
				text::add_line_break_to_layout(state, contents);
				ui::trigger_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(source), trigger::to_generic(target));
			}
			if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::declare_war)].effect; k) {
				auto const r_lo = uint32_t(source.value);
				auto const r_hi = uint32_t(source.index() ^ (target.index() << 4));
				text::add_line_break_to_layout(state, contents);
				ui::effect_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(source), trigger::to_generic(target), r_lo, r_hi);
			}
		}
	}
// 	};

//class diplomacy_action_release_subject_button : public button_element_base {
//	public:
	void diplomacy_action_release_subject_button::on_create(sys::state& state) noexcept {
		base_data.data.button.txt = state.lookup_key("alice_diplo_release_subject");
		button_element_base::on_create(state);
	}

	void diplomacy_action_release_subject_button::on_update(sys::state& state) noexcept {
		auto n = retrieve<dcon::nation_id>(state, parent);
		disabled = !command::can_release_subject(state, state.local_player_nation, n);
	}

	void diplomacy_action_release_subject_button::button_action(sys::state& state) noexcept {
		auto n = retrieve<dcon::nation_id>(state, parent);
		command::release_subject(state, state.local_player_nation, n);
	}

	void diplomacy_action_release_subject_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto target = retrieve<dcon::nation_id>(state, parent);
		auto source = state.local_player_nation;
		text::add_line(state, contents, "alice_diplo_release_subject_desc");
		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::release_subject)].limit; k) {
			text::add_line_break_to_layout(state, contents);
			ui::trigger_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(source), trigger::to_generic(target));
		}
		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::release_subject)].effect; k) {
			auto const r_lo = uint32_t(source.value);
			auto const r_hi = uint32_t(source.index() ^ (target.index() << 4));
			text::add_line_break_to_layout(state, contents);
			ui::effect_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(source), trigger::to_generic(target), r_lo, r_hi);
		}
	}
// 	};

//class diplomacy_action_increase_opinion_button : public button_element_base {
//	public:
	void diplomacy_action_increase_opinion_button::on_create(sys::state& state) noexcept {
		base_data.data.button.txt = state.lookup_key("increaseopinion_button");
		button_element_base::on_create(state);
	}

	void diplomacy_action_increase_opinion_button::on_update(sys::state& state) noexcept {
		auto content = retrieve<dcon::nation_id>(state, parent);
		disabled = !command::can_increase_opinion(state, state.local_player_nation, content);
	}

	void diplomacy_action_increase_opinion_button::button_action(sys::state& state) noexcept {
		auto content = retrieve<dcon::nation_id>(state, parent);
		command::increase_opinion(state, state.local_player_nation, content);
	}

	void diplomacy_action_increase_opinion_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto target = retrieve<dcon::nation_id>(state, parent);
		auto source = state.local_player_nation;
		text::add_line(state, contents, "increaseopinion_desc");
		text::add_line_break_to_layout(state, contents);
		auto rel = state.world.get_gp_relationship_by_gp_influence_pair(target, source);
		text::add_line_with_condition(state, contents, "iaction_explain_2", (state.world.gp_relationship_get_status(rel) & nations::influence::is_banned) == 0);

		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::increase_opinion)].limit; k) {
			text::add_line_break_to_layout(state, contents);
			ui::trigger_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), -1);
		}
		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::increase_opinion)].effect; k) {
			auto const r_lo = uint32_t(source.value);
			auto const r_hi = uint32_t(source.index() ^ (target.index() << 4));
			text::add_line_break_to_layout(state, contents);
			ui::effect_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), -1, r_lo, r_hi);
		}
	}
// 	};

//class diplomacy_action_add_to_sphere_button : public button_element_base {
//	public:
	void diplomacy_action_add_to_sphere_button::on_create(sys::state& state) noexcept {
		base_data.data.button.txt = state.lookup_key("addtosphere_button");
		button_element_base::on_create(state);
	}

	void diplomacy_action_add_to_sphere_button::on_update(sys::state& state) noexcept {
		auto content = retrieve<dcon::nation_id>(state, parent);
		disabled = !command::can_add_to_sphere(state, state.local_player_nation, content);
	}

	void diplomacy_action_add_to_sphere_button::button_action(sys::state& state) noexcept {
		auto content = retrieve<dcon::nation_id>(state, parent);
		command::add_to_sphere(state, state.local_player_nation, content);
	}

	void diplomacy_action_add_to_sphere_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto target = retrieve<dcon::nation_id>(state, parent);
		auto source = state.local_player_nation;

		text::add_line(state, contents, "addtosphere_desc");
		text::add_line_break_to_layout(state, contents);
		auto rel = state.world.get_gp_relationship_by_gp_influence_pair(target, source);
		text::add_line_with_condition(state, contents, "iaction_explain_1", state.world.gp_relationship_get_influence(rel) >= state.defines.addtosphere_influence_cost, text::variable_type::x, int64_t(state.defines.addtosphere_influence_cost));
		text::add_line_with_condition(state, contents, "iaction_explain_2", (state.world.gp_relationship_get_status(rel) & nations::influence::is_banned) == 0);
		auto clevel = (nations::influence::level_mask & state.world.gp_relationship_get_status(rel));
		text::add_line_with_condition(state, contents, "add_sphere_explain_1", clevel == nations::influence::level_friendly);

		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::add_to_sphere)].limit; k) {
			text::add_line_break_to_layout(state, contents);
			ui::trigger_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), -1);
		}
		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::add_to_sphere)].effect; k) {
			auto const r_lo = uint32_t(source.value);
			auto const r_hi = uint32_t(source.index() ^ (target.index() << 4));
			text::add_line_break_to_layout(state, contents);
			ui::effect_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), -1, r_lo, r_hi);
		}
	}
// 	};

//class diplomacy_action_remove_from_sphere_button : public button_element_base {
//	public:
	void diplomacy_action_remove_from_sphere_button::on_create(sys::state& state) noexcept {
		base_data.data.button.txt = state.lookup_key("removefromsphere_button");
		button_element_base::on_create(state);
	}

	void diplomacy_action_remove_from_sphere_button::on_update(sys::state& state) noexcept {
		auto content = retrieve<dcon::nation_id>(state, parent);
		disabled = !command::can_remove_from_sphere(state, state.local_player_nation, content, state.world.nation_get_in_sphere_of(content));
	}

	void diplomacy_action_remove_from_sphere_button::button_action(sys::state& state) noexcept {
		auto content = retrieve<dcon::nation_id>(state, parent);
		command::remove_from_sphere(state, state.local_player_nation, content, state.world.nation_get_in_sphere_of(content));
	}

	void diplomacy_action_remove_from_sphere_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto target = retrieve<dcon::nation_id>(state, parent);
		auto source = state.local_player_nation;
		bool in_players_sphere = state.world.nation_get_in_sphere_of(target) == source;

		text::add_line(state, contents, "removefromsphere_desc");
		text::add_line_break_to_layout(state, contents);
		auto rel = state.world.get_gp_relationship_by_gp_influence_pair(target, state.local_player_nation);
		text::add_line_with_condition(state, contents, "iaction_explain_2", (state.world.gp_relationship_get_status(rel) & nations::influence::is_banned) == 0);
		text::add_line_with_condition(state, contents, "rem_sphere_explain_1", bool(state.world.nation_get_in_sphere_of(target)));
		auto clevel = (nations::influence::level_mask & state.world.gp_relationship_get_status(rel));
		if(!in_players_sphere) {
			text::add_line_with_condition(state, contents, "rem_sphere_explain_2", clevel == nations::influence::level_friendly);
		} else {
			text::add_line_with_condition(state, contents, "rem_sphere_explain_3", true);
		}

		auto affected_gp = state.world.nation_get_in_sphere_of(target);
		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::remove_from_sphere)].limit; k) {
			text::add_line_break_to_layout(state, contents);
			ui::trigger_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), trigger::to_generic(affected_gp));
		}
		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::remove_from_sphere)].effect; k) {
			auto const r_lo = uint32_t(source.value);
			auto const r_hi = uint32_t(source.index() ^ (target.index() << 4));
			text::add_line_break_to_layout(state, contents);
			ui::effect_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(target), trigger::to_generic(affected_gp), r_lo, r_hi);
		}
	}
// 	};

//class diplomacy_action_justify_war_button : public button_element_base {
//	public:
	void diplomacy_action_justify_war_button::on_create(sys::state& state) noexcept {
		base_data.data.button.txt = state.lookup_key("make_cb_button");
		button_element_base::on_create(state);
	}

	void diplomacy_action_justify_war_button::on_update(sys::state& state) noexcept {
		auto source = state.local_player_nation;
		auto target = retrieve<dcon::nation_id>(state, parent);

		auto ol = state.world.nation_get_overlord_as_subject(source);
		disabled = (source == target)
			|| (state.world.nation_get_constructing_cb_type(source))
			|| (state.world.overlord_get_ruler(ol) && state.world.overlord_get_ruler(ol) != target)
			|| (state.world.nation_get_in_sphere_of(target) == source)
			|| (state.world.nation_get_diplomatic_points(source) < state.defines.make_cb_diplomatic_cost)
			|| (military::are_at_war(state, target, source))
			|| (military::has_truce_with(state, target, source))
			|| (!has_any_usable_cb(state, source, target));
	}

	void diplomacy_action_justify_war_button::button_action(sys::state& state) noexcept {
		if(parent) {
			Cyto::Any payload = diplomacy_action::justify_war;
			parent->impl_get(state, payload);
		}
	}

	void diplomacy_action_justify_war_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto source = state.local_player_nation;
		auto target = retrieve<dcon::nation_id>(state, parent);
		text::add_line(state, contents, "make_cb_desc");
		text::add_line_break_to_layout(state, contents);
		if(source == target) {
			text::add_line_with_condition(state, contents, "fab_explain_1", false);
		}
		if(state.defines.make_cb_diplomatic_cost > 0) {
			text::add_line_with_condition(state, contents, "fab_explain_2", state.world.nation_get_diplomatic_points(source) >= state.defines.make_cb_diplomatic_cost, text::variable_type::x, int16_t(state.defines.make_cb_diplomatic_cost));
		}
		text::add_line_with_condition(state, contents, "fab_explain_3", !state.world.nation_get_constructing_cb_type(source));
		auto ol = state.world.nation_get_overlord_as_subject(source);
		text::add_line_with_condition(state, contents, "fab_explain_7", has_any_usable_cb(state, source, target));

		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::fabricate_cb)].limit; k) {
			text::add_line_break_to_layout(state, contents);
			ui::trigger_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(source), trigger::to_generic(target));
		}
		if(auto k = state.national_definitions.static_game_rules[uint8_t(sys::static_game_rule::fabricate_cb)].effect; k) {
			auto const r_lo = uint32_t(source.value);
			auto const r_hi = uint32_t(source.index() ^ (target.index() << 4));
			text::add_line_break_to_layout(state, contents);
			ui::effect_description(state, contents, k, trigger::to_generic(source), trigger::to_generic(source), trigger::to_generic(target), r_lo, r_hi);
		}
	}
// 	};
	std::unique_ptr<element_base> diplomacy_action_gp_dialog_select_window::make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept {
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

	message_result diplomacy_action_gp_dialog_select_window::get(sys::state& state, Cyto::Any& payload) noexcept {
		if(payload.holds_type<gp_choice_num>()) {
			payload.emplace<gp_choice_num>(gp_choice_num{ rank });
			return message_result::consumed;
		}
		return window_element_base::get(state, payload);
	}

	void gp_action_choice_accept_button::on_create(sys::state& state) noexcept {
		base_data.data.button.txt = state.lookup_key("ok");
		button_element_base::on_create(state);
	}

	void gp_action_choice_accept_button::on_update(sys::state& state) noexcept {
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
	void gp_action_choice_accept_button::button_action(sys::state& state) noexcept {
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
	void gp_action_choice_accept_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
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

	void diplomacy_action_state_transfer_button::on_create(sys::state& state) noexcept {
		base_data.data.button.txt = state.lookup_key("state_transfer_button");
		button_element_base::on_create(state);
	}

	void diplomacy_action_state_transfer_button::on_update(sys::state& state) noexcept {
		auto content = retrieve<dcon::nation_id>(state, parent);
		disabled = true;
		for(const auto s : state.world.nation_get_state_ownership(content)) {
			if(command::can_state_transfer(state, state.local_player_nation, content, s.get_state().get_definition())) {
				disabled = false;
				break;
			}
		}
	}

	void diplomacy_action_state_transfer_button::button_action(sys::state& state) noexcept {
		auto const content = retrieve<dcon::nation_id>(state, parent);

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

	void diplomacy_action_state_transfer_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto const target = retrieve<dcon::nation_id>(state, parent);
		auto const source = state.local_player_nation;

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

	std::string_view diplomacy_action_dialog_title_text::get_title_key(diplomacy_action v) noexcept {
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

	std::string_view diplomacy_action_dialog_description_text::get_title_key(diplomacy_action v) noexcept {
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

	void diplomacy_action_dialog_description_text::on_update(sys::state& state) noexcept {
		auto contents = text::create_endless_layout(state, internal_layout,
			text::layout_parameters{ 0, 0, static_cast<int16_t>(base_data.size.x), static_cast<int16_t>(base_data.size.y),
			base_data.data.text.font_handle, 0, text::alignment::left, text::text_color::white, false
		});
		auto box = text::open_layout_box(contents);
		text::localised_format_box(state, contents, box, get_title_key(content));
		text::close_layout_box(contents, box);
	}

	bool diplomacy_action_dialog_agree_button::get_can_perform(sys::state& state) noexcept {
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
		case diplomacy_action::command_units:
		case diplomacy_action::cancel_command_units:
		case diplomacy_action::make_peace:
		case diplomacy_action::crisis_backdown:
		case diplomacy_action::crisis_support:
		case diplomacy_action::add_wargoal:
		case diplomacy_action::state_transfer:
			return false;
		}
		return false;
	}

	void diplomacy_action_dialog_agree_button::button_action(sys::state& state) noexcept {
		auto const target = retrieve<dcon::nation_id>(state, parent);
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
		case diplomacy_action::command_units:
		case diplomacy_action::cancel_command_units:
		case diplomacy_action::make_peace:
		case diplomacy_action::crisis_backdown:
		case diplomacy_action::crisis_support:
		case diplomacy_action::add_wargoal:
		case diplomacy_action::state_transfer:
			break;
		}
		if(parent) {
			parent->set_visible(state, false);
		}
	}

	std::unique_ptr<element_base> diplomacy_action_dialog_window::make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept {
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

	void diplomacy_gp_action_dialog_window::on_create(sys::state& state) noexcept {
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

	std::unique_ptr<element_base> diplomacy_gp_action_dialog_window::make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept {
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

	message_result diplomacy_gp_action_dialog_window::get(sys::state& state, Cyto::Any& payload) noexcept {
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
}
