#pragma once

#include "container_types.hpp"
#include "dcon_generated.hpp"
#include "economy.hpp"

namespace factory {

	inline constexpr float factory_closed_threshold = 0.0001f;

	struct construction_status {
		float progress = 0.0f; // in range [0,1)
		bool is_under_construction = false;
	};
	struct new_factory {
		float progress = 0.0f;
		dcon::factory_type_id type;
	};

	struct upgraded_factory {
		float progress = 0.0f;
		dcon::factory_type_id type;
	};
	float factory_max_employment(sys::state const& state, dcon::factory_id f);

	bool has_factory(sys::state const& state, dcon::state_instance_id si);

	float factory_min_input_available(sys::state& state, dcon::nation_id n, dcon::factory_type_fat_id fac_type);
	float factory_input_total_cost(sys::state& state, dcon::nation_id n, dcon::factory_type_fat_id fac_type);
	float factory_min_e_input_available(sys::state& state, dcon::nation_id n, dcon::factory_type_fat_id fac_type);
	float factory_e_input_total_cost(sys::state& state, dcon::nation_id n, dcon::factory_type_fat_id fac_type);
	float factory_input_multiplier(sys::state& state, dcon::factory_fat_id fac, dcon::nation_id n, dcon::province_id p, dcon::state_instance_id s);
	float factory_throughput_multiplier(sys::state& state, dcon::factory_type_fat_id fac_type, dcon::nation_id n, dcon::province_id p, dcon::state_instance_id s);
	float factory_output_multiplier(sys::state& state, dcon::factory_fat_id fac, dcon::nation_id n, dcon::province_id p);
	float factory_max_production_scale(sys::state& state, dcon::factory_fat_id fac, float mobilization_impact, bool occupied);
	float factory_desired_raw_profit(dcon::factory_fat_id fac, float spendings);
	float factory_total_employment(sys::state const& state, dcon::factory_id f);
	float factory_primary_employment(sys::state const& state, dcon::factory_id f);
	float factory_secondary_employment(sys::state const& state, dcon::factory_id f);
	int32_t factory_priority(sys::state const& state, dcon::factory_id f);
	void set_factory_priority(sys::state& state, dcon::factory_id f, int32_t priority);
	bool factory_is_profitable(sys::state const& state, dcon::factory_id f);
	construction_status factory_upgrade(sys::state& state, dcon::factory_id f);

	bool state_contains_constructed_factory(sys::state& state, dcon::state_instance_id si, dcon::factory_type_id ft);
	bool state_contains_factory(sys::state& state, dcon::state_instance_id s, dcon::factory_type_id ft);
	int32_t state_factory_count(sys::state& state, dcon::state_instance_id sid);
	int32_t state_built_factory_count(sys::state& state, dcon::state_instance_id sid);
	void try_add_factory_to_state(sys::state& state, dcon::state_instance_id s, dcon::factory_type_id t);

	void prune_factories(sys::state& state); // get rid of closed factories in full states
	template<typename T>
	float nation_factory_consumption(sys::state& state, dcon::nation_id n, dcon::commodity_id c);

	float factory_build_cost_modifier(sys::state& state, dcon::nation_id n, bool pop_project);
	void update_single_factory_consumption(sys::state& state, dcon::factory_id f, dcon::nation_id n, dcon::province_id p, dcon::state_instance_id s, float mobilization_impact, float expected_min_wage, bool occupied);
	void update_factory_triggered_modifiers(sys::state& state);
	void update_single_factory_production(sys::state& state, dcon::factory_id f, dcon::nation_id n, float expected_min_wage);
	void add_factory_level_to_state(sys::state& state, dcon::state_instance_id s, dcon::factory_type_id t, bool is_upgrade);
	float sum_of_factory_triggered_modifiers(sys::state& state, dcon::factory_type_id ft, dcon::state_instance_id s);
	economy::profit_distribution distribute_factory_profit(sys::state const& state, dcon::state_instance_const_fat_id s, float min_wage, float total_profit);
}
