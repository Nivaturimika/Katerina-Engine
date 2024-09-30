#include "demographics.hpp"
#include "economy.hpp"
#include "economy_factory.hpp"

namespace economy_estimations {
	float estimate_stockpile_filling_spending(sys::state& state, dcon::nation_id n) {
		float total = 0.0f;
		uint32_t total_commodities = state.world.commodity_size();
	
		for(uint32_t i = 1; i < total_commodities; ++i) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			auto difference = state.world.nation_get_stockpile_targets(n, cid) - state.world.nation_get_stockpiles(n, cid);
			if(difference > 0 && state.world.nation_get_drawing_on_stockpiles(n, cid) == false) {
				total += difference * state.world.commodity_get_current_price(cid) * state.world.nation_get_demand_satisfaction(n, cid);
			}
		}
	
		return total;
	}
	
	float estimate_overseas_penalty_spending(sys::state& state, dcon::nation_id n) {
		float total = 0.0f;
	
		auto overseas_factor = state.defines.province_overseas_penalty * float(state.world.nation_get_owned_province_count(n) - state.world.nation_get_central_province_count(n));
		uint32_t total_commodities = state.world.commodity_size();
	
		if(overseas_factor > 0) {
			for(uint32_t i = 1; i < total_commodities; ++i) {
				dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
	
				auto kf = state.world.commodity_get_key_factory(cid);
				if(state.world.commodity_get_overseas_penalty(cid) && (state.world.commodity_get_is_available_from_start(cid) || (kf && state.world.nation_get_active_building(n, kf)))) {
					total += overseas_factor * state.world.commodity_get_current_price(cid) * state.world.nation_get_demand_satisfaction(n, cid);
				}
			}
		}
	
		return total;
	}
	float estimate_gold_income(sys::state& state, dcon::nation_id n) {
		auto amount = 0.f;
		for(auto poid : state.world.nation_get_province_ownership_as_nation(n)) {
			auto prov = poid.get_province();
			state.world.for_each_commodity([&](dcon::commodity_id c) {
				if(state.world.commodity_get_money_rgo(c)) {
					amount += province::rgo_production_quantity(state, prov.id, c);
				}
			});
		}
		return amount * state.defines.gold_to_cash_rate;
	}

	float estimate_tariff_income(sys::state& state, dcon::nation_id n) {
		return nations::tariff_efficiency(state, n) * economy::nation_total_imports(state, n);
	}

	float estimate_social_spending(sys::state& state, dcon::nation_id n) {
		auto total = 0.f;
		auto const s_spending = state.world.nation_get_administrative_efficiency(n) * float(state.world.nation_get_social_spending(n)) / 100.0f;
		auto const p_level = state.world.nation_get_modifier_values(n, sys::national_mod_offsets::pension_level);
		auto const unemp_level = state.world.nation_get_modifier_values(n, sys::national_mod_offsets::unemployment_benefit);
		state.world.for_each_pop_type([&](dcon::pop_type_id pt) {
			auto adj_pop_of_type = state.world.nation_get_demographics(n, demographics::to_key(state, pt));
			if(adj_pop_of_type <= 0)
				return;
			auto ln_type = culture::income_type(state.world.pop_type_get_life_needs_income_type(pt));
			if(ln_type == culture::income_type::administration || ln_type == culture::income_type::education || ln_type == culture::income_type::military) {
				//nothing
			} else { // unemployment, pensions
				total += s_spending * adj_pop_of_type * p_level * state.world.nation_get_life_needs_costs(n, pt);
				if(state.world.pop_type_get_has_unemployment(pt)) {
					auto emp = state.world.nation_get_demographics(n, demographics::to_employment_key(state, pt));
					//sometimes emp > adj_pop_of_type, why? no idea, perhaps we are doing the pop growth update
					//after the employment one?
					emp = std::min(emp, adj_pop_of_type);
					total += s_spending * (adj_pop_of_type - emp) * unemp_level * state.world.nation_get_life_needs_costs(n, pt);
				}
			}
		});
		return total;
	}

	float estimate_pop_payouts_by_income_type(sys::state& state, dcon::nation_id n, culture::income_type in) {
		auto total = 0.f;
		state.world.for_each_pop_type([&](dcon::pop_type_id pt) {
			auto adj_pop_of_type = state.world.nation_get_demographics(n, demographics::to_key(state, pt));
			if(adj_pop_of_type <= 0)
				return;
			auto ln_type = culture::income_type(state.world.pop_type_get_life_needs_income_type(pt));
			if(ln_type == in) {
				total += adj_pop_of_type * state.world.nation_get_life_needs_costs(n, pt);
			}
			auto en_type = culture::income_type(state.world.pop_type_get_everyday_needs_income_type(pt));
			if(en_type == in) {
				total += adj_pop_of_type * state.world.nation_get_everyday_needs_costs(n, pt);
			}
			auto lx_type = culture::income_type(state.world.pop_type_get_luxury_needs_income_type(pt));
			if(lx_type == in) {
				total += adj_pop_of_type * state.world.nation_get_luxury_needs_costs(n, pt);
			}
		});
		return total * economy::pop_payout_factor;
	}

	float estimate_tax_income_by_strata(sys::state& state, dcon::nation_id n, culture::pop_strata ps) {
		auto const tax_eff = nations::tax_efficiency(state, n);
		switch(ps) {
		default:
		case culture::pop_strata::poor:
			return state.world.nation_get_total_poor_income(n) * tax_eff;
		case culture::pop_strata::middle:
			return state.world.nation_get_total_middle_income(n) * tax_eff;
		case culture::pop_strata::rich:
			return state.world.nation_get_total_rich_income(n) * tax_eff;
		}
	}

	float estimate_subsidy_spending(sys::state& state, dcon::nation_id n) {
		return state.world.nation_get_subsidies_spending(n);
	}

	float estimate_war_subsidies_income(sys::state& state, dcon::nation_id n) {
		float total = 0.0f;
		for(auto uni : state.world.nation_get_unilateral_relationship_as_target(n)) {
			if(uni.get_war_subsidies()) {
				total += uni.get_target().get_maximum_military_costs() * state.defines.warsubsidies_percent;
			}
		}
		return total;
	}
	float estimate_reparations_income(sys::state& state, dcon::nation_id n) {
		float total = 0.0f;
		for(auto uni : state.world.nation_get_unilateral_relationship_as_target(n)) {
			if(uni.get_reparations() && state.current_date < uni.get_source().get_reparations_until()) {
				auto source = uni.get_source();
				auto const tax_eff = nations::tax_efficiency(state, n);
				auto total_tax_base = state.world.nation_get_total_rich_income(source) + state.world.nation_get_total_middle_income(source) + state.world.nation_get_total_poor_income(source);
				auto payout = total_tax_base * tax_eff * state.defines.reparations_tax_hit;
				total += payout;
			}
		}
		return total;
	}

	float estimate_war_subsidies_spending(sys::state& state, dcon::nation_id n) {
		float total = 0.0f;
		for(auto uni : state.world.nation_get_unilateral_relationship_as_source(n)) {
			if(uni.get_war_subsidies()) {
				total += uni.get_target().get_maximum_military_costs() * state.defines.warsubsidies_percent;
			}
		}
		return total;
	}

	float estimate_reparations_spending(sys::state& state, dcon::nation_id n) {
		float total = 0.0f;
		if(state.current_date < state.world.nation_get_reparations_until(n)) {
			for(auto uni : state.world.nation_get_unilateral_relationship_as_source(n)) {
				if(uni.get_reparations()) {
					auto const tax_eff = nations::tax_efficiency(state, n);
					auto total_tax_base = state.world.nation_get_total_rich_income(n) + state.world.nation_get_total_middle_income(n) + state.world.nation_get_total_poor_income(n);
					auto payout = total_tax_base * tax_eff * state.defines.reparations_tax_hit;
					total += payout;
				}
			}
		}
		return total;
	}

	float estimate_diplomatic_balance(sys::state& state, dcon::nation_id n) {
		float w_sub = estimate_war_subsidies_income(state, n) - estimate_war_subsidies_spending(state, n);
		float w_reps = estimate_reparations_income(state, n) - estimate_reparations_spending(state, n);
		return w_sub + w_reps;
	}

	float estimate_domestic_investment(sys::state& state, dcon::nation_id n) {
		auto const di_spending = float(state.world.nation_get_domestic_investment_spending(n)) / 100.0f;
		return di_spending *
			(state.world.nation_get_demographics(n, demographics::to_key(state, state.culture_definitions.capitalists))
			* state.world.nation_get_luxury_needs_costs(n, state.culture_definitions.capitalists)
			+ state.world.nation_get_demographics(n, demographics::to_key(state, state.culture_definitions.aristocrat))
			* state.world.nation_get_luxury_needs_costs(n, state.culture_definitions.aristocrat))
			* economy::pop_payout_factor;
	}

	float estimate_land_spending(sys::state& state, dcon::nation_id n) {
		float total = 0.0f;
		for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			total += state.world.nation_get_army_demand(n, cid) * economy::commodity_effective_price(state, n, cid) * state.world.nation_get_demand_satisfaction(n, cid);
		}
		return total;
	}

	float estimate_naval_spending(sys::state& state, dcon::nation_id n) {
		float total = 0.0f;
		for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			total += state.world.nation_get_navy_demand(n, cid) * economy::commodity_effective_price(state, n, cid) * state.world.nation_get_demand_satisfaction(n, cid);
		}
		return total;
	}

	float estimate_construction_spending(sys::state& state, dcon::nation_id n) {
		float total = 0.0f;
		float admin_eff = state.world.nation_get_administrative_efficiency(n);
		float admin_cost_factor = 2.0f - admin_eff;

		for(auto lc : state.world.nation_get_province_land_construction(n)) {
			auto province = state.world.pop_get_province_from_pop_location(state.world.province_land_construction_get_pop(lc));
			if(state.world.province_get_nation_from_province_control(province) == n) {
				auto& base_cost = state.military_definitions.unit_base_definitions[state.world.province_land_construction_get_type(lc)].build_cost;
				auto& current_purchased = state.world.province_land_construction_get_purchased_goods(lc);
				float construction_time = float(state.military_definitions.unit_base_definitions[state.world.province_land_construction_get_type(lc)].build_time);
				for(uint32_t i = 0; i < economy::commodity_set::set_size; ++i) {
					if(base_cost.commodity_type[i]) {
						if(current_purchased.commodity_amounts[i] < base_cost.commodity_amounts[i] * admin_cost_factor)
							total += economy::commodity_effective_price(state, n, base_cost.commodity_type[i]) * state.world.nation_get_demand_satisfaction(n, base_cost.commodity_type[i]) * base_cost.commodity_amounts[i] * admin_cost_factor / construction_time;
					} else {
						break;
					}
				}
			}
		}
		for(auto po : state.world.nation_get_province_ownership(n)) {
			auto p = po.get_province();
			if(state.world.province_get_nation_from_province_control(p) != n)
				continue;
			auto rng = state.world.province_get_province_naval_construction(p);
			if(rng.begin() != rng.end()) {
				auto c = *(rng.begin());
				auto& base_cost = state.military_definitions.unit_base_definitions[c.get_type()].build_cost;
				auto& current_purchased = c.get_purchased_goods();
				float construction_time = float(state.military_definitions.unit_base_definitions[c.get_type()].build_time);
				for(uint32_t i = 0; i < economy::commodity_set::set_size; ++i) {
					if(base_cost.commodity_type[i]) {
						if(current_purchased.commodity_amounts[i] < base_cost.commodity_amounts[i] * admin_cost_factor)
							total += economy::commodity_effective_price(state, n, base_cost.commodity_type[i]) * state.world.nation_get_demand_satisfaction(n, base_cost.commodity_type[i]) * base_cost.commodity_amounts[i] * admin_cost_factor / construction_time;
					} else {
						break;
					}
				}
			}
		}
		for(auto c : state.world.nation_get_province_building_construction(n)) {
			if(n == c.get_province().get_nation_from_province_control() && !c.get_is_pop_project()) {
				auto t = economy::province_building_type(c.get_type());
				auto& base_cost = state.economy_definitions.building_definitions[int32_t(t)].cost;
				auto& current_purchased = c.get_purchased_goods();
				float construction_time = float(state.economy_definitions.building_definitions[int32_t(t)].time);
				for(uint32_t i = 0; i < economy::commodity_set::set_size; ++i) {
					if(base_cost.commodity_type[i]) {
						if(current_purchased.commodity_amounts[i] < base_cost.commodity_amounts[i] * admin_cost_factor)
							total += economy::commodity_effective_price(state, n, base_cost.commodity_type[i]) * state.world.nation_get_demand_satisfaction(n, base_cost.commodity_type[i]) * base_cost.commodity_amounts[i] * admin_cost_factor / construction_time;
					} else {
						break;
					}
				}
			}
		}
		for(auto c : state.world.nation_get_state_building_construction(n)) {
			if(!c.get_is_pop_project()) {
				auto& base_cost = c.get_type().get_construction_costs();
				auto& current_purchased = c.get_purchased_goods();
				float construction_time = float(c.get_type().get_construction_time());
				float cost_mod = economy_factory::factory_build_cost_modifier(state, c.get_nation(), c.get_is_pop_project());
				for(uint32_t i = 0; i < economy::commodity_set::set_size; ++i) {
					if(base_cost.commodity_type[i]) {
						if(current_purchased.commodity_amounts[i] < base_cost.commodity_amounts[i] * cost_mod)
							total += economy::commodity_effective_price(state, n, base_cost.commodity_type[i]) * state.world.nation_get_demand_satisfaction(n, base_cost.commodity_type[i]) * base_cost.commodity_amounts[i] * cost_mod / construction_time;
					} else {
						break;
					}
				}
			}
		}

		return total;
	}

	float estimate_war_subsidies(sys::state& state, dcon::nation_id n) {
		/* total-nation-expenses x defines:WARSUBSIDIES_PERCENT */
		return state.world.nation_get_maximum_military_costs(n) * state.defines.warsubsidies_percent;
	}

	float estimate_daily_income(sys::state& state, dcon::nation_id n) {
		auto const tax_eff = nations::tax_efficiency(state, n);
		auto const poor_income = state.world.nation_get_total_poor_income(n) * float(state.world.nation_get_poor_tax(n)) / 100.f;
		auto const mid_income = state.world.nation_get_total_middle_income(n) * float(state.world.nation_get_middle_tax(n)) / 100.f;
		auto const rich_income = state.world.nation_get_total_rich_income(n) * float(state.world.nation_get_rich_tax(n)) / 100.f;
		return tax_eff * (poor_income + mid_income + rich_income);
	}
}
