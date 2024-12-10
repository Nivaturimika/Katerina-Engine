#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/benchmark/catch_constructor.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "system_state.hpp"
#include "dcon_generated.hpp"

// Old approach
void populate_ln_en_lx_1(sys::state& state, dcon::nation_id n,
	ve::vectorizable_buffer<float, dcon::pop_type_id>& ln_max,
	ve::vectorizable_buffer<float, dcon::pop_type_id>& en_max,
	ve::vectorizable_buffer<float, dcon::pop_type_id>& lx_max) {
	state.world.for_each_pop_type([&](dcon::pop_type_id pt) {
		float ln_total = 0.0f;
		float en_total = 0.0f;
		float lx_total = 0.0f;

		float ln_value = 0.0f;
		float en_value = 0.0f;
		float lx_value = 0.0f;
		for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
			dcon::commodity_id c{ dcon::commodity_id::value_base_t(i) };
			auto kf = state.world.commodity_get_key_factory(c);
			if(state.world.commodity_get_is_available_from_start(c)
			|| (kf && state.world.nation_get_active_building(n, kf))) {
				auto sat = state.world.nation_get_demand_satisfaction(n, c);
				//
				auto ln_val = state.world.pop_type_get_life_needs(pt, c)
					* state.world.commodity_get_is_life_need(c);
				ln_value += ln_val;
				ln_total += ln_val * sat;
				//
				auto en_val = state.world.pop_type_get_everyday_needs(pt, c)
					* state.world.commodity_get_is_everyday_need(c);
				en_value += en_val;
				en_total += en_val * sat;
				//
				auto lx_val = state.world.pop_type_get_luxury_needs(pt, c)
					* state.world.commodity_get_is_luxury_need(c);
				lx_value += lx_val;
				lx_total += lx_val * sat;
			}
		}
		ln_max.get(pt) = ln_total > 0.f ? ln_total / ln_value : 1.f;
		en_max.get(pt) = en_total > 0.f ? en_total / en_value : 1.f;
		lx_max.get(pt) = lx_total > 0.f ? lx_total / lx_value : 1.f;
	});
}

// New (simdified) code
void populate_ln_en_lx_2(sys::state& state, dcon::nation_id n,
	ve::vectorizable_buffer<float, dcon::pop_type_id>& ln_max,
	ve::vectorizable_buffer<float, dcon::pop_type_id>& en_max,
	ve::vectorizable_buffer<float, dcon::pop_type_id>& lx_max) {
	state.world.execute_serial_over_pop_type([&](auto ids) {
		ve::fp_vector ln_total(0.0f);
		ve::fp_vector en_total(0.0f);
		ve::fp_vector lx_total(0.0f);
		ve::fp_vector ln_value(0.0f);
		ve::fp_vector en_value(0.0f);
		ve::fp_vector lx_value(0.0f);
		for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
			dcon::commodity_id c{ dcon::commodity_id::value_base_t(i) };
			auto kf = state.world.commodity_get_key_factory(c);
			if(state.world.commodity_get_is_available_from_start(c)
			|| (kf && state.world.nation_get_active_building(n, kf))) {
				auto sat = state.world.nation_get_demand_satisfaction(n, c);
				//
				auto ln_val = state.world.pop_type_get_life_needs(ids, c)
					* state.world.commodity_get_is_life_need(c);
				ln_value = ln_value + ln_val;
				ln_total = ln_total + ln_val * sat;
				//
				auto en_val = state.world.pop_type_get_everyday_needs(ids, c)
					* state.world.commodity_get_is_everyday_need(c);
				en_value = en_value + en_val;
				en_total = en_total + en_val * sat;
				//
				auto lx_val = state.world.pop_type_get_luxury_needs(ids, c)
					* state.world.commodity_get_is_luxury_need(c);
				lx_value = lx_value + lx_val;
				lx_total = lx_total + lx_val * sat;
			}
		}

		// Series of sums,
		// S = (x1 + x2 + x3 + x4 + ... + xn)
		// where x = y * z; henceforth:
		// S = (y1 * z1 + y2 * z2 + y3 * z3 + y4 * z4 + ... + yn * zn)
		// 

		ln_max.get(ids) = ve::select(ln_total > 0.f, ln_total / ln_value, 1.f);
		en_max.get(ids) = ve::select(en_total > 0.f, en_total / en_value, 1.f);
		lx_max.get(ids) = ve::select(lx_total > 0.f, lx_total / lx_value, 1.f);
	});
}

// Old approach
void populate_needs_costs_1(sys::state& state, dcon::nation_id n, float base_demand, float invention_factor) {
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
				float effective_price = economy::commodity_effective_price(state, n, c);
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

// New (simdified) code
void populate_needs_costs_2(sys::state& state, dcon::nation_id n, float base_demand, float invention_factor) {
	ve::fp_vector ln_mul[] = {
		ve::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::poor_life_needs) + 1.0f),
		ve::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::middle_life_needs) + 1.0f),
		ve::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::rich_life_needs) + 1.0f)
	};
	ve::fp_vector en_mul[] = {
		ve::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::poor_everyday_needs) + 1.0f),
		ve::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::middle_everyday_needs) + 1.0f),
		ve::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::rich_everyday_needs) + 1.0f)
	};
	ve::fp_vector lx_mul[] = {
		ve::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::poor_luxury_needs) + 1.0f),
		ve::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::middle_luxury_needs) + 1.0f),
		ve::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::rich_luxury_needs) + 1.0f)
	};
	float admin_eff = state.world.nation_get_administrative_efficiency(n);
	float admin_cost_factor = 2.0f - admin_eff;
	state.world.execute_serial_over_pop_type([&](auto ids) {
		ve::fp_vector ln_total(0.f);
		ve::fp_vector en_total(0.f);
		ve::fp_vector lx_total(0.f);
		for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
			dcon::commodity_id c{ dcon::commodity_id::value_base_t(i) };
			auto kf = state.world.commodity_get_key_factory(c);
			if(state.world.commodity_get_is_available_from_start(c)
			|| (kf && state.world.nation_get_active_building(n, kf))) {
				float effective_price = economy::commodity_effective_price(state, n, c);
				ve::fp_vector nmod = admin_cost_factor * effective_price * base_demand;
				ln_total = ln_total + (state.world.pop_type_get_life_needs(ids, c) * nmod);
				en_total = en_total + (state.world.pop_type_get_everyday_needs(ids, c) * nmod);
				lx_total = lx_total + (state.world.pop_type_get_luxury_needs(ids, c) * nmod);
			}
		}
		ve::int_vector strata = state.world.pop_type_get_strata(ids);
		ve::mask_vector mask_poor = (strata == 0);
		ve::mask_vector mask_mid = (strata == 1);
		ln_total = ln_total
			* ve::select(mask_poor, ln_mul[0], ve::select(mask_mid, ln_mul[1], ln_mul[2]));
		en_total = en_total * invention_factor
			* ve::select(mask_poor, en_mul[0], ve::select(mask_mid, en_mul[1], en_mul[2]));
		lx_total = lx_total * invention_factor
			* ve::select(mask_poor, lx_mul[0], ve::select(mask_mid, lx_mul[1], lx_mul[2]));
		ve::apply([&](dcon::pop_type_id pt, float ln, float ev, float lx) {
			state.world.nation_set_life_needs_costs(n, pt, ln);
			state.world.nation_set_everyday_needs_costs(n, pt, ev);
			state.world.nation_set_luxury_needs_costs(n, pt, lx);
			assert(std::isfinite(state.world.nation_get_life_needs_costs(n, pt)) && state.world.nation_get_life_needs_costs(n, pt) >= 0.f);
			assert(std::isfinite(state.world.nation_get_everyday_needs_costs(n, pt)) && state.world.nation_get_everyday_needs_costs(n, pt) >= 0.f);
			assert(std::isfinite(state.world.nation_get_luxury_needs_costs(n, pt)) && state.world.nation_get_luxury_needs_costs(n, pt) >= 0.f);
		}, ids, ln_total, en_total, lx_total);
	});
}

// Old method
void nation_spending_calc_1(sys::state& state, dcon::nation_id n) {
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
			refund += val * (1.f - sat) * spending_level * state.world.commodity_get_current_price(c);
			total += val;
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
			auto val = state.world.nation_get_construction_demand(n, c);
			assert(sat >= 0.f && sat <= 1.f);
			assert(val >= 0.f);
			refund += val * (1.f - sat) * spending_level * state.world.commodity_get_current_price(c);
			total += val;
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
}

//
// TEST CASES
//
sys::state& load_testing_scenario_file();
TEST_CASE("bench_simdify") {
	BENCHMARK_ADVANCED("populate-ln-en-lx-1")(Catch::Benchmark::Chronometer meter) {
		auto& state = load_testing_scenario_file();
		// buffers
		auto ln_max = state.world.pop_type_make_vectorizable_float_buffer();
		auto en_max = state.world.pop_type_make_vectorizable_float_buffer();
		auto lx_max = state.world.pop_type_make_vectorizable_float_buffer();
		meter.measure([&] {
			for(const auto n : state.world.in_nation) {
				populate_ln_en_lx_1(state, n, ln_max, en_max, lx_max);
			}
		});
		// prevent optimizing away
		float s = 0.f;
		for(uint32_t i = 0; i < state.world.pop_type_size(); ++i) {
			s += ln_max.get(dcon::pop_type_id(i))
				+ en_max.get(dcon::pop_type_id(i))
				+ lx_max.get(dcon::pop_type_id(i));
		}
		return s;
	};
	BENCHMARK_ADVANCED("populate-ln-en-lx-2")(Catch::Benchmark::Chronometer meter) {
		auto& state = load_testing_scenario_file();
		// buffers
		auto ln_max = state.world.pop_type_make_vectorizable_float_buffer();
		auto en_max = state.world.pop_type_make_vectorizable_float_buffer();
		auto lx_max = state.world.pop_type_make_vectorizable_float_buffer();
		meter.measure([&] {
			for(const auto n : state.world.in_nation) {
				populate_ln_en_lx_2(state, n, ln_max, en_max, lx_max);
			}
		});
		// prevent optimizing away
		float s = 0.f;
		for(uint32_t i = 0; i < state.world.pop_type_size(); ++i) {
			s += ln_max.get(dcon::pop_type_id(i))
				+ en_max.get(dcon::pop_type_id(i))
				+ lx_max.get(dcon::pop_type_id(i));
		}
		return s;
	};
}

TEST_CASE("bench_simdify2") {
	BENCHMARK_ADVANCED("populate-needs-costs-1")(Catch::Benchmark::Chronometer meter) {
		auto& state = load_testing_scenario_file();
		// buffers
		meter.measure([&] {
			for(const auto n : state.world.in_nation) {
				populate_needs_costs_1(state, n, n.get_administrative_efficiency(), n.get_capital_ship_score());
			}
		});
	};
	BENCHMARK_ADVANCED("populate-needs-costs-2")(Catch::Benchmark::Chronometer meter) {
		auto& state = load_testing_scenario_file();
		// buffers
		auto ln_max = state.world.pop_type_make_vectorizable_float_buffer();
		auto en_max = state.world.pop_type_make_vectorizable_float_buffer();
		auto lx_max = state.world.pop_type_make_vectorizable_float_buffer();
		meter.measure([&] {
			for(const auto n : state.world.in_nation) {
				populate_needs_costs_2(state, n, n.get_administrative_efficiency(), n.get_capital_ship_score());
			}
		});
	};
}
