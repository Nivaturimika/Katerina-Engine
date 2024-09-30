
#include "province_templates.hpp"
#include "demographics.hpp"
#include "economy_rgo.hpp"
#include "economy.hpp"

namespace economy_rgo {
	float rgo_effective_size(sys::state const& state, dcon::nation_id n, dcon::province_id p, dcon::commodity_id c) {
		/*
		effective size = base size x (technology-bonus-to-specific-rgo-good-size
		+ technology-general-farm-or-mine-size-bonus + provincial-mine-or-farm-size-modifier + 1)
		*/
		bool is_mine = state.world.commodity_get_is_mine(c);
		auto rgo = state.world.province_get_rgo(p);
		if(rgo != c)
			return 0.f;
		auto sz = state.world.province_get_rgo_size(p);
		auto pmod = state.world.province_get_modifier_values(p, is_mine ? sys::provincial_mod_offsets::mine_rgo_size : sys::provincial_mod_offsets::farm_rgo_size);
		auto nmod = state.world.nation_get_modifier_values(n, is_mine ? sys::national_mod_offsets::mine_rgo_size : sys::national_mod_offsets::farm_rgo_size);
		auto specific_pmod = state.world.nation_get_rgo_size(n, c);
		auto bonus = std::max(0.001f, pmod + nmod + specific_pmod + 1.0f);
		return sz * bonus;
	}
	
	float rgo_total_effective_size(sys::state& state, dcon::nation_id n, dcon::province_id p) {
		float total = 0.f;
		state.world.for_each_commodity([&](dcon::commodity_id c) {
			total += rgo_effective_size(state, n, p, c);
		});
		return total;
	}
	
	float rgo_total_employment(sys::state& state, dcon::nation_id n, dcon::province_id p) {
		float total = 0.f;
		state.world.for_each_commodity([&](dcon::commodity_id c) {
			total += state.world.province_get_rgo_employment_per_good(p, c);
		});
		return total;
	}
	
	float rgo_max_employment(sys::state& state, dcon::nation_id n, dcon::province_id p, dcon::commodity_id c) {
		return rgo_effective_size(state, n, p, c);
	}
	
	float rgo_total_max_employment(sys::state& state, dcon::nation_id n, dcon::province_id p) {
		float total = 0.f;
		state.world.for_each_commodity([&](dcon::commodity_id c) {
			total += rgo_max_employment(state, n, p, c);
		});
		return total;
	}
	
	void update_rgo_employment(sys::state& state) {
		province::for_each_land_province(state, [&](dcon::province_id p) {
			auto owner = state.world.province_get_nation_from_province_ownership(p);
			if(!owner)
				return; //why bother? - pops are frozen
	
			auto current_employment = 0.f;
			state.world.for_each_commodity([&](dcon::commodity_id c) {
				current_employment += state.world.province_get_rgo_employment_per_good(p, c);
			});
	
			bool is_mine = state.world.commodity_get_is_mine(state.world.province_get_rgo(p));
			float worker_pool = 0.0f;
			for(auto wt : state.culture_definitions.rgo_workers) {
				worker_pool += state.world.province_get_demographics(p, demographics::to_key(state, wt));
			}
			float slave_pool = state.world.province_get_demographics(p, demographics::to_key(state, state.culture_definitions.slaves));
			float labor_pool = worker_pool + slave_pool;
	
			float total_population = state.world.province_get_demographics(p, demographics::total);
	
			//sorting goods by profitability
			static std::vector<dcon::commodity_id> ordered_rgo_goods;
			ordered_rgo_goods.clear();
			state.world.for_each_commodity([&](dcon::commodity_id c) {
				if(rgo_max_employment(state, owner, p, c) > 0.f) {
					ordered_rgo_goods.push_back(c);
				} else {
					state.world.province_set_rgo_employment_per_good(p, c, 0.f);
				}
			});
	
			// distributing workers in almost the same way as factories:
			constexpr float speed = 0.125f;
			float total_workforce = labor_pool;
			float max_employment_total = 0.f;
			float total_employed = 0.f;
			for(uint32_t i = 0; i < ordered_rgo_goods.size(); ++i) {
				auto c = ordered_rgo_goods[i];
				float max_employment = rgo_max_employment(state, owner, p, c);
				max_employment_total += max_employment;
				float current_workforce = state.world.province_get_rgo_employment_per_good(p, c);
				float new_employment = std::min(current_workforce * (1.f - speed) + max_employment * speed, total_workforce);
				//
				total_workforce -= new_employment;
				new_employment = std::clamp(new_employment, 0.f, max_employment);
				total_employed += new_employment;
				state.world.province_set_rgo_employment_per_good(p, c, new_employment);
			}
			assert(total_employed <= total_population + 1.f);
	
			float employment_ratio = 0.f;
			if(max_employment_total > 1.f) {
				employment_ratio = total_employed / (max_employment_total + 1.f);
			} else {
				employment_ratio = 1.f;
			}
			state.world.province_set_rgo_employment(p, employment_ratio);
	
			auto slave_fraction = (slave_pool > current_employment) ? current_employment / slave_pool : 0.0f;
			auto free_fraction = std::max(0.0f, (worker_pool > current_employment - slave_pool) ? (current_employment - slave_pool) / std::max(worker_pool, 0.01f) : 0.0f);
	
			assert(slave_fraction >= 0.f && slave_fraction <= 1.f);
			assert(free_fraction >= 0.f && free_fraction <= 1.f);
	
			assert(slave_fraction + free_fraction <= 1.f);
	
			for(auto pop : state.world.province_get_pop_location(p)) {
				auto pt = pop.get_pop().get_poptype();
				if(pt == state.culture_definitions.slaves) {
					pop.get_pop().set_employment(pop.get_pop().get_size() * slave_fraction);
				} else if(pt.get_is_paid_rgo_worker()) {
					pop.get_pop().set_employment(pop.get_pop().get_size() * free_fraction);
				}
			}
		});
	}
	float rgo_efficiency(sys::state& state, dcon::nation_id n, dcon::province_id p, dcon::commodity_id c) {
		bool is_mine = state.world.commodity_get_is_mine(c);
		auto nmod = state.world.nation_get_modifier_values(n, is_mine ? sys::national_mod_offsets::mine_rgo_eff : sys::national_mod_offsets::farm_rgo_eff);
		auto pmod = state.world.province_get_modifier_values(p, is_mine ? sys::provincial_mod_offsets::mine_rgo_eff : sys::provincial_mod_offsets::farm_rgo_eff);
		auto specific_pmod = state.world.nation_get_rgo_goods_output(n, c);
		return std::max(0.f, 1.0f + nmod + pmod + specific_pmod + state.defines.alice_base_rgo_efficiency_bonus);
	}

	float rgo_full_production_quantity(sys::state& state, dcon::nation_id n, dcon::province_id p, dcon::commodity_id c) {
		/*
		- We calculate its effective size which is its base size x (technology-bonus-to-specific-rgo-good-size +
		technology-general-farm-or-mine-size-bonus + provincial-mine-or-farm-size-modifier + 1)
		- We add its production to domestic supply, calculating that amount basically in the same way we do for factories, by
		computing RGO-throughput x RGO-output x RGO-size x base-commodity-production-quantity, except that it is affected by different
		modifiers.
		*/
		auto sz = rgo_effective_size(state, n, p, c);
		auto ef = rgo_efficiency(state, n, p, c);
		auto fp = dcon::fatten(state.world, p);
		float workforce = float(fp.get_rgo().get_rgo_workforce());
		auto vl = fp.get_rgo().get_rgo_amount();
		auto base = workforce == 0.f ? economy::default_workforce : std::ceil(sz / workforce);
		auto tp = rgo_total_employment(state, n, p) / rgo_max_employment(state, n, p, c);
		if(!std::isfinite(tp) || std::isnan(tp)) {
			tp = 0.0f;
		}
		return vl * tp * base * ef;
	}

	economy::rgo_workers_breakdown rgo_relevant_population(sys::state& state, dcon::province_id p, dcon::nation_id n) {
		auto relevant_paid_population = 0.f;
		for(auto wt : state.culture_definitions.rgo_workers) {
			relevant_paid_population += state.world.province_get_demographics(p, demographics::to_key(state, wt));
		}
		auto slaves = state.world.province_get_demographics(p, demographics::to_employment_key(state, state.culture_definitions.slaves));

		economy::rgo_workers_breakdown result = {
			.paid_workers = relevant_paid_population,
			.slaves = slaves,
			.total = relevant_paid_population + slaves
		};

		return result;
	}

	float rgo_desired_worker_norm_profit(sys::state& state, dcon::province_id p, dcon::nation_id n, float min_wage, float total_relevant_population) {
		auto pops_max = rgo_total_max_employment(state, n, p); // maximal amount of workers which rgo could potentially employ

		//we assume a "perfect ratio" of 1 aristo per N pops
		float perfect_aristos_amount = total_relevant_population / 10000.f;
		float aristos_desired_cut = perfect_aristos_amount * (
		state.world.nation_get_everyday_needs_costs(n, state.culture_definitions.aristocrat)
		+ state.world.nation_get_life_needs_costs(n, state.culture_definitions.aristocrat)
		);
		float aristo_burden_per_worker = aristos_desired_cut / (total_relevant_population + 1);

		bool is_mine = state.world.commodity_get_is_mine(state.world.province_get_rgo(p));

		dcon::pop_type_id pop_type = is_mine ? state.culture_definitions.laborers : state.culture_definitions.farmers;

		auto ln_costs = state.world.nation_get_life_needs_costs(n, pop_type);
		auto en_costs = state.world.nation_get_everyday_needs_costs(n, pop_type);
		auto lx_costs = state.world.nation_get_luxury_needs_costs(n, pop_type);

		float min_wage_burden_per_worker = min_wage;

		float desired_profit_by_worker = aristo_burden_per_worker + min_wage_burden_per_worker / (1.f - economy::rgo_owners_cut);

		// we want to employ at least someone, so we decrease our desired profits when employment is low.
		// aristocracy would prefer to gain less money instead of suffering constant revolts
		// not exactly an ideal solution but it works and doesn't create goods or wealth out of thin air
		float employment_ratio = state.world.province_get_rgo_employment(p);
		desired_profit_by_worker = desired_profit_by_worker * employment_ratio * employment_ratio;
		assert(std::isfinite(desired_profit_by_worker));
		return desired_profit_by_worker;
	}

	void update_province_rgo_consumption(sys::state& state, dcon::province_id p, dcon::nation_id n, float mobilization_impact, float expected_min_wage, bool occupied) {
		state.world.for_each_commodity([&](dcon::commodity_id c) {
			auto max_production = rgo_full_production_quantity(state, n, p, c);
			state.world.province_set_rgo_actual_production_per_good(p, c, max_production);
		});
	}

	void update_province_rgo_production(sys::state& state, dcon::province_id p, dcon::nation_id n) {
		state.world.province_set_rgo_full_profit(p, 0.f);
		state.world.for_each_commodity([&](dcon::commodity_id c) {
			auto amount = state.world.province_get_rgo_actual_production_per_good(p, c);
			economy::register_domestic_supply(state, n, c, amount);

			float profit = amount * state.world.commodity_get_current_price(c);
			assert(std::isfinite(profit) && profit >= 0.f);

			state.world.province_set_rgo_profit_per_good(p, c, profit);
			state.world.province_get_rgo_full_profit(p) += profit;
			if(state.world.commodity_get_money_rgo(c)) {
				assert(std::isfinite(amount * state.defines.gold_to_cash_rate) && amount * state.defines.gold_to_cash_rate >= 0.0f);
				state.world.nation_get_stockpiles(n, economy::money) += amount * state.defines.gold_to_cash_rate;
			}
		});
	}
}


