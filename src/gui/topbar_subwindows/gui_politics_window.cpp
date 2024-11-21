#include "gui_politics_window.hpp"

namespace ui {
	void reform_rules_description(sys::state& state, text::columnar_layout& contents, uint32_t rules) {
		if((rules & (issue_rule::primary_culture_voting | issue_rule::culture_voting | issue_rule::culture_voting | issue_rule::all_voting | issue_rule::largest_share | issue_rule::dhont | issue_rule::sainte_laque | issue_rule::same_as_ruling_party | issue_rule::rich_only | issue_rule::state_vote | issue_rule::population_vote)) != 0) {
			text::add_line(state, contents, "voting_rules");
			if((rules & issue_rule::primary_culture_voting) != 0) {
				text::add_line(state, contents, "rule_primary_culture_voting");
			}
			if((rules & issue_rule::culture_voting) != 0) {
				text::add_line(state, contents, "rule_culture_voting");
			}
			if((rules & issue_rule::all_voting) != 0) {
				text::add_line(state, contents, "rule_all_voting");
			}
			if((rules & issue_rule::largest_share) != 0) {
				text::add_line(state, contents, "rule_largest_share");
			}
			if((rules & issue_rule::dhont) != 0) {
				text::add_line(state, contents, "rule_dhont");
			}
			if((rules & issue_rule::sainte_laque) != 0) {
				text::add_line(state, contents, "rule_sainte_laque");
			}
			if((rules & issue_rule::same_as_ruling_party) != 0) {
				text::add_line(state, contents, "rule_same_as_ruling_party");
			}
			if((rules & issue_rule::rich_only) != 0) {
				text::add_line(state, contents, "rule_rich_only");
			}
			if((rules & issue_rule::state_vote) != 0) {
				text::add_line(state, contents, "rule_state_vote");
			}
			if((rules & issue_rule::population_vote) != 0) {
				text::add_line(state, contents, "rule_population_vote");
			}
		}

		if((rules & (issue_rule::build_factory | issue_rule::expand_factory | issue_rule::open_factory | issue_rule::destroy_factory | issue_rule::factory_priority | issue_rule::can_subsidise | issue_rule::pop_build_factory | issue_rule::pop_expand_factory | issue_rule::pop_open_factory | issue_rule::delete_factory_if_no_input | issue_rule::allow_foreign_investment | issue_rule::slavery_allowed | issue_rule::build_railway | issue_rule::build_bank | issue_rule::build_university)) != 0) {
			text::add_line(state, contents, "special_rules");
			text::add_line_with_condition(state, contents, "rule_build_factory", (rules & issue_rule::build_factory) != 0);
			text::add_line_with_condition(state, contents, "rule_expand_factory", (rules & issue_rule::expand_factory) != 0);
			text::add_line_with_condition(state, contents, "remove_rule_open_factory", (rules & issue_rule::open_factory) != 0);
			text::add_line_with_condition(state, contents, "rule_destroy_factory", (rules & issue_rule::destroy_factory) != 0);
			text::add_line_with_condition(state, contents, "rule_factory_priority", (rules & issue_rule::factory_priority) != 0);
			text::add_line_with_condition(state, contents, "rule_can_subsidise", (rules & issue_rule::can_subsidise) != 0);
			text::add_line_with_condition(state, contents, "rule_pop_build_factory", (rules & issue_rule::pop_build_factory) != 0);
			text::add_line_with_condition(state, contents, "rule_pop_expand_factory", (rules & issue_rule::pop_expand_factory) != 0);
			text::add_line_with_condition(state, contents, "rule_pop_open_factory", (rules & issue_rule::pop_open_factory) != 0);
			text::add_line_with_condition(state, contents, "rule_delete_factory_if_no_input", (rules & issue_rule::delete_factory_if_no_input) != 0);
			text::add_line_with_condition(state, contents, "rule_allow_foreign_investment", (rules & issue_rule::allow_foreign_investment) != 0);
			text::add_line_with_condition(state, contents, "rule_build_factory_invest", (rules & issue_rule::build_factory_invest) != 0);
			text::add_line_with_condition(state, contents, "rule_build_railway_invest", (rules & issue_rule::build_railway_invest) != 0);
			text::add_line_with_condition(state, contents, "rule_slavery_allowed", (rules & issue_rule::slavery_allowed) != 0);
			text::add_line_with_condition(state, contents, "rule_build_railway", (rules & issue_rule::build_railway) != 0);
			if(state.economy_definitions.building_definitions[int32_t(economy::province_building_type::bank)].defined) {
				text::add_line_with_condition(state, contents, "rule_build_bank", (rules & issue_rule::build_bank) != 0);
			}
			if(state.economy_definitions.building_definitions[int32_t(economy::province_building_type::university)].defined) {
				text::add_line_with_condition(state, contents, "rule_build_university", (rules & issue_rule::build_university) != 0);
			}
		}
	}

	void reform_description(sys::state& state, text::columnar_layout& contents, dcon::issue_option_id ref) {
		auto reform = fatten(state.world, ref);
		{
			auto box = text::open_layout_box(contents);
			text::add_to_layout_box(state, contents, box, reform.get_name(), text::text_color::yellow);
			text::close_layout_box(contents, box);
		}
		if(auto desc = reform.get_desc();  state.key_is_localized(desc)) {
		text::substitution_map sub{};
				text::add_to_substitution_map(sub, text::variable_type::country, state.local_player_nation);
			text::add_to_substitution_map(sub, text::variable_type::country_adj, text::get_adjective(state, state.local_player_nation));
			text::add_to_substitution_map(sub, text::variable_type::capital, state.world.nation_get_capital(state.local_player_nation));
			auto box = text::open_layout_box(contents);
			text::add_to_layout_box(state, contents, box, desc, sub);
			text::close_layout_box(contents, box);
		}

		auto total = state.world.nation_get_demographics(state.local_player_nation, demographics::total);
		auto support = state.world.nation_get_demographics(state.local_player_nation, demographics::to_key(state, ref));
		if(total > 0) {
			text::add_line(state, contents, "there_are_backing", text::variable_type::val, text::fp_percentage{ support / total });
		}

		auto mod_id = reform.get_modifier();
		if(bool(mod_id)) {
			modifier_description(state, contents, mod_id);
		}

		auto time_limit = state.world.nation_get_last_issue_or_reform_change(state.local_player_nation);
		auto parent = state.world.issue_option_get_parent_issue(ref);
		if(parent.get_issue_type() != uint8_t(culture::issue_type::party) && time_limit && !(time_limit + int32_t(state.defines.min_delay_between_reforms * 30) <= state.current_date)) {
			text::add_line_with_condition(state, contents, "too_soon_for_reform", false, text::variable_type::date, time_limit + int32_t(state.defines.min_delay_between_reforms * 30));
		}

		auto allow = reform.get_allow();
		if(allow) {
			//allow_reform_cond
			text::add_line(state, contents, "allow_reform_cond");
			trigger_description(state, contents, allow, trigger::to_generic(state.local_player_nation),
				trigger::to_generic(state.local_player_nation), -1);
		}

		auto ext = reform.get_on_execute_trigger();
		if(ext) {
			text::add_line(state, contents, "reform_effect_if_desc");
			trigger_description(state, contents, ext, trigger::to_generic(state.local_player_nation),
				trigger::to_generic(state.local_player_nation), -1);
		}
		auto ex = reform.get_on_execute_effect();
		if(ex) {
			if(ext)
			text::add_line(state, contents, "reform_effect_then_desc");
			else
			text::add_line(state, contents, "reform_effect_desc");

			effect_description(state, contents, ex, trigger::to_generic(state.local_player_nation),
				trigger::to_generic(state.local_player_nation), -1, uint32_t(state.current_date.value),
				uint32_t((ref.index() << 2) ^ state.local_player_nation.index()));
		}
		reform_rules_description(state, contents, reform.get_rules());

		text::add_line_break_to_layout(state, contents);
		auto current = state.world.nation_get_issues(state.local_player_nation, reform.get_parent_issue()).id;

		if(current == ref)
		return;

		bool some_support_shown = false;
		text::add_line(state, contents, "political_support_for_reform_header");
		auto tag = state.world.nation_get_identity_from_identity_holder(state.local_player_nation);
		auto start = state.world.national_identity_get_political_party_first(tag).id.index();
		auto end = start + state.world.national_identity_get_political_party_count(tag);
		auto count_party_issues = state.world.political_party_get_party_issues_size();

		for(uint32_t icounter = state.world.ideology_size(); icounter-- > 0;) {
		dcon::ideology_id iid{ dcon::ideology_id::value_base_t(icounter) };
			dcon::value_modifier_key condition;
			if(parent.get_issue_type() == uint8_t(culture::issue_type::political)) {
				condition = ref.index() > current.index() ? state.world.ideology_get_add_political_reform(iid) : state.world.ideology_get_remove_political_reform(iid);
			} else if(parent.get_issue_type() == uint8_t(culture::issue_type::social)) {
				condition = ref.index() > current.index() ? state.world.ideology_get_add_social_reform(iid) : state.world.ideology_get_remove_social_reform(iid);
			}
			auto upperhouse_weight = 0.01f * state.world.nation_get_upper_house(state.local_player_nation, iid);

			float party_special_issues_support_total = 0.0f;
			float count_found = 0.0f;

			for(int32_t i = start; i < end; i++) {
				auto pid = dcon::political_party_id(dcon::political_party_id::value_base_t(i));
				if(politics::political_party_is_active(state, state.local_player_nation, pid)
				&& (state.world.nation_get_government_type(state.local_player_nation).get_ideologies_allowed() & culture::to_bits(state.world.political_party_get_ideology(pid))) != 0
				&& state.world.political_party_get_ideology(pid) == iid) {

					for(uint32_t j = 0; j < count_party_issues; ++j) {
					auto popt = state.world.political_party_get_party_issues(pid, dcon::issue_id{ dcon::issue_id::value_base_t(j) });
						auto opt_mod = state.world.issue_option_get_support_modifiers(popt, ref);

						if(opt_mod) {
							party_special_issues_support_total += upperhouse_weight * trigger::evaluate_additive_modifier(state, opt_mod, trigger::to_generic(state.local_player_nation), trigger::to_generic(state.local_player_nation), 0);
							count_found += 1.0f;
						}
					}

					break; // only look at one active party per ideology
				}
			}

			if(count_found > 0.0f) {
				auto result = std::clamp(party_special_issues_support_total / count_found, -1.0f, 1.0f);
				if(abs(result) >= 0.01) {
					auto box = text::open_layout_box(contents, 10);
					text::add_to_layout_box(state, contents, box, state.world.ideology_get_name(iid));
					text::add_space_to_layout_box(state, contents, box);
				text::add_to_layout_box(state, contents, box, text::fp_percentage{ result }, result > 0 ? text::text_color::green : text::text_color::red);
					text::close_layout_box(contents, box);
					some_support_shown = true;
				}
			} else if(condition && upperhouse_weight > 0.0f) {
				auto result = upperhouse_weight * std::clamp(trigger::evaluate_additive_modifier(state, condition, trigger::to_generic(state.local_player_nation), trigger::to_generic(state.local_player_nation), 0), -1.0f, 1.0f);
				if(abs(result) >= 0.01) {
					auto box = text::open_layout_box(contents, 10);
					text::add_to_layout_box(state, contents, box, state.world.ideology_get_name(iid));
					text::add_space_to_layout_box(state, contents, box);
				text::add_to_layout_box(state, contents, box, text::fp_percentage{ result }, result > 0 ? text::text_color::green : text::text_color::red);
					text::close_layout_box(contents, box);
					some_support_shown = true;
				}
			}
		}

		if(!some_support_shown) {
			text::add_line(state, contents, "no_political_support_for_reform");
		}
	}

	void reform_description(sys::state& state, text::columnar_layout& contents, dcon::reform_option_id ref) {
		auto reform = fatten(state.world, ref);

		{
			auto box = text::open_layout_box(contents);
			text::add_to_layout_box(state, contents, box, reform.get_name(), text::text_color::yellow);
			text::close_layout_box(contents, box);
		}
		if(auto desc = reform.get_desc();  state.key_is_localized(desc)) {
		text::substitution_map sub{};
			text::add_to_substitution_map(sub, text::variable_type::country, state.local_player_nation);
			text::add_to_substitution_map(sub, text::variable_type::country_adj, text::get_adjective(state, state.local_player_nation));
			text::add_to_substitution_map(sub, text::variable_type::capital, state.world.nation_get_capital(state.local_player_nation));
			auto box = text::open_layout_box(contents);
			text::add_to_layout_box(state, contents, box, desc, sub);
			text::close_layout_box(contents, box);
		}

		float cost = 0.0f;
		if(state.world.reform_get_reform_type(state.world.reform_option_get_parent_reform(ref)) == uint8_t(culture::issue_type::military)) {
			float base_cost = float(state.world.reform_option_get_technology_cost(ref));
			float reform_factor = politics::get_military_reform_multiplier(state, state.local_player_nation);
			cost = base_cost * reform_factor;
		} else {
			float base_cost = float(state.world.reform_option_get_technology_cost(ref));
			float reform_factor = politics::get_economic_reform_multiplier(state, state.local_player_nation);
			cost = base_cost * reform_factor;
		}

		text::add_line(state, contents, "reform_research_cost", text::variable_type::cost, int64_t(cost + 0.99f));
		text::add_line_break_to_layout(state, contents);

		auto mod_id = reform.get_modifier();
		if(bool(mod_id)) {
			modifier_description(state, contents, mod_id);
			text::add_line_break_to_layout(state, contents);
		}

		auto allow = reform.get_allow();
		if(allow) {
			// allow_reform_cond
			text::add_line(state, contents, "allow_reform_cond");
			trigger_description(state, contents, allow, trigger::to_generic(state.local_player_nation),
				trigger::to_generic(state.local_player_nation), -1);
			text::add_line_break_to_layout(state, contents);
		}

		auto ext = reform.get_on_execute_trigger();
		if(ext) {
			text::add_line(state, contents, "reform_effect_if_desc");
			trigger_description(state, contents, ext, trigger::to_generic(state.local_player_nation),
				trigger::to_generic(state.local_player_nation), -1);
		}
		auto ex = reform.get_on_execute_effect();
		if(ex) {
			if(ext)
			text::add_line(state, contents, "reform_effect_then_desc");
			else
			text::add_line(state, contents, "reform_effect_desc");

			effect_description(state, contents, ex, trigger::to_generic(state.local_player_nation),
				trigger::to_generic(state.local_player_nation), -1, uint32_t(state.current_date.value),
				uint32_t((ref.index() << 2) ^ state.local_player_nation.index()));
			text::add_line_break_to_layout(state, contents);
		}
		reform_rules_description(state, contents, reform.get_rules());
	}
}
