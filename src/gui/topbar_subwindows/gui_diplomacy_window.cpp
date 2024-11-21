namespace ui {
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
}
