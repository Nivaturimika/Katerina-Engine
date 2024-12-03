#pragma once

#include "container_types.hpp"
#include "dcon_generated.hpp"

namespace economy_rgo {
	float rgo_effective_size(sys::state const& state, dcon::nation_id n, dcon::province_id p, dcon::commodity_id c);
	float rgo_total_effective_size(sys::state& state, dcon::nation_id n, dcon::province_id p);
	float rgo_total_employment(sys::state& state, dcon::nation_id n, dcon::province_id p);
	float rgo_full_production_quantity(sys::state const& state, dcon::nation_id n, dcon::province_id p, dcon::commodity_id c);
	float rgo_max_employment(sys::state& state, dcon::nation_id n, dcon::province_id p, dcon::commodity_id c);
	float rgo_total_max_employment(sys::state& state, dcon::nation_id n, dcon::province_id p);
	void update_province_rgo_consumption(sys::state& state, dcon::province_id p, dcon::nation_id n, float mobilization_impact, float expected_min_wage, bool occupied);
	economy::rgo_workers_breakdown rgo_relevant_population(sys::state& state, dcon::province_id p, dcon::nation_id n);
	void update_province_rgo_production(sys::state& state, dcon::province_id p, dcon::nation_id n);
	float rgo_desired_worker_norm_profit(sys::state& state, dcon::province_id p, dcon::nation_id n, float min_wage, float total_relevant_population);
	//float rgo_expected_worker_norm_profit(sys::state& state, dcon::province_id p, dcon::nation_id n, dcon::commodity_id c);

	void update_rgo_employment(sys::state& state);
	
}
