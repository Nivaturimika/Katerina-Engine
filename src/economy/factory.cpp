#include "factory.hpp"
#include "demographics.hpp"
#include "dcon_generated.hpp"
#include "ai.hpp"
#include "system_state.hpp"
#include "prng.hpp"
#include "math_fns.hpp"
#include "nations_templates.hpp"
#include "province_templates.hpp"
#include "triggers.hpp"
#include "factory_templates.hpp"
#include "economy.hpp"

namespace factory {
	template void for_each_new_factory<std::function<void(new_factory)>>(sys::state&, dcon::state_instance_id, std::function<void(new_factory)>&&);
	template void for_each_upgraded_factory<std::function<void(upgraded_factory)>>(sys::state&, dcon::state_instance_id, std::function<void(upgraded_factory)>&&);

	float factory_build_cost_modifier(sys::state& state, dcon::nation_id n, bool pop_project) {
		float admin_eff = state.world.nation_get_administrative_efficiency(n);
		float factory_mod = std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::factory_cost) + 1.0f);
		float pop_factory_mod = std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::factory_owner_cost));
		return (pop_project ? pop_factory_mod : (2.0f - admin_eff)) * factory_mod;
	}

	bool has_factory(sys::state const& state, dcon::state_instance_id si) {
		auto sdef = state.world.state_instance_get_definition(si);
		auto owner = state.world.state_instance_get_nation_from_state_ownership(si);
		auto crng = state.world.state_instance_get_state_building_construction(si);
		if(crng.begin() != crng.end())
			return true;

		for(auto p : state.world.state_definition_get_abstract_state_membership(sdef)) {
			if(p.get_province().get_nation_from_province_ownership() == owner) {
				auto rng = p.get_province().get_factory_location();
				if(rng.begin() != rng.end())
					return true;
			}
		}
		return false;
	}

	bool nation_is_constructing_factories(sys::state& state, dcon::nation_id n) {
		auto rng = state.world.nation_get_state_building_construction(n);
		return rng.begin() != rng.end();
	}

	bool nation_has_closed_factories(sys::state& state, dcon::nation_id n) { // TODO - should be "good" now
		auto nation_fat = dcon::fatten(state.world, n);
		for(auto prov_owner : nation_fat.get_province_ownership()) {
			auto prov = prov_owner.get_province();
			for(auto factloc : prov.get_factory_location()) {
				auto scale = factloc.get_factory().get_production_scale();
				if(scale < factory_closed_threshold) {
					return true;
				}
			}
		}
		return false;
	}
	float sum_of_factory_triggered_modifiers(sys::state& state, dcon::factory_type_id ft, dcon::state_instance_id s) {
		auto fat_id = fatten(state.world, ft);
		auto n = state.world.state_instance_get_nation_from_state_ownership(s);
		float sum = 1.f;
		for(uint32_t i = 0; i < sys::max_factory_bonuses; i++) {
			if(auto mod_a = fat_id.get_bonus_trigger()[i]; mod_a && trigger::evaluate(state, mod_a, trigger::to_generic(s), trigger::to_generic(n), 0)) {
				sum += fat_id.get_bonus_amount()[i];
			}
		}
		return sum;
	}
	void update_factory_triggered_modifiers(sys::state& state) {
		state.world.for_each_factory([&](dcon::factory_id f) {
			auto fac_type = fatten(state.world, state.world.factory_get_building_type(f));
			float sum = 1.0f;
			auto prov = state.world.factory_get_province_from_factory_location(f);
			auto pstate = state.world.province_get_state_membership(prov);
			auto powner = state.world.province_get_nation_from_province_ownership(prov);
			if(powner && pstate) {
				sum = sum_of_factory_triggered_modifiers(state, state.world.factory_get_building_type(f), pstate);
			}
			state.world.factory_set_triggered_modifiers(f, sum);
		});
	}

	int32_t factory_priority(sys::state const& state, dcon::factory_id f) {
		return (state.world.factory_get_priority_low(f) ? 1 : 0) + (state.world.factory_get_priority_high(f) ? 2 : 0);
	}

	void set_factory_priority(sys::state& state, dcon::factory_id f, int32_t priority) {
		state.world.factory_set_priority_high(f, priority >= 2);
		state.world.factory_set_priority_low(f, (priority & 1) != 0);
	}

	bool factory_is_profitable(sys::state const& state, dcon::factory_id f) {
		return state.world.factory_get_unprofitable(f) == false || state.world.factory_get_subsidized(f);
	}

	float factory_max_employment(sys::state const& state, dcon::factory_id f) {
		return state.defines.alice_factory_per_level_employment * state.world.factory_get_level(f);
	}

	float factory_primary_employment(sys::state const& state, dcon::factory_id f) {
		auto primary_employment = state.world.factory_get_primary_employment(f);
		return factory_max_employment(state, f) * (state.economy_definitions.craftsmen_fraction * primary_employment);
	}

	float factory_secondary_employment(sys::state const& state, dcon::factory_id f) {
		auto secondary_employment = state.world.factory_get_secondary_employment(f);
		return factory_max_employment(state, f) * ((1 - state.economy_definitions.craftsmen_fraction) * secondary_employment);
	}

	float factory_total_employment(sys::state const& state, dcon::factory_id f) {
		// TODO: Document this, also is this a stub?
		auto primary_employment = state.world.factory_get_primary_employment(f);
		auto secondary_employment = state.world.factory_get_secondary_employment(f);
		return factory_max_employment(state, f) * (state.economy_definitions.craftsmen_fraction * primary_employment + (1 - state.economy_definitions.craftsmen_fraction) * secondary_employment);
	}

	void update_factory_employment(sys::state& state) {
		state.world.for_each_state_instance([&](dcon::state_instance_id si) {
			float primary_pool = state.world.state_instance_get_demographics(si,
			demographics::to_key(state, state.culture_definitions.primary_factory_worker));
			float secondary_pool = state.world.state_instance_get_demographics(si,
			demographics::to_key(state, state.culture_definitions.secondary_factory_worker));

			static std::vector<dcon::factory_id> ordered_factories;
			ordered_factories.clear();

			province::for_each_province_in_state_instance(state, si, [&](dcon::province_id p) {
				for(auto fac : state.world.province_get_factory_location(p)) {
					ordered_factories.push_back(fac.get_factory());
				}
			});

			std::sort(ordered_factories.begin(), ordered_factories.end(), [&](dcon::factory_id a, dcon::factory_id b) {
				if(factory_is_profitable(state, a) != factory_is_profitable(state, b)) {
					return factory_is_profitable(state, a);
				}
				if(factory_priority(state, a) != factory_priority(state, b)) {
					return factory_priority(state, a) > factory_priority(state, b);
				}
				return a.index() < b.index();
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
					float type_share = state.economy_definitions.craftsmen_fraction * total_workforce;
					float scale = primary_pool_copy >= type_share ? 1.0f : primary_pool_copy / type_share;
					primary_pool_copy = std::max(0.0f, primary_pool_copy - type_share);
					for(uint32_t i = index; i < next_index; ++i) {
						float old_employment = state.world.factory_get_primary_employment(ordered_factories[i]);
						float new_employment = old_employment * (1.f - employment_shift_speed)
							+ scale * state.world.factory_get_production_scale(ordered_factories[i]) * employment_shift_speed;
						state.world.factory_set_primary_employment(ordered_factories[i], new_employment);
					}
				}
				{
					float type_share = (1.0f - state.economy_definitions.craftsmen_fraction) * total_workforce;
					float scale = secondary_pool_copy >= type_share ? 1.0f : secondary_pool_copy / type_share;
					secondary_pool_copy = std::max(0.0f, secondary_pool_copy - type_share);

					for(uint32_t i = index; i < next_index; ++i) {
						float old_employment = state.world.factory_get_secondary_employment(ordered_factories[i]);
						float new_employment = old_employment * (1.f - employment_shift_speed)
							+ scale * state.world.factory_get_production_scale(ordered_factories[i]) * employment_shift_speed;
						state.world.factory_set_secondary_employment(ordered_factories[i], new_employment);
					}
				}

				index = next_index;
			}

			float prim_employment = 1.0f - (primary_pool > 0 ? primary_pool_copy / primary_pool : 0.0f);
			float sec_employment = 1.0f - (secondary_pool > 0 ? secondary_pool_copy / secondary_pool : 0.0f);

			assert(prim_employment >= 0.f && prim_employment <= 1.f);
			assert(sec_employment >= 0.f && sec_employment <= 1.f);

			assert(prim_employment + sec_employment <= 2.f);

			province::for_each_province_in_state_instance(state, si, [&](dcon::province_id p) {
				for(auto pop : state.world.province_get_pop_location(p)) {
					if(pop.get_pop().get_poptype() == state.culture_definitions.primary_factory_worker) {
						pop.get_pop().set_employment(pop.get_pop().get_size() * prim_employment);
					} else if(pop.get_pop().get_poptype() == state.culture_definitions.secondary_factory_worker) {
						pop.get_pop().set_employment(pop.get_pop().get_size() * sec_employment);
					}
				}
			});
		});
	}

	float factory_full_production_quantity(sys::state const& state, dcon::factory_id f, dcon::nation_id n, float mobilization_impact) {
		auto fac = fatten(state.world, f);
		auto fac_type = fac.get_building_type();
		float throughput_multiplier = (state.world.nation_get_factory_goods_throughput(n, fac_type.get_output()) + 1.0f);
		float output_multiplier = state.world.nation_get_factory_goods_output(n, fac_type.get_output())
			+ 1.0f
			+ fac.get_secondary_employment()
			* (1.0f - state.economy_definitions.craftsmen_fraction)
			* 1.5f
			* 2.f; // additional multiplier to give advantage to "old industrial giants" which have a bunch of clerks already
		float max_production_scale = fac.get_primary_employment()
			* fac.get_level()
			* std::max(0.0f, mobilization_impact);
		return throughput_multiplier * output_multiplier * max_production_scale;
	}

	float factory_min_input_available(sys::state& state, dcon::nation_id n, dcon::factory_type_fat_id fac_type) {
		float min_input_available = 1.0f;
		auto& inputs = fac_type.get_inputs();
		for(uint32_t i = 0; i < economy::commodity_set::set_size; ++i) {
			if(inputs.commodity_type[i]) {
				min_input_available = std::min(min_input_available, state.world.nation_get_demand_satisfaction(n, inputs.commodity_type[i]));
			} else {
				break;
			}
		}
		return min_input_available;
	}

	float factory_input_total_cost(sys::state& state, dcon::nation_id n, dcon::factory_type_fat_id fac_type) {
		float input_total = 0.0f;
		auto& inputs = fac_type.get_inputs();
		for(uint32_t i = 0; i < economy::commodity_set::set_size; ++i) {
			if(inputs.commodity_type[i]) {
				input_total += inputs.commodity_amounts[i] * economy::commodity_effective_price(state, n, inputs.commodity_type[i]);
			} else {
				break;
			}
		}
		return input_total;
	}

	float factory_min_e_input_available(sys::state& state, dcon::nation_id n, dcon::factory_type_fat_id fac_type) {
		float min_e_input_available = 1.0f;
		auto& e_inputs = fac_type.get_efficiency_inputs();
		for(uint32_t i = 0; i < economy::small_commodity_set::set_size; ++i) {
			if(e_inputs.commodity_type[i]) {
				min_e_input_available = std::min(min_e_input_available, state.world.nation_get_demand_satisfaction(n, e_inputs.commodity_type[i]));
			} else {
				break;
			}
		}
		return min_e_input_available;
	}

	float factory_e_input_total_cost(sys::state& state, dcon::nation_id n, dcon::factory_type_fat_id fac_type) {
		float e_input_total = 0.0f;
		auto& e_inputs = fac_type.get_efficiency_inputs();
		for(uint32_t i = 0; i < economy::small_commodity_set::set_size; ++i) {
			if(e_inputs.commodity_type[i]) {
				e_input_total += e_inputs.commodity_amounts[i] * economy::commodity_effective_price(state, n, e_inputs.commodity_type[i]);
			} else {
				break;
			}
		}
		return e_input_total;
	}

	float factory_input_multiplier(sys::state& state, dcon::factory_fat_id fac, dcon::nation_id n, dcon::province_id p, dcon::state_instance_id s) {
		float total_workers = factory_max_employment(state, fac);
		float small_size_effect = 1.f;
		float small_bound = 5.f * state.defines.alice_factory_per_level_employment;
		if(total_workers < small_bound) {
			small_size_effect = 0.5f + total_workers / small_bound * 0.5f;
		}

		float total_state_pop = std::max(0.0001f, state.world.state_instance_get_demographics(s, demographics::total));
		float capitalists = state.world.state_instance_get_demographics(s, demographics::to_key(state, state.culture_definitions.capitalists));
		float owner_fraction = total_state_pop > 0.f
			? std::min(0.05f, capitalists / total_state_pop)
			: 0.0f;

		return small_size_effect *
			std::max(0.1f,
				(state.defines.alice_inputs_base_factor
					+ state.world.province_get_modifier_values(p, sys::provincial_mod_offsets::local_factory_input)
					+ state.world.nation_get_modifier_values(n, sys::national_mod_offsets::factory_input)
					+ owner_fraction * -2.5f));
	}

	float factory_throughput_multiplier(sys::state& state, dcon::factory_type_fat_id fac_type, dcon::nation_id n, dcon::province_id p, dcon::state_instance_id s) {
		return state.world.nation_get_factory_goods_throughput(n, fac_type.get_output())
			+ state.world.province_get_modifier_values(p, sys::provincial_mod_offsets::local_factory_throughput)
			+ state.world.nation_get_modifier_values(n, sys::national_mod_offsets::factory_throughput)
			+ 1.0f;
	}

	float factory_output_multiplier(sys::state& state, dcon::factory_fat_id fac, dcon::nation_id n, dcon::province_id p) {
		auto fac_type = fac.get_building_type();

		return state.world.nation_get_factory_goods_output(n, fac_type.get_output())
			+ state.world.province_get_modifier_values(p, sys::provincial_mod_offsets::local_factory_output)
			+ state.world.nation_get_modifier_values(n, sys::national_mod_offsets::factory_output)
			+ fac.get_secondary_employment()
			* (1.0f - state.economy_definitions.craftsmen_fraction)
			* 1.5f
			+ 1.0f;
	}

	float factory_max_production_scale(sys::state& state, dcon::factory_fat_id fac, float mobilization_impact, bool occupied) {
		return fac.get_primary_employment() * fac.get_level() * (occupied ? 0.1f : 1.0f) * std::max(0.0f, mobilization_impact);
	}

	float update_factory_scale(sys::state& state, dcon::factory_fat_id fac, float max_production_scale, float raw_profit, float desired_raw_profit) {
		float total_workers = factory_max_employment(state, fac);
		float several_workers_scale = 10.f / total_workers;

		// we don't want for factories to change "world balance" too much individually
		// when relative production is high, we want to reduce our speed
		// for example, if relative production is 1.0, then we want to clamp our speed with ~0.01 or something small like this;
		// and if relative production is ~0, then clamps are not needed
		float relative_production_amount
			=
			state.world.factory_type_get_output_amount(fac.get_building_type())
			/ (
				state.world.commodity_get_total_production(fac.get_building_type().get_output())
				+ state.world.commodity_get_total_real_demand(fac.get_building_type().get_output())
				+ 10.f
			);

		float relative_modifier = (1.f / (relative_production_amount + 0.01f)) / 1000.f;

		float effective_production_scale = 0.0f;
		if(state.world.factory_get_subsidized(fac)) {
			auto new_production_scale = std::min(1.0f, fac.get_production_scale() + several_workers_scale * fac.get_level() * 10.f);
			fac.set_production_scale(new_production_scale);
			return std::min(new_production_scale * fac.get_level(), max_production_scale);
		}

		float over_profit_ratio = (raw_profit) / (desired_raw_profit + 0.0001f) - 1.f;
		float under_profit_ratio = (desired_raw_profit) / (raw_profit + 0.0001f) - 1.f;

		float speed_modifier = (over_profit_ratio - under_profit_ratio);
		float speed = economy::production_scale_delta * speed_modifier + several_workers_scale * ((raw_profit - desired_raw_profit > 0.f) ? 1.f : -1.f);

		speed = std::clamp(speed, -relative_modifier, relative_modifier);

		auto new_production_scale = std::clamp(fac.get_production_scale() + speed, 0.f, 1.f);
		fac.set_production_scale(new_production_scale);
		return std::min(new_production_scale * fac.get_level(), max_production_scale);
	}

	float factory_desired_raw_profit(dcon::factory_fat_id fac, float spendings) {
		return spendings * (1.2f + fac.get_secondary_employment() * fac.get_level() / 150.f);
	}

	void update_single_factory_consumption(sys::state& state, dcon::factory_id f, dcon::nation_id n, dcon::province_id p, dcon::state_instance_id s, float mobilization_impact, float expected_min_wage, bool occupied) {
		auto fac = fatten(state.world, f);
		auto fac_type = fac.get_building_type();

		assert(fac_type);
		assert(fac_type.get_output());
		assert(n);
		assert(p);
		assert(s);

		float total_workers = factory_max_employment(state, f);
		float several_workers_scale = 10.f / total_workers;
		float max_production_scale = factory_max_production_scale(state, fac, mobilization_impact, occupied);
		float current_workers = total_workers * max_production_scale;

		//inputs

		float input_total = factory_input_total_cost(state, n, fac_type);
		float min_input_available = factory_min_input_available(state, n, fac_type);
		float e_input_total = factory_e_input_total_cost(state, n, fac_type);
		float min_e_input_available = factory_min_e_input_available(state, n, fac_type);

		//modifiers

		float input_multiplier = std::max(0.f, factory_input_multiplier(state, fac, n, p, s));
		float throughput_multiplier = std::max(0.f, factory_throughput_multiplier(state, fac_type, n, p, s) + fac.get_triggered_modifiers());
		float output_multiplier = std::max(0.f, factory_output_multiplier(state, fac, n, p));

		//this value represents total production if 1 lvl of this factory is filled with workers
		float total_production = fac_type.get_output_amount()
			* (0.75f + 0.25f * min_e_input_available)
			* throughput_multiplier
			* output_multiplier
			* min_input_available;

		//this value represents raw profit if 1 lvl of this factory is filled with workers
		float profit =
			total_production
			* state.world.commodity_get_current_price(fac_type.get_output());

		//this value represents spendings if 1 lvl of this factory is filled with workers
		float spendings = expected_min_wage * state.defines.alice_factory_per_level_employment
			+ input_multiplier * throughput_multiplier * input_total * min_input_available
			+ input_multiplier * e_input_total * min_e_input_available * min_input_available;

		float desired_profit = factory_desired_raw_profit(fac, spendings);
		float max_pure_profit = profit - spendings;
		state.world.factory_set_unprofitable(f, !(max_pure_profit > 0.0f));

		float effective_production_scale = update_factory_scale(state, fac, max_production_scale, profit, desired_profit);

		auto& inputs = fac_type.get_inputs();
		auto& e_inputs = fac_type.get_efficiency_inputs();

		// register real demand : input_multiplier * throughput_multiplier * level * primary_employment
		// also multiply by target production scale... otherwise too much excess demand is generated
		// also multiply by something related to minimal satisfied input
		// to prevent generation of too much demand on rgos already influenced by a shortage
		float input_scale = input_multiplier * throughput_multiplier * effective_production_scale * (0.1f + min_input_available * 0.9f);
		for(uint32_t i = 0; i < economy::commodity_set::set_size; ++i) {
			if(inputs.commodity_type[i]) {
				economy::register_intermediate_demand(state, n, inputs.commodity_type[i], input_scale * inputs.commodity_amounts[i]);
			} else {
				break;
			}
		}

		// and for efficiency inputs
		//  the consumption of efficiency inputs is (national-factory-maintenance-modifier + 1) x input-multiplier x
		//  throughput-multiplier x factory level
		auto const mfactor = state.world.nation_get_modifier_values(n, sys::national_mod_offsets::factory_maintenance) + 1.0f;
		float e_input_scale = input_multiplier * throughput_multiplier * effective_production_scale * (0.1f + min_e_input_available * 0.9f);
		for(uint32_t i = 0; i < economy::small_commodity_set::set_size; ++i) {
			if(e_inputs.commodity_type[i]) {
				economy::register_intermediate_demand(state, n, e_inputs.commodity_type[i], mfactor * e_input_scale * e_inputs.commodity_amounts[i] * (0.1f + min_e_input_available * 0.9f));
			} else {
				break;
			}
		}

		float actual_production = total_production * effective_production_scale;
		float pure_profit = max_pure_profit * effective_production_scale;

		state.world.factory_set_actual_production(f, actual_production);
		state.world.factory_set_full_profit(f, pure_profit);
	}

	void update_single_factory_production(sys::state& state, dcon::factory_id f, dcon::nation_id n, float expected_min_wage) {
		auto production = state.world.factory_get_actual_production(f);
		if(production > 0.f) {
			auto fac = fatten(state.world, f);
			auto fac_type = fac.get_building_type();

			auto amount = production;
			auto money_made = state.world.factory_get_full_profit(f);

			state.world.factory_set_actual_production(f, amount);
			economy::register_domestic_supply(state, n, fac_type.get_output(), amount);

			if(!fac.get_subsidized()) {
				state.world.factory_set_full_profit(f, money_made);
			} else {
				float min_wages = expected_min_wage * fac.get_level() * fac.get_primary_employment() * state.defines.alice_factory_per_level_employment;
				if(money_made < min_wages) {
					auto diff = min_wages - money_made;
					assert(diff > 0.0f);
					if(state.world.nation_get_stockpiles(n, economy::money) > diff || economy::can_take_loans(state, n)) {
						state.world.factory_set_full_profit(f, min_wages);
						state.world.nation_get_stockpiles(n, economy::money) -= diff;
						state.world.nation_get_subsidies_spending(n) += diff;
					} else {
						state.world.factory_set_full_profit(f, std::max(money_made, 0.0f));
						fac.set_subsidized(false);
					}
				} else {
					state.world.factory_set_full_profit(f, money_made);
				}
			}
		}
	}

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

	float global_factory_construction_time_modifier(sys::state& state) {
		return 1.f;
	}

	float global_non_factory_construction_time_modifier(sys::state& state) {
		return 1.f;
	}

	economy::profit_distribution distribute_factory_profit(sys::state const& state, dcon::state_instance_const_fat_id s, float min_wage, float total_profit) {
		float total_min_to_pworkers = min_wage * state.world.state_instance_get_demographics(s, demographics::to_employment_key(state, state.culture_definitions.primary_factory_worker));
		float total_min_to_sworkers = min_wage * state.world.state_instance_get_demographics(s, demographics::to_employment_key(state, state.culture_definitions.secondary_factory_worker));
		float num_pworkers = state.world.state_instance_get_demographics(s, demographics::to_key(state, state.culture_definitions.primary_factory_worker));
		float num_sworkers = state.world.state_instance_get_demographics(s, demographics::to_key(state, state.culture_definitions.secondary_factory_worker));
		float num_owners = state.world.state_instance_get_demographics(s, demographics::to_key(state, state.culture_definitions.capitalists));

		auto n = state.world.state_instance_get_nation_from_state_ownership(s);
		auto desired_per_pworker_profit = state.world.nation_get_life_needs_costs(n, state.culture_definitions.primary_factory_worker) * (1.f / 5.f);
		auto desired_per_sworker_profit = state.world.nation_get_life_needs_costs(n, state.culture_definitions.secondary_factory_worker) * (1.f / 5.f);
		auto desired_per_owner_profit = state.world.nation_get_life_needs_costs(n, state.culture_definitions.capitalists) * (1.f / 5.f);
		auto total_desired_profit = desired_per_pworker_profit + desired_per_sworker_profit + desired_per_owner_profit;
		if(total_desired_profit == 0.f) {
			return {
				.per_primary_worker = 0.f,
				.per_secondary_worker = 0.f,
				.per_owner = 0.f
			};
		}
		auto per_owner_profit = total_profit * (desired_per_owner_profit / total_desired_profit);
		auto per_pworker_profit = total_profit * (desired_per_pworker_profit / total_desired_profit);
		auto per_sworker_profit = total_profit * (desired_per_sworker_profit / total_desired_profit);
		return {
			.per_primary_worker = per_pworker_profit / num_pworkers,
			.per_secondary_worker = per_sworker_profit / num_sworkers,
			.per_owner = per_owner_profit / num_owners
		};
	}

	float nation_factory_consumption(sys::state& state, dcon::nation_id n, dcon::commodity_id c) {
		auto amount = 0.f;
		for(const auto po : state.world.nation_get_province_ownership(n)) {
			for(const auto fl : po.get_province().get_factory_location()) {
				auto fac = fl.get_factory();
				auto fac_type = fl.get_factory().get_building_type();
				auto occupied = po.get_province().get_nation_from_province_control() != po.get_province().get_nation_from_province_ownership();
				auto mobilization_impact = military::mobilization_impact(state, n);
				auto p = po.get_province();
				auto s = po.get_province().get_state_membership();
				//
				float total_workers = factory_max_employment(state, fac);
				float several_workers_scale = 10.f / total_workers;
				float max_production_scale = factory_max_production_scale(state, fac, mobilization_impact, occupied);
				float current_workers = total_workers * max_production_scale;

				//inputs
				float input_total = factory_input_total_cost(state, n, fac_type);
				float min_input_available = factory_min_input_available(state, n, fac_type);
				//modifiers
				float input_multiplier = std::max(0.f, factory_input_multiplier(state, fac, n, p, s));
				float throughput_multiplier = std::max(0.f, factory_throughput_multiplier(state, fac_type, n, p, s) + fac.get_triggered_modifiers());
				float effective_production_scale = std::min(fac.get_production_scale() * fac.get_level(), max_production_scale);
				//
				auto const& inputs = fac_type.get_inputs();
				float input_scale = input_multiplier * throughput_multiplier * effective_production_scale * (0.1f + min_input_available * 0.9f);
				for(uint32_t i = 0; i < economy::commodity_set::set_size; i++) {
					if(inputs.commodity_type[i]) {
						if(inputs.commodity_type[i] == c) {
							amount += inputs.commodity_amounts[i] * fl.get_factory().get_actual_production();
						}
					} else {
						break;
					}
				}
				//
				auto const& e_inputs = fac_type.get_efficiency_inputs();
				float e_input_total = factory_e_input_total_cost(state, n, fac_type);
				float min_e_input_available = factory_min_e_input_available(state, n, fac_type);
				auto const mfactor = state.world.nation_get_modifier_values(n, sys::national_mod_offsets::factory_maintenance) + 1.0f;
				float e_input_scale = input_multiplier * throughput_multiplier * effective_production_scale * (0.1f + min_e_input_available * 0.9f);
				for(uint32_t i = 0; i < economy::small_commodity_set::set_size; ++i) {
					if(e_inputs.commodity_type[i] == c) {
						if(e_inputs.commodity_type[i] == c) {
							economy::register_intermediate_demand(state, n, e_inputs.commodity_type[i], mfactor * e_input_scale * e_inputs.commodity_amounts[i] * (0.1f + min_e_input_available * 0.9f));
						}
					} else {
						break;
					}
				}
			}
		}
		return amount;
	}

	construction_status factory_upgrade(sys::state& state, dcon::factory_id f) {
		auto in_prov = state.world.factory_get_province_from_factory_location(f);
		auto in_state = state.world.province_get_state_membership(in_prov);
		auto fac_type = state.world.factory_get_building_type(f);

		for(auto st_con : state.world.state_instance_get_state_building_construction(in_state)) {
			if(st_con.get_type() == fac_type) {
				float admin_eff = state.world.nation_get_administrative_efficiency(st_con.get_nation());
				float cost_mod = factory_build_cost_modifier(state, st_con.get_nation(), st_con.get_is_pop_project());
				float total = 0.0f;
				float purchased = 0.0f;
				auto& goods = state.world.factory_type_get_construction_costs(fac_type);
				for(uint32_t i = 0; i < economy::commodity_set::set_size; ++i) {
					total += goods.commodity_amounts[i] * cost_mod;
					purchased += std::clamp(st_con.get_purchased_goods().commodity_amounts[i], 0.f, goods.commodity_amounts[i] * cost_mod);
				}
				return construction_status{ total > 0.0f ? purchased / total : 0.0f, true };
			}
		}

		return construction_status{ 0.0f, false };
	}

	bool state_contains_constructed_factory(sys::state& state, dcon::state_instance_id s, dcon::factory_type_id ft) {
		auto d = state.world.state_instance_get_definition(s);
		for(auto p : state.world.state_definition_get_abstract_state_membership(d)) {
			if(p.get_province().get_state_membership() == s) {
				for(auto f : p.get_province().get_factory_location()) {
					if(f.get_factory().get_building_type() == ft)
						return true;
				}
			}
		}
		return false;
	}

	bool state_contains_factory(sys::state& state, dcon::state_instance_id s, dcon::factory_type_id ft) {
		auto d = state.world.state_instance_get_definition(s);

		for(auto p : state.world.state_definition_get_abstract_state_membership(d)) {
			if(p.get_province().get_state_membership() == s) {
				for(auto f : p.get_province().get_factory_location()) {
					if(f.get_factory().get_building_type() == ft)
						return true;
				}
			}
		}
		for(auto sc : state.world.state_instance_get_state_building_construction(s)) {
			if(sc.get_type() == ft)
				return true;
		}

		return false;
	}

	int32_t state_factory_count(sys::state& state, dcon::state_instance_id sid) {
		dcon::nation_id n = state.world.state_instance_get_nation_from_state_ownership(sid);
		int32_t num_factories = 0;
		auto d = state.world.state_instance_get_definition(sid);
		for(auto p : state.world.state_definition_get_abstract_state_membership(d))
			if(p.get_province().get_nation_from_province_ownership() == n) {
				auto fl_range = p.get_province().get_factory_location();
				num_factories += int32_t(fl_range.end() - fl_range.begin());
			}
		for(auto p : state.world.state_instance_get_state_building_construction(sid))
			if(p.get_is_upgrade() == false)
				++num_factories;
		// For new factories: no more than defines:FACTORIES_PER_STATE existing + under construction new factories must be
		//assert(num_factories <= int32_t(state.defines.factories_per_state));
		return num_factories;
	}

	int32_t state_built_factory_count(sys::state& state, dcon::state_instance_id sid) {
		dcon::nation_id n = state.world.state_instance_get_nation_from_state_ownership(sid);
		int32_t num_factories = 0;
		auto d = state.world.state_instance_get_definition(sid);
		for(auto p : state.world.state_definition_get_abstract_state_membership(d))
			if(p.get_province().get_nation_from_province_ownership() == n)
				num_factories += int32_t(state.world.province_get_factory_location(p.get_province()).end() - state.world.province_get_factory_location(p.get_province()).begin());
		// For new factories: no more than defines:FACTORIES_PER_STATE existing + under construction new factories must be
		assert(num_factories <= int32_t(state.defines.factories_per_state));
		return num_factories;
	}

	void add_factory_level_to_state(sys::state& state, dcon::state_instance_id s, dcon::factory_type_id t, bool is_upgrade) {
		if(is_upgrade) {
			auto d = state.world.state_instance_get_definition(s);
			auto o = state.world.state_instance_get_nation_from_state_ownership(s);
			for(auto p : state.world.state_definition_get_abstract_state_membership(d)) {
				if(p.get_province().get_nation_from_province_ownership() == o) {
					for(auto f : p.get_province().get_factory_location()) {
						if(f.get_factory().get_building_type() == t) {
							auto factory_level = f.get_factory().get_level();
							auto new_factory_level = std::min(float(std::numeric_limits<uint8_t>::max()), float(factory_level) + 1.f + math::sqrt(f.get_factory().get_level()) / 2.f);
							f.get_factory().get_level() = uint8_t(new_factory_level);
							return;
						}
					}
				}
			}
			assert(false);
			return;
		}

		/* fail if adding new factory results in more than allowed factories! */
		if(state_factory_count(state, s) + 1 >= int32_t(state.defines.factories_per_state))
			return;

		auto state_cap = state.world.state_instance_get_capital(s);
		auto new_fac = fatten(state.world, state.world.create_factory());
		new_fac.set_building_type(t);
		new_fac.set_level(uint8_t(1));
		new_fac.set_production_scale(1.0f);
		state.world.try_create_factory_location(new_fac, state_cap);
	}

	void try_add_factory_to_state(sys::state& state, dcon::state_instance_id s, dcon::factory_type_id t) {
		auto n = state.world.state_instance_get_nation_from_state_ownership(s);

		if(state.world.factory_type_get_is_coastal(t)) {
			if(!province::state_is_coastal(state, s))
				return; // requires coast to build coastal factory
		}

		auto existing_constructions = state.world.state_instance_get_state_building_construction(s);
		int32_t num_factories = 0;
		for(auto prj : existing_constructions) {
			if(!prj.get_is_upgrade())
				++num_factories;
			if(prj.get_type() == t)
				return; // can't duplicate type
		}

		// is there an upgrade target ?
		auto d = state.world.state_instance_get_definition(s);
		for(auto p : state.world.state_definition_get_abstract_state_membership(d)) {
			if(p.get_province().get_nation_from_province_ownership() == n) {
				for(auto f : p.get_province().get_factory_location()) {
					++num_factories;
					if(f.get_factory().get_building_type() == t)
						return; // can't build another of this type
				}
			}
		}

		if(num_factories < int32_t(state.defines.factories_per_state)) {
			add_factory_level_to_state(state, s, t, false);
		}
	}

	void prune_factories(sys::state& state) {
		for(auto si : state.world.in_state_instance) {
			auto owner = si.get_nation_from_state_ownership();
			auto rules = owner.get_combined_issue_rules();

			if(owner.get_is_player_controlled() && (rules & issue_rule::destroy_factory) != 0) // not for players who can manually destroy
				continue;

			dcon::factory_id deletion_choice;
			int32_t factory_count = 0;

			province::for_each_province_in_state_instance(state, si, [&](dcon::province_id p) {
				for(auto f : state.world.province_get_factory_location(p)) {
					++factory_count;
					auto scale = f.get_factory().get_production_scale();
					float ten_workers = 10.f / factory_max_employment(state, f.get_factory());
					bool unprofitable = f.get_factory().get_unprofitable();
					if(((scale < ten_workers) && unprofitable) && (!deletion_choice || state.world.factory_get_level(deletion_choice) > f.get_factory().get_level())) {
						deletion_choice = f.get_factory();
					}
				}
			});

			// aggressive pruning
			// to help building more healthy economy instead of 1 profitable giant factory with 6 small 0 scale factories
			if(deletion_choice && (4 + factory_count) >= int32_t(state.defines.factories_per_state)) {
				auto production_type = state.world.factory_get_building_type(deletion_choice);
				state.world.delete_factory(deletion_choice);

				for(auto proj : si.get_state_building_construction()) {
					if(proj.get_type() == production_type) {
						state.world.delete_state_building_construction(proj);
						break;
					}
				}
			}
		}
	}
}
