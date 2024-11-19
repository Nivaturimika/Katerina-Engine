#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/benchmark/catch_constructor.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "ve.hpp"

TEST_CASE("bench_cache_locality_interleave") {
	BENCHMARK("normal sum") {
		constexpr uint32_t num_values = 1000;
		constexpr uint32_t num_subvalues = 3000;
		struct values_state {
			alignas(64) ve::fp_vector val_a[num_values * num_subvalues];
			alignas(64) ve::fp_vector val_b[num_values * num_subvalues];
			alignas(64) ve::fp_vector factor[num_values * num_subvalues];
			alignas(64) ve::fp_vector final_a[num_values];
			alignas(64) ve::fp_vector final_b[num_values];
		};
		auto* state = new values_state();
		// clear
		for(uint32_t i = 0; i < num_values; ++i) {
			state->final_a[i] = ve::fp_vector{};
		}
		for(uint32_t i = 0; i < num_values; ++i) {
			state->final_b[i] = ve::fp_vector{};
		}
		// run
		for(uint32_t i = 0; i < num_values; ++i) {
			for(uint32_t j = 0; j < num_subvalues; ++j) {
				auto v_offset = i * num_subvalues + j;
				state->final_a[i] = state->final_a[i] + state->val_a[v_offset] * state->factor[v_offset];
			}
		}
		for(uint32_t i = 0; i < num_values; ++i) {
			for(uint32_t j = 0; j < num_subvalues; ++j) {
				auto v_offset = i * num_subvalues + j;
				state->final_b[i] = state->final_b[i] + state->val_b[v_offset] * state->factor[v_offset];
			}
		}
		//
		delete state;
	};

	BENCHMARK("optimized sum") {
		constexpr uint32_t num_values = 1000;
		constexpr uint32_t num_subvalues = 3000;
		struct values_state {
			alignas(64) ve::fp_vector val_ab[num_values * num_subvalues * 2];
			alignas(64) ve::fp_vector factor[num_values * num_subvalues];
			alignas(64) ve::fp_vector final_a[num_values];
			alignas(64) ve::fp_vector final_b[num_values];
		};
		auto* state = new values_state();
		// clear
		for(uint32_t i = 0; i < num_values; ++i) {
			state->final_a[i] = ve::fp_vector{};
		}
		for(uint32_t i = 0; i < num_values; ++i) {
			state->final_b[i] = ve::fp_vector{};
		}
		// run
		for(uint32_t i = 0; i < num_values; ++i) {
			for(uint32_t j = 0; j < num_subvalues; ++j) {
				auto v_offset = i * num_subvalues + j;
				state->final_a[i] = state->final_a[i] + state->val_ab[v_offset * 2 + 0] * state->factor[v_offset];
				state->final_b[i] = state->final_b[i] + state->val_ab[v_offset * 2 + 1] * state->factor[v_offset];
			}
		}
		//
		delete state;
	};
}
