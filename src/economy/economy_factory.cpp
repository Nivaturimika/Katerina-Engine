#include "economy_factory.hpp"
#include "demographics.hpp"
#include "dcon_generated.hpp"
#include "ai.hpp"
#include "system_state.hpp"
#include "prng.hpp"
#include "math_fns.hpp"
#include "nations_templates.hpp"
#include "province_templates.hpp"
#include "triggers.hpp"
#include "economy_factory_templates.hpp"
#include "economy.hpp"

namespace economy_factory {

	bool factory_is_profitable(sys::state const& state, dcon::factory_id factory_id) {
		return state.world.factory_get_unprofitable(factory_id) == false || state.world.factory_get_subsidized(factory_id);
	}


	bool has_factory(sys::state const& state, dcon::state_instance_id state_instance_id) {
		auto state_instance_def = state.world.state_instance_get_definition(state_instance_id);
		auto owner_of_state = state.world.state_instance_get_nation_from_state_ownership(state_instance_id);
		auto state_building_construction = state.world.state_instance_get_state_building_construction(state_instance_id);
		if(state_building_construction.begin() != state_building_construction.end())
			return true;

		for(auto state_membership_fat_id : state.world.state_definition_get_abstract_state_membership(state_instance_def)) {
			if(state_membership_fat_id.get_province().get_nation_from_province_ownership() == owner_of_state) {
				auto factory_location = state_membership_fat_id.get_province().get_factory_location();
				if(factory_location.begin() != factory_location.end())
					return true;
			}
		}
		return false;
	}


	bool nation_is_constructing_factories(sys::state& state, dcon::nation_id nation_id) {
		auto state_building_construction = state.world.nation_get_state_building_construction(nation_id);
		return state_building_construction.begin() != state_building_construction.end();
	}

	bool nation_has_closed_factories(sys::state& state, dcon::nation_id nation_id) { // TODO - should be "good" now
		auto nation_fat_id = dcon::fatten(state.world, nation_id);
		for(auto province_owner : nation_fat_id.get_province_ownership()) {
			auto province = province_owner.get_province();
			for(auto factory_location : province.get_factory_location()) {
				auto production_scale = factory_location.get_factory().get_production_scale();
				if(production_scale < factory_closed_threshold) {
					return true;
				}
			}
		}
		return false;
	}


	bool state_contains_constructed_factory(sys::state& state, dcon::state_instance_id state_instance_id, dcon::factory_type_id factory_type_id) {
		auto state_definition_fat_id = state.world.state_instance_get_definition(state_instance_id);
		for(auto state_membership_id : state.world.state_definition_get_abstract_state_membership(state_definition_fat_id)) {
			if(state_membership_id.get_province().get_state_membership() == state_instance_id) {
				for(auto factory_location_fat_id : state_membership_id.get_province().get_factory_location()) {
					if(factory_location_fat_id.get_factory().get_building_type() == factory_type_id)
						return true;
				}
			}
		}
		return false;
	}

	bool state_contains_factory(sys::state& state, dcon::state_instance_id state_instance_id, dcon::factory_type_id factory_type_id) {
		auto state_definition_fat_id = state.world.state_instance_get_definition(state_instance_id);
		for(auto state_membership_id : state.world.state_definition_get_abstract_state_membership(state_definition_fat_id)) {
			if(state_membership_id.get_province().get_state_membership() == state_instance_id) {
				for(auto factory_location_fat_id : state_membership_id.get_province().get_factory_location()) {
					if(factory_location_fat_id.get_factory().get_building_type() == factory_type_id)
						return true;
				}
			}
		}
		for(auto state_building_construction_fat_id : state.world.state_instance_get_state_building_construction(state_instance_id)) {
			if(state_building_construction_fat_id.get_type() == factory_type_id)
				return true;
		}
		return false;
	}



	float factory_build_cost_modifier(sys::state& state, dcon::nation_id nation_id, bool pop_project) {
		float admin_efficiency = state.world.nation_get_administrative_efficiency(nation_id);
		float factory_modifier = std::max(0.001f, state.world.nation_get_modifier_values(nation_id, sys::national_mod_offsets::factory_cost) + 1.0f);
		float pop_factory_modifier = std::max(0.001f, state.world.nation_get_modifier_values(nation_id, sys::national_mod_offsets::factory_owner_cost));
		return (pop_project ? pop_factory_modifier : (2.0f - admin_efficiency)) * factory_modifier;
	}

	float factory_desired_raw_profit(dcon::factory_fat_id factory_fat_id, float spendings) {
		return spendings * (1.2f + factory_fat_id.get_secondary_employment() * factory_fat_id.get_level() / 150.f);
	}

	float factory_efficiency_input_total_cost(sys::state& state, dcon::nation_id nation_id, dcon::factory_type_fat_id factory_type_fat_id) {
		float efficiency_input_total = 0.0f;
		auto& efficiency_inputs = factory_type_fat_id.get_efficiency_inputs();
		for(uint32_t i = 0; i < economy::small_commodity_set::set_size; ++i) {
			if(efficiency_inputs.commodity_type[i]) {
				efficiency_input_total += efficiency_inputs.commodity_amounts[i]
					* economy::commodity_effective_price(state, nation_id, efficiency_inputs.commodity_type[i]);
			} else {
				break;
			}
		}
		return efficiency_input_total;
	}

	float factory_full_production_quantity(sys::state const& state, dcon::factory_id factory_id, dcon::nation_id nation_id, float mobilization_impact) {
		auto factory_fat_id = fatten(state.world, factory_id);
		auto factory_type_fat_id = factory_fat_id.get_building_type();
		float throughput_multiplier = (state.world.nation_get_factory_goods_throughput(nation_id, factory_type_fat_id.get_output()) + 1.0f);
		float output_multiplier = state.world.nation_get_factory_goods_output(nation_id, factory_type_fat_id.get_output())
			+ 1.0f
			+ factory_fat_id.get_secondary_employment()
			* (1.0f - state.economy_definitions.craftsmen_fraction)
			* 1.5f
			* 2.f; // additional multiplier to give advantage to "old industrial giants" which have a bunch of clerks already
		float max_production_scale = factory_fat_id.get_primary_employment()
			* factory_fat_id.get_level()
			* std::max(0.0f, mobilization_impact);
		return throughput_multiplier * output_multiplier * max_production_scale;
	}


	float factory_input_multiplier(sys::state& state, dcon::factory_fat_id factory_fat_id, dcon::nation_id nation_id, dcon::province_id province_id, dcon::state_instance_id state_instance_id) {
		float total_workers = factory_max_employment(state, factory_fat_id);
		float small_size_effect = 1.f;
		float small_bound = 5.f * state.defines.alice_factory_per_level_employment;
		if(total_workers < small_bound) {
			small_size_effect = 0.5f + total_workers / small_bound * 0.5f;
		}

		float total_state_pop = std::max(0.0001f, state.world.state_instance_get_demographics(state_instance_id, demographics::total));
		float capitalists = state.world.state_instance_get_demographics(state_instance_id, demographics::to_key(state, state.culture_definitions.capitalists));
		float owner_fraction = total_state_pop > 0.f ? std::min(0.05f, capitalists / total_state_pop) : 0.0f;

		return small_size_effect * std::max(0.1f, (state.defines.alice_inputs_base_factor
					+ state.world.province_get_modifier_values(province_id, sys::provincial_mod_offsets::local_factory_input)
					+ state.world.nation_get_modifier_values(nation_id, sys::national_mod_offsets::factory_input)
					+ owner_fraction * -2.5f));
	}

	float factory_input_total_cost(sys::state& state, dcon::nation_id nation_id, dcon::factory_type_fat_id factory_type_fat_id) {
		float input_total = 0.0f;
		auto& inputs = factory_type_fat_id.get_inputs();
		for(uint32_t i = 0; i < economy::commodity_set::set_size; ++i) {
			if(inputs.commodity_type[i]) {
				input_total += inputs.commodity_amounts[i] * economy::commodity_effective_price(state, nation_id, inputs.commodity_type[i]);
			} else {
				break;
			}
		}
		return input_total;
	}


	float factory_max_employment(sys::state const& state, dcon::factory_id factory_id) {
		return state.defines.alice_factory_per_level_employment * state.world.factory_get_level(factory_id);
	}

	float factory_max_production_scale(sys::state& state, dcon::factory_fat_id factory_fat_id, float mobilization_impact, bool occupied) {
		return factory_fat_id.get_primary_employment() * factory_fat_id.get_level() * (occupied ? 0.1f : 1.0f) * std::max(0.0f, mobilization_impact);
	}


	float factory_min_input_available(sys::state& state, dcon::nation_id nation_id, dcon::factory_type_fat_id factory_type_fat_id) {
		float min_input_available = 1.0f;
		auto& inputs = factory_type_fat_id.get_inputs();
		for(uint32_t i = 0; i < economy::commodity_set::set_size; ++i) {
			if(inputs.commodity_type[i]) {
				min_input_available = std::min(min_input_available, state.world.nation_get_demand_satisfaction(nation_id, inputs.commodity_type[i]));
			} else {
				break;
			}
		}
		return min_input_available;
	}

	float factory_min_efficiency_input_available(sys::state& state, dcon::nation_id nation_id, dcon::factory_type_fat_id factory_type_fat_id) {
		float min_efficiency_input_available = 1.0f;
		auto& efficiency_inputs = factory_type_fat_id.get_efficiency_inputs();
		for(uint32_t i = 0; i < economy::small_commodity_set::set_size; ++i) {
			if(efficiency_inputs.commodity_type[i]) {
				min_efficiency_input_available = std::min(min_efficiency_input_available, state.world.nation_get_demand_satisfaction(nation_id, efficiency_inputs.commodity_type[i]));
			} else {
				break;
			}
		}
		return min_efficiency_input_available;
	}


	float factory_output_multiplier(sys::state& state, dcon::factory_fat_id factory_fat_id, dcon::nation_id nation_id, dcon::province_id province_id) {
		auto fac_type = factory_fat_id.get_building_type();

		return state.world.nation_get_factory_goods_output(nation_id, fac_type.get_output())
			+ state.world.province_get_modifier_values(province_id, sys::provincial_mod_offsets::local_factory_output)
			+ state.world.nation_get_modifier_values(nation_id, sys::national_mod_offsets::factory_output)
			+ factory_fat_id.get_secondary_employment()
			* (1.0f - state.economy_definitions.craftsmen_fraction) * 1.5f + 1.0f;
	}

	float factory_primary_employment(sys::state const& state, dcon::factory_id factory_id) {
		auto primary_employment = state.world.factory_get_primary_employment(factory_id);
		return factory_max_employment(state, factory_id) * (state.economy_definitions.craftsmen_fraction * primary_employment);
	}

	float factory_secondary_employment(sys::state const& state, dcon::factory_id factory_id) {
		auto secondary_employment = state.world.factory_get_secondary_employment(factory_id);
		return factory_max_employment(state, factory_id) * ((1 - state.economy_definitions.craftsmen_fraction) * secondary_employment);
	}


	float factory_throughput_multiplier(sys::state& state, dcon::factory_type_fat_id factory_type_fat_id, dcon::nation_id nationd_id, dcon::province_id province_id, dcon::state_instance_id state_instance_id) {
		return state.world.nation_get_factory_goods_throughput(nationd_id, factory_type_fat_id.get_output())
			+ state.world.province_get_modifier_values(province_id, sys::provincial_mod_offsets::local_factory_throughput)
			+ state.world.nation_get_modifier_values(nationd_id, sys::national_mod_offsets::factory_throughput) + 1.0f;
	}

	float factory_total_employment(sys::state const& state, dcon::factory_id factory_id) {
		// TODO: Document this, also is this a stub?
		auto primary_employment = state.world.factory_get_primary_employment(factory_id);
		auto secondary_employment = state.world.factory_get_secondary_employment(factory_id);
		return factory_max_employment(state, factory_id) * (state.economy_definitions.craftsmen_fraction
			* primary_employment + (1 - state.economy_definitions.craftsmen_fraction) * secondary_employment);
	}


	float global_factory_construction_time_modifier(sys::state& state) {
		return 1.f;
	}

	float global_non_factory_construction_time_modifier(sys::state& state) {
		return 1.f;
	}


	float nation_factory_consumption(sys::state& state, dcon::nation_id nation_id, dcon::commodity_id commodity_id) {
		auto factory_consumption_amount = 0.f;
		for(const auto province_ownership_fat_id : state.world.nation_get_province_ownership(nation_id)) {
			for(const auto factory_location_fat_id : province_ownership_fat_id.get_province().get_factory_location()) {
				auto factory_fat_id = factory_location_fat_id.get_factory();
				auto factory_type_fat_id = factory_location_fat_id.get_factory().get_building_type();
				auto occupied = province_ownership_fat_id.get_province().get_nation_from_province_control() != province_ownership_fat_id.get_province().get_nation_from_province_ownership();
				auto mobilization_impact = military::mobilization_impact(state, nation_id);
				auto province_fat_id = province_ownership_fat_id.get_province();
				auto state_instance_fat_id = province_ownership_fat_id.get_province().get_state_membership();
				//
				float total_workers = factory_max_employment(state, factory_fat_id);
				float several_workers_scale = 10.f / total_workers;
				float max_production_scale = factory_max_production_scale(state, factory_fat_id, mobilization_impact, occupied);
				float current_workers = total_workers * max_production_scale;
				//inputs
				float input_total = factory_input_total_cost(state, nation_id, factory_type_fat_id);
				float min_input_available = factory_min_input_available(state, nation_id, factory_type_fat_id);
				//modifiers
				float input_multiplier = std::max(0.f, factory_input_multiplier(state, factory_fat_id, nation_id, province_fat_id, state_instance_fat_id));
				float throughput_multiplier = std::max(0.f, factory_throughput_multiplier(state, factory_type_fat_id, nation_id, province_fat_id, state_instance_fat_id) + factory_fat_id.get_triggered_modifiers());
				float effective_production_scale = std::min(factory_fat_id.get_production_scale() * factory_fat_id.get_level(), max_production_scale);
				//
				auto const& inputs = factory_type_fat_id.get_inputs();
				float input_scale = input_multiplier * throughput_multiplier * effective_production_scale * (0.1f + min_input_available * 0.9f);
				for(uint32_t i = 0; i < economy::commodity_set::set_size; i++) {
					if(inputs.commodity_type[i]) {
						if(inputs.commodity_type[i] == commodity_id) {
							factory_consumption_amount += inputs.commodity_amounts[i] * factory_location_fat_id.get_factory().get_actual_production();
						}
					} else {
						break;
					}
				}
				//
				auto const& efficiency_inputs = factory_type_fat_id.get_efficiency_inputs();
				float efficiency_input_total = factory_efficiency_input_total_cost(state, nation_id, factory_type_fat_id);
				float min_efficiency_input_available = factory_min_efficiency_input_available(state, nation_id, factory_type_fat_id);
				auto const modifier_values = state.world.nation_get_modifier_values(nation_id, sys::national_mod_offsets::factory_maintenance) + 1.0f;
				float efficiency_input_scale = input_multiplier * throughput_multiplier * effective_production_scale * (0.1f + min_efficiency_input_available * 0.9f);
				for(uint32_t i = 0; i < economy::small_commodity_set::set_size; ++i) {
					if(efficiency_inputs.commodity_type[i] == commodity_id) {
						if(efficiency_inputs.commodity_type[i] == commodity_id) {
							economy::register_intermediate_demand(state, nation_id, efficiency_inputs.commodity_type[i], modifier_values * efficiency_input_scale * efficiency_inputs.commodity_amounts[i] * (0.1f + min_efficiency_input_available * 0.9f));
						}
					} else {
						break;
					}
				}
			}
		}
		return factory_consumption_amount;
	}

	float pop_factory_min_wage(sys::state& state, dcon::nation_id nation_id, float min_wage_factor) {
		float employed = state.world.nation_get_demographics(nation_id, demographics::to_employment_key(state, state.culture_definitions.primary_factory_worker));
		float total = state.world.nation_get_demographics(nation_id, demographics::to_key(state, state.culture_definitions.primary_factory_worker));
		float unemployement_crisis_measures = 1.f;
		if(total > 0.f) {
			unemployement_crisis_measures = employed / total;
		}
		float life_needs_cost = state.world.nation_get_life_needs_costs(nation_id, state.culture_definitions.primary_factory_worker);
		float everyday_needs_cost = state.world.nation_get_everyday_needs_costs(nation_id, state.culture_definitions.primary_factory_worker);
		return min_wage_factor * (life_needs_cost + everyday_needs_cost) * 1.1f * unemployement_crisis_measures * unemployement_crisis_measures * unemployement_crisis_measures;
	}


	float sum_of_factory_triggered_modifiers(sys::state& state, dcon::factory_type_id factory_type_id, dcon::state_instance_id state_instance_id) {
		auto factory_type_fat_id = fatten(state.world, factory_type_id);
		auto nation_id = state.world.state_instance_get_nation_from_state_ownership(state_instance_id);
		float sum = 1.f;
		for(uint32_t i = 0; i < sys::max_factory_bonuses; i++) {
			if(auto bonus_trigger = factory_type_fat_id.get_bonus_trigger()[i]; bonus_trigger && trigger::evaluate(state, bonus_trigger, trigger::to_generic(state_instance_id), trigger::to_generic(nation_id), 0)) {
				sum += factory_type_fat_id.get_bonus_amount()[i];
			}
		}
		return sum;
	}

	float update_factory_scale(sys::state& state, dcon::factory_fat_id factory_fat_id, float max_production_scale, float raw_profit, float desired_raw_profit) {
		float total_workers = factory_max_employment(state, factory_fat_id);
		float several_workers_scale = 10.f / total_workers;
		// we don't want for factories to change "world balance" too much individually
		// when relative production is high, we want to reduce our speed
		// for example, if relative production is 1.0, then we want to clamp our speed with ~0.01 or something small like this;
		// and if relative production is ~0, then clamps are not needed
		float relative_production_amount
			=
			state.world.factory_type_get_output_amount(factory_fat_id.get_building_type())
			/ (
				state.world.commodity_get_total_production(factory_fat_id.get_building_type().get_output())
				+ state.world.commodity_get_total_real_demand(factory_fat_id.get_building_type().get_output())
				+ 10.f
			);

		float relative_modifier = (1.f / (relative_production_amount + 0.01f)) / 1000.f;

		float effective_production_scale = 0.0f;
		if(state.world.factory_get_subsidized(factory_fat_id)) {
			auto new_production_scale = std::min(1.0f, factory_fat_id.get_production_scale() + several_workers_scale * factory_fat_id.get_level() * 10.f);
			factory_fat_id.set_production_scale(new_production_scale);
			return std::min(new_production_scale * factory_fat_id.get_level(), max_production_scale);
		}

		float over_profit_ratio = (raw_profit) / (desired_raw_profit + 0.0001f) - 1.f;
		float under_profit_ratio = (desired_raw_profit) / (raw_profit + 0.0001f) - 1.f;
		float speed_modifier = (over_profit_ratio - under_profit_ratio);
		float speed = economy::production_scale_delta * speed_modifier + several_workers_scale * ((raw_profit - desired_raw_profit > 0.f) ? 1.f : -1.f);

		speed = std::clamp(speed, -relative_modifier, relative_modifier);

		auto new_production_scale = std::clamp(factory_fat_id.get_production_scale() + speed, 0.f, 1.f);
		factory_fat_id.set_production_scale(new_production_scale);
		return std::min(new_production_scale * factory_fat_id.get_level(), max_production_scale);
	}



	int32_t factory_priority(sys::state const& state, dcon::factory_id factory_id) {
		return (state.world.factory_get_priority_low(factory_id) ? 1 : 0) + (state.world.factory_get_priority_high(factory_id) ? 2 : 0);
	}


	int32_t state_built_factory_count(sys::state& state, dcon::state_instance_id state_instance_id) {
		dcon::nation_id nation_id = state.world.state_instance_get_nation_from_state_ownership(state_instance_id);
		int32_t number_of_factories = 0;
		auto state_definition_fat_id = state.world.state_instance_get_definition(state_instance_id);
		for(auto state_membership_fat_id : state.world.state_definition_get_abstract_state_membership(state_definition_fat_id))
			if(state_membership_fat_id.get_province().get_nation_from_province_ownership() == nation_id)
				number_of_factories += int32_t(state.world.province_get_factory_location(state_membership_fat_id.get_province()).end() - state.world.province_get_factory_location(state_membership_fat_id.get_province()).begin());
		// For new factories: no more than defines:FACTORIES_PER_STATE existing + under construction new factories must be
		assert(number_of_factories <= int32_t(state.defines.factories_per_state));
		return number_of_factories;
	}

	int32_t state_factory_count(sys::state& state, dcon::state_instance_id state_instance_id) {
		dcon::nation_id nationd_id = state.world.state_instance_get_nation_from_state_ownership(state_instance_id);
		int32_t number_of_factories = 0;
		auto state_definition_fat_id = state.world.state_instance_get_definition(state_instance_id);
		for(auto state_membership_fat_id : state.world.state_definition_get_abstract_state_membership(state_definition_fat_id))
			if(state_membership_fat_id.get_province().get_nation_from_province_ownership() == nationd_id) {
				auto factory_location_range = state_membership_fat_id.get_province().get_factory_location();
				number_of_factories += int32_t(factory_location_range.end() - factory_location_range.begin());
			}
		for(auto state_building_construction_fat_id : state.world.state_instance_get_state_building_construction(state_instance_id))
			if(state_building_construction_fat_id.get_is_upgrade() == false)
				++number_of_factories;
		// For new factories: no more than defines:FACTORIES_PER_STATE existing + under construction new factories must be
		//assert(num_factories <= int32_t(state.defines.factories_per_state));
		return number_of_factories;
	}



	void add_factory_level_to_state(sys::state& state, dcon::state_instance_id state_instance_id, dcon::factory_type_id factory_type_id, bool is_upgrade) {
		if(is_upgrade) {
			auto state_definition_fat_id = state.world.state_instance_get_definition(state_instance_id);
			auto nation_id = state.world.state_instance_get_nation_from_state_ownership(state_instance_id);
			for(auto state_membership_fat_id : state.world.state_definition_get_abstract_state_membership(state_definition_fat_id)) {
				if(state_membership_fat_id.get_province().get_nation_from_province_ownership() == nation_id) {
					for(auto factory_location_fat_id : state_membership_fat_id.get_province().get_factory_location()) {
						if(factory_location_fat_id.get_factory().get_building_type() == factory_type_id) {
							auto factory_level = factory_location_fat_id.get_factory().get_level();
							auto new_factory_level = std::min(float(std::numeric_limits<uint8_t>::max()), float(factory_level) + 1.f + math::sqrt(factory_location_fat_id.get_factory().get_level()) / 2.f);
							factory_location_fat_id.get_factory().get_level() = uint8_t(new_factory_level);
							return;
						}
					}
				}
			}
			assert(false);
			return;
		}

		/* fail if adding new factory results in more than allowed factories! */
		if(state_factory_count(state, state_instance_id) + 1 >= int32_t(state.defines.factories_per_state))
			return;

		auto state_capital_province_fat_id = state.world.state_instance_get_capital(state_instance_id);
		auto new_factory_fat_id = fatten(state.world, state.world.create_factory());
		new_factory_fat_id.set_building_type(factory_type_id);
		new_factory_fat_id.set_level(uint8_t(1));
		new_factory_fat_id.set_production_scale(1.0f);
		state.world.try_create_factory_location(new_factory_fat_id, state_capital_province_fat_id);
	}

	void prune_factories(sys::state& state) {
		for(auto state_instance_fat_id : state.world.in_state_instance) {
			auto owner_nation_fat_id = state_instance_fat_id.get_nation_from_state_ownership();
			auto combined_issue_rules = owner_nation_fat_id.get_combined_issue_rules();

			if(owner_nation_fat_id.get_is_player_controlled() && (combined_issue_rules & issue_rule::destroy_factory) != 0) // not for players who can manually destroy
				continue;

			dcon::factory_id deletion_choice;
			int32_t factory_count = 0;

			province::for_each_province_in_state_instance(state, state_instance_fat_id, [&](dcon::province_id p) {
				for(auto factory_location_fat_id : state.world.province_get_factory_location(p)) {
					++factory_count;
					auto scale = factory_location_fat_id.get_factory().get_production_scale();
					float ten_workers = 10.f / factory_max_employment(state, factory_location_fat_id.get_factory());
					bool unprofitable = factory_location_fat_id.get_factory().get_unprofitable();
					if(((scale < ten_workers) && unprofitable) && (!deletion_choice || state.world.factory_get_level(deletion_choice) > factory_location_fat_id.get_factory().get_level())) {
						deletion_choice = factory_location_fat_id.get_factory();
					}
				}
			});

			// aggressive pruning
			// to help building more healthy economy instead of 1 profitable giant factory with 6 small 0 scale factories
			if(deletion_choice && (4 + factory_count) >= int32_t(state.defines.factories_per_state)) {
				auto production_type = state.world.factory_get_building_type(deletion_choice);
				state.world.delete_factory(deletion_choice);

				for(auto state_building_construction_fat_id : state_instance_fat_id.get_state_building_construction()) {
					if(state_building_construction_fat_id.get_type() == production_type) {
						state.world.delete_state_building_construction(state_building_construction_fat_id);
						break;
					}
				}
			}
		}
	}


	void set_factory_priority(sys::state& state, dcon::factory_id factory_id, int32_t priority) {
		state.world.factory_set_priority_high(factory_id, priority >= 2);
		state.world.factory_set_priority_low(factory_id, (priority & 1) != 0);
	}

	void try_add_factory_to_state(sys::state& state, dcon::state_instance_id state_instance_id, dcon::factory_type_id factory_type_id) {
		auto nation_id = state.world.state_instance_get_nation_from_state_ownership(state_instance_id);

		if(state.world.factory_type_get_is_coastal(factory_type_id)) {
			if(!province::state_is_coastal(state, state_instance_id))
				return; // requires coast to build coastal factory
		}

		auto existing_constructions = state.world.state_instance_get_state_building_construction(state_instance_id);
		int32_t number_of_factories = 0;
		for(auto state_building_construction_fat_id : existing_constructions) {
			if(!state_building_construction_fat_id.get_is_upgrade())
				++number_of_factories;
			if(state_building_construction_fat_id.get_type() == factory_type_id)
				return; // can't duplicate type
		}

		// is there an upgrade target ?
		auto state_definition_fat_id = state.world.state_instance_get_definition(state_instance_id);
		for(auto state_membership_fat_id : state.world.state_definition_get_abstract_state_membership(state_definition_fat_id)) {
			if(state_membership_fat_id.get_province().get_nation_from_province_ownership() == nation_id) {
				for(auto factory_location_fat_id : state_membership_fat_id.get_province().get_factory_location()) {
					++number_of_factories;
					if(factory_location_fat_id.get_factory().get_building_type() == factory_type_id)
						return; // can't build another of this type
				}
			}
		}

		if(number_of_factories < int32_t(state.defines.factories_per_state)) {
			add_factory_level_to_state(state, state_instance_id, factory_type_id, false);
		}
	}


	void update_factory_employment(sys::state& state) {
		state.world.for_each_state_instance([&](dcon::state_instance_id state_instance_id) {
			float primary_pool = state.world.state_instance_get_demographics(state_instance_id,
			demographics::to_key(state, state.culture_definitions.primary_factory_worker));
			float secondary_pool = state.world.state_instance_get_demographics(state_instance_id,
			demographics::to_key(state, state.culture_definitions.secondary_factory_worker));

			static std::vector<dcon::factory_id> ordered_factories;
			ordered_factories.clear();

			province::for_each_province_in_state_instance(state, state_instance_id, [&](dcon::province_id province_id) {
				for(auto factory_location_fat_id : state.world.province_get_factory_location(province_id)) {
					ordered_factories.push_back(factory_location_fat_id.get_factory());
				}
			});

			std::sort(ordered_factories.begin(), ordered_factories.end(), [&](dcon::factory_id factory_id_a, dcon::factory_id factory_id_b) {
				if(factory_is_profitable(state, factory_id_a) != factory_is_profitable(state, factory_id_b)) {
					return factory_is_profitable(state, factory_id_a);
				}
				if(factory_priority(state, factory_id_a) != factory_priority(state, factory_id_b)) {
					return factory_priority(state, factory_id_a) > factory_priority(state, factory_id_b);
				}
				return factory_id_a.index() < factory_id_b.index();
			});

			float employment_shift_speed = 0.09f;

			float primary_pool_copy = primary_pool;
			float secondary_pool_copy = secondary_pool;
			for(uint32_t index = 0; index < ordered_factories.size();) {
				uint32_t next_index = index;
				float total_workforce = 0.0f;
				for(; next_index < ordered_factories.size(); ++next_index) {
					if(factory_is_profitable(state, ordered_factories[index]) != factory_is_profitable(state, ordered_factories[next_index])
					|| factory_priority(state, ordered_factories[index]) != factory_priority(state, ordered_factories[next_index])) {
						break;
					}
					total_workforce += factory_max_employment(state, ordered_factories[next_index]) *
						state.world.factory_get_production_scale(ordered_factories[next_index]);
				}
				{
					float craftsmen_share = state.economy_definitions.craftsmen_fraction * total_workforce;
					float scale = primary_pool_copy >= craftsmen_share ? 1.0f : primary_pool_copy / craftsmen_share;
					primary_pool_copy = std::max(0.0f, primary_pool_copy - craftsmen_share);
					for(uint32_t i = index; i < next_index; ++i) {
						float old_employment = state.world.factory_get_primary_employment(ordered_factories[i]);
						float new_employment = old_employment * (1.f - employment_shift_speed)
							+ scale * state.world.factory_get_production_scale(ordered_factories[i]) * employment_shift_speed;
						state.world.factory_set_primary_employment(ordered_factories[i], new_employment);
					}
				}
				{
					float non_craftsmen_share = (1.0f - state.economy_definitions.craftsmen_fraction) * total_workforce;
					float scale = secondary_pool_copy >= non_craftsmen_share ? 1.0f : secondary_pool_copy / non_craftsmen_share;
					secondary_pool_copy = std::max(0.0f, secondary_pool_copy - non_craftsmen_share);

					for(uint32_t i = index; i < next_index; ++i) {
						float old_employment = state.world.factory_get_secondary_employment(ordered_factories[i]);
						float new_employment = old_employment * (1.f - employment_shift_speed)
							+ scale * state.world.factory_get_production_scale(ordered_factories[i]) * employment_shift_speed;
						state.world.factory_set_secondary_employment(ordered_factories[i], new_employment);
					}
				}
				index = next_index;
			}

			float primary_employment = 1.0f - (primary_pool > 0 ? primary_pool_copy / primary_pool : 0.0f);
			float secondary_employment = 1.0f - (secondary_pool > 0 ? secondary_pool_copy / secondary_pool : 0.0f);

			assert(primary_employment >= 0.f && primary_employment <= 1.f);
			assert(secondary_employment >= 0.f && secondary_employment <= 1.f);

			assert(primary_employment + secondary_employment <= 2.f);

			province::for_each_province_in_state_instance(state, state_instance_id, [&](dcon::province_id p) {
				for(auto pop_location_fat_id : state.world.province_get_pop_location(p)) {
					if(pop_location_fat_id.get_pop().get_poptype() == state.culture_definitions.primary_factory_worker) {
						pop_location_fat_id.get_pop().set_employment(pop_location_fat_id.get_pop().get_size() * primary_employment);
					} else if(pop_location_fat_id.get_pop().get_poptype() == state.culture_definitions.secondary_factory_worker) {
						pop_location_fat_id.get_pop().set_employment(pop_location_fat_id.get_pop().get_size() * secondary_employment);
					}
				}
			});
		});
	}

	void update_factory_triggered_modifiers(sys::state& state) {
		state.world.for_each_factory([&](dcon::factory_id factory_id) {
			auto fac_type = fatten(state.world, state.world.factory_get_building_type(factory_id));
			float sum = 1.0f;
			auto province_id = state.world.factory_get_province_from_factory_location(factory_id);
			auto state_instance_fat_id = state.world.province_get_state_membership(province_id);
			auto owner_nation_id = state.world.province_get_nation_from_province_ownership(province_id);
			if(owner_nation_id && state_instance_fat_id) {
				sum = sum_of_factory_triggered_modifiers(state, state.world.factory_get_building_type(factory_id), state_instance_fat_id);
			}
			state.world.factory_set_triggered_modifiers(factory_id, sum);
		});
	}

	
	void update_single_factory_consumption(sys::state& state, dcon::factory_id factory_id, dcon::nation_id nation_id, dcon::province_id province_id, dcon::state_instance_id state_instance_id, float mobilization_impact, float expected_min_wage, bool occupied) {
		auto factory_fat_id = fatten(state.world, factory_id);
		auto factory_type_fat_id = factory_fat_id.get_building_type();

		assert(factory_type_fat_id);
		assert(factory_type_fat_id.get_output());
		assert(nation_id);
		assert(province_id);
		assert(state_instance_id);

		float total_workers = factory_max_employment(state, factory_id);
		float several_workers_scale = 10.f / total_workers;
		float max_production_scale = factory_max_production_scale(state, factory_fat_id, mobilization_impact, occupied);
		float current_workers = total_workers * max_production_scale;

		//inputs

		float input_total = factory_input_total_cost(state, nation_id, factory_type_fat_id);
		float min_input_available = factory_min_input_available(state, nation_id, factory_type_fat_id);
		float efficiency_input_total = factory_efficiency_input_total_cost(state, nation_id, factory_type_fat_id);
		float min_efficiency_input_available = factory_min_efficiency_input_available(state, nation_id, factory_type_fat_id);

		//modifiers

		float input_multiplier = std::max(0.f, factory_input_multiplier(state, factory_fat_id, nation_id, province_id, state_instance_id));
		float throughput_multiplier = std::max(0.f, factory_throughput_multiplier(state, factory_type_fat_id, nation_id, province_id, state_instance_id) + factory_fat_id.get_triggered_modifiers());
		float output_multiplier = std::max(0.f, factory_output_multiplier(state, factory_fat_id, nation_id, province_id));

		//this value represents total production if 1 lvl of this factory is filled with workers
		float total_production = factory_type_fat_id.get_output_amount()
			* (0.75f + 0.25f * min_efficiency_input_available)
			* throughput_multiplier
			* output_multiplier
			* min_input_available;

		//this value represents raw profit if 1 lvl of this factory is filled with workers
		float profit =
			total_production
			* state.world.commodity_get_current_price(factory_type_fat_id.get_output());

		//this value represents spendings if 1 lvl of this factory is filled with workers
		float spendings = expected_min_wage * state.defines.alice_factory_per_level_employment
			+ input_multiplier * throughput_multiplier * input_total * min_input_available
			+ input_multiplier * efficiency_input_total * min_efficiency_input_available * min_input_available;

		float desired_profit = factory_desired_raw_profit(factory_fat_id, spendings);
		float max_pure_profit = profit - spendings;
		state.world.factory_set_unprofitable(factory_id, !(max_pure_profit > 0.0f));

		float effective_production_scale = update_factory_scale(state, factory_fat_id, max_production_scale, profit, desired_profit);

		auto& inputs = factory_type_fat_id.get_inputs();
		auto& efficiency_inputs = factory_type_fat_id.get_efficiency_inputs();

		// register real demand : input_multiplier * throughput_multiplier * level * primary_employment
		// also multiply by target production scale... otherwise too much excess demand is generated
		// also multiply by something related to minimal satisfied input
		// to prevent generation of too much demand on rgos already influenced by a shortage
		float input_scale = input_multiplier * throughput_multiplier * effective_production_scale * (0.1f + min_input_available * 0.9f);
		for(uint32_t i = 0; i < economy::commodity_set::set_size; ++i) {
			if(inputs.commodity_type[i]) {
				economy::register_intermediate_demand(state, nation_id, inputs.commodity_type[i], input_scale * inputs.commodity_amounts[i]);
			} else {
				break;
			}
		}

		// and for efficiency inputs
		//  the consumption of efficiency inputs is (national-factory-maintenance-modifier + 1) x input-multiplier x
		//  throughput-multiplier x factory level
		auto const mfactor = state.world.nation_get_modifier_values(nation_id, sys::national_mod_offsets::factory_maintenance) + 1.0f;
		float efficiency_input_scale = input_multiplier * throughput_multiplier * effective_production_scale * (0.1f + min_efficiency_input_available * 0.9f);
		for(uint32_t i = 0; i < economy::small_commodity_set::set_size; ++i) {
			if(efficiency_inputs.commodity_type[i]) {
				economy::register_intermediate_demand(state, nation_id, efficiency_inputs.commodity_type[i], mfactor * efficiency_input_scale * efficiency_inputs.commodity_amounts[i] * (0.1f + min_efficiency_input_available * 0.9f));
			} else {
				break;
			}
		}

		float actual_production = total_production * effective_production_scale;
		float pure_profit = max_pure_profit * effective_production_scale;

		state.world.factory_set_actual_production(factory_id, actual_production);
		state.world.factory_set_full_profit(factory_id, pure_profit);
	}

	void update_single_factory_production(sys::state& state, dcon::factory_id factory_id, dcon::nation_id nation_id, float expected_min_wage) {
		auto factory_production = state.world.factory_get_actual_production(factory_id);
		if(factory_production > 0.f) {
			auto factory_fat_id = fatten(state.world, factory_id);
			auto factory_type_fat_id = factory_fat_id.get_building_type();

			auto money_made = state.world.factory_get_full_profit(factory_id);

			state.world.factory_set_actual_production(factory_id, factory_production);
			economy::register_domestic_supply(state, nation_id, factory_type_fat_id.get_output(), factory_production);

			if(!factory_fat_id.get_subsidized()) {
				state.world.factory_set_full_profit(factory_id, money_made);
			} else {
				float min_wages = expected_min_wage * factory_fat_id.get_level() * factory_fat_id.get_primary_employment() * state.defines.alice_factory_per_level_employment;
				if(money_made < min_wages) {
					auto difference = min_wages - money_made;
					assert(difference > 0.0f);
					if(state.world.nation_get_stockpiles(nation_id, economy::money) > difference || economy::can_take_loans(state, nation_id)) {
						state.world.factory_set_full_profit(factory_id, min_wages);
						state.world.nation_get_stockpiles(nation_id, economy::money) -= difference;
						state.world.nation_get_subsidies_spending(nation_id) += difference;
					} else {
						state.world.factory_set_full_profit(factory_id, std::max(money_made, 0.0f));
						factory_fat_id.set_subsidized(false);
					}
				} else {
					state.world.factory_set_full_profit(factory_id, money_made);
				}
			}
		}
	}
	


	economy::construction_status factory_upgrade(sys::state& state, dcon::factory_id factory_id) {
		auto in_province = state.world.factory_get_province_from_factory_location(factory_id);
		auto in_state = state.world.province_get_state_membership(in_province);
		auto factory_type_fat_id = state.world.factory_get_building_type(factory_id);

		for(auto state_building_construction_fat_id : state.world.state_instance_get_state_building_construction(in_state)) {
			if(state_building_construction_fat_id.get_type() == factory_type_fat_id) {
				float admin_eff = state.world.nation_get_administrative_efficiency(state_building_construction_fat_id.get_nation());
				float build_cost_modifier = factory_build_cost_modifier(state, state_building_construction_fat_id.get_nation(), state_building_construction_fat_id.get_is_pop_project());
				float total = 0.0f;
				float purchased = 0.0f;
				auto& goods = state.world.factory_type_get_construction_costs(factory_type_fat_id);
				for(uint32_t i = 0; i < economy::commodity_set::set_size; ++i) {
					total += goods.commodity_amounts[i] * build_cost_modifier;
					purchased += std::clamp(state_building_construction_fat_id.get_purchased_goods().commodity_amounts[i], 0.f, goods.commodity_amounts[i] * build_cost_modifier);
				}
				return economy::construction_status{ total > 0.0f ? purchased / total : 0.0f, true };
			}
		}

		return economy::construction_status{ 0.0f, false };
	}



	economy::profit_distribution distribute_factory_profit(sys::state const& state, dcon::state_instance_const_fat_id state_instance_fat_id, float min_wage, float total_profit) {
		float total_min_primary_workers_wage = min_wage * state.world.state_instance_get_demographics(state_instance_fat_id, demographics::to_employment_key(state, state.culture_definitions.primary_factory_worker));
		float total_min_secondary_workers_wage = min_wage * state.world.state_instance_get_demographics(state_instance_fat_id, demographics::to_employment_key(state, state.culture_definitions.secondary_factory_worker));
		float number_of_primary_workers = state.world.state_instance_get_demographics(state_instance_fat_id, demographics::to_key(state, state.culture_definitions.primary_factory_worker));
		float number_of_secondary_workers = state.world.state_instance_get_demographics(state_instance_fat_id, demographics::to_key(state, state.culture_definitions.secondary_factory_worker));
		float number_of_owners = state.world.state_instance_get_demographics(state_instance_fat_id, demographics::to_key(state, state.culture_definitions.capitalists));

		auto nation_id = state.world.state_instance_get_nation_from_state_ownership(state_instance_fat_id);
		auto desired_per_primary_worker_profit = state.world.nation_get_life_needs_costs(nation_id, state.culture_definitions.primary_factory_worker) * (1.f / 5.f);
		auto desired_per_secondary_worker_profit = state.world.nation_get_life_needs_costs(nation_id, state.culture_definitions.secondary_factory_worker) * (1.f / 5.f);
		auto desired_per_owner_profit = state.world.nation_get_life_needs_costs(nation_id, state.culture_definitions.capitalists) * (1.f / 5.f);
		auto total_desired_profit = desired_per_primary_worker_profit + desired_per_secondary_worker_profit + desired_per_owner_profit;
		if(total_desired_profit == 0.f) {
			return {
				.per_primary_worker = 0.f,
				.per_secondary_worker = 0.f,
				.per_owner = 0.f
			};
		}
		auto per_owner_profit = total_profit * (desired_per_owner_profit / total_desired_profit);
		auto per_primary_worker_profit = total_profit * (desired_per_primary_worker_profit / total_desired_profit);
		auto per_secondary_worker_profit = total_profit * (desired_per_secondary_worker_profit / total_desired_profit);
		return {
			.per_primary_worker = per_primary_worker_profit / number_of_primary_workers,
			.per_secondary_worker = per_secondary_worker_profit / number_of_secondary_workers,
			.per_owner = per_owner_profit / number_of_owners
		};
	}



	template void for_each_new_factory<std::function<void(new_factory)>>(sys::state&, dcon::state_instance_id, std::function<void(new_factory)>&&);
	template void for_each_upgraded_factory<std::function<void(upgraded_factory)>>(sys::state&, dcon::state_instance_id, std::function<void(upgraded_factory)>&&);	
}
