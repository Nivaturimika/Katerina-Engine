#include "system_state.hpp"
#include "economy_factory.hpp"
#include "demographics.hpp"
#include "dcon_generated.hpp"
#include "ai.hpp"
#include "prng.hpp"
#include "math_fns.hpp"
#include "nations_templates.hpp"
#include "province_templates.hpp"
#include "triggers.hpp"
#include "economy_factory_templates.hpp"
#include "economy.hpp"
#include "pdqsort.h"

namespace economy_factory {
	constexpr inline float production_scale_delta = 0.1f;

	bool factory_is_profitable(sys::state const& state, dcon::factory_id f) {
		return !state.world.factory_get_unprofitable(f)
			|| state.world.factory_get_subsidized(f);
	}

	/*	Checks if the given state has any factory, be it a in-progress one or one that is already built
		Type is not checked for */
	bool has_factory(sys::state const& state, dcon::state_instance_id si) {
		auto const sdef = state.world.state_instance_get_definition(si);
		auto const owner_of_state = state.world.state_instance_get_nation_from_state_ownership(si);
		auto const sbc = state.world.state_instance_get_state_building_construction(si);
		if(sbc.begin() != sbc.end()) {
			return true;
		}
		for(auto const memb : state.world.state_definition_get_abstract_state_membership(sdef)) {
			if(memb.get_province().get_nation_from_province_ownership() == owner_of_state) {
				auto const fl = memb.get_province().get_factory_location();
				if(fl.begin() != fl.end())
					return true;
			}
		}
		return false;
	}

	/* Whetever the nation is constructing a factory of any type */
	bool nation_is_constructing_factories(sys::state& state, dcon::nation_id n) {
		auto const sbc = state.world.nation_get_state_building_construction(n);
		return sbc.begin() != sbc.end();
	}

	bool nation_has_closed_factories(sys::state& state, dcon::nation_id n) { // TODO - should be "good" now
		for(auto const po : state.world.nation_get_province_ownership(n)) {
			for(auto const fl : po.get_province().get_factory_location()) {
				auto const production_scale = fl.get_factory().get_production_scale();
				if(production_scale < factory_closed_threshold) {
					return true;
				}
			}
		}
		return false;
	}

	bool state_instance_has_factory_being_built(sys::state const& state, dcon::state_instance_id sid, dcon::factory_type_id ft) {
		for(auto const p : state.world.state_instance_get_state_building_construction(sid)) {
			if(p.get_type() == ft) {
				return true;
			}
		}
		return false;
	}

	bool state_contains_constructed_factory(sys::state& state, dcon::state_instance_id sid, dcon::factory_type_id f) {
		auto sdef = state.world.state_instance_get_definition(sid);
		for(auto const memb : state.world.state_definition_get_abstract_state_membership(sdef)) {
			if(memb.get_province().get_state_membership() == sid) {
				for(auto fl : memb.get_province().get_factory_location()) {
					if(fl.get_factory().get_building_type() == f) {
						return true;
					}
				}
			}
		}
		return false;
	}

	/* Checks if the given state contains a factory:
		- Either one that is being built
		- Or one that is already in-place */
	bool state_contains_factory(sys::state& state, dcon::state_instance_id state_instance_id, dcon::factory_type_id factory_type_id) {
		auto sdef = state.world.state_instance_get_definition(state_instance_id);
		for(auto state_membership_id : state.world.state_definition_get_abstract_state_membership(sdef)) {
			if(state_membership_id.get_province().get_state_membership() == state_instance_id) {
				for(auto fl : state_membership_id.get_province().get_factory_location()) {
					if(fl.get_factory().get_building_type() == factory_type_id) {
						return true;
					}
				}
			}
		}
		for(auto sbc : state.world.state_instance_get_state_building_construction(state_instance_id)) {
			if(sbc.get_type() == factory_type_id) {
				return true;
			}
		}
		return false;
	}

	float factory_build_cost_modifier(sys::state& state, dcon::nation_id nation_id, bool pop_project) {
		float admin_efficiency = state.world.nation_get_administrative_efficiency(nation_id);
		float factory_modifier = std::max(0.001f, state.world.nation_get_modifier_values(nation_id, sys::national_mod_offsets::factory_cost) + 1.0f);
		float pop_factory_modifier = std::max(0.001f, state.world.nation_get_modifier_values(nation_id, sys::national_mod_offsets::factory_owner_cost));
		return (pop_project ? pop_factory_modifier : (2.0f - admin_efficiency)) * factory_modifier;
	}

	float factory_desired_raw_profit(sys::state& state, dcon::factory_type_id ft, float level) {
		auto const output = state.world.factory_type_get_output(ft);
		auto const max_output = state.world.factory_type_get_output_amount(ft);
		auto const output_price = state.world.commodity_get_current_price(output);
		return (max_output * output_price * level);
	}

	float factory_efficiency_input_total_cost(sys::state& state, dcon::nation_id n, dcon::factory_type_id ft) {
		return economy::commodity_set_effective_cost(state, n, state.world.factory_type_get_efficiency_inputs(ft));
	}

	/* - Then, for input/output/throughput we sum up national and provincial modifiers to general factory
		input/output/throughput are added, plus technology modifiers to its specific output commodity, add
		one to the sum, and then multiply the input/output/throughput modifier from the workforce by it. */
	float factory_input_multiplier(sys::state& state, dcon::factory_id f, dcon::nation_id n, dcon::province_id p, dcon::state_instance_id sid) {
		auto const total_workers = factory_max_employment(state, f);
		auto const total_state_pop = std::max(0.001f, state.world.state_instance_get_demographics(sid, demographics::total));
		auto const capitalists = state.world.state_instance_get_demographics(sid, demographics::to_key(state, state.culture_definitions.capitalists));
		auto const owner_fraction = total_state_pop > 0.f ? std::min(0.05f, capitalists / total_state_pop) : 0.0f;
		return std::max(0.1f, (1.f
			+ state.world.province_get_modifier_values(p, sys::provincial_mod_offsets::local_factory_input)
			+ state.world.nation_get_modifier_values(n, sys::national_mod_offsets::factory_input)
			+ owner_fraction * -2.5f));
	}

	float factory_input_total_cost(sys::state& state, dcon::nation_id n, dcon::factory_type_id ft) {
		return economy::commodity_set_effective_cost(state, n, state.world.factory_type_get_inputs(ft));
	}

	float factory_max_employment(sys::state const& state, dcon::factory_id f) {
		auto ft = state.world.factory_get_building_type(f);
		return state.world.factory_type_get_base_workforce(ft) * float(state.world.factory_get_level(f));
	}

	float factory_max_production_scale(sys::state& state, dcon::factory_id f, float mobilization_impact, bool occupied) {
		return state.world.factory_get_primary_employment(f)
			* state.world.factory_get_level(f)
			* (occupied ? 0.1f : 1.0f)
			* std::max(0.0f, mobilization_impact);
	}

	float factory_min_input_available(sys::state& state, dcon::nation_id n, dcon::factory_type_id ft) {
		return economy::commodity_set_total_satisfaction(state, n, state.world.factory_type_get_inputs(ft));
	}

	float factory_min_efficiency_input_available(sys::state& state, dcon::nation_id n, dcon::factory_type_id ft) {
		return economy::commodity_set_total_satisfaction(state, n, state.world.factory_type_get_efficiency_inputs(ft));
	}

	float factory_output_multiplier(sys::state& state, dcon::factory_id f, dcon::nation_id n, dcon::province_id p) {
		auto fac_type = state.world.factory_get_building_type(f);
		return state.world.nation_get_factory_goods_output(n, fac_type.get_output())
			+ state.world.province_get_modifier_values(p, sys::provincial_mod_offsets::local_factory_output)
			+ state.world.nation_get_modifier_values(n, sys::national_mod_offsets::factory_output)
			+ state.world.factory_get_secondary_employment(f)
			* (1.0f - state.economy_definitions.craftsmen_fraction) * 1.5f + 1.0f;
	}

	float factory_primary_employment(sys::state const& state, dcon::factory_id f) {
		auto const primary_employment = state.world.factory_get_primary_employment(f);
		return factory_max_employment(state, f) * (state.economy_definitions.craftsmen_fraction * primary_employment);
	}

	float factory_secondary_employment(sys::state const& state, dcon::factory_id f) {
		auto const secondary_employment = state.world.factory_get_secondary_employment(f);
		return factory_max_employment(state, f) * ((1 - state.economy_definitions.craftsmen_fraction) * secondary_employment);
	}

	float factory_throughput_multiplier(sys::state& state, dcon::factory_type_id ft, dcon::nation_id n, dcon::province_id p, dcon::state_instance_id sid) {
		return state.world.nation_get_factory_goods_throughput(n, state.world.factory_type_get_output(ft))
			+ state.world.province_get_modifier_values(p, sys::provincial_mod_offsets::local_factory_throughput)
			+ state.world.nation_get_modifier_values(n, sys::national_mod_offsets::factory_throughput) + 1.0f;
	}

	float factory_total_employment(sys::state const& state, dcon::factory_id f) {
		// TODO: Document this, also is this a stub?
		auto const primary_employment = state.world.factory_get_primary_employment(f);
		auto const secondary_employment = state.world.factory_get_secondary_employment(f);
		return factory_max_employment(state, f) * (state.economy_definitions.craftsmen_fraction
			* primary_employment + (1 - state.economy_definitions.craftsmen_fraction) * secondary_employment);
	}

	float global_factory_construction_time_modifier(sys::state& state) {
		return 1.f;
	}

	float global_non_factory_construction_time_modifier(sys::state& state) {
		return 1.f;
	}

	/* Obtains the consumption done by factories of a given good */
	float nation_factory_consumption(sys::state& state, dcon::nation_id n, dcon::commodity_id c) {
		auto total = 0.f;
		auto const mobilization_impact = military::mobilization_impact(state, n);
		for(const auto po : state.world.nation_get_province_ownership(n)) {
			auto const occupied = po.get_province().get_nation_from_province_control() != po.get_province().get_nation_from_province_ownership();
			for(const auto fl : po.get_province().get_factory_location()) {
				auto const f = fl.get_factory();
				auto const ft = fl.get_factory().get_building_type();
				auto const province_fat_id = po.get_province();
				auto const state_instance_fat_id = po.get_province().get_state_membership();
				//
				auto const total_workers = factory_max_employment(state, f);
				auto const max_production_scale = factory_max_production_scale(state, f, mobilization_impact, occupied);
				auto const current_workers = total_workers * max_production_scale;
				//inputs
				auto const min_input_available = factory_min_input_available(state, n, ft);
				//modifiers
				auto const input_multiplier = std::max(0.f, factory_input_multiplier(state, f, n, province_fat_id, state_instance_fat_id) + f.get_triggered_input_modifiers());
				auto const throughput_multiplier = std::max(0.f, factory_throughput_multiplier(state, ft, n, province_fat_id, state_instance_fat_id) + f.get_triggered_modifiers());
				auto const effective_production_scale = std::min(f.get_production_scale() * f.get_level(), max_production_scale);
				//
				auto const& inputs = ft.get_inputs();
				auto const input_scale = input_multiplier * throughput_multiplier * effective_production_scale * (0.1f + min_input_available * 0.9f);
				for(uint32_t i = 0; i < economy::commodity_set::set_size; i++) {
					if(inputs.commodity_type[i]) {
						if(inputs.commodity_type[i] == c) {
							total += input_scale * inputs.commodity_amounts[i] * fl.get_factory().get_actual_production();
						}
					} else {
						break;
					}
				}
				//
				auto const& efficiency_inputs = ft.get_efficiency_inputs();
				auto const min_efficiency_input_available = factory_min_efficiency_input_available(state, n, ft);
				auto const modifier_values = state.world.nation_get_modifier_values(n, sys::national_mod_offsets::factory_maintenance) + 1.0f;
				auto const efficiency_input_scale = input_multiplier * throughput_multiplier * effective_production_scale * (0.1f + min_efficiency_input_available * 0.9f);
				for(uint32_t i = 0; i < economy::small_commodity_set::set_size; ++i) {
					if(efficiency_inputs.commodity_type[i]) {
						if(efficiency_inputs.commodity_type[i] == c) {
							total += modifier_values * efficiency_input_scale * efficiency_inputs.commodity_amounts[i] * (0.1f + min_efficiency_input_available * 0.9f);
						}
					} else {
						break;
					}
				}
			}
		}
		return total;
	}

	/* Returns the minimum wage that should be paid **for a single pop unit** (scale it with base workforce) */
	float pop_factory_min_wage(sys::state& state, dcon::nation_id n, float min_wage_factor) {
		auto const employed = state.world.nation_get_demographics(n, demographics::to_employment_key(state, state.culture_definitions.primary_factory_worker));
		auto const total = state.world.nation_get_demographics(n, demographics::to_key(state, state.culture_definitions.primary_factory_worker));
		auto const unemp_ratio = total > 0.f ? employed / total : 0.f;
		auto const life_needs_cost = state.world.nation_get_life_needs_costs(n, state.culture_definitions.primary_factory_worker);
		auto const everyday_needs_cost = state.world.nation_get_everyday_needs_costs(n, state.culture_definitions.primary_factory_worker);
		return min_wage_factor * (life_needs_cost + everyday_needs_cost)
			* 1.1f * unemp_ratio * unemp_ratio * unemp_ratio;
	}

	/* Obtains the sum of factory throughput modifiers */
	float sum_of_factory_triggered_modifiers(sys::state& state, dcon::factory_type_id ft, dcon::state_instance_id sid) {
		auto const n = state.world.state_instance_get_nation_from_state_ownership(sid);
		if(auto mod_k = state.world.factory_type_get_throughput_bonus(ft); mod_k) {
			return trigger::evaluate_additive_modifier(state, mod_k, trigger::to_generic(sid), trigger::to_generic(n), 0);
		}
		return 0.f;
	}

	float sum_of_factory_triggered_input_modifiers(sys::state& state, dcon::factory_type_id ft, dcon::state_instance_id sid) {
		auto const n = state.world.state_instance_get_nation_from_state_ownership(sid);
		if(auto mod_k = state.world.factory_type_get_input_bonus(ft); mod_k) {
			return trigger::evaluate_additive_modifier(state, mod_k, trigger::to_generic(sid), trigger::to_generic(n), 0);
		}
		return 0.f;
	}

	float update_factory_scale(sys::state& state, dcon::factory_id f, float max_production_scale, float raw_profit, float desired_raw_profit) {
		auto fat_id = dcon::fatten(state.world, f);
		auto ft = state.world.factory_get_building_type(f);

		auto const total_workers = factory_max_employment(state, f);
		auto const several_workers_scale = 10.f / total_workers;
		auto const total_activity = state.world.commodity_get_total_production(fat_id.get_building_type().get_output())
			+ state.world.commodity_get_total_real_demand(fat_id.get_building_type().get_output()) + 10.f;
		auto const relative_production_amount = state.world.factory_type_get_output_amount(ft) / total_activity;
		auto const relative_modifier = (1.f / std::max(0.01f, relative_production_amount)) / 1000.f;
		auto const effective_production_scale = 0.0f;
		auto new_production_scale = 0.0f;
		if(state.world.factory_get_subsidized(f)) {
			new_production_scale = std::min(1.0f, state.world.factory_get_production_scale(f) + several_workers_scale * state.world.factory_get_level(f) * 10.f);
		} else {
			auto const over_profit_ratio = raw_profit / std::max(desired_raw_profit, 0001f) - 1.f;
			auto const under_profit_ratio = desired_raw_profit / std::max(raw_profit, 0.0001f) - 1.f;
			auto const speed_modifier = (over_profit_ratio - under_profit_ratio);
			auto speed = economy_factory::production_scale_delta * speed_modifier + several_workers_scale * ((raw_profit - desired_raw_profit > 0.f) ? 1.f : -1.f);
			speed = std::clamp(speed, -relative_modifier, relative_modifier);
			new_production_scale = std::clamp(state.world.factory_get_production_scale(f) + speed, 0.f, 1.f);
		}
		state.world.factory_set_production_scale(f, new_production_scale);
		return std::min(new_production_scale * state.world.factory_get_level(f), max_production_scale);
	}

	/* Obtains the priority of a factory, can be 0 (none), 1 (low), 2 (mid) or 3 (high) */
	int32_t factory_priority(sys::state const& state, dcon::factory_id factory_id) {
		return (state.world.factory_get_priority_low(factory_id) ? 1 : 0)
			+ (state.world.factory_get_priority_high(factory_id) ? 2 : 0);
	}

	/* Count built factories only */
	int32_t state_built_factory_count(sys::state& state, dcon::state_instance_id sid) {
		dcon::nation_id nation_id = state.world.state_instance_get_nation_from_state_ownership(sid);
		int32_t number_of_factories = 0;
		auto state_definition_fat_id = state.world.state_instance_get_definition(sid);
		for(auto memb : state.world.state_definition_get_abstract_state_membership(state_definition_fat_id))
			if(memb.get_province().get_nation_from_province_ownership() == nation_id)
				number_of_factories += int32_t(state.world.province_get_factory_location(memb.get_province()).end() - state.world.province_get_factory_location(memb.get_province()).begin());
		// For new factories: no more than defines:FACTORIES_PER_STATE existing + under construction new factories must be
		assert(number_of_factories <= int32_t(state.defines.factories_per_state));
		return number_of_factories;
	}

	/* Count of total factories the already built ones and the ones being built */
	int32_t state_factory_count(sys::state& state, dcon::state_instance_id sid) {
		dcon::nation_id nationd_id = state.world.state_instance_get_nation_from_state_ownership(sid);
		int32_t number_of_factories = 0;
		auto sdef = state.world.state_instance_get_definition(sid);
		for(auto const memb : state.world.state_definition_get_abstract_state_membership(sdef)) {
			if(memb.get_province().get_nation_from_province_ownership() == nationd_id) {
				auto const fl = memb.get_province().get_factory_location();
				number_of_factories += int32_t(fl.end() - fl.begin());
			}
		}
		for(auto const sbc : state.world.state_instance_get_state_building_construction(sid)) {
			if(sbc.get_is_upgrade() == false) {
				++number_of_factories;
			}
		}
		// For new factories: no more than defines:FACTORIES_PER_STATE existing + under construction new factories must be
		//assert(num_factories <= int32_t(state.defines.factories_per_state));
		return number_of_factories;
	}

	void add_factory_level_to_state(sys::state& state, dcon::state_instance_id si, dcon::factory_type_id ft, bool is_upgrade) {
		if(is_upgrade) {
			auto const sdef = state.world.state_instance_get_definition(si);
			auto const n = state.world.state_instance_get_nation_from_state_ownership(si);
			for(auto const memb : state.world.state_definition_get_abstract_state_membership(sdef)) {
				if(memb.get_province().get_nation_from_province_ownership() == n) {
					for(auto fl : memb.get_province().get_factory_location()) {
						if(fl.get_factory().get_building_type() == ft) {
							auto const factory_level = fl.get_factory().get_level();
							auto const new_factory_level = std::min(float(std::numeric_limits<uint8_t>::max()), float(factory_level) + 1.f + math::sqrt(fl.get_factory().get_level()) / 2.f);
							fl.get_factory().get_level() = uint8_t(new_factory_level);
							return;
						}
					}
				}
			}
			assert(false);
			return;
		}

		/* fail if adding new factory results in more than allowed factories! */
		if(state_factory_count(state, si) + 1 >= int32_t(state.defines.factories_per_state))
			return;

		auto cap = state.world.state_instance_get_capital(si);
		auto new_factory = fatten(state.world, state.world.create_factory());
		new_factory.set_building_type(ft);
		new_factory.set_level(uint8_t(1));
		new_factory.set_production_scale(1.0f);
		// new factories start subsidized
		if(auto owner = state.world.state_instance_get_nation_from_state_ownership(si); owner) {
			auto rules = state.world.nation_get_combined_issue_rules(owner);
			if((rules & issue_rule::can_subsidise) != 0) {
				new_factory.set_subsidized(true);
			}
		}
		state.world.try_create_factory_location(new_factory, cap);
	}

	void delete_factory(sys::state& state, dcon::factory_id f) {
		auto const p = state.world.factory_get_province_from_factory_location(f);
		auto const si = state.world.province_get_state_membership(p);
		auto const ft = state.world.factory_get_building_type(f);
		/* Delete the factory */
		state.world.delete_factory(f);
		/* Delete all projects to upgrade the factory */
		for(auto sbc : state.world.state_instance_get_state_building_construction(si)) {
			if(sbc.get_type() == ft && sbc.get_is_upgrade()) {
				state.world.delete_state_building_construction(sbc);
				break;
			}
		}
	}

	void prune_factories(sys::state& state) {
		/*
		for(auto state_instance_fat_id : state.world.in_state_instance) {
			auto owner = state_instance_fat_id.get_nation_from_state_ownership();
			auto combined_issue_rules = owner.get_combined_issue_rules();
			if(owner.get_is_player_controlled() && (combined_issue_rules & issue_rule::destroy_factory) != 0) // not for players who can manually destroy
				continue;

			dcon::factory_id deletion_choice;
			int32_t factory_count = 0;
			province::for_each_province_in_state_instance(state, state_instance_fat_id, [&](dcon::province_id p) {
				for(auto fl : state.world.province_get_factory_location(p)) {
					++factory_count;
					auto const scale = fl.get_factory().get_production_scale();
					auto const ten_workers = 10.f / factory_max_employment(state, fl.get_factory());
					bool unprofitable = fl.get_factory().get_unprofitable();
					if(((scale < ten_workers) && unprofitable)
					&& (!deletion_choice || state.world.factory_get_level(deletion_choice) > fl.get_factory().get_level())) {
						deletion_choice = fl.get_factory();
					}
				}
			});

			// do way less pruning of factories so we have a more stable economy
			// (as opposed to a diverse one)
			if(deletion_choice && (1 + factory_count) >= int32_t(state.defines.factories_per_state)) {
				auto production_type = state.world.factory_get_building_type(deletion_choice);
				// and only iff production > real demand
				if(production_type.get_output().get_total_production() > production_type.get_output().get_total_real_demand()) {
					economy_factory::delete_factory(state, deletion_choice);
				}
			}
		}
		*/
	}

	void set_factory_priority(sys::state& state, dcon::factory_id factory_id, int32_t priority) {
		state.world.factory_set_priority_high(factory_id, priority >= 2);
		state.world.factory_set_priority_low(factory_id, (priority & 1) != 0);
	}

	void try_add_factory_to_state(sys::state& state, dcon::state_instance_id sid, dcon::factory_type_id ft) {
		if(state.world.factory_type_get_is_coastal(ft)
		&& !province::state_is_coastal(state, sid)) {
			return; // requires coast to build coastal factory
		}
		
		int32_t number_of_factories = 0;
		for(auto const sbc : state.world.state_instance_get_state_building_construction(sid)) {
			if(!sbc.get_is_upgrade())
				++number_of_factories;
			if(sbc.get_type() == ft)
				return; // can't duplicate type
		}

		// is there an upgrade target ?
		auto const n = state.world.state_instance_get_nation_from_state_ownership(sid);
		auto const sdef = state.world.state_instance_get_definition(sid);
		for(auto const memb : state.world.state_definition_get_abstract_state_membership(sdef)) {
			if(memb.get_province().get_nation_from_province_ownership() == n) {
				for(auto const fl : memb.get_province().get_factory_location()) {
					++number_of_factories;
					if(fl.get_factory().get_building_type() == ft)
						return; // can't build another of this type
				}
			}
		}
		if(number_of_factories < int32_t(state.defines.factories_per_state)) {
			add_factory_level_to_state(state, sid, ft, false);
		}
	}

	void update_factory_employment(sys::state& state) {
		auto const pw_key = demographics::to_key(state, state.culture_definitions.primary_factory_worker);
		auto const sw_key = demographics::to_key(state, state.culture_definitions.secondary_factory_worker);
		state.world.for_each_state_instance([&](dcon::state_instance_id sid) {
			auto const primary_pool = state.world.state_instance_get_demographics(sid, pw_key);
			auto const secondary_pool = state.world.state_instance_get_demographics(sid, sw_key);

			static std::vector<dcon::factory_id, dcon::cache_aligned_allocator<dcon::factory_id>> ordered_factories;
			ordered_factories.clear();

			province::for_each_province_in_state_instance(state, sid, [&](dcon::province_id p) {
				for(auto fl : state.world.province_get_factory_location(p)) {
					ordered_factories.push_back(fl.get_factory());
				}
			});

			pdqsort(ordered_factories.begin(), ordered_factories.end(), [&](dcon::factory_id a, dcon::factory_id b) {
				if(factory_is_profitable(state, a) != factory_is_profitable(state, b)) {
					return factory_is_profitable(state, a);
				}
				if(factory_priority(state, a) != factory_priority(state, b)) {
					return factory_priority(state, a) > factory_priority(state, b);
				}
				return a.index() < b.index();
			});

			auto const employment_shift_speed = 0.05f;

			// Remaining employed for pool
			auto emp_pw_workers = primary_pool;
			auto emp_sw_workers = secondary_pool;
			for(uint32_t index = 0; index < ordered_factories.size();) {
				uint32_t next_index = index;
				auto total_workforce = 0.0f;
				for(; next_index < ordered_factories.size(); ++next_index) {
					if(factory_is_profitable(state, ordered_factories[index]) != factory_is_profitable(state, ordered_factories[next_index])
					|| factory_priority(state, ordered_factories[index]) != factory_priority(state, ordered_factories[next_index])) {
						break;
					}
					total_workforce += factory_max_employment(state, ordered_factories[next_index])
						* state.world.factory_get_production_scale(ordered_factories[next_index]);
				}
				auto const pw_workers_share = state.economy_definitions.craftsmen_fraction * total_workforce;
				auto const sw_workers_share = (1.0f - state.economy_definitions.craftsmen_fraction) * total_workforce;
				{
					auto scale = emp_pw_workers >= pw_workers_share ? 1.0f : emp_pw_workers / pw_workers_share;
					emp_pw_workers = std::max(0.0f, emp_pw_workers - pw_workers_share);
					for(uint32_t i = index; i < next_index; ++i) {
						auto const old_employment = state.world.factory_get_primary_employment(ordered_factories[i]);
						auto const new_employment = old_employment * (1.f - employment_shift_speed)
							+ scale * state.world.factory_get_production_scale(ordered_factories[i]) * employment_shift_speed;
						state.world.factory_set_primary_employment(ordered_factories[i], new_employment);
					}
				}
				{
					auto scale = emp_sw_workers >= sw_workers_share ? 1.0f : emp_sw_workers / sw_workers_share;
					emp_sw_workers = std::max(0.0f, emp_sw_workers - sw_workers_share);
					for(uint32_t i = index; i < next_index; ++i) {
						auto const old_employment = state.world.factory_get_secondary_employment(ordered_factories[i]);
						auto const new_employment = old_employment * (1.f - employment_shift_speed)
							+ scale * state.world.factory_get_production_scale(ordered_factories[i]) * employment_shift_speed;
						state.world.factory_set_secondary_employment(ordered_factories[i], new_employment);
					}
				}
				index = next_index;
			}

			auto const primary_employment = 1.f - (primary_pool > 0.f ? emp_pw_workers / primary_pool : 0.f);
			auto const secondary_employment = 1.f - (secondary_pool > 0.f ? emp_sw_workers / secondary_pool : 0.f);
			assert(primary_employment >= 0.f && primary_employment <= 1.f);
			assert(secondary_employment >= 0.f && secondary_employment <= 1.f);
			assert(primary_employment + secondary_employment <= 2.f);
			province::for_each_province_in_state_instance(state, sid, [&](dcon::province_id p) {
				for(auto pl : state.world.province_get_pop_location(p)) {
					if(pl.get_pop().get_poptype() == state.culture_definitions.primary_factory_worker) {
						pl.get_pop().set_employment(pl.get_pop().get_size() * primary_employment);
					} else if(pl.get_pop().get_poptype() == state.culture_definitions.secondary_factory_worker) {
						pl.get_pop().set_employment(pl.get_pop().get_size() * secondary_employment);
					}
				}
			});
		});
	}

	void update_factory_triggered_modifiers(sys::state& state) {
		state.world.for_each_factory([&](dcon::factory_id f) {
			auto const ft = fatten(state.world, state.world.factory_get_building_type(f));
			auto const p = state.world.factory_get_province_from_factory_location(f);
			auto const sid = state.world.province_get_state_membership(p);
			auto const owner = state.world.province_get_nation_from_province_ownership(p);
			// Throughput bonuses
			auto t_sum = 1.f;
			if(owner && sid) {
				t_sum += sum_of_factory_triggered_modifiers(state, ft, sid);
			}
			state.world.factory_set_triggered_modifiers(f, t_sum);
			// Input bonuses
			auto i_sum = 0.f;
			if(owner && sid) {
				i_sum = sum_of_factory_triggered_input_modifiers(state, ft, sid);
			}
			state.world.factory_set_triggered_input_modifiers(f, i_sum);
		});
	}

	/*	Obtains the total production from a factory (assuming it is fully employed)
	- A factory *produces* efficiency-adjusted-consumption-scale x output-multiplier x throughput-multiplier x level x
	production quantity amount of its output good, which gets added to domestic supply
	*/
	float total_employed_factory_production(sys::state& state, dcon::factory_id f, dcon::nation_id n, dcon::state_instance_id sid, dcon::province_id p) {
		auto const ft = state.world.factory_get_building_type(f);
		// Inputs
		auto const min_input_available = factory_min_input_available(state, n, ft);
		auto const min_efficiency_input_available = factory_min_efficiency_input_available(state, n, ft);
		// Modifiers
		auto const throughput_multiplier = std::max(0.f, factory_throughput_multiplier(state, ft, n, p, sid) + state.world.factory_get_triggered_modifiers(f));
		auto const output_multiplier = std::max(0.f, factory_output_multiplier(state, f, n, p));
		return state.world.factory_type_get_output_amount(ft)
			* float(state.world.factory_get_level(f))
			* (0.75f + 0.25f * min_efficiency_input_available)
			* throughput_multiplier
			* output_multiplier
			* min_input_available;
	}
	
	void update_single_factory_consumption(sys::state& state, dcon::factory_id f, dcon::nation_id n, dcon::province_id p, dcon::state_instance_id si, float mobilization_impact, float expected_min_wage, bool occupied) {
		auto const ft = state.world.factory_get_building_type(f);
		assert(ft);
		assert(ft.get_output());
		assert(n);
		assert(p);
		assert(si);

		auto const total_workers = factory_max_employment(state, f);
		auto const max_production_scale = factory_max_production_scale(state, f, mobilization_impact, occupied);
		auto const current_workers = total_workers * max_production_scale;

		//inputs
		auto const input_total = factory_input_total_cost(state, n, ft);
		auto const min_input_available = factory_min_input_available(state, n, ft);
		auto const efficiency_input_total = factory_efficiency_input_total_cost(state, n, ft);
		auto const min_efficiency_input_available = factory_min_efficiency_input_available(state, n, ft);

		//modifiers
		auto const input_multiplier = std::max(0.f, factory_input_multiplier(state, f, n, p, si));
		auto const throughput_multiplier = std::max(0.f, factory_throughput_multiplier(state, ft, n, p, si) + state.world.factory_get_triggered_modifiers(f));
		auto const output_multiplier = std::max(0.f, factory_output_multiplier(state, f, n, p));

		auto const total_production = total_employed_factory_production(state, f, n, si, p);

		//this value represents raw profit if 1 lvl of this factory is filled with workers
		auto const max_income = total_production * state.world.commodity_get_current_price(ft.get_output());
		//this value represents spendings if 1 lvl of this factory is filled with workers
		auto const max_expenses = expected_min_wage * state.world.factory_type_get_base_workforce(ft)
			+ input_multiplier * throughput_multiplier * input_total * min_input_available
			+ input_multiplier * efficiency_input_total * min_efficiency_input_available * min_input_available;

		auto const desired_profit = factory_desired_raw_profit(state, ft, state.world.factory_get_level(f));
		auto const max_pure_profit = max_income - max_expenses;
		state.world.factory_set_unprofitable(f, max_pure_profit <= 0.0f);

		auto const effective_production_scale = update_factory_scale(state, f, max_production_scale, max_income, desired_profit);
		auto const& inputs = ft.get_inputs();
		auto const& efficiency_inputs = ft.get_efficiency_inputs();

		// register real demand : input_multiplier * throughput_multiplier * level * primary_employment
		// also multiply by target production scale... otherwise too much excess demand is generated
		// also multiply by something related to minimal satisfied input
		// to prevent generation of too much demand on rgos already influenced by a shortage
		auto const input_scale = input_multiplier * throughput_multiplier * effective_production_scale * (0.1f + min_input_available * 0.9f);
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
		auto const efficiency_input_scale = input_multiplier * throughput_multiplier * effective_production_scale * (0.1f + min_efficiency_input_available * 0.9f);
		for(uint32_t i = 0; i < economy::small_commodity_set::set_size; ++i) {
			if(efficiency_inputs.commodity_type[i]) {
				economy::register_intermediate_demand(state, n, efficiency_inputs.commodity_type[i], mfactor * efficiency_input_scale * efficiency_inputs.commodity_amounts[i] * (0.1f + min_efficiency_input_available * 0.9f));
			} else {
				break;
			}
		}

		auto const actual_production = total_production * effective_production_scale;
		auto const pure_profit = max_pure_profit * effective_production_scale;
		state.world.factory_set_actual_production(f, actual_production);
		state.world.factory_set_full_profit(f, pure_profit);
	}

	/* Updates the production of a given factory, additionally it also returns the costs of subsidizing said factory
	   0 is returned if it isn't subsidized */
	float update_single_factory_production(sys::state& state, dcon::factory_id f, dcon::nation_id n, float expected_min_wage) {
		auto factory_production = state.world.factory_get_actual_production(f);
		if(factory_production > 0.f) {
			auto ft = state.world.factory_get_building_type(f);
			auto money_made = state.world.factory_get_full_profit(f);
			state.world.factory_set_actual_production(f, factory_production);
			economy::register_domestic_supply(state, n, state.world.factory_type_get_output(ft), factory_production);
			if(state.world.factory_get_subsidized(f)) {
				float min_wages = expected_min_wage * state.world.factory_get_level(f) * state.world.factory_get_primary_employment(f);
				if(money_made < min_wages) {
					auto diff = min_wages - money_made;
					assert(diff > 0.0f);
					auto nat_money = state.world.nation_get_stockpiles(n, economy::money);
					if(nat_money > diff || economy::can_take_loans(state, n)) {
						state.world.factory_set_full_profit(f, min_wages);
						return diff;
					} else {
						state.world.factory_set_full_profit(f, std::max(money_made, 0.0f));
						state.world.factory_set_subsidized(f, false);
					}
				} else {
					state.world.factory_set_full_profit(f, money_made);
				}
			} else {
				state.world.factory_set_full_profit(f, money_made);
			}
		}
		return 0.f;
	}

	economy::construction_status factory_upgrade(sys::state& state, dcon::factory_id f) {
		auto const in_province = state.world.factory_get_province_from_factory_location(f);
		auto const in_state = state.world.province_get_state_membership(in_province);
		auto const ft = state.world.factory_get_building_type(f);
		for(auto c : state.world.state_instance_get_state_building_construction(in_state)) {
			if(c.get_type() == ft) {
				float admin_eff = state.world.nation_get_administrative_efficiency(c.get_nation());
				float build_cost_modifier = factory_build_cost_modifier(state, c.get_nation(), c.get_is_pop_project());
				float total = c.get_type().get_construction_time();
				float value = c.get_remaining_construction_time();
				return economy::construction_status{ total > 0.f ? 1.f - (value / total) : 0.f, true };
			}
		}
		return economy::construction_status{ 0.0f, false };
	}

	/* Obtains the profit distribution (for primary, secondary and owners) of a given factory
		@param min_wage The minimum wage factor that is to be paid to primary/secondary workers
		@param total_profit The total profit of a given factory
	*/
	economy::profit_distribution distribute_factory_profit(sys::state const& state, dcon::state_instance_id sid, float min_wage, float total_profit) {
		auto const total_min_primary_workers_wage = min_wage * state.world.state_instance_get_demographics(sid, demographics::to_employment_key(state, state.culture_definitions.primary_factory_worker));
		auto const total_min_secondary_workers_wage = min_wage * state.world.state_instance_get_demographics(sid, demographics::to_employment_key(state, state.culture_definitions.secondary_factory_worker));
		auto const number_of_primary_workers = state.world.state_instance_get_demographics(sid, demographics::to_key(state, state.culture_definitions.primary_factory_worker));
		auto const number_of_secondary_workers = state.world.state_instance_get_demographics(sid, demographics::to_key(state, state.culture_definitions.secondary_factory_worker));
		auto const number_of_owners = state.world.state_instance_get_demographics(sid, demographics::to_key(state, state.culture_definitions.capitalists));

		auto const n = state.world.state_instance_get_nation_from_state_ownership(sid);
		auto const desired_per_primary_worker_profit = state.world.nation_get_life_needs_costs(n, state.culture_definitions.primary_factory_worker) * (1.f / 5.f);
		auto const desired_per_secondary_worker_profit = state.world.nation_get_life_needs_costs(n, state.culture_definitions.secondary_factory_worker) * (1.f / 5.f);
		auto const desired_per_owner_profit = state.world.nation_get_life_needs_costs(n, state.culture_definitions.capitalists) * (1.f / 5.f);
		auto const total_desired_profit = desired_per_primary_worker_profit + desired_per_secondary_worker_profit + desired_per_owner_profit;
		if(total_desired_profit == 0.f) {
			return economy::profit_distribution{};
		}
		auto const per_owner_profit = total_profit * (desired_per_owner_profit / total_desired_profit);
		auto const per_primary_worker_profit = total_profit * (desired_per_primary_worker_profit / total_desired_profit);
		auto const per_secondary_worker_profit = total_profit * (desired_per_secondary_worker_profit / total_desired_profit);
		return economy::profit_distribution{
			per_primary_worker_profit / number_of_primary_workers,
			per_secondary_worker_profit / number_of_secondary_workers,
			per_owner_profit / number_of_owners
		};
	}

	template void for_each_new_factory<std::function<void(new_factory)>>(sys::state&, dcon::state_instance_id, std::function<void(new_factory)>&&);
	template void for_each_upgraded_factory<std::function<void(upgraded_factory)>>(sys::state&, dcon::state_instance_id, std::function<void(upgraded_factory)>&&);	
}
