#pragma once

#include "container_types.hpp"
#include "dcon_generated.hpp"

namespace economy_estimations {
	float estimate_gold_income(sys::state& state, dcon::nation_id n);
	float estimate_tariff_income(sys::state& state, dcon::nation_id n);
	float estimate_social_spending(sys::state& state, dcon::nation_id n);
	float estimate_pop_payouts_by_income_type(sys::state& state, dcon::nation_id n, culture::income_type in);
	float estimate_tax_income_by_strata(sys::state& state, dcon::nation_id n, culture::pop_strata ps);
	float estimate_subsidy_spending(sys::state& state, dcon::nation_id n);
	float estimate_diplomatic_balance(sys::state& state, dcon::nation_id n);
	float estimate_domestic_investment(sys::state& state, dcon::nation_id n);
	
	float estimate_land_spending(sys::state& state, dcon::nation_id n);
	float estimate_naval_spending(sys::state& state, dcon::nation_id n);
	float estimate_construction_spending(sys::state& state, dcon::nation_id n);
	float estimate_war_subsidies_spending(sys::state& state, dcon::nation_id n);
	float estimate_reparations_spending(sys::state& state, dcon::nation_id n);
	float estimate_war_subsidies_income(sys::state& state, dcon::nation_id n);
	float estimate_reparations_income(sys::state& state, dcon::nation_id n);
	float estimate_overseas_penalty_spending(sys::state& state, dcon::nation_id n);
	float estimate_stockpile_filling_spending(sys::state& state, dcon::nation_id n);
	
	// NOTE: used to estimate how much you will pay if you were to subsidize a particular nation,
	// *not* how much you are paying at the moment
	float estimate_war_subsidies(sys::state& state, dcon::nation_id n);
	float estimate_daily_income(sys::state& state, dcon::nation_id n);
}
