#pragma once

#include "container_types.hpp"
#include "dcon_generated.hpp"
#include "economy.hpp"

namespace economy_factory {

	inline constexpr float factory_closed_threshold = 0.0001f;

	inline constexpr float day_1_build_time_modifier_non_factory = 2.f;
	inline constexpr float day_inf_build_time_modifier_non_factory = 0.5f;
	inline constexpr float day_1_derivative_non_factory = -0.2f;

	inline constexpr float diff_non_factory = day_1_build_time_modifier_non_factory - day_inf_build_time_modifier_non_factory;
	inline constexpr float shift_non_factory = -diff_non_factory / day_1_derivative_non_factory;
	inline constexpr float slope_non_factory = diff_non_factory * shift_non_factory;

	inline constexpr float day_1_build_time_modifier_factory = 1.f;
	inline constexpr float day_inf_build_time_modifier_factory = 0.5f;
	inline constexpr float day_1_derivative_factory = -0.2f;

	inline constexpr float diff_factory = day_1_build_time_modifier_factory - day_inf_build_time_modifier_factory;
	inline constexpr float shift_factory = -diff_factory / day_1_derivative_factory;
	inline constexpr float slope_factory = diff_factory * shift_factory;

	struct new_factory {
		float progress = 0.0f;
		dcon::factory_type_id type;
	};

	struct upgraded_factory {
		float progress = 0.0f;
		dcon::factory_type_id type;
	};	


	bool factory_is_profitable(sys::state const& state, dcon::factory_id factory_id);

	bool has_factory(sys::state const& state, dcon::state_instance_id state_instance_id);

	bool nation_is_constructing_factories(sys::state& state, dcon::nation_id nation_id);
	bool nation_has_closed_factories(sys::state& state, dcon::nation_id nation_id);

	bool state_contains_constructed_factory(sys::state& state, dcon::state_instance_id state_instance_id, dcon::factory_type_id factory_type_id);
	bool state_contains_factory(sys::state& state, dcon::state_instance_id state_instance_id, dcon::factory_type_id factory_type_id);


	float factory_build_cost_modifier(sys::state& state, dcon::nation_id nation_id, bool pop_project);
	float factory_desired_raw_profit(dcon::factory_fat_id factory_fat_id, float spendings);
	float factory_efficiency_input_total_cost(sys::state& state, dcon::nation_id nation_id, dcon::factory_type_fat_id factory_type_fat_id);
	float factory_full_production_quantity(sys::state const& state, dcon::factory_id factory_id, dcon::nation_id nation_id, float mobilization_impact);

	float factory_input_multiplier(sys::state& state, dcon::factory_fat_id factory_fat_id, dcon::nation_id nation_id, dcon::province_id province_id, dcon::state_instance_id state_instance_id);
	float factory_input_total_cost(sys::state& state, dcon::nation_id nation_id, dcon::factory_type_fat_id factory_type_fat_id);

	float factory_max_employment(sys::state const& state, dcon::factory_id factory_id);
	float factory_max_production_scale(sys::state& state, dcon::factory_fat_id factory_fat_id, float mobilization_impact, bool occupied);

	float factory_min_input_available(sys::state& state, dcon::nation_id nation_id, dcon::factory_type_fat_id factory_type_fat_id);
	float factory_min_efficiency_input_available(sys::state& state, dcon::nation_id nation_id, dcon::factory_type_fat_id factory_type_fat_id);

	float factory_output_multiplier(sys::state& state, dcon::factory_fat_id factory_fat_id, dcon::nation_id nation_id, dcon::province_id province_id);
	float factory_primary_employment(sys::state const& state, dcon::factory_id factory_id);
	float factory_secondary_employment(sys::state const& state, dcon::factory_id factory_id);

	float factory_throughput_multiplier(sys::state& state, dcon::factory_type_fat_id factory_type_fat_id, dcon::nation_id nation_id, dcon::province_id province_id, dcon::state_instance_id state_instance_id);
	float factory_total_employment(sys::state const& state, dcon::factory_id factory_id);

	float global_factory_construction_time_modifier(sys::state& state);
	float global_non_factory_construction_time_modifier(sys::state& state);

	float nation_factory_consumption(sys::state& state, dcon::nation_id nation_id, dcon::commodity_id commodity_id);
	float pop_factory_min_wage(sys::state& state, dcon::nation_id nation_id, float min_wage_factor);

	float sum_of_factory_triggered_modifiers(sys::state& state, dcon::factory_type_id factory_type_id, dcon::state_instance_id state_instance_id);
	float update_factory_scale(sys::state& state, dcon::factory_fat_id factory_fat_id, float max_production_scale, float raw_profit, float desired_raw_profit);


	int32_t factory_priority(sys::state const& state, dcon::factory_id factory_id);

	int32_t state_built_factory_count(sys::state& state, dcon::state_instance_id state_instance_id);
	int32_t state_factory_count(sys::state& state, dcon::state_instance_id state_instance_id);
	

	void add_factory_level_to_state(sys::state& state, dcon::state_instance_id state_instance_id, dcon::factory_type_id factory_type_id, bool is_upgrade);
	void prune_factories(sys::state& state); // get rid of closed factories in full states	

	void set_factory_priority(sys::state& state, dcon::factory_id factory_id, int32_t priority);
	void try_add_factory_to_state(sys::state& state, dcon::state_instance_id state_instance_id, dcon::factory_type_id factory_type_id);

	void update_factory_employment(sys::state& state);
	void update_factory_triggered_modifiers(sys::state& state);

	void update_single_factory_consumption(sys::state& state, dcon::factory_id factory_id, dcon::nation_id nation_id, dcon::province_id province_id, dcon::state_instance_id state_instance_id, float mobilization_impact, float expected_min_wage, bool occupied);;
	void update_single_factory_production(sys::state& state, dcon::factory_id factory_id, dcon::nation_id nation_id, float expected_min_wage);

		
	economy::construction_status factory_upgrade(sys::state& state, dcon::factory_id factory_id);


	economy::profit_distribution distribute_factory_profit(sys::state const& state, dcon::state_instance_const_fat_id state_instance_fat_id, float min_wage, float total_profit);
}
