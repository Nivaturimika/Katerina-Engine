#include "system_state.hpp"
#include "economy.hpp"
#include "demographics.hpp"
#include "dcon_generated.hpp"
#include "ai.hpp"
#include "prng.hpp"
#include "math_fns.hpp"
#include "nations_templates.hpp"
#include "province_templates.hpp"
#include "triggers.hpp"
#include "economy_factory.hpp"
#include "economy_rgo.hpp"
#include "pdqsort.h"
#include "news.hpp"
#include "economy_templates.hpp"

namespace economy {
	void register_demand(sys::state& state, dcon::nation_id n, dcon::commodity_id commodity_type, float amount) {
		state.world.nation_get_real_demand(n, commodity_type) += amount;
		assert(std::isfinite(state.world.nation_get_real_demand(n, commodity_type)));
	}

	void register_intermediate_demand(sys::state& state, dcon::nation_id n, dcon::commodity_id commodity_type, float amount) {
		register_demand(state, n, commodity_type, amount);
		state.world.nation_get_intermediate_demand(n, commodity_type) += amount;
	}

	// it's registered as a demand separately
	void register_construction_demand(sys::state& state, dcon::nation_id n, dcon::commodity_id commodity_type, float amount) {
		state.world.nation_get_construction_demand(n, commodity_type) += amount;
	}

	void register_domestic_supply(sys::state& state, dcon::nation_id n, dcon::commodity_id commodity_type, float amount) {
		state.world.nation_get_domestic_market_pool(n, commodity_type) += amount;
	}

	/*	A country cannot borrow if it is less than define:BANKRUPTCY_EXTERNAL_LOAN_YEARS since their last bankruptcy. */
	bool can_take_loans(sys::state& state, dcon::nation_id n) {
		if(!state.world.nation_get_is_debt_spending(n))
			return false;
		auto last_br = state.world.nation_get_bankrupt_until(n);
		if(last_br && state.current_date < last_br)
			return false;
		return true;
	}

	/*	There is an income cap to how much may be borrowed, namely:
		define:MAX_LOAN_CAP_FROM_BANKS x (national-modifier-to-max-loan-amount + 1) x national-tax-base. */
	float max_loan(sys::state& state, dcon::nation_id n) {
		auto mod = state.world.nation_get_modifier_values(n, sys::national_mod_offsets::max_loan_modifier) + 1.f;
		auto total_tax_base = state.world.nation_get_total_rich_income(n) + state.world.nation_get_total_middle_income(n) + state.world.nation_get_total_poor_income(n);
		return std::max(0.001f, state.defines.max_loan_cap_from_banks * mod * total_tax_base);
	}

	int32_t most_recent_price_record_index(sys::state& state) {
		return (state.current_date.value >> 4) % price_history_length;
	}
	int32_t previous_price_record_index(sys::state& state) {
		return ((state.current_date.value >> 4) + price_history_length - 1) % price_history_length;
	}
	
	float commodity_daily_production_amount(sys::state& state, dcon::commodity_id c) {
		return state.world.commodity_get_total_production(c);
	}

	float stockpile_commodity_daily_increase(sys::state& state, dcon::commodity_id c, dcon::nation_id n) {
		// TODO
		return 0.f;
	}

	float global_market_commodity_daily_increase(sys::state& state, dcon::commodity_id c) {
		// TODO
		return 0.f;
	}

	void initialize_artisan_distribution(sys::state& state) {
		state.world.nation_resize_artisan_distribution(state.world.commodity_size());
		state.world.nation_resize_artisan_actual_production(state.world.commodity_size());

		auto const csize = state.world.commodity_size();

		for(auto n : state.world.in_nation) {
			for(uint32_t i = 1; i < csize; ++i) {
				dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
				auto kf = state.world.commodity_get_key_factory(cid);
				if(state.world.commodity_get_artisan_output_amount(cid) > 0.0f && (state.world.commodity_get_is_available_from_start(cid) || (kf && state.world.nation_get_active_building(n, kf)))) {
					n.set_artisan_distribution(cid, 0.f);
				}
			}
		}
	}

	float pop_get_life_needs_weight(sys::state& state, dcon::nation_id n, dcon::commodity_id c) {
		auto kf = state.world.commodity_get_key_factory(c);
		if(state.world.commodity_get_is_life_need(c) && (state.world.commodity_get_is_available_from_start(c) || (kf && state.world.nation_get_active_building(n, kf)))) {
			return 1.f;
		}
		return 0.f;
	}

	float pop_get_everyday_needs_weight(sys::state& state, dcon::nation_id n, dcon::commodity_id c) {
		auto kf = state.world.commodity_get_key_factory(c);
		if(state.world.commodity_get_is_everyday_need(c) && (state.world.commodity_get_is_available_from_start(c) || (kf && state.world.nation_get_active_building(n, kf)))) {
			return 1.f;
		}
		return 0.f;
	}

	float pop_get_luxury_needs_weight(sys::state& state, dcon::nation_id n, dcon::commodity_id c) {
		auto kf = state.world.commodity_get_key_factory(c);
		if(state.world.commodity_get_is_luxury_need(c) && (state.world.commodity_get_is_available_from_start(c) || (kf && state.world.nation_get_active_building(n, kf)))) {
			return 1.f;
		}
		return 0.f;
	}

	void presimulate(sys::state& state) {
		// economic updates without construction
	}

	bool has_building(sys::state const& state, dcon::state_instance_id si, dcon::factory_type_id fac) {
		auto sdef = state.world.state_instance_get_definition(si);
		auto owner = state.world.state_instance_get_nation_from_state_ownership(si);
		for(auto p : state.world.state_definition_get_abstract_state_membership(sdef)) {
			if(p.get_province().get_nation_from_province_ownership() == owner) {
				for(auto b : p.get_province().get_factory_location()) {
					if(b.get_factory().get_building_type() == fac)
					return true;
				}
			}
		}
		return false;
	}

	bool is_bankrupt_debtor_to(sys::state& state, dcon::nation_id debt_holder, dcon::nation_id debtor) {
		return state.world.nation_get_is_bankrupt(debt_holder)
			&& state.world.unilateral_relationship_get_owns_debt_of(state.world.get_unilateral_relationship_by_unilateral_pair(debtor, debt_holder)) > 0.1f;
	}

	float base_artisan_profit(sys::state& state, dcon::nation_id n, dcon::commodity_id c) {
		auto const& inputs = state.world.commodity_get_artisan_inputs(c);
		float input_total = 0.0f;
		for(uint32_t i = 0; i < commodity_set::set_size; ++i) {
			if(inputs.commodity_type[i]) {
				input_total += inputs.commodity_amounts[i] * state.world.commodity_get_current_price(inputs.commodity_type[i]);
			} else {
				break;
			}
		}
		float output_total = state.world.commodity_get_artisan_output_amount(c) * state.world.commodity_get_current_price(c);
		float input_multiplier = std::max(0.1f, artisan_buff_factor + state.world.nation_get_modifier_values(n, sys::national_mod_offsets::artisan_input));
		float output_multiplier = std::max(0.1f, artisan_buff_factor + state.world.nation_get_modifier_values(n, sys::national_mod_offsets::artisan_output));
		return output_total * output_multiplier - input_multiplier * input_total;
	}

	float artisan_scale_limit(sys::state& state, dcon::nation_id n, dcon::commodity_id c) {
		float least = 1.0f;
		auto const& inputs = state.world.commodity_get_artisan_inputs(c);
		float input_total = 0.0f;
		for(uint32_t i = 0; i < economy::commodity_set::set_size; ++i) {
			if(inputs.commodity_type[i]) {
				least = std::min(least, state.world.nation_get_demand_satisfaction(n, inputs.commodity_type[i]));
			} else {
				break;
			}
		}
		return least;
	}

	//crude approximation of exp
	float pseudo_exp_for_negative(float f) {
		if(f < -128.f) {
			return 0.f;
		}
		f = f / 128.f;
		f = 1 + f + f * f / 2 + f * f * f / 6;
		f = f * f; // 2
		f = f * f; // 4
		f = f * f; // 8
		f = f * f; // 16
		f = f * f; // 32
		f = f * f; // 64
		f = f * f; // 128
		return f;
	}

	float get_artisans_multiplier(sys::state& state, dcon::nation_id n) {
		float multiplier = 0.000001f * state.world.nation_get_everyday_needs_costs(n, state.culture_definitions.artisans);
		return 1.f / (multiplier + 1.f);
	}

	float max_artisan_score(sys::state& state, dcon::nation_id n, float multiplier) {
		auto const csize = state.world.commodity_size();
		float baseline = artisan_baseline_score / multiplier;
		float max_score = std::numeric_limits<float>::lowest();
		for(uint32_t i = 1; i < csize; ++i) {
		dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			float score = state.world.nation_get_artisan_distribution(n, cid);
			if(score > max_score) {
				max_score = score;
			}
		}
		if(baseline > max_score) {
			max_score = baseline;
		}
		return max_score;
	}

	float total_artisan_exp_score(sys::state& state, dcon::nation_id n, float multiplier, float max_score) {
		auto const csize = state.world.commodity_size();
		float total = 0.f;
		float baseline = artisan_baseline_score / multiplier;
		// crude approximation of softmax
		for(uint32_t i = 1; i < csize; ++i) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			float score = state.world.nation_get_artisan_distribution(n, cid);
			float dist = pseudo_exp_for_negative((score - max_score) * multiplier);
			total += dist;
		}
		total += pseudo_exp_for_negative((baseline - max_score) * multiplier);
		return total;
	}

	float get_artisan_distribution_fast(sys::state& state, dcon::nation_id n, dcon::commodity_id c, float max_score, float total_score, float multiplier) {
		float score = state.world.nation_get_artisan_distribution(n, c);
		return pseudo_exp_for_negative((score - max_score) * multiplier) / (total_score + 0.001f);
	}

	float get_artisan_distribution_slow(sys::state& state, dcon::nation_id n, dcon::commodity_id c) {
		auto const csize = state.world.commodity_size();
		float multiplier = get_artisans_multiplier(state, n);
		float max_score = max_artisan_score(state, n, multiplier);
		float total_score = total_artisan_exp_score(state, n, multiplier, max_score);
		return get_artisan_distribution_fast(state, n, c, max_score, total_score, multiplier);
	}

	void adjust_artisan_balance(sys::state& state, dcon::nation_id n) {
		auto const csize = state.world.commodity_size();
		float distribution_drift_speed = 0.01f;

		std::vector<float> current_distribution;
		std::vector<float> profits;
		profits.resize(csize + 1);

		float mult = get_artisans_multiplier(state, n);

		for(uint32_t i = 1; i < csize; ++i) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			if(valid_artisan_good(state, n, cid)) {
				auto profit = base_artisan_profit(state, n, cid);
				//if(profit < 0.f) {
					//	profit = profit * 10000.f;
				//}
				profits[cid.index()] = profit;
			} else {
				profits[cid.index()] = -256.f / mult / distribution_drift_speed * 10.f;
			}
		}

		float multiplier = get_artisans_multiplier(state, n);
		float max_score = max_artisan_score(state, n, multiplier);
		float total_score = total_artisan_exp_score(state, n, multiplier, max_score);

		for(uint32_t i = 1; i < csize; ++i) {
		dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			auto& w = state.world.nation_get_artisan_distribution(n, cid);
			auto last_distribution = get_artisan_distribution_fast(state, n, cid, max_score, total_score, multiplier);
			auto output = state.world.commodity_get_artisan_output_amount(cid);
			auto next_score = w * 0.8f + distribution_drift_speed * profits[cid.index()] * (1 - last_distribution) / output;
			w = next_score;
		}
	}

	void initialize(sys::state& state) {
		initialize_artisan_distribution(state);

		state.world.for_each_commodity([&](dcon::commodity_id c) {
			auto fc = fatten(state.world, c);
			fc.set_current_price(fc.get_cost());
			fc.set_total_consumption(0.0f);
			fc.set_total_production(0.0f);
			fc.set_total_real_demand(0.0f);
			fc.set_last_total_production(0.0f);
			fc.set_last_total_real_demand(0.0f);
			for(uint32_t i = 0; i < price_history_length; ++i) {
				fc.set_price_record(i, fc.get_cost());
			}
		});

		state.world.for_each_pop([&](dcon::pop_id p) {
			auto fp = fatten(state.world, p);
			fp.set_life_needs_satisfaction(1.0f);
			fp.set_everyday_needs_satisfaction(0.75f);
			fp.set_luxury_needs_satisfaction(0.25f);
			fp.set_savings(fp.get_size() / 10000.f);
		});

		/* - Newly built factories (upgrading from 0) seem to have some special employment logic to get them up
			to 1000. ... Look I can only figure out so many details. */
		state.world.for_each_factory([&](dcon::factory_id f) {
			auto ff = fatten(state.world, f);
			ff.set_production_scale(1.f);
			ff.set_primary_employment(1.f);
			ff.set_secondary_employment(1.f);
		});

		province::for_each_land_province(state, [&](dcon::province_id p) {
			auto fp = dcon::fatten(state.world, p);
			bool is_mine = state.world.commodity_get_is_mine(state.world.province_get_rgo(p));
			float pop_amount = 0.0f;
			pop_amount += state.world.province_get_demographics(p, demographics::to_key(state, state.culture_definitions.slaves));
			for(auto pt : state.world.in_pop_type) {
				if(pt.get_is_paid_rgo_worker()) {
					pop_amount += state.world.province_get_demographics(p, demographics::to_key(state, pt));
				}
			}
			//floor(ceil(workforce/BaseWorkforce)/2)+ceil(workforce/BaseWorkforce)
			float workforce = float(fp.get_rgo().get_rgo_workforce());
			workforce = workforce == 0.f ? default_workforce : workforce;
			fp.set_rgo_size(std::max(0.001f, (std::floor(std::ceil(pop_amount / workforce) / 2.f) + std::ceil(pop_amount / workforce)) * workforce));
		});

		state.world.for_each_nation([&](dcon::nation_id n) {
			auto fn = dcon::fatten(state.world, n);
			fn.set_administrative_spending(int8_t(50));
			fn.set_military_spending(int8_t(50));
			fn.set_education_spending(int8_t(50));
			fn.set_social_spending(int8_t(50));
			fn.set_land_spending(int8_t(100));
			fn.set_naval_spending(int8_t(100));
			fn.set_construction_spending(int8_t(100));
			fn.set_overseas_spending(int8_t(100));
			fn.set_stockpiles(economy::money, std::max(fn.get_non_colonial_population() / 100.f, 3000.f));

			fn.set_poor_tax(int8_t(50));
			fn.set_middle_tax(int8_t(50));
			fn.set_rich_tax(int8_t(50));

			fn.set_spending_level(1.0f);

			state.world.for_each_commodity([&](dcon::commodity_id c) {
				state.world.nation_set_demand_satisfaction(n, c, 1.0f);
				// set domestic market pool
			});
		});

		economy_rgo::update_rgo_employment(state);
		economy_factory::update_factory_employment(state);

		populate_army_consumption(state);
		populate_navy_consumption(state);
		populate_construction_consumption(state);
		for(const auto gp : state.great_nations) {
			/* The banks of great powers start with (8 - rank) x 20 + 100 in them. */
			auto t = (state.defines.great_nations_count - state.world.nation_get_rank(gp.nation)) * 20.f + 100.f;
			state.world.nation_set_stockpiles(gp.nation, economy::money, t);
		}
	}

	/*	Share factor : If the nation is a civ and is a secondary power start with define : SECOND_RANK_BASE_SHARE_FACTOR, and
		otherwise start with define : CIV_BASE_SHARE_FACTOR.Also calculate the sphere owner's foreign investment in the nation as a
		fraction of the total foreign investment in the nation (I believe that this is treated as zero if there is no foreign
		investment at all). The share factor is (1 - base share factor) x sphere owner investment fraction + base share factor. For
		uncivs, the share factor is simply equal to define:UNCIV_BASE_SHARE_FACTOR (so 1, by default). If a nation isn't in a sphere,
		we let the share factor be 0 if it needs to be used in any other calculation. */
	float sphere_leader_share_factor(sys::state& state, dcon::nation_id sphere_leader, dcon::nation_id sphere_member) {
		if(state.world.nation_get_is_civilized(sphere_member)) {
			float base = state.world.nation_get_rank(sphere_member) <= state.defines.colonial_rank
			? state.defines.second_rank_base_share_factor
			: state.defines.civ_base_share_factor;
			auto const ul = state.world.get_unilateral_relationship_by_unilateral_pair(sphere_member, sphere_leader);
			float sl_investment = state.world.unilateral_relationship_get_foreign_investment(ul);
			float total_investment = nations::get_foreign_investment(state, sphere_member);
			float investment_fraction = total_investment > 0.0001f ? sl_investment / total_investment : 0.0f;
			return base + (1.0f - base) * investment_fraction;
		} else {
			return state.defines.unciv_base_share_factor;
		}
	}

	/*	- Each sphere member has its domestic x its - share - factor(see above) of its base supply and demand added to its
		sphere leader's domestic supply and demand (this does not affect global supply and demand) */
	void absorb_sphere_member_production(sys::state& state, dcon::nation_id n) {
		for(auto gp : state.world.nation_get_gp_relationship_as_great_power(n)) {
			if((gp.get_status() & nations::influence::level_mask) == nations::influence::level_in_sphere) {
				auto t = gp.get_influence_target();
				float share = sphere_leader_share_factor(state, n, t);
				state.world.for_each_commodity([&](dcon::commodity_id c) {
					state.world.nation_get_domestic_market_pool(n, c) += share * state.world.nation_get_domestic_market_pool(t, c);
				});
			}
		}
	}

	/* - Every nation in a sphere(after the above has been calculated for the entire sphere) has their effective domestic
	 supply set to (1 - its-share-factor) x original-domestic-supply + sphere-leader's-domestic supply */
	void give_sphere_leader_production(sys::state& state, dcon::nation_id n) {
		if(auto sl = state.world.nation_get_in_sphere_of(n); sl) {
			float share = sphere_leader_share_factor(state, sl, n);
			if(bool(state.defines.ke_allow_sphere_dup)) {
				state.world.for_each_commodity([&](dcon::commodity_id c) {
					auto local_supply = state.world.nation_get_domestic_market_pool(n, c);
					state.world.nation_set_domestic_market_pool(n, c, (1.0f - share) * local_supply);
				});
			} else {
				// Leader supply is added to sphereling, hence the sphere dup bug
				state.world.for_each_commodity([&](dcon::commodity_id c) {
					auto local_supply = state.world.nation_get_domestic_market_pool(n, c);
					auto leader_supply = state.world.nation_get_domestic_market_pool(sl, c);
					state.world.nation_set_domestic_market_pool(n, c, (1.0f - share) * local_supply + leader_supply);
				});
			}
		}
	}

	float effective_tariff_rate(sys::state& state, dcon::nation_id n) {
		auto t_total = nations::tariff_efficiency(state, n) * float(state.world.nation_get_tariffs(n)) / 100.0f;
		//assert(t_total >= 0.f);
		return t_total;
	}

	float global_market_price_multiplier(sys::state& state, dcon::nation_id n) {
		float i_mod = std::clamp(1.f + state.world.nation_get_modifier_values(n, sys::national_mod_offsets::import_cost), 0.01f, 1.f);
		return effective_tariff_rate(state, n) + i_mod;
	}

	void update_national_artisan_consumption(sys::state& state, dcon::nation_id n, float expected_min_wage, float mobilization_impact) {
		auto const csize = state.world.commodity_size();
		auto num_artisans = state.world.nation_get_demographics(n, demographics::to_key(state, state.culture_definitions.artisans));
		float total_profit = 0.0f;

		float multiplier = get_artisans_multiplier(state, n);
		float max_score = max_artisan_score(state, n, multiplier);
		float total_score = total_artisan_exp_score(state, n, multiplier, max_score);

		for(uint32_t i = 1; i < csize; ++i) {
		dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			state.world.nation_set_artisan_actual_production(n, cid, 0.0f);
			if(valid_artisan_good(state, n, cid)) {
				float input_total = 0.0f;
				auto const& inputs = state.world.commodity_get_artisan_inputs(cid);
				float min_available = 1.0f;
				for(uint32_t j = 0; j < commodity_set::set_size; ++j) {
					if(inputs.commodity_type[j]) {
						input_total += inputs.commodity_amounts[j] * commodity_effective_price(state, n, inputs.commodity_type[j]);
						min_available = std::min(min_available, state.world.nation_get_demand_satisfaction(n, inputs.commodity_type[j]));
					} else {
						break;
					}
				}

				float output_total = state.world.commodity_get_artisan_output_amount(cid) * state.world.commodity_get_current_price(cid);
				float input_multiplier = std::max(0.1f, artisan_buff_factor + state.world.nation_get_modifier_values(n, sys::national_mod_offsets::artisan_input));
				float throughput_multiplier = std::max(0.1f, artisan_buff_factor + state.world.nation_get_modifier_values(n, sys::national_mod_offsets::artisan_throughput));
				float output_multiplier = std::max(0.1f, artisan_buff_factor + state.world.nation_get_modifier_values(n, sys::national_mod_offsets::artisan_output));

				float distribution = get_artisan_distribution_fast(state, n, cid, max_score, total_score, multiplier);
				float max_production_scale = num_artisans * distribution / 1000.0f * std::max(0.0f, mobilization_impact);

				auto profitability_factor = (output_total * output_multiplier * throughput_multiplier * min_available - input_multiplier * input_total * throughput_multiplier * min_available) / (expected_min_wage * state.defines.ke_profitability_factor);
				bool profitable = (output_total * output_multiplier - input_multiplier * input_total) >= 0.0f;

				//if(profitability_factor <= -1.0f) {

				//} else {
					//profitability_factor = std::clamp(profitability_factor * 0.5f + 0.5f, 0.0f, 1.0f);
					for(uint32_t j = 0; j < commodity_set::set_size; ++j) {
						if(inputs.commodity_type[j]) {
							register_intermediate_demand(state, n, inputs.commodity_type[j], input_multiplier * throughput_multiplier * max_production_scale * inputs.commodity_amounts[j] * (0.1f + 0.9f * min_available));
						} else {
							break;
						}
					}
					state.world.nation_set_artisan_actual_production(n, cid, state.world.commodity_get_artisan_output_amount(cid) * throughput_multiplier * output_multiplier * max_production_scale * min_available);
					total_profit += std::max(0.0f, (output_total * output_multiplier - input_multiplier * input_total) * throughput_multiplier * max_production_scale * min_available);
				//}
			}
		}

		state.world.nation_set_artisan_profit(n, total_profit);
	}

	void update_national_artisan_production(sys::state& state, dcon::nation_id n) {
		auto const csize = state.world.commodity_size();
		auto num_artisans = state.world.nation_get_demographics(n, demographics::to_key(state, state.culture_definitions.artisans));
		float total_profit = 0.0f;

		for(uint32_t i = 1; i < csize; ++i) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			if(valid_artisan_good(state, n, cid)) {
				auto production = state.world.nation_get_artisan_actual_production(n, cid);
				if(production > 0.f) {
					auto const& inputs = state.world.commodity_get_artisan_inputs(cid);
					float min_input = 1.0f;
					for(uint32_t j = 0; j < commodity_set::set_size; ++j) {
						if(inputs.commodity_type[j]) {
							min_input = std::min(min_input, state.world.nation_get_demand_satisfaction(n, inputs.commodity_type[j]));
						} else {
							break;
						}
					}
					auto amount = min_input * production;
					state.world.nation_set_artisan_actual_production(n, cid, amount);
					register_domestic_supply(state, n, cid, amount);
				}
			}
		}
	}

	void populate_army_consumption(sys::state& state) {
		for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			state.world.execute_serial_over_nation([&](auto ids) { state.world.nation_set_army_demand(ids, cid, 0.0f); });
		}

		state.world.for_each_regiment([&](dcon::regiment_id r) {
			auto reg = fatten(state.world, r);
			auto type = state.world.regiment_get_type(r);
			auto owner = reg.get_army_from_army_membership().get_controller_from_army_control();
			if(owner && type) {
				float admin_eff = state.world.nation_get_administrative_efficiency(owner);
				float admin_cost_factor = 2.0f - admin_eff;
				auto o_sc_mod = std::max(0.01f, state.world.nation_get_modifier_values(owner, sys::national_mod_offsets::supply_consumption) + 1.0f);
				auto& supply_cost = state.military_definitions.unit_base_definitions[type].supply_cost;
				for(uint32_t i = 0; i < commodity_set::set_size; ++i) {
					if(supply_cost.commodity_type[i]) {
						state.world.nation_get_army_demand(owner, supply_cost.commodity_type[i]) +=
						supply_cost.commodity_amounts[i] * state.world.nation_get_unit_stats(owner, type).supply_consumption
						* o_sc_mod * admin_cost_factor;
					} else {
						break;
					}
				}
			}
		});
	}

	void populate_navy_consumption(sys::state& state) {
		for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			state.world.execute_serial_over_nation([&](auto ids) { state.world.nation_set_navy_demand(ids, cid, 0.0f); });
		}

		state.world.for_each_ship([&](dcon::ship_id r) {
			auto shp = fatten(state.world, r);
			auto type = state.world.ship_get_type(r);
			auto owner = shp.get_navy_from_navy_membership().get_controller_from_navy_control();
			if(owner && type) {
				float admin_eff = state.world.nation_get_administrative_efficiency(owner);
				float admin_cost_factor = 2.0f - admin_eff;
				auto o_sc_mod = std::max(0.01f, state.world.nation_get_modifier_values(owner, sys::national_mod_offsets::supply_consumption) + 1.0f);
				auto& supply_cost = state.military_definitions.unit_base_definitions[type].supply_cost;
				for(uint32_t i = 0; i < commodity_set::set_size; ++i) {
					if(supply_cost.commodity_type[i]) {
						state.world.nation_get_navy_demand(owner, supply_cost.commodity_type[i]) +=
							supply_cost.commodity_amounts[i] * state.world.nation_get_unit_stats(owner, type).supply_consumption
							* o_sc_mod * admin_cost_factor;
					} else {
						break;
					}
				}
			}
		});
	}

	// we want "cheaper per day"(= slower) construction at the start to avoid initial demand bomb
	// and "more expensive"(=faster) construction at late game

	void populate_construction_consumption(sys::state& state) {
		uint32_t total_commodities = state.world.commodity_size();
		for(uint32_t i = 1; i < total_commodities; ++i) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			state.world.execute_serial_over_nation([&](auto ids) { state.world.nation_set_construction_demand(ids, cid, 0.0f); });
		}

		for(auto lc : state.world.in_province_land_construction) {
			auto province = state.world.pop_get_province_from_pop_location(state.world.province_land_construction_get_pop(lc));
			auto owner = state.world.province_get_nation_from_province_ownership(province);

			float admin_eff = state.world.nation_get_administrative_efficiency(owner);
			float admin_cost_factor = 2.0f - admin_eff;

			if(owner && state.world.province_get_nation_from_province_control(province) == owner) {
				auto& base_cost = state.military_definitions.unit_base_definitions[state.world.province_land_construction_get_type(lc)].build_cost;
				auto& current_purchased = state.world.province_land_construction_get_purchased_goods(lc);
				float construction_time = float(state.military_definitions.unit_base_definitions[state.world.province_land_construction_get_type(lc)].build_time);
				for(uint32_t i = 0; i < commodity_set::set_size; ++i) {
					if(base_cost.commodity_type[i]) {
						if(current_purchased.commodity_amounts[i] < base_cost.commodity_amounts[i] * admin_cost_factor) {
							float amount = std::max(0.f, base_cost.commodity_amounts[i] * admin_cost_factor - current_purchased.commodity_amounts[i]);
							register_construction_demand(state, owner, base_cost.commodity_type[i], excess_construction_demand * amount / construction_time);
						}
					} else {
						break;
					}
				}
			}
		}

		province::for_each_land_province(state, [&](dcon::province_id p) {
			auto owner = state.world.province_get_nation_from_province_ownership(p);
			if(!owner || state.world.province_get_nation_from_province_control(p) != owner) {
				return;
			}
			auto rng = state.world.province_get_province_naval_construction(p);
			if(rng.begin() != rng.end()) {
				auto c = *(rng.begin());

				float admin_eff = state.world.nation_get_administrative_efficiency(owner);
				float admin_cost_factor = 2.0f - admin_eff;

				auto& base_cost = state.military_definitions.unit_base_definitions[c.get_type()].build_cost;
				auto& current_purchased = c.get_purchased_goods();
				float construction_time = float(state.military_definitions.unit_base_definitions[c.get_type()].build_time);
				for(uint32_t i = 0; i < commodity_set::set_size; ++i) {
					if(base_cost.commodity_type[i]) {
						if(current_purchased.commodity_amounts[i] < base_cost.commodity_amounts[i] * admin_cost_factor) {
							float amount = std::max(0.f, base_cost.commodity_amounts[i] * admin_cost_factor - current_purchased.commodity_amounts[i]);
							register_construction_demand(state, owner, base_cost.commodity_type[i], excess_construction_demand * amount / construction_time);
						}
					} else {
						break;
					}
				}
			}
		});
		for(auto c : state.world.in_province_building_construction) {
			auto owner = c.get_nation().id;
			if(owner && c.get_province().get_nation_from_province_ownership() == c.get_province().get_nation_from_province_control() && !c.get_is_pop_project()) {
				auto t = economy::province_building_type(c.get_type());
				float admin_eff = state.world.nation_get_administrative_efficiency(owner);
				float admin_cost_factor = 2.0f - admin_eff;
				auto& base_cost = state.economy_definitions.building_definitions[int32_t(t)].cost;
				auto& current_purchased = c.get_purchased_goods();
				float construction_time = float(state.economy_definitions.building_definitions[int32_t(t)].time);
				for(uint32_t i = 0; i < commodity_set::set_size; ++i) {
					if(base_cost.commodity_type[i]) {
						if(current_purchased.commodity_amounts[i] < base_cost.commodity_amounts[i] * admin_cost_factor) {
							float amount = std::max(0.f, base_cost.commodity_amounts[i] * admin_cost_factor - current_purchased.commodity_amounts[i]);
							register_construction_demand(state, owner, base_cost.commodity_type[i], excess_construction_demand * amount / construction_time);
						}
					} else {
						break;
					}
				}
			}
		}
		for(auto c : state.world.in_state_building_construction) {
			auto owner = c.get_nation().id;
			if(owner && !c.get_is_pop_project()) {
				auto& base_cost = c.get_type().get_construction_costs();
				auto& current_purchased = c.get_purchased_goods();
				float construction_time = float(c.get_type().get_construction_time());
				float cost_mod = economy_factory::factory_build_cost_modifier(state, c.get_nation(), c.get_is_pop_project());
				for(uint32_t i = 0; i < commodity_set::set_size; ++i) {
					if(base_cost.commodity_type[i]) {
						if(current_purchased.commodity_amounts[i] < base_cost.commodity_amounts[i] * cost_mod) {
							float amount = std::max(0.f, base_cost.commodity_amounts[i] * cost_mod - current_purchased.commodity_amounts[i]);
							register_construction_demand(state, owner, base_cost.commodity_type[i], excess_construction_demand * amount / construction_time);
						}
					} else {
						break;
					}
				}
			}
		}
	}

	void populate_private_construction_consumption(sys::state& state) {
		uint32_t total_commodities = state.world.commodity_size();
		for(uint32_t i = 1; i < total_commodities; ++i) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			state.world.execute_serial_over_nation([&](auto ids) { state.world.nation_set_private_construction_demand(ids, cid, 0.0f); });
		}

		for(auto c : state.world.in_province_building_construction) {
			auto owner = c.get_nation().id;
			// Rationale for not checking building type: Its an invalid state; should not occur under normal circumstances
			if(owner && owner == c.get_province().get_nation_from_province_control() && c.get_is_pop_project()) {
				auto t = economy::province_building_type(c.get_type());
				auto& base_cost = state.economy_definitions.building_definitions[int32_t(t)].cost;
				auto& current_purchased = c.get_purchased_goods();
				float construction_time = float(state.economy_definitions.building_definitions[int32_t(t)].time);
				for(uint32_t i = 0; i < commodity_set::set_size; ++i) {
					if(base_cost.commodity_type[i]) {
						if(current_purchased.commodity_amounts[i] < base_cost.commodity_amounts[i]) {
							state.world.nation_get_private_construction_demand(owner, base_cost.commodity_type[i]) += base_cost.commodity_amounts[i] / construction_time;
						}
					} else {
						break;
					}
				}
			}
		}
		for(auto c : state.world.in_state_building_construction) {
			auto owner = c.get_nation().id;
			if(owner && c.get_is_pop_project()) {
				auto& base_cost = c.get_type().get_construction_costs();
				auto& current_purchased = c.get_purchased_goods();
				float construction_time = float(c.get_type().get_construction_time());
				float cost_mod = economy_factory::factory_build_cost_modifier(state, c.get_nation(), c.get_is_pop_project());
				for(uint32_t i = 0; i < commodity_set::set_size; ++i) {
					if(base_cost.commodity_type[i]) {
						if(current_purchased.commodity_amounts[i] < base_cost.commodity_amounts[i] * cost_mod) {
							auto amount = base_cost.commodity_amounts[i] * cost_mod;
							state.world.nation_get_private_construction_demand(owner, base_cost.commodity_type[i]) += amount / construction_time;
						}
					} else {
						break;
					}
				}
			}
		}
	}

	float full_pop_spending_cost(sys::state& state, dcon::nation_id n) {
		// direct payments to pops
		auto const a_spending = float(state.world.nation_get_administrative_spending(n)) / 100.0f;
		auto const s_spending = state.world.nation_get_administrative_efficiency(n) * float(state.world.nation_get_social_spending(n)) / 100.0f;
		auto const e_spending = float(state.world.nation_get_education_spending(n)) / 100.0f;
		auto const m_spending = float(state.world.nation_get_military_spending(n)) / 100.0f;
		auto const p_level = state.world.nation_get_modifier_values(n, sys::national_mod_offsets::pension_level);
		assert(p_level >= 0.f);
		auto const unemp_level = state.world.nation_get_modifier_values(n, sys::national_mod_offsets::unemployment_benefit);
		assert(unemp_level >= 0.f);
		auto const di_spending = float(state.world.nation_get_domestic_investment_spending(n)) / 100.0f;
		float total = di_spending *
			(state.world.nation_get_demographics(n, demographics::to_key(state, state.culture_definitions.capitalists))
			* state.world.nation_get_luxury_needs_costs(n, state.culture_definitions.capitalists)
			+ state.world.nation_get_demographics(n, demographics::to_key(state, state.culture_definitions.aristocrat))
			* state.world.nation_get_luxury_needs_costs(n, state.culture_definitions.aristocrat));
		assert(std::isfinite(total) && total >= 0.0f);
		state.world.for_each_pop_type([&](dcon::pop_type_id pt) {
			auto adj_pop_of_type = state.world.nation_get_demographics(n, demographics::to_key(state, pt));
			if(adj_pop_of_type <= 0)
				return;
			assert(std::isfinite(adj_pop_of_type) && adj_pop_of_type >= 0.0f);
			//
			auto ln_type = culture::income_type(state.world.pop_type_get_life_needs_income_type(pt));
			if(ln_type == culture::income_type::administration) {
				total += a_spending * adj_pop_of_type * state.world.nation_get_life_needs_costs(n, pt);
			} else if(ln_type == culture::income_type::education) {
				total += e_spending * adj_pop_of_type * state.world.nation_get_life_needs_costs(n, pt);
			} else if(ln_type == culture::income_type::military) {
				total += m_spending * adj_pop_of_type * state.world.nation_get_life_needs_costs(n, pt);
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
			auto en_type = culture::income_type(state.world.pop_type_get_everyday_needs_income_type(pt));
			if(en_type == culture::income_type::administration) {
				total += a_spending * adj_pop_of_type * state.world.nation_get_everyday_needs_costs(n, pt);
			} else if(en_type == culture::income_type::education) {
				total += e_spending * adj_pop_of_type * state.world.nation_get_everyday_needs_costs(n, pt);
			} else if(en_type == culture::income_type::military) {
				total += m_spending * adj_pop_of_type * state.world.nation_get_everyday_needs_costs(n, pt);
			}
			auto lx_type = culture::income_type(state.world.pop_type_get_luxury_needs_income_type(pt));
			if(lx_type == culture::income_type::administration) {
				total += a_spending * adj_pop_of_type * state.world.nation_get_luxury_needs_costs(n, pt);
			} else if(lx_type == culture::income_type::education) {
				total += e_spending * adj_pop_of_type * state.world.nation_get_luxury_needs_costs(n, pt);
			} else if(lx_type == culture::income_type::military) {
				total += m_spending * adj_pop_of_type * state.world.nation_get_luxury_needs_costs(n, pt);
			}
			assert(std::isfinite(total) && total >= 0.0f);
		});
		total *= pop_payout_factor;
		return total;
	}

	float military_spending_cost(sys::state& state, dcon::nation_id n) {
		float total = 0.0f;
		float l_spending = float(state.world.nation_get_land_spending(n)) / 100.0f;
		float n_spending = float(state.world.nation_get_naval_spending(n)) / 100.0f;
		for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			auto v = state.world.nation_get_army_demand(n, cid) * l_spending * state.world.commodity_get_current_price(cid);
			assert(std::isfinite(v) && v >= 0.0f);
			total += v;
		}
		for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			auto v = state.world.nation_get_navy_demand(n, cid) * n_spending * state.world.commodity_get_current_price(cid);
			assert(std::isfinite(v) && v >= 0.0f);
			total += v;
		}
		assert(std::isfinite(total) && total >= 0.0f);
		return total;
	}

	float full_spending_cost(sys::state& state, dcon::nation_id n) {
		float total = 0.0f;
		float c_spending = float(state.world.nation_get_construction_spending(n)) / 100.0f;
		float o_spending = float(state.world.nation_get_overseas_spending(n)) / 100.f;
		for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			auto v = state.world.nation_get_construction_demand(n, cid) * c_spending * state.world.commodity_get_current_price(cid);
			assert(std::isfinite(v) && v >= 0.0f);
			total += v * true_construction_demand;
		}
		for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			auto difference = state.world.nation_get_stockpile_targets(n, cid) - state.world.nation_get_stockpiles(n, cid);
			if(difference > 0.f && state.world.nation_get_drawing_on_stockpiles(n, cid) == false) {
				auto v = difference * state.world.commodity_get_current_price(cid);
				assert(std::isfinite(v) && v >= 0.0f);
				total += v;
			}
		}
		assert(std::isfinite(total) && total >= 0.0f);
		auto overseas_factor = state.defines.province_overseas_penalty * float(state.world.nation_get_owned_province_count(n) - state.world.nation_get_central_province_count(n));
		if(overseas_factor > 0.f) {
			for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
				dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
				auto kf = state.world.commodity_get_key_factory(cid);
				if(state.world.commodity_get_overseas_penalty(cid) && (state.world.commodity_get_is_available_from_start(cid) || (kf && state.world.nation_get_active_building(n, kf)))) {
					auto v = overseas_factor * state.world.commodity_get_current_price(cid) * o_spending;
					assert(std::isfinite(v) && v >= 0.0f);
					total += v;
				}
			}
		}
		assert(std::isfinite(total) && total >= 0.0f);
		return total;
	}
	
	float full_private_investment_cost(sys::state& state, dcon::nation_id n) {
		float total = 0.0f;
		uint32_t total_commodities = state.world.commodity_size();
		for(uint32_t i = 1; i < total_commodities; ++i) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			total += state.world.nation_get_private_construction_demand(n, cid) * state.world.commodity_get_current_price(cid);
		}
		return total;
	}

	void update_national_consumption(sys::state& state, dcon::nation_id n, float spending_scale, float private_investment_scale) {
		uint32_t total_commodities = state.world.commodity_size();
		float l_spending = float(state.world.nation_get_land_spending(n)) / 100.0f;
		float n_spending = float(state.world.nation_get_naval_spending(n)) / 100.0f;
		float c_spending = float(state.world.nation_get_construction_spending(n)) / 100.0f;
		float o_spending = float(state.world.nation_get_overseas_spending(n)) / 100.0f;
		for(uint32_t i = 1; i < total_commodities; ++i) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			register_demand(state, n, cid, state.world.nation_get_army_demand(n, cid) * l_spending * spending_scale);
		}
		for(uint32_t i = 1; i < total_commodities; ++i) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			register_demand(state, n, cid, state.world.nation_get_navy_demand(n, cid) * n_spending * spending_scale);
		}
		for(uint32_t i = 1; i < total_commodities; ++i) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			register_demand(state, n, cid, state.world.nation_get_construction_demand(n, cid) * c_spending * spending_scale);
		}
		for(uint32_t i = 1; i < total_commodities; ++i) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			register_demand(state, n, cid, state.world.nation_get_private_construction_demand(n, cid) * private_investment_scale);
		}
		for(uint32_t i = 1; i < total_commodities; ++i) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			auto difference = state.world.nation_get_stockpile_targets(n, cid) - state.world.nation_get_stockpiles(n, cid);
			if(difference > 0 && state.world.nation_get_drawing_on_stockpiles(n, cid) == false) {
				register_demand(state, n, cid, difference * spending_scale);
			}
		}
		auto overseas_factor = state.defines.province_overseas_penalty * float(state.world.nation_get_owned_province_count(n) - state.world.nation_get_central_province_count(n));
		if(overseas_factor > 0.f) {
			for(uint32_t i = 1; i < total_commodities; ++i) {
				dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
				auto kf = state.world.commodity_get_key_factory(cid);
				if(state.world.commodity_get_overseas_penalty(cid) && (state.world.commodity_get_is_available_from_start(cid) || (kf && state.world.nation_get_active_building(n, kf)))) {
					register_demand(state, n, cid, overseas_factor * spending_scale * o_spending);
				}
			}
		}
	}

	void update_pop_consumption(sys::state& state, dcon::nation_id n, float base_demand, float invention_factor) {
		uint32_t total_commodities = state.world.commodity_size();

		auto nation_rules = state.world.nation_get_combined_issue_rules(n);
		bool nation_allows_investment = state.world.nation_get_is_civilized(n) && (nation_rules & (issue_rule::pop_build_factory | issue_rule::pop_expand_factory)) != 0;
		for(auto p : state.world.nation_get_province_ownership(n)) {
			for(auto pl : state.world.province_get_pop_location(p.get_province())) {
				auto t = pl.get_pop().get_poptype();
				assert(t);
				auto total_budget = pl.get_pop().get_savings();
				assert(total_budget >= 0.f);
				auto total_pop = pl.get_pop().get_size();
				/*
				We calculate an adjusted pop-size as (0.5 + pop-consciousness / define:PDEF_BASE_CON) x (for non-colonial pops: 1 +
				national-plurality (as a fraction of 100)) x pop-size
				*/
				float nmod = (0.5f + pl.get_pop().get_consciousness() / state.defines.pdef_base_con)
				* (p.get_province().get_is_colonial() ? 1.f : 1.f + state.world.nation_get_plurality(n) * 0.01f);
				float ln_cost = nmod * state.world.nation_get_life_needs_costs(n, t) * total_pop;
				float en_cost = nmod * state.world.nation_get_everyday_needs_costs(n, t) * total_pop;
				float xn_cost = nmod * state.world.nation_get_luxury_needs_costs(n, t) * total_pop;
				float life_needs_budget = std::min(ln_cost, total_budget);
				total_budget -= std::min(total_budget, ln_cost);
				float everyday_needs_budget = std::min(en_cost, total_budget);
				total_budget -= std::min(total_budget, en_cost);
				float luxury_needs_budget = std::min(xn_cost, total_budget);
				total_budget -= std::min(total_budget, xn_cost);
				/* handle investment before everyday goods - they could be very hard to satisfy, depending on a mod : */
				if(!nation_allows_investment || (t != state.culture_definitions.aristocrat && t != state.culture_definitions.capitalists)) {

				} else if(t == state.culture_definitions.capitalists) {
					state.world.nation_get_private_investment(n) += total_budget * capitalist_investment_ratio;
					total_budget -= total_budget * capitalist_investment_ratio;
				} else {
					state.world.nation_get_private_investment(n) += total_budget * aristocrat_investment_ratio;
					total_budget -= total_budget * aristocrat_investment_ratio;
				}

				/* Induce demand across all categories maybe we need some kind of banking and ability to save up money for future instead of spending them all */
			
				float life_needs_fraction = life_needs_budget / std::max(0.001f, ln_cost);
				float everyday_needs_fraction = everyday_needs_budget / std::max(0.001f, en_cost);
				float luxury_needs_fraction = luxury_needs_budget / std::max(0.001f, xn_cost);

				float old_life = pl.get_pop().get_life_needs_satisfaction();
				float old_everyday = pl.get_pop().get_everyday_needs_satisfaction();
				float old_luxury = pl.get_pop().get_luxury_needs_satisfaction();

				auto result_life = std::clamp(old_life * 0.9f + life_needs_fraction * 0.9f, 0.f, 1.f);
				auto result_everyday = std::clamp(old_everyday * 0.9f + everyday_needs_fraction * 0.9f, 0.f, 1.f);
				auto result_luxury = std::clamp(old_luxury * 0.9f + luxury_needs_fraction * 0.9f, 0.f, 1.f);

				state.world.pop_set_life_needs_satisfaction(pl.get_pop(), result_life);
				state.world.pop_set_everyday_needs_satisfaction(pl.get_pop(), result_everyday);
				state.world.pop_set_luxury_needs_satisfaction(pl.get_pop(), result_luxury);
			}
		}

		float ln_mul[] = {
			std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::poor_life_needs) + 1.0f),
			std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::middle_life_needs) + 1.0f),
			std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::rich_life_needs) + 1.0f)
		};
		float en_mul[] = {
			std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::poor_everyday_needs) + 1.0f),
			std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::middle_everyday_needs) + 1.0f),
			std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::rich_everyday_needs) + 1.0f)
		};
		float lx_mul[] = {
			std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::poor_luxury_needs) + 1.0f),
			std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::middle_luxury_needs) + 1.0f),
			std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::rich_luxury_needs) + 1.0f)
		};

		for(uint32_t i = 1; i < total_commodities; ++i) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(i) };
			auto kf = state.world.commodity_get_key_factory(cid);
			if(state.world.commodity_get_is_available_from_start(cid)
			|| (kf && state.world.nation_get_active_building(n, kf))) {
				for(const auto t : state.world.in_pop_type) {
					auto strata = state.world.pop_type_get_strata(t);
					float base_life = state.world.pop_type_get_life_needs(t, cid);
					float base_everyday = state.world.pop_type_get_everyday_needs(t, cid);
					float base_luxury = state.world.pop_type_get_luxury_needs(t, cid);
					float pop_size = state.world.nation_get_demographics(n, demographics::to_key(state, t));

					/* Invention factor doesn't factor for life needs */
					float demand_life = base_life * pop_size * ln_mul[strata];
					float demand_everyday = base_everyday * pop_size * invention_factor * en_mul[strata];
					float demand_luxury = base_luxury * pop_size * invention_factor * lx_mul[strata];
					register_demand(state, n, cid, demand_life);
					register_demand(state, n, cid, demand_everyday);
					register_demand(state, n, cid, demand_luxury);
				}
			}
		}
	}

	void limit_pop_demand_to_production(sys::state& state) {
		// Not a long operation -- doesn't warrant parallel (or thread dispatch)
		state.world.execute_serial_over_commodity([&](auto ids) {
			// Properties like "is_life_needs, is_everyday_needs, etc" from commodity
			// are already implicitly present, by the mere fact that pop types
			// would accumulate to >0 when queried for base_life/ev/lux, etc.
			// No need for filtering here.
			//
			// Accumulators:
			// a += f(x) > 0 ? 1 : 0
			// a += ceil(f(x)), ceil(x) > 0 ? 1 : 0
			// TODO: add operator+= for vector_fp
			ve::fp_vector base_life{};
			ve::fp_vector base_everyday{};
			ve::fp_vector base_luxury{};
			ve::apply([&](dcon::commodity_id c) {
				state.world.execute_serial_over_pop_type([&](auto pids) {
					base_life = base_life
						+ ve::min(1.f, ve::ceil(state.world.pop_type_get_life_needs(pids, c)));
					base_everyday = base_everyday
						+ ve::min(1.f, ve::ceil(state.world.pop_type_get_everyday_needs(pids, c)));
					base_luxury = base_luxury
						+ ve::min(1.f, ve::ceil(state.world.pop_type_get_luxury_needs(pids, c)));
				});
			}, ids);
			//
			const ve::fp_vector lf_factor(1.0f);
			const ve::fp_vector ev_factor(2.0f);
			const ve::fp_vector lx_factor(4.5f);
			//
			// Ratio of "weighted" needs versus base weights (without weights)
			auto ratio =
				(base_life * lf_factor + base_everyday * ev_factor + base_luxury * lx_factor)
				/ ve::max(base_life + base_everyday + base_luxury, 1.0f);
			//
			// Conditionally selected values for choosing limits (of supply and demand, respectively)
			ve::fp_vector sel_a = ratio * ve::select(ratio < 2.0f, ve::fp_vector(0.125f), 0.4f);
			ve::fp_vector sel_b = ve::select(ratio >= 2.5f, ve::fp_vector(1.0f), 0.0f);
			//
			auto total_r_demand = state.world.commodity_get_total_real_demand(ids);
			auto last_t_production = state.world.commodity_get_last_total_production(ids);
			//
			auto avg_sup = last_t_production * ve::max(1.0f - sel_a, 0.0f);
			auto avg_dem = total_r_demand * ve::max(((5.5f - ratio) * 1.5f), 0.0f) * sel_b;
			auto new_limit = ve::min(ve::max(avg_sup + avg_dem, 1.f) / ve::max(total_r_demand, 1.f), 1.f);
			// Only write if filter was passed
			ve::mask_vector is_pop_need = state.world.commodity_get_is_life_need(ids)
				|| state.world.commodity_get_is_everyday_need(ids)
				|| state.world.commodity_get_is_luxury_need(ids);
			ve::apply([&](dcon::commodity_id c, bool passed_filter, float l) {
				if(passed_filter) {
					state.world.commodity_get_total_real_demand(c) *= l;
					ve::fp_vector local_l(l);
					state.world.execute_serial_over_nation([&](auto nids) {
						state.world.nation_get_real_demand(nids, c) =
							state.world.nation_get_real_demand(nids, c) * local_l;
					});
				}
			}, ids, is_pop_need, new_limit);
		});
	}

	/*	- Each pop strata and needs type has its own demand modifier, calculated as follows:
		- (national-modifier-to-goods-demand + define:BASE_GOODS_DEMAND) x (national-modifier-to-specific-strata-and-needs-type + 1) x
		(define:INVENTION_IMPACT_ON_DEMAND x number-of-unlocked-inventions + 1, but for non-life-needs only)
		- Each needs demand is also multiplied by  2 - the nation's administrative efficiency if the pop has education / admin /
		military income for that need category
		- We calculate an adjusted pop-size as (0.5 + pop-consciousness / define:PDEF_BASE_CON) x (for non-colonial pops: 1 +
		national-plurality (as a fraction of 100)) x pop-size */
	void populate_needs_costs(sys::state& state, dcon::nation_id n, float base_demand, float invention_factor) {
		float ln_mul[] = {
			std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::poor_life_needs) + 1.0f),
			std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::middle_life_needs) + 1.0f),
			std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::rich_life_needs) + 1.0f)
		};
		float en_mul[] = {
			std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::poor_everyday_needs) + 1.0f),
			std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::middle_everyday_needs) + 1.0f),
			std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::rich_everyday_needs) + 1.0f)
		};
		float lx_mul[] = {
			std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::poor_luxury_needs) + 1.0f),
			std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::middle_luxury_needs) + 1.0f),
			std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::rich_luxury_needs) + 1.0f)
		};
		float admin_eff = state.world.nation_get_administrative_efficiency(n);
		float admin_cost_factor = 2.0f - admin_eff;
		for(const auto ids : state.world.in_pop_type) {
			float ln_total = 0.f;
			float en_total = 0.f;
			float lx_total = 0.f;
			for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
				dcon::commodity_id c{ dcon::commodity_id::value_base_t(i) };
				auto kf = state.world.commodity_get_key_factory(c);
				if(state.world.commodity_get_is_available_from_start(c) || (kf && state.world.nation_get_active_building(n, kf))) {
					float effective_price = commodity_effective_price(state, n, c);
					assert(effective_price >= 0.f);
					auto nmod = admin_cost_factor * effective_price * base_demand;
					auto strata = state.world.pop_type_get_strata(ids);
					ln_total += state.world.pop_type_get_life_needs(ids, c) * nmod * ln_mul[strata];
					en_total += state.world.pop_type_get_everyday_needs(ids, c) * nmod * invention_factor * en_mul[strata];
					lx_total += state.world.pop_type_get_luxury_needs(ids, c) * nmod * invention_factor * lx_mul[strata];
				}
			}
			state.world.nation_set_life_needs_costs(n, ids, ln_total);
			state.world.nation_set_everyday_needs_costs(n, ids, en_total);
			state.world.nation_set_luxury_needs_costs(n, ids, lx_total);
			assert(std::isfinite(state.world.nation_get_life_needs_costs(n, ids)) && state.world.nation_get_life_needs_costs(n, ids) >= 0.f);
			assert(std::isfinite(state.world.nation_get_everyday_needs_costs(n, ids)) && state.world.nation_get_everyday_needs_costs(n, ids) >= 0.f);
			assert(std::isfinite(state.world.nation_get_luxury_needs_costs(n, ids)) && state.world.nation_get_luxury_needs_costs(n, ids) >= 0.f);
		}
	}

	void advance_construction(sys::state& state, dcon::nation_id n) {
		uint32_t total_commodities = state.world.commodity_size();

		float c_spending = state.world.nation_get_spending_level(n) * float(state.world.nation_get_construction_spending(n)) / 100.0f;
		float p_spending = state.world.nation_get_private_investment_effective_fraction(n);

		// Calculate refunds of extra goods bought (that werent needed
		float refund_amount = 0.0f;
		for(uint32_t i = 1; i < total_commodities; ++i) {
			dcon::commodity_id c{ dcon::commodity_id::value_base_t(i) };
			auto d_sat = state.world.nation_get_demand_satisfaction(n, c);
			auto& nat_demand = state.world.nation_get_construction_demand(n, c);
			assert(d_sat >= 0.f && d_sat <= 1.f);
			refund_amount += nat_demand * c_spending * (1.0f - d_sat) * state.world.commodity_get_current_price(c);
			nat_demand *= c_spending * d_sat;
			state.world.nation_get_private_construction_demand(n, c) *= p_spending * d_sat;
		}
		state.world.nation_get_stockpiles(n, economy::money) += refund_amount;

		//Replicate demand property into a temporal buffer (as to not modify the principal demand)
		ve::vectorizable_buffer<float, dcon::commodity_id> gbuf(state.world.commodity_size());
		ve::vectorizable_buffer<float, dcon::commodity_id> pbuf(state.world.commodity_size());
		for(const auto c : state.world.in_commodity) {
			gbuf.set(c, state.world.nation_get_construction_demand(n, c));
			pbuf.set(c, state.world.nation_get_private_construction_demand(n, c));
		}

		float admin_eff = state.world.nation_get_administrative_efficiency(n);
		float admin_cost_factor = 2.0f - admin_eff;
		for(auto p : state.world.nation_get_province_ownership(n)) {
			if(p.get_province().get_nation_from_province_control() != n)
				continue;

			for(auto pops : p.get_province().get_pop_location()) {
				auto rng = pops.get_pop().get_province_land_construction();
				if(rng.begin() != rng.end()) {
					auto c = *(rng.begin());
					auto& base_cost = state.military_definitions.unit_base_definitions[c.get_type()].build_cost;
					auto& current_purchased = c.get_purchased_goods();
					float construction_time = float(state.military_definitions.unit_base_definitions[c.get_type()].build_time);
					for(uint32_t i = 0; i < commodity_set::set_size; ++i) {
						if(base_cost.commodity_type[i]) {
							if(current_purchased.commodity_amounts[i] < base_cost.commodity_amounts[i] * admin_cost_factor) {
								auto& source = gbuf.get(base_cost.commodity_type[i]);
								auto delta = std::clamp(base_cost.commodity_amounts[i] * admin_cost_factor / construction_time, 0.f, source);
								current_purchased.commodity_amounts[i] += delta;
								source -= delta;
							}
						} else {
							break;
						}
					}
					break; // only advance one construction per province
				}
			}
			{
				auto rng = p.get_province().get_province_naval_construction();
				if(rng.begin() != rng.end()) {
					auto c = *(rng.begin());
					auto& base_cost = state.military_definitions.unit_base_definitions[c.get_type()].build_cost;
					auto& current_purchased = c.get_purchased_goods();
					float construction_time = float(state.military_definitions.unit_base_definitions[c.get_type()].build_time);
					for(uint32_t i = 0; i < commodity_set::set_size; ++i) {
						if(base_cost.commodity_type[i]) {
							if(current_purchased.commodity_amounts[i] < base_cost.commodity_amounts[i] * admin_cost_factor) {
								auto& source = gbuf.get(base_cost.commodity_type[i]);
								auto delta = std::clamp(base_cost.commodity_amounts[i] * admin_cost_factor / construction_time, 0.f, source);
								current_purchased.commodity_amounts[i] += delta;
								source -= delta;
							}
						} else {
							break;
						}
					}
				}
			}
		}

		for(auto c : state.world.nation_get_province_building_construction(n)) {
			if(c.get_province().get_nation_from_province_ownership() == c.get_province().get_nation_from_province_control()) {
				auto t = economy::province_building_type(c.get_type());
				// Rationale for not checking the building type:
				// Pop projects created for forts and naval bases should NOT happen in the first place, so checking against them
				// is a waste of resources
				auto& base_cost = state.economy_definitions.building_definitions[int32_t(t)].cost;
				auto& current_purchased = c.get_purchased_goods();
				float construction_time = float(state.economy_definitions.building_definitions[int32_t(t)].time);
				for(uint32_t i = 0; i < commodity_set::set_size; ++i) {
					if(base_cost.commodity_type[i]) {
						if(current_purchased.commodity_amounts[i] < base_cost.commodity_amounts[i] * admin_cost_factor) {
							auto& source = c.get_is_pop_project()
								? pbuf.get(base_cost.commodity_type[i])
								: gbuf.get(base_cost.commodity_type[i]);
							auto delta = std::clamp(base_cost.commodity_amounts[i] * admin_cost_factor / construction_time, 0.f, source);
							current_purchased.commodity_amounts[i] += delta;
							source -= delta;
						}
					} else {
						break;
					}
				}
			}
		}

		for(auto c : state.world.nation_get_state_building_construction(n)) {
			auto& base_cost = c.get_type().get_construction_costs();
			auto& current_purchased = c.get_purchased_goods();
			float construction_time = float(c.get_type().get_construction_time());
			float cost_mod = economy_factory::factory_build_cost_modifier(state, c.get_nation(), c.get_is_pop_project());
			for(uint32_t i = 0; i < commodity_set::set_size; ++i) {
				if(base_cost.commodity_type[i]) {
					if(current_purchased.commodity_amounts[i] < base_cost.commodity_amounts[i] * cost_mod) {
						auto& source = c.get_is_pop_project()
							? pbuf.get(base_cost.commodity_type[i])
							: gbuf.get(base_cost.commodity_type[i]);
						auto delta = std::clamp(base_cost.commodity_amounts[i] * cost_mod / construction_time, 0.f, source);
						current_purchased.commodity_amounts[i] += delta;
						source -= delta;
					}
				} else {
					break;
				}
			}
		}
	}

	float pop_min_wage_factor(sys::state& state, dcon::nation_id n) {
		return state.world.nation_get_modifier_values(n, sys::national_mod_offsets::minimum_wage);
	}

	float pop_artisan_min_wage(sys::state& state, dcon::nation_id n, float min_wage_factor) {
		float life = state.world.nation_get_life_needs_costs(n, state.culture_definitions.artisans);
		float everyday = state.world.nation_get_everyday_needs_costs(n, state.culture_definitions.artisans);
		return min_wage_factor * (life + everyday) * 1.1f;
	}

	float pop_farmer_min_wage(sys::state& state, dcon::nation_id n, float min_wage_factor) {
		float life = state.world.nation_get_life_needs_costs(n, state.culture_definitions.farmers);
		float everyday = state.world.nation_get_everyday_needs_costs(n, state.culture_definitions.farmers);
		return min_wage_factor * (life + everyday) * 1.1f;
	}

	float pop_laborer_min_wage(sys::state& state, dcon::nation_id n, float min_wage_factor) {
		float life = state.world.nation_get_life_needs_costs(n, state.culture_definitions.laborers);
		float everyday = state.world.nation_get_everyday_needs_costs(n, state.culture_definitions.laborers);
		return min_wage_factor * (life + everyday) * 1.1f;
	}
	
	float commodity_effective_price(sys::state& state, dcon::nation_id n, dcon::commodity_id c) {
		auto global_price_multiplier = global_market_price_multiplier(state, n);
		auto base_price = state.world.commodity_get_current_price(c);
		auto eff_p = base_price * global_price_multiplier;
		assert(std::isfinite(eff_p));
		return std::max(0.001f, eff_p); //sanity
	}

	void daily_update(sys::state& state) {
		/* initialization parallel block */
		concurrency::parallel_for(0, 10, [&](int32_t index) {
			switch(index) {
			case 0:
				populate_army_consumption(state);
				break;
			case 1:
				populate_navy_consumption(state);
				break;
			case 2:
				populate_construction_consumption(state);
				break;
			case 3:
				populate_private_construction_consumption(state);
				break;
			case 4:
				economy_factory::update_factory_triggered_modifiers(state);
				break;
			case 5:
				state.world.for_each_pop_type([&](dcon::pop_type_id t) {
					state.world.execute_serial_over_nation([&](auto nids) {
						state.world.nation_set_everyday_needs_costs(nids, t, ve::fp_vector{});
					});
				});
				break;
			case 6:
				state.world.for_each_pop_type([&](dcon::pop_type_id t) {
					state.world.execute_serial_over_nation([&](auto nids) {
						state.world.nation_set_luxury_needs_costs(nids, t, ve::fp_vector{});
					});
				});
				break;
			case 7:
				state.world.for_each_pop_type([&](dcon::pop_type_id t) {
					state.world.execute_serial_over_nation([&](auto nids) {
						state.world.nation_set_life_needs_costs(nids, t, ve::fp_vector{});
					});
				});
				break;
			case 8:
				state.world.execute_serial_over_nation([&](auto ids) {
					state.world.nation_set_subsidies_spending(ids, 0.0f);
				});
				break;
			case 9:
				state.world.execute_serial_over_nation([&](auto ids) {
					auto treasury = state.world.nation_get_stockpiles(ids, economy::money);
					state.world.nation_set_last_treasury(ids, treasury);
				});
				break;
			}
		});

		/* end initialization parallel block */

		auto const num_nation = state.world.nation_size();
		uint32_t total_commodities = state.world.commodity_size();

		/*
		As the day starts, we move production, fractionally, into the sphere leaders domestic production pool,
		following the same logic as Victoria 2
		*/
		for(auto n : state.nations_by_rank) {
			if(!n) // test for running out of sorted nations
				break;
			absorb_sphere_member_production(state, n); // no need for redundant checks here
		}
		for(auto n : state.nations_by_rank) {
			if(!n) // test for running out of sorted nations
				break;
			give_sphere_leader_production(state, n); // no need for redundant checks here
		}

		for(auto n : state.nations_by_rank) {
			if(!n) // test for running out of sorted nations
				break;

			/*
			### Calculate effective prices
			We will use the real demand from the *previous* day to determine how much of the purchasing will be done from the domestic
			and global pools (i.e. what percentage was able to be done from the cheaper pool). We will use that to calculate an
			effective price. And then, at the end of the current day, we will see how much of that purchasing actually came from each
			pool, etc. Depending on the stability of the simulation, we may, instead of taking the previous day, instead build this
			value iteratively as a linear combination of the new day and the previous day.

			when purchasing from global supply, prices are multiplied by (the nation's current effective tariff rate + its blockaded
			fraction
			+ 1)
			*/

			auto global_price_multiplier = global_market_price_multiplier(state, n);
			auto sl = state.world.nation_get_in_sphere_of(n);

			float base_demand = std::max(0.001f, state.defines.base_goods_demand + state.world.nation_get_modifier_values(n, sys::national_mod_offsets::goods_demand));
			int32_t num_inventions = 0;
			state.world.for_each_invention([&](auto iid) { num_inventions += int32_t(state.world.nation_get_active_inventions(n, iid)); });
			float invention_factor = std::max(0.001f, float(num_inventions) * state.defines.invention_impact_on_demand + 1.0f);
			populate_needs_costs(state, n, base_demand, invention_factor);

			float mobilization_impact = state.world.nation_get_is_mobilized(n) ? military::mobilization_impact(state, n) : 1.0f;

			auto const min_wage_factor = pop_min_wage_factor(state, n);
			float factory_min_wage = economy_factory::pop_factory_min_wage(state, n, min_wage_factor);
			float artisan_min_wage = pop_artisan_min_wage(state, n, min_wage_factor);
			float farmer_min_wage = pop_farmer_min_wage(state, n, min_wage_factor);
			float laborer_min_wage = pop_laborer_min_wage(state, n, min_wage_factor);

			// clear real demand
			state.world.for_each_commodity([&](dcon::commodity_id c) {
				state.world.nation_set_real_demand(n, c, 0.0f);
				state.world.nation_set_intermediate_demand(n, c, 0.f);
			});

			/*
			consumption updates
			*/
			auto cap_prov = state.world.nation_get_capital(n);
			auto cap_continent = state.world.province_get_continent(cap_prov);
			auto cap_region = state.world.province_get_connected_region_id(cap_prov);

			update_national_artisan_consumption(state, n, artisan_min_wage, mobilization_impact);

			for(auto p : state.world.nation_get_province_ownership(n)) {
				bool is_occupied = p.get_province().get_nation_from_province_control() != n;
				for(auto f : state.world.province_get_factory_location(p.get_province())) {
					// factory
					economy_factory::update_single_factory_consumption(state, f.get_factory(), n, p.get_province(), p.get_province().get_state_membership(), mobilization_impact, factory_min_wage, is_occupied);
				}
				// rgo
				bool is_mine = state.world.commodity_get_is_mine(state.world.province_get_rgo(p.get_province()));
				economy_rgo::update_province_rgo_consumption(state, p.get_province(), n, mobilization_impact, is_mine ? laborer_min_wage : farmer_min_wage, is_occupied);
			}

			update_pop_consumption(state, n, base_demand, invention_factor);
			limit_pop_demand_to_production(state);

			{
				// update national spending
				// step 1: figure out total
				float total = full_spending_cost(state, n);
				float military_total = military_spending_cost(state, n);
				state.world.nation_set_maximum_military_costs(n, military_total);
				// step 2: sum military and interest
				total += military_total;
				total += interest_payment(state, n);
				total += full_pop_spending_cost(state, n);
				// step 2: limit to actual budget
				float spending_scale = 0.0f;
				float budget = 0.f;
				assert(budget >= 0.f);
				if(can_take_loans(state, n)) {
					budget = total;
					spending_scale = 1.0f;
				} else {
					budget = std::max(0.f, state.world.nation_get_stockpiles(n, economy::money));
					spending_scale = (total == 0.f || total <= budget) ? 1.0f : budget / total;
				}

				assert(std::isfinite(total) && total >= 0.f);
				assert(std::isfinite(spending_scale) && spending_scale >= 0.f && spending_scale <= 1.f);
				assert(std::isfinite(budget) && budget >= 0.f);

				state.world.nation_get_stockpiles(n, economy::money) -= std::min(budget, total * spending_scale);
				state.world.nation_set_spending_level(n, spending_scale);

				float pi_total = full_private_investment_cost(state, n);
				float pi_budget = state.world.nation_get_private_investment(n);
				auto pi_scale = pi_total <= pi_budget ? 1.0f : pi_budget / pi_total;
				state.world.nation_set_private_investment_effective_fraction(n, pi_scale);
				state.world.nation_set_private_investment(n, std::max(0.0f, pi_budget - pi_total));

				update_national_consumption(state, n, spending_scale, pi_scale);
			}

			/* perform actual consumption / purchasing subject to availability */
			for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
				dcon::commodity_id c{ dcon::commodity_id::value_base_t(i) };

				auto dom_pool = state.world.nation_get_domestic_market_pool(n, c);
				auto sl_pool = (sl ? state.world.nation_get_domestic_market_pool(sl, c) : 0.0f);
				auto sp_pool = state.world.nation_get_stockpiles(n, c); //(state.world.nation_get_drawing_on_stockpiles(n, c) ? state.world.nation_get_stockpiles(n, c) : 0.0f);
				auto wm_pool = state.world.commodity_get_global_market_pool(c);

				auto total_supply = dom_pool + sl_pool + sp_pool + wm_pool;

				auto rd = state.world.nation_get_real_demand(n, c);
				auto old_sat = state.world.nation_get_demand_satisfaction(n, c);
				auto new_sat = rd > 0.0001f ? total_supply / rd : total_supply;
				auto adj_sat = old_sat * satisfaction_delay_factor + new_sat * (1.0f - satisfaction_delay_factor);
				state.world.nation_set_demand_satisfaction(n, c, std::max(0.0f, std::min(1.0f, adj_sat)));

				if(global_price_multiplier >= 1.0f) { // prefer domestic
					state.world.nation_set_domestic_market_pool(n, c, std::max(0.0f, dom_pool - rd));
					rd = std::max(rd - dom_pool, 0.0f);
					if(sl) {
						state.world.nation_set_domestic_market_pool(sl, c, std::max(0.0f, sl_pool - rd));
						rd = std::max(rd - sl_pool, 0.0f);
					}
					//if(state.world.nation_get_drawing_on_stockpiles(n, c)) {
					state.world.nation_set_stockpiles(n, c, std::max(0.0f, sp_pool - rd));
					rd = std::max(rd - sp_pool, 0.0f);
					//}
					state.world.commodity_set_global_market_pool(c, std::max(0.0f, wm_pool - rd));
					state.world.nation_set_imports(n, c, std::max(0.0f, std::min(wm_pool, rd)));
				} else {
					state.world.nation_set_imports(n, c, std::max(0.0f, std::min(wm_pool, rd)));
					state.world.commodity_set_global_market_pool(c, std::max(0.0f, wm_pool - rd));
					rd = std::max(rd - wm_pool, 0.0f);
					state.world.nation_set_domestic_market_pool(n, c, std::max(0.0f, dom_pool - rd));
					rd = std::max(rd - dom_pool, 0.0f);
					if(sl) {
						state.world.nation_set_domestic_market_pool(sl, c, std::max(0.0f, sl_pool - rd));
						rd = std::max(rd - sl_pool, 0.0f);
					}
					//if(state.world.nation_get_drawing_on_stockpiles(n, c)) {
					state.world.nation_set_stockpiles(n, c, std::max(0.0f, sp_pool - rd));
					//}
				}
			}
		}

		/*
		move remaining domestic supply to global pool, clear domestic market
		*/
		state.world.for_each_commodity([&](dcon::commodity_id c) {
			// per good decay would be nice...
			float decay = 0.5f;
			float world_pool = state.world.commodity_get_global_market_pool(c) * decay;
			ve::fp_vector sum;
			state.world.execute_serial_over_nation([&](auto nids) {
				sum = sum + state.world.nation_get_domestic_market_pool(nids, c);
				state.world.nation_set_domestic_market_pool(nids, c, 0.0f);
			});
			state.world.commodity_set_global_market_pool(c, world_pool + sum.reduce());
		});

		/* pay non "employed" pops (also zeros money for "employed" pops) */
		state.world.execute_parallel_over_pop([&](auto ids) {
			auto owners = nations::owner_of_pop(state, ids);
			auto owner_spending = state.world.nation_get_spending_level(owners);

			auto pop_of_type = state.world.pop_get_size(ids);
			auto adj_pop_of_type = pop_of_type * pop_payout_factor;

			auto const a_spending = owner_spending * ve::to_float(state.world.nation_get_administrative_spending(owners)) / 100.f;
			auto const s_spending = owner_spending * state.world.nation_get_administrative_efficiency(owners) * ve::to_float(state.world.nation_get_social_spending(owners)) / 100.0f;
			auto const e_spending = owner_spending * ve::to_float(state.world.nation_get_education_spending(owners)) / 100.f;
			auto const m_spending = owner_spending * ve::to_float(state.world.nation_get_military_spending(owners)) / 100.0f;
			auto const p_level = state.world.nation_get_modifier_values(owners, sys::national_mod_offsets::pension_level);
			auto const unemp_level = state.world.nation_get_modifier_values(owners, sys::national_mod_offsets::unemployment_benefit);
			auto const di_level = owner_spending * ve::to_float(state.world.nation_get_domestic_investment_spending(owners)) / 100.f;

			auto types = state.world.pop_get_poptype(ids);

			auto ln_types = state.world.pop_type_get_life_needs_income_type(types);
			auto en_types = state.world.pop_type_get_everyday_needs_income_type(types);
			auto lx_types = state.world.pop_type_get_luxury_needs_income_type(types);

			auto ln_costs = ve::apply([&](dcon::pop_type_id pt, dcon::nation_id n) { return pt ? state.world.nation_get_life_needs_costs(n, pt) : 0.0f; }, types, owners);
			auto en_costs = ve::apply([&](dcon::pop_type_id pt, dcon::nation_id n) { return pt ? state.world.nation_get_everyday_needs_costs(n, pt) : 0.0f; }, types, owners);
			auto lx_costs = ve::apply([&](dcon::pop_type_id pt, dcon::nation_id n) { return pt ? state.world.nation_get_luxury_needs_costs(n, pt) : 0.0f; }, types, owners);

			auto acc_a = ve::select(ln_types == int32_t(culture::income_type::administration), a_spending * adj_pop_of_type * ln_costs, 0.0f);
			auto acc_e = ve::select(ln_types == int32_t(culture::income_type::education), e_spending * adj_pop_of_type * ln_costs, 0.0f);
			auto acc_m = ve::select(ln_types == int32_t(culture::income_type::military), m_spending * adj_pop_of_type * ln_costs, 0.0f);

			auto none_of_above = ln_types != int32_t(culture::income_type::military) &&
			ln_types != int32_t(culture::income_type::education) &&
			ln_types != int32_t(culture::income_type::administration);

			auto acc_u = ve::select(none_of_above, s_spending * adj_pop_of_type * p_level * ln_costs, 0.0f);

			acc_a = acc_a + ve::select(en_types == int32_t(culture::income_type::administration), a_spending * adj_pop_of_type * en_costs, 0.0f);
			acc_e = acc_e + ve::select(en_types == int32_t(culture::income_type::education), e_spending * adj_pop_of_type * en_costs, 0.0f);
			acc_m = acc_m + ve::select(en_types == int32_t(culture::income_type::military), m_spending * adj_pop_of_type * en_costs, 0.0f);

			acc_u = acc_u + ve::select(types == state.culture_definitions.capitalists, di_level * adj_pop_of_type * lx_costs, 0.0f);
			acc_u = acc_u + ve::select(types == state.culture_definitions.aristocrat, di_level * adj_pop_of_type * lx_costs, 0.0f);

			acc_a = acc_a + ve::select(lx_types == int32_t(culture::income_type::administration), a_spending * adj_pop_of_type * lx_costs, 0.0f);
			acc_e = acc_e + ve::select(lx_types == int32_t(culture::income_type::education), e_spending * adj_pop_of_type * lx_costs, 0.0f);
			acc_m = acc_m + ve::select(lx_types == int32_t(culture::income_type::military), m_spending * adj_pop_of_type * lx_costs, 0.0f);

			auto employment = state.world.pop_get_employment(ids);

			acc_u = acc_u + ve::select(none_of_above && state.world.pop_type_get_has_unemployment(types), s_spending * (pop_of_type - employment) * pop_payout_factor * unemp_level * ln_costs, 0.0f);

			/* Savings represent the daily income of the pop - hence, daily means it is overwriten each day with a new value - w.r.t. to it's old value - no relation is given */
			state.world.pop_set_savings(ids, ((acc_e + acc_m) + (acc_u + acc_a)));
			//state.world.pop_set_savings(ids, state.world.pop_get_savings(ids) + ((acc_e + acc_m) + (acc_u + acc_a)));
			ve::apply([](float v) { assert(std::isfinite(v) && v >= 0.f); }, acc_e);
			ve::apply([](float v) { assert(std::isfinite(v) && v >= 0.f); }, acc_m);
			ve::apply([](float v) { assert(std::isfinite(v) && v >= 0.f); }, acc_u);
			ve::apply([](float v) { assert(std::isfinite(v) && v >= 0.f); }, acc_a);
		});

		/* add up production, collect taxes and tariffs, other updates purely internal to each nation */
		concurrency::parallel_for(uint32_t(0), state.world.nation_size(), [&](uint32_t i) {
			auto n = dcon::nation_id{ dcon::nation_id::value_base_t(i) };
			if(state.world.nation_get_owned_province_count(n) == 0)
				return;

			/* prepare needs satisfaction caps */
			auto ln_max = state.world.pop_type_make_vectorizable_float_buffer();
			auto en_max = state.world.pop_type_make_vectorizable_float_buffer();
			auto lx_max = state.world.pop_type_make_vectorizable_float_buffer();
			state.world.for_each_pop_type([&](dcon::pop_type_id pt) {
				float ln_total = 0.0f;
				float en_total = 0.0f;
				float lx_total = 0.0f;
				for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
					dcon::commodity_id c{ dcon::commodity_id::value_base_t(i) };
					auto kf = state.world.commodity_get_key_factory(c);
					if(state.world.commodity_get_is_available_from_start(c) || (kf && state.world.nation_get_active_building(n, kf))) {
						auto sat = state.world.nation_get_demand_satisfaction(n, c);

						auto ln_val = state.world.pop_type_get_life_needs(pt, c) * pop_get_life_needs_weight(state, n, c);
						ln_total += ln_val;
						ln_max.get(pt) += ln_val * sat;

						auto en_val = state.world.pop_type_get_everyday_needs(pt, c) * pop_get_everyday_needs_weight(state, n, c);
						en_total += en_val;
						en_max.get(pt) += en_val * sat;

						auto lx_val = state.world.pop_type_get_luxury_needs(pt, c) * pop_get_luxury_needs_weight(state, n, c);
						lx_total += lx_val;
						lx_max.get(pt) += lx_val * sat;
					}
				}
				if(ln_total > 0.f)
					ln_max.get(pt) /= ln_total;
				else
					ln_max.get(pt) = 1.f;
				if(en_total > 0.f)
					en_max.get(pt) /= en_total;
				else
					en_max.get(pt) = 1.f;
				if(lx_total > 0.f)
					lx_max.get(pt) /= lx_total;
				else
					lx_max.get(pt) = 1.f;
			});

			/* determine effective spending levels */
			auto nations_commodity_spending = state.world.nation_get_spending_level(n);
			float refund = 0.0f;
			{
				float max_sp = 0.0f;
				float total = 0.0f;
				float spending_level = nations_commodity_spending * float(state.world.nation_get_naval_spending(n)) / 100.0f;
				for(uint32_t k = 1; k < state.world.commodity_size(); ++k) {
					dcon::commodity_id c{ dcon::commodity_id::value_base_t(k) };
					auto sat = state.world.nation_get_demand_satisfaction(n, c);
					auto val = state.world.nation_get_navy_demand(n, c);
					assert(sat >= 0.f && sat <= 1.f);
					assert(val >= 0.f);
					total += val;
					refund += val * (1.f - sat) * spending_level * state.world.commodity_get_current_price(c);
					max_sp += val * sat;
				}
				if(total > 0.f)
					max_sp /= total;
				state.world.nation_set_effective_naval_spending(n, max_sp * spending_level);
			}
			{
				float max_sp = 0.0f;
				float total = 0.0f;
				float spending_level = nations_commodity_spending * float(state.world.nation_get_land_spending(n)) / 100.0f;
				for(uint32_t k = 1; k < state.world.commodity_size(); ++k) {
					dcon::commodity_id c{ dcon::commodity_id::value_base_t(k) };
					auto sat = state.world.nation_get_demand_satisfaction(n, c);
					auto val = state.world.nation_get_army_demand(n, c);
					assert(sat >= 0.f && sat <= 1.f);
					assert(val >= 0.f);
					refund += val * (1.f - sat) * spending_level * state.world.commodity_get_current_price(c);
					total += val;
					max_sp += val * sat;
				}
				if(total > 0.f)
					max_sp /= total;
				state.world.nation_set_effective_land_spending(n, max_sp * spending_level);
			}
			{
				float max_sp = 0.0f;
				float total = 0.0f;
				float spending_level = nations_commodity_spending * float(state.world.nation_get_construction_spending(n)) / 100.0f;
				for(uint32_t k = 1; k < state.world.commodity_size(); ++k) {
					dcon::commodity_id c{ dcon::commodity_id::value_base_t(k) };
					auto sat = state.world.nation_get_demand_satisfaction(n, c);
					auto val = state.world.nation_get_construction_demand(n, c) * true_construction_demand;
					assert(sat >= 0.f && sat <= 1.f);
					assert(val >= 0.f);
					total += val;
					refund += val * (1.f - sat) * spending_level * state.world.commodity_get_current_price(c);
					max_sp += val * sat;
				}
				if(total > 0.f)
					max_sp /= total;
				state.world.nation_set_effective_construction_spending(n, max_sp * spending_level);
			}
			/* fill stockpiles */
			for(uint32_t k = 1; k < state.world.commodity_size(); ++k) {
				dcon::commodity_id c{ dcon::commodity_id::value_base_t(k) };
				auto difference = state.world.nation_get_stockpile_targets(n, c) - state.world.nation_get_stockpiles(n, c);
				if(difference > 0.f && state.world.nation_get_drawing_on_stockpiles(n, c) == false) {
					auto sat = state.world.nation_get_demand_satisfaction(n, c);
					state.world.nation_get_stockpiles(n, c) += difference * nations_commodity_spending * sat;
					refund += difference * (1.0f - sat) * nations_commodity_spending * state.world.commodity_get_current_price(c);
				}
			}

			/* calculate overseas penalty */
			{
				auto overseas_factor = state.defines.province_overseas_penalty * float(state.world.nation_get_owned_province_count(n) - state.world.nation_get_central_province_count(n));
				auto overseas_budget = float(state.world.nation_get_overseas_spending(n)) / 100.f;
				auto overseas_budget_satisfaction = 1.f;

				if(overseas_factor > 0) {
					for(uint32_t k = 1; k < state.world.commodity_size(); ++k) {
						dcon::commodity_id c{ dcon::commodity_id::value_base_t(k) };
						auto kf = state.world.commodity_get_key_factory(c);
						if(state.world.commodity_get_overseas_penalty(c) &&
						(state.world.commodity_get_is_available_from_start(c) || (kf && state.world.nation_get_active_building(n, kf)))) {
							auto sat = state.world.nation_get_demand_satisfaction(n, c);
							overseas_budget_satisfaction = std::min(sat, overseas_budget_satisfaction);
							refund += overseas_factor * (1.0f - sat) * nations_commodity_spending * state.world.commodity_get_current_price(c);
						}
					}
					state.world.nation_set_overseas_penalty(n, overseas_budget * overseas_budget_satisfaction);
				} else {
					state.world.nation_set_overseas_penalty(n, 1.0f);
				}
			}

			assert(std::isfinite(refund) && refund >= 0.0f);
			// TODO: Fix refund
			state.world.nation_get_stockpiles(n, economy::money) += refund;

			auto const min_wage_factor = pop_min_wage_factor(state, n);

			float factory_min_wage = economy_factory::pop_factory_min_wage(state, n, min_wage_factor);
			assert(factory_min_wage >= 0.f);
			float farmer_min_wage = pop_farmer_min_wage(state, n, min_wage_factor);
			assert(farmer_min_wage >= 0.f);
			float laborer_min_wage = pop_laborer_min_wage(state, n, min_wage_factor);
			assert(laborer_min_wage >= 0.f);

			update_national_artisan_production(state, n);

			for(auto p : state.world.nation_get_province_ownership(n)) {
				// perform production
				for(auto f : state.world.province_get_factory_location(p.get_province())) {
					economy_factory::update_single_factory_production(state, f.get_factory(), n, factory_min_wage);
				}
				// rgo
				economy_rgo::update_province_rgo_production(state, p.get_province(), n);
				/* adjust pop satisfaction based on consumption */
				for(auto pl : p.get_province().get_pop_location()) {
					auto t = pl.get_pop().get_poptype();
					auto ln = pl.get_pop().get_life_needs_satisfaction();
					auto en = pl.get_pop().get_everyday_needs_satisfaction();
					auto lx = pl.get_pop().get_luxury_needs_satisfaction();
					ln = std::min(ln, ln_max.get(t));
					en = std::min(en, en_max.get(t));
					lx = std::min(lx, lx_max.get(t));
					pl.get_pop().set_life_needs_satisfaction(ln);
					pl.get_pop().set_everyday_needs_satisfaction(en);
					pl.get_pop().set_luxury_needs_satisfaction(lx);
				}
			}
			/* pay "employed" pops */
			{
				// ARTISAN
				auto const artisan_type = state.culture_definitions.artisans;
				float artisan_profit = state.world.nation_get_artisan_profit(n);
				float num_artisans = state.world.nation_get_demographics(n, demographics::to_key(state, artisan_type));
				if(num_artisans > 0.f) {
					auto per_profit = artisan_profit / num_artisans;
					for(auto p : state.world.nation_get_province_ownership(n)) {
						for(auto pl : p.get_province().get_pop_location()) {
							if(artisan_type == pl.get_pop().get_poptype()) {
								pl.get_pop().set_savings(pl.get_pop().get_savings() + pl.get_pop().get_size() * per_profit);
								assert(std::isfinite(pl.get_pop().get_savings()) && pl.get_pop().get_savings() >= 0);
							}
						}
					}
				}
			}

			/* pay factory workers / capitalists */
			for(auto si : state.world.nation_get_state_ownership(n)) {
				float num_aristocrat = state.world.state_instance_get_demographics(si.get_state(), demographics::to_key(state, state.culture_definitions.aristocrat));
				float rgo_owner_profit = 0.f;
				float total_profit = 0.f;
				province::for_each_province_in_state_instance(state, si.get_state(), [&](dcon::province_id p) {
					for(auto f : state.world.province_get_factory_location(p)) {
						total_profit += std::max(0.f, f.get_factory().get_full_profit());
					}
					{
						// FARMER / LABORER
						bool is_mine = state.world.commodity_get_is_mine(state.world.province_get_rgo(p));
						auto const min_wage = (is_mine ? laborer_min_wage : farmer_min_wage);
						float total_min_to_workers = 0.0f;
						float num_workers = 0.0f;
						for(auto wt : state.culture_definitions.rgo_workers) {
							total_min_to_workers += min_wage * state.world.province_get_demographics(p, demographics::to_employment_key(state, wt));
							//num_workers += state.world.province_get_demographics(p, demographics::to_key(state, wt));
						}
						for(auto const pl : state.world.province_get_pop_location(p)) {
							num_workers += pl.get_pop().get_employment();
						}
						auto const total_rgo_profit = state.world.province_get_rgo_full_profit(p);
						/* owners ALWAYS get "some" chunk of income */
						rgo_owner_profit += rgo_owners_cut * total_rgo_profit;
						auto const rgo_worker_profit = (1.f - rgo_owners_cut) * total_rgo_profit;
						auto const per_worker_profit = num_workers > 0.f ? rgo_worker_profit / num_workers : 0.0f;
						for(auto const pl : state.world.province_get_pop_location(p)) {
							if(pl.get_pop().get_poptype().get_is_paid_rgo_worker()) {
								if(pl.get_pop().get_employment() > 0.0f) {
									pl.get_pop().set_savings(pl.get_pop().get_savings() + per_worker_profit * pl.get_pop().get_employment());
									assert(std::isfinite(pl.get_pop().get_savings()) && pl.get_pop().get_savings() >= 0);
								}
							}
						}
					}
				});
				rgo_owner_profit = num_aristocrat > 0.f ? rgo_owner_profit / num_aristocrat : 0.0f;

				auto const min_wage = factory_min_wage;
				auto profit = economy_factory::distribute_factory_profit(state, si.get_state(), min_wage, total_profit);
				province::for_each_province_in_state_instance(state, si.get_state(), [&](dcon::province_id p) {
					for(auto pl : state.world.province_get_pop_location(p)) {
						if(state.culture_definitions.primary_factory_worker == pl.get_pop().get_poptype()) {
							pl.get_pop().set_savings(pl.get_pop().get_savings() + profit.per_primary_worker * pl.get_pop().get_employment());
							assert(std::isfinite(pl.get_pop().get_savings()) && pl.get_pop().get_savings() >= 0);
						} else if(state.culture_definitions.secondary_factory_worker == pl.get_pop().get_poptype()) {
							pl.get_pop().set_savings(pl.get_pop().get_savings() + profit.per_secondary_worker * pl.get_pop().get_employment());
							assert(std::isfinite(pl.get_pop().get_savings()) && pl.get_pop().get_savings() >= 0);
						} else if(state.culture_definitions.capitalists == pl.get_pop().get_poptype()) {
							pl.get_pop().set_savings(pl.get_pop().get_savings() + pl.get_pop().get_size() * profit.per_owner);
							assert(std::isfinite(pl.get_pop().get_savings()) && pl.get_pop().get_savings() >= 0);
						} else if(state.culture_definitions.aristocrat == pl.get_pop().get_poptype()) {
							pl.get_pop().set_savings(pl.get_pop().get_savings() + pl.get_pop().get_size() * rgo_owner_profit);
							assert(std::isfinite(pl.get_pop().get_savings()) && pl.get_pop().get_savings() >= 0);
						}
					}
				});
			}

			/* collect taxes */
			auto const tax_eff = nations::tax_efficiency(state, n);
			float total_poor_tax_base = 0.0f;
			float total_mid_tax_base = 0.0f;
			float total_rich_tax_base = 0.0f;
			auto const poor_effect = float(state.world.nation_get_poor_tax(n)) / 100.0f;
			auto const middle_effect = float(state.world.nation_get_middle_tax(n)) / 100.0f;
			auto const rich_effect = float(state.world.nation_get_rich_tax(n)) / 100.0f;
			assert(poor_effect >= 0.f && middle_effect >= 0.f && rich_effect >= 0.f);
			for(auto p : state.world.nation_get_province_ownership(n)) {
				auto province = p.get_province();
				if(state.world.province_get_nation_from_province_ownership(province) == state.world.province_get_nation_from_province_control(province)) {
					for(auto pl : province.get_pop_location()) {
						// Money drain when admin effectiveness is <100%
						auto total_pop = pl.get_pop().get_size();
						// Life needs and buying deduction occurs on a previous pass
						auto& pop_money = pl.get_pop().get_savings();
						auto strata = culture::pop_strata(pl.get_pop().get_poptype().get_strata());
						if(strata == culture::pop_strata::poor) {
							total_poor_tax_base += pop_money;
							pop_money -= pop_money * tax_eff * poor_effect;
						} else if(strata == culture::pop_strata::middle) {
							total_mid_tax_base += pop_money;
							pop_money -= pop_money * tax_eff * middle_effect;
						} else if(strata == culture::pop_strata::rich) {
							total_rich_tax_base += pop_money;
							pop_money -= pop_money * tax_eff * rich_effect;
						}

					}
				}
			}
			float max_change_in_tax = 2000.f;
			auto previous_rich_tax = state.world.nation_get_total_rich_income(n);
			total_rich_tax_base = std::clamp(total_rich_tax_base, 0.0f, previous_rich_tax + max_change_in_tax);
			state.world.nation_set_total_rich_income(n, total_rich_tax_base);

			auto previous_mid_tax = state.world.nation_get_total_middle_income(n);
			total_mid_tax_base = std::clamp(total_mid_tax_base, 0.0f, previous_mid_tax + max_change_in_tax);
			state.world.nation_set_total_middle_income(n, total_mid_tax_base);

			auto previous_poor_tax = state.world.nation_get_total_poor_income(n);
			total_poor_tax_base = std::clamp(total_poor_tax_base, 0.0f, previous_poor_tax + max_change_in_tax);
			state.world.nation_set_total_poor_income(n, total_poor_tax_base);

			auto collected_tax = tax_eff * (total_rich_tax_base * rich_effect + total_mid_tax_base * middle_effect + total_poor_tax_base * poor_effect);
			assert(std::isfinite(collected_tax) && collected_tax >= 0.f);
			state.world.nation_get_stockpiles(n, economy::money) += collected_tax;
			{
				/* collect tariffs */
				float t_total = effective_tariff_rate(state, n) * nation_total_imports(state, n);
				assert(std::isfinite(t_total));
				state.world.nation_get_stockpiles(n, economy::money) += t_total;
			}
			advance_construction(state, n);
			adjust_artisan_balance(state, n);
		});

		/*
		adjust prices based on global production & consumption
		*/

		concurrency::parallel_for(uint32_t(0), state.world.commodity_size(), [&](uint32_t k) {
			dcon::commodity_id cid{ dcon::commodity_id::value_base_t(k) };

			//handling gold cost separetely
			if(state.world.commodity_get_money_rgo(cid)) {
				return;
			}

			state.world.commodity_set_last_total_production(cid, state.world.commodity_get_total_production(cid));
			state.world.commodity_set_last_total_real_demand(cid, state.world.commodity_get_total_real_demand(cid));

			float total_r_demand = 0.0f;
			float total_consumption = 0.0f;
			float total_production = 0.0f;
			state.world.for_each_nation([&](dcon::nation_id n) {
				total_r_demand += state.world.nation_get_real_demand(n, cid);
				total_consumption += state.world.nation_get_real_demand(n, cid) * state.world.nation_get_demand_satisfaction(n, cid);
				total_production += state.world.nation_get_domestic_market_pool(n, cid);
			});
			state.world.commodity_set_total_consumption(cid, total_consumption);
			state.world.commodity_set_total_real_demand(cid, total_r_demand);
			state.world.commodity_set_total_production(cid, total_production);

			/*
			- price of money remains the price defined by the commodities file
			- for each other commodity, check its real demand (see below for what real demand is), and if its real demand is positive,
			check its supply. If its supply is also positive, we then take the square root of real-demand / supply and multiply that
			result by the base price of the commodity. Let us call this the ratio-adjusted-price.
			- If the ratio-adjusted-price is less than the current price - 0.01, then the price decreases by 0.01.
			- If the ratio-adjusted-price is greater than the current price + 0.01, then the price increases by 0.01.
			- Prices are then limited to being within 1/5th to 5x the base price of the commodity.
			- Unadjusted supply and demand: While most of what goes on below is expressed directly in terms of adding to
			domestic supply or demand, unless noted otherwise, it should be taken as given that the same is added to world
			supply or demand. Also, world supply and demand for all commodities should be treated as at least 1.
			*/
			float supply = total_production;
			float demand = total_r_demand;
			auto base_price = state.world.commodity_get_cost(cid);
			auto current_price = state.world.commodity_get_current_price(cid);
			float ratio_adjusted_price = math::sqrt(std::max(demand, 1.f) / std::max(supply, 1.f)) * base_price;
			if(ratio_adjusted_price < current_price - 0.01f) {
				current_price -= 0.01f;
			} else if(ratio_adjusted_price > current_price + 0.01f) {
				current_price += 0.01f;
			}
			auto min_price = base_price * (1.f / 5.f);
			auto max_price = base_price * 5.f;
			state.world.commodity_set_current_price(cid, std::clamp(current_price, min_price, max_price));
		});

		/* DIPLOMATIC EXPENSES */
		for(auto n : state.world.in_nation) {
			for(auto uni : n.get_unilateral_relationship_as_source()) {
				if(uni.get_war_subsidies()) {
					auto target_m_costs = uni.get_target().get_maximum_military_costs() * state.defines.warsubsidies_percent;
					if(target_m_costs <= n.get_stockpiles(money)) {
						n.get_stockpiles(money) -= target_m_costs;
						uni.get_target().get_stockpiles(money) += target_m_costs;
					} else {
						uni.set_war_subsidies(false);

						notification::post(state, notification::message{
							[source = n.id, target = uni.get_target().id](sys::state& state, text::layout_base& contents) {
								text::add_line(state, contents, "msg_wsub_end_1", text::variable_type::x, source, text::variable_type::y, target);
							},
							"msg_wsub_end_title",
						n.id, uni.get_target().id, dcon::nation_id{},
							sys::message_base_type::war_subsidies_end
						});
					}
				}
				if(uni.get_reparations() && state.current_date < n.get_reparations_until()) {
					auto const tax_eff = nations::tax_efficiency(state, n);
					auto total_tax_base = n.get_total_rich_income() + n.get_total_middle_income() + n.get_total_poor_income();

					auto payout = total_tax_base * tax_eff * state.defines.reparations_tax_hit;
					auto capped_payout = std::min(n.get_stockpiles(money), payout);
					assert(capped_payout >= 0.0f);

					n.get_stockpiles(money) -= capped_payout;
					uni.get_target().get_stockpiles(money) += capped_payout;
				}
			}
		}

		/*
		BANKRUPTCY
		*/
		for(auto n : state.world.in_nation) {
			auto m = n.get_stockpiles(money);
			if(m < 0 && m < -max_loan(state, n)) {
				go_bankrupt(state, n);
			}
		}

		/* update inflation */
		state.inflation = 0.55f;
		/*
		float primary_commodity_basket = 0.0f;
		state.world.for_each_commodity([&](dcon::commodity_id c) {
			state.world.for_each_pop_type([&](dcon::pop_type_id pt) {
				primary_commodity_basket += 2.0f * state.world.commodity_get_cost(c) * state.world.pop_type_get_life_needs(pt, c);
				primary_commodity_basket += 2.0f * state.world.commodity_get_cost(c) * state.world.pop_type_get_everyday_needs(pt, c);
			});
		});
		primary_commodity_basket /= float(state.world.pop_type_size());
		float total_pop = 0.0f;
		float total_pop_money = 0.0f;
		state.world.for_each_nation([&](dcon::nation_id n) {
			total_pop += state.world.nation_get_demographics(n, demographics::total);
			total_pop_money += state.world.nation_get_total_rich_income(n) + state.world.nation_get_total_middle_income(n) + state.world.nation_get_total_poor_income(n);
		});

		float target_money = total_pop * primary_commodity_basket;
		if(total_pop_money > 0.001f) {
			state.inflation = (state.inflation * 0.9f) + (0.1f * target_money / total_pop_money);
		}
		*/

		// make constructions:
		resolve_constructions(state);

		// make new investments
		for(auto n : state.world.in_nation) {
			auto nation_rules = n.get_combined_issue_rules();

			// check if current projects are already too expensive for capitalists to manage
			float total_cost = 0.f;

			for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
				dcon::commodity_id c{ dcon::commodity_id::value_base_t(i) };
				total_cost += state.world.nation_get_private_construction_demand(n, c) * state.world.commodity_get_current_price(c);
			}

			float total_cost_added = 0.f;

			if(n.get_private_investment() > total_cost
			&& n.get_is_civilized()
			&& (nation_rules & (issue_rule::pop_build_factory | issue_rule::pop_expand_factory)) != 0) {

				static std::vector<dcon::state_instance_id> states_in_order;
				states_in_order.clear();
				for(auto si : n.get_state_ownership()) {
					if(si.get_state().get_capital().get_is_colonial() == false) {
						// Do not spam factories in states (let the ones being built finish first)
						auto sc = si.get_state().get_state_building_construction();
						if(sc.begin() == sc.end()) {
							states_in_order.push_back(si.get_state().id);
						}
					}
				}
				pdqsort(states_in_order.begin(), states_in_order.end(), [&](dcon::state_instance_id a, dcon::state_instance_id b) {
					auto a_pop = state.world.state_instance_get_demographics(a, demographics::total);
					auto b_pop = state.world.state_instance_get_demographics(b, demographics::total);
					if(a_pop != b_pop)
						return a_pop > b_pop;
					return a.index() < b.index(); // force total ordering
				});

				static std::vector<dcon::factory_type_id> desired_types;
				desired_types.clear();
				if(!states_in_order.empty() && (nation_rules & issue_rule::pop_build_factory) != 0) {
					ai::get_desired_factory_types(state, n, desired_types);
				}

				//upgrade all good targets!!!
				//upgrading only one per run is too slow and leads to massive unemployment!!!

				for(auto s : states_in_order) {
					auto pw_num = state.world.state_instance_get_demographics(s, demographics::to_key(state, state.culture_definitions.primary_factory_worker));
					auto pw_employed = state.world.state_instance_get_demographics(s, demographics::to_employment_key(state, state.culture_definitions.primary_factory_worker));

					if(pw_employed >= pw_num && pw_num > 0.0f)
						continue; // no spare workers

					int32_t num_factories = 0;
					float profit = 0.0f;
					dcon::factory_id selected_factory;
					// is there an upgrade target ?
					auto d = state.world.state_instance_get_definition(s);
					for(auto p : state.world.state_definition_get_abstract_state_membership(d)) {
						if(p.get_province().get_nation_from_province_ownership() == n) {
							auto fl_range = p.get_province().get_factory_location();
							num_factories += int32_t(fl_range.end() - fl_range.begin());
							for(auto f : fl_range) {
								if((nation_rules & issue_rule::pop_expand_factory) != 0
								&& f.get_factory().get_production_scale() >= 0.9985f
								&& f.get_factory().get_primary_employment() >= 0.9985f
								&& f.get_factory().get_level() < uint8_t(255)) {
									auto type = f.get_factory().get_building_type();
									auto ug_in_progress = false;
									for(auto c : state.world.state_instance_get_state_building_construction(s)) {
										if(c.get_type() == type) {
											ug_in_progress = true;
											break;
										}
									}
									if(ug_in_progress) {
										continue;
									}
									if(auto new_p = f.get_factory().get_full_profit() / f.get_factory().get_level(); new_p > profit) {
										profit = new_p;
										selected_factory = f.get_factory();
									}
								}
							}
						}
					}
					if(selected_factory && profit > 1.f) {
						auto type = state.world.factory_get_building_type(selected_factory);
						auto new_up = fatten(state.world, state.world.force_create_state_building_construction(s, n));
						new_up.set_remaining_time(state.world.factory_type_get_construction_time(type));
						new_up.set_is_pop_project(true);
						new_up.set_is_upgrade(true);
						new_up.set_type(type);
					}

					//try to invest into something new...
					//bool found_investment = false;
					auto existing_constructions = state.world.state_instance_get_state_building_construction(s);
					if(existing_constructions.begin() != existing_constructions.end())
						continue; // already building

					if(n.get_private_investment() * 0.01f < total_cost + total_cost_added)
						continue;

					if(num_factories < int32_t(state.defines.factories_per_state) && (nation_rules & issue_rule::pop_build_factory) != 0) {
						// randomly try a valid (check coastal, unlocked, non existing) factory
						if(economy_factory::state_factory_count(state, s) >= int32_t(state.defines.factories_per_state))
							continue;
						if(!desired_types.empty()) {
							std::vector<dcon::factory_type_id> valid_desired_types;
							for(const auto ft : desired_types) {
								if(state.world.factory_type_get_is_coastal(ft) && !province::state_is_coastal(state, s))
									continue;
								if(economy_factory::state_instance_has_factory_being_built(state, s, ft))
									continue;
								bool present_in_location = false;
								province::for_each_province_in_state_instance(state, s, [&](dcon::province_id p) {
									for(auto fac : state.world.province_get_factory_location(p)) {
										auto type = fac.get_factory().get_building_type();
										if(ft == type) {
											present_in_location = true;
											return;
										}
									}
								});
								if(present_in_location)
									continue;
								valid_desired_types.push_back(ft);
							}
							pdqsort(valid_desired_types.begin(), valid_desired_types.end(), [&](dcon::factory_type_id a, dcon::factory_type_id b) {
								auto a_bonus = economy_factory::sum_of_factory_triggered_modifiers(state, a, s) + economy_factory::sum_of_factory_triggered_input_modifiers(state, a, s);
								auto b_bonus = economy_factory::sum_of_factory_triggered_modifiers(state, b, s) + economy_factory::sum_of_factory_triggered_input_modifiers(state, a, s);
								if(a_bonus != b_bonus)
									return a_bonus > b_bonus;
								return a.index() < b.index(); // force total ordering
							});
							if(!valid_desired_types.empty()) {
								auto type = valid_desired_types[0];
								auto new_up = fatten(state.world, state.world.force_create_state_building_construction(s, n));
								new_up.set_remaining_time(state.world.factory_type_get_construction_time(type));
								new_up.set_is_pop_project(true);
								new_up.set_is_upgrade(false);
								new_up.set_type(type);
								auto costs = new_up.get_type().get_construction_costs();
								for(uint32_t i = 0; i < commodity_set::set_size; ++i) {
									if(costs.commodity_type[i]) {
										total_cost_added += commodity_effective_price(state, n, costs.commodity_type[i]) * costs.commodity_amounts[i];
									} else {
										break;
									}
								}
								//found_investment = true;
							}
						}
					}
				}

				if((nation_rules & issue_rule::pop_build_factory) != 0) {
					static std::vector<std::pair<dcon::province_id, int32_t>> provinces_in_order;
					provinces_in_order.clear();
					for(auto si : n.get_state_ownership()) {
						if(si.get_state().get_capital().get_is_colonial() == false) {
							auto s = si.get_state().id;
							auto d = state.world.state_instance_get_definition(s);
							int32_t num_factories = 0;
							for(auto p : state.world.state_definition_get_abstract_state_membership(d)) {
								if(province::generic_can_build_railroads(state, p.get_province(), n) && p.get_province().get_nation_from_province_ownership() == n) {
									for(auto f : p.get_province().get_factory_location())
										num_factories += int32_t(f.get_factory().get_level());
									provinces_in_order.emplace_back(p.get_province().id, num_factories);
								}
							}
							// The state's number of factories is intentionally given to all the provinces within the state so the
							// railroads aren't just built on a single province within a state
							for(auto p : state.world.state_definition_get_abstract_state_membership(d)) {
								if(province::generic_can_build_railroads(state, p.get_province(), n) && p.get_province().get_nation_from_province_ownership() == n)
									provinces_in_order.emplace_back(p.get_province().id, num_factories);
							}
						}
					}
					if(!provinces_in_order.empty()) {
						std::pair<dcon::province_id, int32_t> best_p = provinces_in_order[0];
						for(auto e : provinces_in_order)
							if(e.second > best_p.second)
								best_p = e;

						auto new_rr = fatten(state.world, state.world.force_create_province_building_construction(best_p.first, n));
						new_rr.set_remaining_time(state.economy_definitions.building_definitions[uint8_t(economy::province_building_type::railroad)].time);
						new_rr.set_is_pop_project(true);
						new_rr.set_type(uint8_t(province_building_type::railroad));
						//found_investment = true;
					}
				}
			}
			n.set_private_investment(0.0f);
		}
	}

	void regenerate_unsaved_values(sys::state& state) {
		state.culture_definitions.rgo_workers.clear();
		for(auto pt : state.world.in_pop_type) {
			if(pt.get_is_paid_rgo_worker())
			state.culture_definitions.rgo_workers.push_back(pt);
		}
		for(uint32_t k = 1; k < state.world.commodity_size(); ++k) {
		dcon::commodity_id cid{ dcon::commodity_id::value_base_t(k) };
			for(auto pt : state.world.in_pop_type) {
				if(pt != state.culture_definitions.slaves) {
					if(pt.get_life_needs(cid) > 0.0f)
					state.world.commodity_set_is_life_need(cid, true);
					if(pt.get_everyday_needs(cid) > 0.0f)
					state.world.commodity_set_is_everyday_need(cid, true);
					if(pt.get_luxury_needs(cid) > 0.0f)
					state.world.commodity_set_is_luxury_need(cid, true);
				}
			}
		}

		state.world.nation_resize_intermediate_demand(state.world.commodity_size());

		state.world.nation_resize_life_needs_costs(state.world.pop_type_size());
		state.world.nation_resize_everyday_needs_costs(state.world.pop_type_size());
		state.world.nation_resize_luxury_needs_costs(state.world.pop_type_size());

		state.world.province_resize_rgo_actual_production_per_good(state.world.commodity_size());

		state.world.for_each_commodity([&](dcon::commodity_id c) {
			auto fc = fatten(state.world, c);
		state.world.commodity_set_key_factory(c, dcon::factory_type_id{});
			if(fc.get_total_production() > 0.0001f) {
				fc.set_producer_payout_fraction(std::min(fc.get_total_consumption() / fc.get_total_production(), 1.0f));
			} else {
				fc.set_producer_payout_fraction(1.0f);
			}
		});
		state.world.for_each_factory_type([&](dcon::factory_type_id t) {
			auto o = state.world.factory_type_get_output(t);
			if(o)
			state.world.commodity_set_key_factory(o, t);
		});
	}

	float government_consumption(sys::state& state, dcon::nation_id n, dcon::commodity_id c) {
		auto overseas_factor = state.defines.province_overseas_penalty * float(state.world.nation_get_owned_province_count(n) - state.world.nation_get_central_province_count(n));
		auto o_adjust = 0.0f;
		if(overseas_factor > 0) {
			auto kf = state.world.commodity_get_key_factory(c);
			if(state.world.commodity_get_overseas_penalty(c)
			&& (state.world.commodity_get_is_available_from_start(c) || (kf && state.world.nation_get_active_building(n, kf)))) {
				o_adjust = overseas_factor;
			}
		}
		return (state.world.nation_get_army_demand(n, c) + state.world.nation_get_navy_demand(n, c) + state.world.nation_get_construction_demand(n, c) + o_adjust);
	}

	float nation_pop_consumption(sys::state& state, dcon::nation_id n, dcon::commodity_id c) {
		auto amount = 0.f;
		auto kf = state.world.commodity_get_key_factory(c);
		if(state.world.commodity_get_is_available_from_start(c) || (kf && state.world.nation_get_active_building(n, kf))) {
			state.world.for_each_pop_type([&](dcon::pop_type_id pt) {
				float needs = state.world.pop_type_get_life_needs(pt, c)
				+ state.world.pop_type_get_everyday_needs(pt, c)
				+ state.world.pop_type_get_luxury_needs(pt, c);
				amount += needs * state.world.nation_get_demographics(n, demographics::to_key(state, pt));
			});
		}
		return amount;
	}

	float nation_total_imports(sys::state& state, dcon::nation_id n) {
		float t_total = 0.0f;
		for(uint32_t k = 1; k < state.world.commodity_size(); ++k) {
		dcon::commodity_id cid{ dcon::commodity_id::value_base_t(k) };
			t_total += state.world.commodity_get_current_price(cid) * state.world.nation_get_imports(n, cid);
		}
		return t_total;
	}

	float pop_income(sys::state& state, dcon::pop_id p) {
		auto saved = state.world.pop_get_savings(p);
		if(saved <= 0.0f)
		return 0.0f;

		auto n = nations::owner_of_pop(state, p);
		auto const tax_eff = nations::tax_efficiency(state, n);
		auto strata = culture::pop_strata(state.world.pop_type_get_strata(state.world.pop_get_poptype(p)));
		switch(strata) {
			default:
			case culture::pop_strata::poor:
			return saved / std::max(0.0001f, (1.0f - tax_eff * float(state.world.nation_get_poor_tax(n)) / 100.0f));
			case culture::pop_strata::middle:
			return saved / std::max(0.0001f, (1.0f - tax_eff * float(state.world.nation_get_middle_tax(n)) / 100.0f));
			case culture::pop_strata::rich:
			return saved / std::max(0.0001f, (1.0f - tax_eff * float(state.world.nation_get_rich_tax(n)) / 100.0f));
		}
	}

	construction_status state_building_construction(sys::state& state, dcon::state_instance_id s, dcon::factory_type_id t) {
		for(auto st_con : state.world.state_instance_get_state_building_construction(s)) {
			if(st_con.get_type() == t) {
				auto& goods = state.world.factory_type_get_construction_costs(st_con.get_type());
				float cost_mod = economy_factory::factory_build_cost_modifier(state, st_con.get_nation(), st_con.get_is_pop_project());
				float total = 0.0f;
				float purchased = 0.0f;
				for(uint32_t i = 0; i < commodity_set::set_size; ++i) {
					total += goods.commodity_amounts[i] * cost_mod;
					purchased += std::clamp(st_con.get_purchased_goods().commodity_amounts[i], 0.f, goods.commodity_amounts[i] * cost_mod);
				}
				return construction_status{ total > 0.0f ? purchased / total : 0.0f, true };
			}
		}
		return construction_status{ 0.0f, false };
	}

	construction_status province_building_construction(sys::state& state, dcon::province_id p, province_building_type t) {
		for(auto pb_con : state.world.province_get_province_building_construction(p)) {
			if(pb_con.get_type() == uint8_t(t)) {
				float admin_eff = state.world.nation_get_administrative_efficiency(state.world.province_get_nation_from_province_ownership(p));
				float admin_cost_factor = pb_con.get_is_pop_project() ? 1.0f : 2.0f - admin_eff;

				float total = 0.0f;
				float purchased = 0.0f;
				for(uint32_t i = 0; i < commodity_set::set_size; ++i) {
					total += state.economy_definitions.building_definitions[int32_t(t)].cost.commodity_amounts[i] * admin_cost_factor;
					purchased += std::clamp(pb_con.get_purchased_goods().commodity_amounts[i], 0.0f, state.economy_definitions.building_definitions[int32_t(t)].cost.commodity_amounts[i] * admin_cost_factor);
				}
				return construction_status{ total > 0.0f ? purchased / total : 0.0f, true };
			}
		}
		return construction_status{ 0.0f, false };
	}

	float unit_construction_progress(sys::state& state, dcon::province_land_construction_id c) {

		float admin_eff = state.world.nation_get_administrative_efficiency(state.world.province_land_construction_get_nation(c));
		float admin_cost_factor = 2.0f - admin_eff;

		auto& goods = state.military_definitions.unit_base_definitions[state.world.province_land_construction_get_type(c)].build_cost;
		auto& cgoods = state.world.province_land_construction_get_purchased_goods(c);

		float total = 0.0f;
		float purchased = 0.0f;

		for(uint32_t i = 0; i < commodity_set::set_size; ++i) {
			total += goods.commodity_amounts[i] * admin_cost_factor;
			purchased += std::clamp(cgoods.commodity_amounts[i],0.0f, goods.commodity_amounts[i] * admin_cost_factor);
		}

		return total > 0.0f ? purchased / total : 0.0f;
	}

	float unit_construction_progress(sys::state& state, dcon::province_naval_construction_id c) {
		float admin_eff = state.world.nation_get_administrative_efficiency(state.world.province_naval_construction_get_nation(c));
		float admin_cost_factor = 2.0f - admin_eff;

		auto& goods = state.military_definitions.unit_base_definitions[state.world.province_naval_construction_get_type(c)].build_cost;
		auto& cgoods = state.world.province_naval_construction_get_purchased_goods(c);

		float total = 0.0f;
		float purchased = 0.0f;

		for(uint32_t i = 0; i < commodity_set::set_size; ++i) {
			total += goods.commodity_amounts[i] * admin_cost_factor;
			purchased += std::clamp(cgoods.commodity_amounts[i], 0.0f, goods.commodity_amounts[i] * admin_cost_factor);
		}

		return total > 0.0f ? purchased / total : 0.0f;
	}

	void resolve_constructions(sys::state& state) {
		for(uint32_t i = state.world.province_land_construction_size(); i-- > 0;) {
			auto c = fatten(state.world, dcon::province_land_construction_id{ dcon::province_land_construction_id::value_base_t(i) });

			float admin_eff = state.world.nation_get_administrative_efficiency(state.world.province_land_construction_get_nation(c));
			float admin_cost_factor = 2.0f - admin_eff;

			auto& base_cost = state.military_definitions.unit_base_definitions[c.get_type()].build_cost;
			auto& current_purchased = c.get_purchased_goods();
			float construction_time = float(state.military_definitions.unit_base_definitions[c.get_type()].build_time);

			bool all_finished = true;
			if(!(c.get_nation().get_is_player_controlled() && state.cheat_data.instant_army)) {
				for(uint32_t j = 0; j < commodity_set::set_size && all_finished; ++j) {
					if(base_cost.commodity_type[j]) {
						if(current_purchased.commodity_amounts[j] < base_cost.commodity_amounts[j] * admin_cost_factor) {
							all_finished = false;
						}
					} else {
						break;
					}
				}
			}

			if(all_finished) {
				auto pop_location = c.get_pop().get_province_from_pop_location();

				auto new_reg = military::create_new_regiment(state, c.get_nation(), c.get_type());
				auto a = fatten(state.world, state.world.create_army());

				a.set_controller_from_army_control(c.get_nation());
				state.world.try_create_army_membership(new_reg, a);
				state.world.try_create_regiment_source(new_reg, c.get_pop());
				military::army_arrives_in_province(state, a, pop_location, military::crossing_type::none);
				military::move_land_to_merge(state, c.get_nation(), a, pop_location, c.get_template_province());

				if(c.get_nation() == state.local_player_nation) {
					notification::post(state, notification::message{ [](sys::state& state, text::layout_base& contents) {
							text::add_line(state, contents, "amsg_army_built");
						},
						"amsg_army_built",
						c.get_nation(), dcon::nation_id{ }, dcon::nation_id{ },
						sys::message_base_type::army_built
					});
				}

				state.world.delete_province_land_construction(c);
			}
		}

		province::for_each_land_province(state, [&](dcon::province_id p) {
			auto rng = state.world.province_get_province_naval_construction(p);
			if(rng.begin() != rng.end()) {
				auto c = *(rng.begin());

				float admin_eff = state.world.nation_get_administrative_efficiency(state.world.province_naval_construction_get_nation(c));
				float admin_cost_factor = 2.0f - admin_eff;

				auto& base_cost = state.military_definitions.unit_base_definitions[c.get_type()].build_cost;
				auto& current_purchased = c.get_purchased_goods();
				float construction_time = float(state.military_definitions.unit_base_definitions[c.get_type()].build_time);

				bool all_finished = true;
				if(!(c.get_nation().get_is_player_controlled() && state.cheat_data.instant_navy)) {
					for(uint32_t i = 0; i < commodity_set::set_size && all_finished; ++i) {
						if(base_cost.commodity_type[i]) {
							if(current_purchased.commodity_amounts[i] < base_cost.commodity_amounts[i] * admin_cost_factor) {
								all_finished = false;
							}
						} else {
							break;
						}
					}
				}

				if(all_finished) {
					auto new_ship = military::create_new_ship(state, c.get_nation(), c.get_type());
					auto a = fatten(state.world, state.world.create_navy());
					a.set_controller_from_navy_control(c.get_nation());
					a.set_location_from_navy_location(p);
					state.world.try_create_navy_membership(new_ship, a);
					military::move_navy_to_merge(state, c.get_nation(), a, c.get_province(), c.get_template_province());

					if(c.get_nation() == state.local_player_nation) {
						notification::post(state, notification::message{ [](sys::state& state, text::layout_base& contents) {
								text::add_line(state, contents, "amsg_navy_built");
							},
							"amsg_navy_built",
							state.local_player_nation, dcon::nation_id{}, dcon::nation_id{},
							sys::message_base_type::navy_built
						});
					}

					state.world.delete_province_naval_construction(c);
				}
			}
		});

		for(uint32_t i = state.world.province_building_construction_size(); i-- > 0;) {
			dcon::province_building_construction_id c{ dcon::province_building_construction_id::value_base_t(i) };
			auto for_province = state.world.province_building_construction_get_province(c);

			float admin_eff = state.world.nation_get_administrative_efficiency(state.world.province_building_construction_get_nation(c));
			float admin_cost_factor = state.world.province_building_construction_get_is_pop_project(c) ? 1.0f : 2.0f - admin_eff;

			auto t = province_building_type(state.world.province_building_construction_get_type(c));
			auto& base_cost = state.economy_definitions.building_definitions[int32_t(t)].cost;
			auto& current_purchased = state.world.province_building_construction_get_purchased_goods(c);
			bool all_finished = true;

			for(uint32_t j = 0; j < commodity_set::set_size && all_finished; ++j) {
				if(base_cost.commodity_type[j]) {
					if(current_purchased.commodity_amounts[j] < base_cost.commodity_amounts[j] * admin_cost_factor) {
						all_finished = false;
					}
				} else {
					break;
				}
			}

			if(all_finished) {
				if(state.world.province_get_building_level(for_province, t) < state.world.nation_get_max_building_level(state.world.province_get_nation_from_province_ownership(for_province), t)) {
					state.world.province_get_building_level(for_province, t) += 1;

					if(t == province_building_type::railroad) {
						/* Notify the railroad mesh builder to update the railroads! */
						state.railroad_built.store(true, std::memory_order::release);
					}

					if(state.world.province_building_construction_get_nation(c) == state.local_player_nation) {
						switch(t) {
						case province_building_type::naval_base:
							notification::post(state, notification::message{ [](sys::state& state, text::layout_base& contents) {
									text::add_line(state, contents, "amsg_naval_base_complete");
								},
								"amsg_naval_base_complete",
								state.local_player_nation, dcon::nation_id{}, dcon::nation_id{},
								sys::message_base_type::naval_base_complete
							});
							break;
						case province_building_type::fort:
							notification::post(state, notification::message{ [](sys::state& state, text::layout_base& contents) {
									text::add_line(state, contents, "amsg_fort_complete");
								},
								"amsg_fort_complete",
								state.local_player_nation, dcon::nation_id{}, dcon::nation_id{},
								sys::message_base_type::fort_complete
							});
							break;
						case province_building_type::railroad:
							notification::post(state, notification::message{ [](sys::state& state, text::layout_base& contents) {
									text::add_line(state, contents, "amsg_rr_complete");
								},
								"amsg_rr_complete",
								state.local_player_nation, dcon::nation_id{}, dcon::nation_id{},
								sys::message_base_type::rr_complete
							});
							break;
						default:
							break;
						}
						news::news_scope scope;
						scope.type = news::news_generator_type::construction_complete;
						//value 0,0 is province id
						scope.values[0][1] = state.world.province_get_building_level(for_province, t);
						scope.tags[0][0] = state.world.nation_get_identity_from_identity_holder(state.world.province_building_construction_get_nation(c));
						scope.strings[0][0] = state.world.province_get_name(for_province);
						/*auto it = state.key_to_text_sequence.find(economy::province_building_type_get_name(t));
						if(it != state.key_to_text_sequence.end()) {
							scope.strings[0][1] = it->second;
							scope.strings[0][2] = it->second;
						}*/
						scope.dates[0][0] = state.current_date;
						news::collect_news_scope(state, scope);
					}
				}
				state.world.delete_province_building_construction(c);
			}
		}

		for(uint32_t i = state.world.state_building_construction_size(); i-- > 0;) {
			dcon::state_building_construction_id c{ dcon::state_building_construction_id::value_base_t(i) };
			auto n = state.world.state_building_construction_get_nation(c);
			auto type = state.world.state_building_construction_get_type(c);
			auto& base_cost = state.world.factory_type_get_construction_costs(type);
			auto& current_purchased = state.world.state_building_construction_get_purchased_goods(c);
			float cost_mod = economy_factory::factory_build_cost_modifier(state, n, state.world.state_building_construction_get_is_pop_project(c));
			bool all_finished = true;
			if(!(state.world.nation_get_is_player_controlled(n) && state.cheat_data.instant_industry)) {
				for(uint32_t j = 0; j < commodity_set::set_size && all_finished; ++j) {
					if(base_cost.commodity_type[j]) {
						if(current_purchased.commodity_amounts[j] < base_cost.commodity_amounts[j] * cost_mod) {
							all_finished = false;
						}
					} else {
						break;
					}
				}
			}
			if(all_finished) {
				economy_factory::add_factory_level_to_state(state, state.world.state_building_construction_get_state(c), type, state.world.state_building_construction_get_is_upgrade(c));
				if(state.world.state_building_construction_get_nation(c) == state.local_player_nation
				&& !state.world.state_building_construction_get_is_pop_project(c)) {
					notification::post(state, notification::message{ [](sys::state& state, text::layout_base& contents) {
							text::add_line(state, contents, "amsg_factory_complete");
						},
						"amsg_factory_complete",
						state.world.state_building_construction_get_nation(c), dcon::nation_id{}, dcon::nation_id{},
						sys::message_base_type::factory_complete
					});
				}
				state.world.delete_state_building_construction(c);
			}
		}
	}
	
	void bound_budget_settings(sys::state& state, dcon::nation_id n) {
		{
			auto min_tariff = int32_t(100.0f * state.world.nation_get_modifier_values(n, sys::national_mod_offsets::min_tariff));
			auto max_tariff = int32_t(100.0f * state.world.nation_get_modifier_values(n, sys::national_mod_offsets::max_tariff));
			min_tariff = std::max(min_tariff, 0);
			max_tariff = std::max(min_tariff, max_tariff <= 0 ? 100 : max_tariff);
			auto& tariff = state.world.nation_get_tariffs(n);
			tariff = int8_t(std::clamp(int32_t(tariff), min_tariff, max_tariff));
		}
		{
			auto min_tax = int32_t(100.0f * state.world.nation_get_modifier_values(n, sys::national_mod_offsets::min_tax));
			auto max_tax = int32_t(100.0f * state.world.nation_get_modifier_values(n, sys::national_mod_offsets::max_tax));
			min_tax = std::max(min_tax, 0);
			max_tax = std::max(min_tax, max_tax <= 0 ? 100 : max_tax);
			auto& ptax = state.world.nation_get_poor_tax(n);
			ptax = int8_t(std::clamp(int32_t(ptax), min_tax, max_tax));
			auto& mtax = state.world.nation_get_middle_tax(n);
			mtax = int8_t(std::clamp(int32_t(mtax), min_tax, max_tax));
			auto& rtax = state.world.nation_get_rich_tax(n);
			rtax = int8_t(std::clamp(int32_t(rtax), min_tax, max_tax));
		}
		{
			auto min_spend = int32_t(100.0f * state.world.nation_get_modifier_values(n, sys::national_mod_offsets::min_military_spending));
			auto max_spend = int32_t(100.0f * state.world.nation_get_modifier_values(n, sys::national_mod_offsets::max_military_spending));
			min_spend = std::max(min_spend, 0);
			max_spend = std::max(min_spend, max_spend <= 0 ? 100 : max_spend);
			auto& v = state.world.nation_get_military_spending(n);
			v = int8_t(std::clamp(int32_t(v), min_spend, max_spend));
		}
		{
			auto min_spend = int32_t(100.0f * state.world.nation_get_modifier_values(n, sys::national_mod_offsets::min_social_spending));
			auto max_spend = int32_t(100.0f * state.world.nation_get_modifier_values(n, sys::national_mod_offsets::max_social_spending));
			min_spend = std::max(min_spend, 0);
			max_spend = std::max(min_spend, max_spend <= 0 ? 100 : max_spend);
			auto& v = state.world.nation_get_social_spending(n);
			v = int8_t(std::clamp(int32_t(v), min_spend, max_spend));
		}
		{
			auto min_spend = int32_t(100.0f * state.world.nation_get_modifier_values(n, sys::national_mod_offsets::min_military_spending));
			auto max_spend = int32_t(100.0f * state.world.nation_get_modifier_values(n, sys::national_mod_offsets::max_military_spending));
			min_spend = std::max(min_spend, 0);
			max_spend = std::max(min_spend, max_spend <= 0 ? 100 : max_spend);
			auto& v = state.world.nation_get_military_spending(n);
			v = int8_t(std::clamp(int32_t(v), min_spend, max_spend));
		}
		{
			auto min_spend = int32_t(100.0f * state.world.nation_get_modifier_values(n, sys::national_mod_offsets::min_domestic_investment));
			auto max_spend = int32_t(100.0f * state.world.nation_get_modifier_values(n, sys::national_mod_offsets::max_domestic_investment));
			min_spend = std::max(min_spend, 0);
			max_spend = std::max(min_spend, max_spend <= 0 ? 100 : max_spend);
			auto& v = state.world.nation_get_domestic_investment_spending(n);
			v = int8_t(std::clamp(int32_t(v), min_spend, max_spend));
		}
		{
			auto min_spend = std::max(0, int32_t(100.0f * state.defines.trade_cap_low_limit_constructions));
			auto max_spend = int32_t(100.f);
			min_spend = std::max(min_spend, 0);
			max_spend = std::max(min_spend, max_spend <= 0 ? 100 : max_spend);
			auto& v = state.world.nation_get_construction_spending(n);
			v = int8_t(std::clamp(int32_t(v), min_spend, max_spend));
		}
		{
			auto min_spend = std::max(0, int32_t(100.0f * state.defines.trade_cap_low_limit_land));
			auto max_spend = int32_t(100.f);
			min_spend = std::max(min_spend, 0);
			max_spend = std::max(min_spend, max_spend <= 0 ? 100 : max_spend);
			auto& v = state.world.nation_get_land_spending(n);
			v = int8_t(std::clamp(int32_t(v), min_spend, max_spend));
		}
		{
			auto min_spend = std::max(0, int32_t(100.0f * state.defines.trade_cap_low_limit_naval));
			auto max_spend = int32_t(100.f);
			min_spend = std::max(min_spend, 0);
			max_spend = std::max(min_spend, max_spend <= 0 ? 100 : max_spend);
			auto& v = state.world.nation_get_naval_spending(n);
			v = int8_t(std::clamp(int32_t(v), min_spend, max_spend));
		}
		{
			auto min_spend = int32_t(0);
			auto max_spend = int32_t(100);
			auto& v = state.world.nation_get_overseas_spending(n);
			v = int8_t(std::clamp(int32_t(v), min_spend, max_spend));
		}
		{
			auto min_spend = int32_t(0);
			auto max_spend = int32_t(100);
			auto& v = state.world.nation_get_administrative_spending(n);
			v = int8_t(std::clamp(int32_t(v), min_spend, max_spend));
		}
		{
			auto min_spend = int32_t(0);
			auto max_spend = int32_t(100);
			auto& v = state.world.nation_get_education_spending(n);
			v = int8_t(std::clamp(int32_t(v), min_spend, max_spend));
		}
	}

	dcon::modifier_id get_province_selector_modifier(sys::state& state) {
		return state.economy_definitions.selector_modifier;
	}

	dcon::modifier_id get_province_immigrator_modifier(sys::state& state) {
		return state.economy_definitions.immigrator_modifier;
	}

	void go_bankrupt(sys::state& state, dcon::nation_id n) {
		auto& debt = state.world.nation_get_stockpiles(n, economy::money);

		/*
		 If a nation cannot pay and the amount it owes is less than define:SMALL_DEBT_LIMIT, the nation it owes money to gets an on_debtor_default_small event (with the nation defaulting in the from slot). Otherwise, the event is pulled from on_debtor_default. The nation then goes bankrupt. It receives the bad_debter modifier for define:BANKRUPCY_EXTERNAL_LOAN_YEARS years (if it goes bankrupt again within this period, creditors receive an on_debtor_default_second event). It receives the in_bankrupcy modifier for define:BANKRUPCY_DURATION days. Its prestige is reduced by a factor of define:BANKRUPCY_FACTOR, and each of its pops has their militancy increase by 2.
		*/
		auto existing_br = state.world.nation_get_bankrupt_until(n);
		if(existing_br && state.current_date < existing_br) {
			for(auto gn : state.great_nations) {
				if(gn.nation && gn.nation != n) {
					event::fire_fixed_event(state, state.national_definitions.on_debtor_default_second, trigger::to_generic(gn.nation), event::slot_type::nation, gn.nation, trigger::to_generic(n), event::slot_type::nation);
				}
			}
		} else if(debt >= -state.defines.small_debt_limit) {
			for(auto gn : state.great_nations) {
				if(gn.nation && gn.nation != n) {
					event::fire_fixed_event(state, state.national_definitions.on_debtor_default_small, trigger::to_generic(gn.nation), event::slot_type::nation, gn.nation, trigger::to_generic(n), event::slot_type::nation);
				}
			}
		} else {
			for(auto gn : state.great_nations) {
				if(gn.nation && gn.nation != n) {
					event::fire_fixed_event(state, state.national_definitions.on_debtor_default, trigger::to_generic(gn.nation), event::slot_type::nation, gn.nation, trigger::to_generic(n), event::slot_type::nation);
				}
			}
		}

		sys::add_modifier_to_nation(state, n, state.national_definitions.static_modifiers[uint8_t(nations::static_modifier::in_bankrupcy)], state.current_date + int32_t(state.defines.bankrupcy_duration * 365));
		sys::add_modifier_to_nation(state, n, state.national_definitions.static_modifiers[uint8_t(nations::static_modifier::bad_debter)], state.current_date + int32_t(state.defines.bankruptcy_external_loan_years * 365));

		debt = 0.0f;
		state.world.nation_set_is_debt_spending(n, false);
		state.world.nation_set_bankrupt_until(n, state.current_date + int32_t(state.defines.bankrupcy_duration * 365));

		notification::post(state, notification::message{
			[n](sys::state& state, text::layout_base& contents) {
				text::add_line(state, contents, "msg_bankruptcy_1", text::variable_type::x, n);
			},
			"msg_bankruptcy_title",
		n, dcon::nation_id{}, dcon::nation_id{},
			sys::message_base_type::bankruptcy
		});
	}

	commodity_production_type get_commodity_production_type(sys::state& state, dcon::commodity_id c) {
		auto commodity = dcon::fatten(state.world, c);
		if(commodity.get_rgo_amount() > 0.f && (commodity.get_artisan_output_amount() > 0.f || commodity.get_key_factory()))
			return commodity_production_type::both;
		else if(commodity.get_key_factory())
			return commodity_production_type::derivative;
		else
			return commodity_production_type::primary;
	}

	float commodity_set_effective_cost(sys::state& state, dcon::nation_id n, economy::commodity_set const& cset) {
		float total = 0.f;
		for(uint32_t i = 0; i < economy::commodity_set::set_size && cset.commodity_type[i]; ++i) {
			total += cset.commodity_amounts[i] * economy::commodity_effective_price(state, n, cset.commodity_type[i]);
		}
		return total;
	}

	float commodity_set_effective_cost(sys::state& state, dcon::nation_id n, economy::small_commodity_set const& cset) {
		float total = 0.f;
		for(uint32_t i = 0; i < economy::small_commodity_set::set_size && cset.commodity_type[i]; ++i) {
			total += cset.commodity_amounts[i] * economy::commodity_effective_price(state, n, cset.commodity_type[i]);
		}
		return total;
	}

	float commodity_set_total_satisfaction(sys::state& state, dcon::nation_id n, economy::commodity_set const& cset) {
		float total = 1.f;
		for(uint32_t i = 0; i < economy::commodity_set::set_size && cset.commodity_type[i]; ++i) {
			total = std::min(total, state.world.nation_get_demand_satisfaction(n, cset.commodity_type[i]));
		}
		return total;
	}

	float commodity_set_total_satisfaction(sys::state& state, dcon::nation_id n, economy::small_commodity_set const& cset) {
		float total = 1.f;
		for(uint32_t i = 0; i < economy::small_commodity_set::set_size && cset.commodity_type[i]; ++i) {
			total = std::min(total, state.world.nation_get_demand_satisfaction(n, cset.commodity_type[i]));
		}
		return total;
	}

	float commodity_market_activity(sys::state& state, dcon::nation_id n, dcon::commodity_id c) {
		float produced = state.world.nation_get_domestic_market_pool(n, c);
		float consumed = state.world.nation_get_real_demand(n, c) * state.world.nation_get_demand_satisfaction(n, c);
		return produced + consumed;
	}
} // namespace economy
