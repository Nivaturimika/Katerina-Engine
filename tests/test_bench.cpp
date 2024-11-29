#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/benchmark/catch_constructor.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "ve.hpp"
#include "math_fns.hpp"

//
// based on The "dead reckoning" signed distance transform
// Grevera, George J. (2004) Computer Vision and Image Understanding 95 pages 317–333
//
constexpr int magnification_factor = 4;
constexpr int dr_size = 64 * magnification_factor;
constexpr float rt_2 = 1.41421356237309504f;
void dead_reckoning_1(float distance_map[dr_size * dr_size], bool const in_map[dr_size * dr_size]) {
	for(uint32_t i = 0; i < dr_size * dr_size; ++i) {
		distance_map[i] = std::numeric_limits<float>::infinity();
	}
	alignas(64) int16_t yborder[dr_size * dr_size] = { 0 };
	alignas(64) int16_t xborder[dr_size * dr_size] = { 0 };
	for(uint32_t j = 1; j < dr_size - 1; ++j) {
		for(uint32_t i = 1; i < dr_size - 1; ++i) {
			if(in_map[i - 1 + dr_size * j] != in_map[i + dr_size * j]
			|| in_map[i + 1 + dr_size * j] != in_map[i + dr_size * j]
			|| in_map[i + dr_size * (j + 1)] != in_map[i + dr_size * j]
			|| in_map[i + dr_size * (j - 1)] != in_map[i + dr_size * j]) {
				distance_map[i + dr_size * j] = 0.0f;
				yborder[i + dr_size * j] = int16_t(j);
				xborder[i + dr_size * j] = int16_t(i);
			}
		}
	}
	for(uint32_t j = 1; j < dr_size - 1; ++j) {
		for(uint32_t i = 1; i < dr_size - 1; ++i) {
			const int16_t yvalues[4] = {
				yborder[(i - 1) + dr_size * (j - 1)],
				yborder[(i + 0) + dr_size * (j - 1)],
				yborder[(i + 1) + dr_size * (j - 1)],
				yborder[(i - 1) + dr_size * (j + 0)]
			};
			const int16_t xvalues[4] = {
				xborder[(i - 1) + dr_size * (j - 1)],
				xborder[(i + 0) + dr_size * (j - 1)],
				xborder[(i + 1) + dr_size * (j - 1)],
				xborder[(i - 1) + dr_size * (j + 0)]
			};
			const bool masks[4] = {
				distance_map[(i - 1) + dr_size * (j - 1)] + rt_2 < distance_map[(i) + dr_size * (j)],
				distance_map[(i) + dr_size * (j - 1)] + 1.0f < distance_map[(i) + dr_size * (j)],
				distance_map[(i + 1) + dr_size * (j - 1)] + rt_2 < distance_map[(i) + dr_size * (j)],
				distance_map[(i - 1) + dr_size * (j)] + 1.0f < distance_map[(i) + dr_size * (j)]
			};
			if(masks[0]) {
				// inherit from previous (i - 1, j - 1)
				yborder[i + dr_size * j] = yvalues[0];
				xborder[i + dr_size * j] = xvalues[0];
				distance_map[i + dr_size * j] = math::sqrti(
					(i - xvalues[0]) * (i - xvalues[0])
					+ (j - yvalues[0]) * (j - yvalues[0])
				);
			} else if(masks[1]) {
				// inherit from previous (i, j - 1)
				yborder[i + dr_size * j] = yvalues[1];
				xborder[i + dr_size * j] = xvalues[1];
				distance_map[i + dr_size * j] = math::sqrti(
					(i - xvalues[1]) * (i - xvalues[1])
					+ (j - yvalues[1]) * (j - yvalues[1])
				);
			} else if(masks[2]) {
				// inherit from previous (i + 1, j - 1)
				yborder[i + dr_size * j] = yvalues[2];
				xborder[i + dr_size * j] = xvalues[2];
				distance_map[i + dr_size * j] = math::sqrti(
					(i - xvalues[2]) * (i - xvalues[2])
					+ (j - yvalues[2]) * (j - yvalues[2])
				);
			} else if(masks[3]) {
				// inherit from previous (i - 1, j)
				yborder[i + dr_size * j] = yvalues[3];
				xborder[i + dr_size * j] = xvalues[3];
				distance_map[i + dr_size * j] = math::sqrti(
					(i - xvalues[3]) * (i - xvalues[3])
					+ (j - yvalues[3]) * (j - yvalues[3])
				);
			}
		}
	}
	for(uint32_t j = dr_size - 2; j > 0; --j) {
		for(uint32_t i = dr_size - 2; i > 0; --i) {
			if(distance_map[(i + 1) + dr_size * (j)] + 1.0f < distance_map[(i) + dr_size * (j)]) {
				yborder[i + dr_size * j] = yborder[(i + 1) + dr_size * (j)];
				xborder[i + dr_size * j] = xborder[(i + 1) + dr_size * (j)];
				distance_map[(i) + dr_size * (j)] = math::sqrti((i - xborder[i + dr_size * j]) * (i - xborder[i + dr_size * j]) + (j - yborder[i + dr_size * j]) * (j - yborder[i + dr_size * j]));
			}
			if(distance_map[(i - 1) + dr_size * (j + 1)] + rt_2 < distance_map[(i) + dr_size * (j)]) {
				yborder[i + dr_size * j] = yborder[(i - 1) + dr_size * (j + 1)];
				xborder[i + dr_size * j] = xborder[(i - 1) + dr_size * (j + 1)];
				distance_map[(i) + dr_size * (j)] = math::sqrti((i - xborder[i + dr_size * j]) * (i - xborder[i + dr_size * j]) + (j - yborder[i + dr_size * j]) * (j - yborder[i + dr_size * j]));
			}
			if(distance_map[(i) + dr_size * (j + 1)] + 1.0f < distance_map[(i) + dr_size * (j)]) {
				yborder[i + dr_size * j] = yborder[(i) + dr_size * (j + 1)];
				xborder[i + dr_size * j] = xborder[(i) + dr_size * (j + 1)];
				distance_map[(i) + dr_size * (j)] = math::sqrti((i - xborder[i + dr_size * j]) * (i - xborder[i + dr_size * j]) + (j - yborder[i + dr_size * j]) * (j - yborder[i + dr_size * j]));
			}
			if(distance_map[(i + 1) + dr_size * (j + 1)] + rt_2 < distance_map[(i) + dr_size * (j)]) {
				yborder[i + dr_size * j] = yborder[(i + 1) + dr_size * (j + 1)];
				xborder[i + dr_size * j] = xborder[(i + 1) + dr_size * (j + 1)];
				distance_map[(i) + dr_size * (j)] = math::sqrti((i - xborder[i + dr_size * j]) * (i - xborder[i + dr_size * j]) + (j - yborder[i + dr_size * j]) * (j - yborder[i + dr_size * j]));
			}
		}
	}
	for(uint32_t i = 0; i < dr_size * dr_size; ++i) {
		if(in_map[i]) {
			distance_map[i] *= -1.0f;
		}
	}
}

void dead_reckoning_2(float distance_map[dr_size * dr_size], bool const in_map[dr_size * dr_size]) {
	for(uint32_t i = 0; i < dr_size * dr_size; ++i) {
		distance_map[i] = std::numeric_limits<float>::infinity();
	}
	alignas(64) int16_t yborder[dr_size * dr_size] = { 0 };
	alignas(64) int16_t xborder[dr_size * dr_size] = { 0 };
	for(uint32_t j = 1; j < dr_size - 1; ++j) {
		for(uint32_t i = 1; i < dr_size - 1; ++i) {
			if(in_map[i - 1 + dr_size * j] != in_map[i + dr_size * j]
			|| in_map[i + 1 + dr_size * j] != in_map[i + dr_size * j]
			|| in_map[i + dr_size * (j + 1)] != in_map[i + dr_size * j]
			|| in_map[i + dr_size * (j - 1)] != in_map[i + dr_size * j]) {
				distance_map[i + dr_size * j] = 0.0f;
				yborder[i + dr_size * j] = int16_t(j);
				xborder[i + dr_size * j] = int16_t(i);
			}
		}
	}
	for(uint32_t j = 1; j < dr_size - 1; ++j) {
		for(uint32_t i = 1; i < dr_size - 1; ++i) {
			// xxx
			// x..
			const int16_t yvalues[4] = {
				yborder[(i - 1) + dr_size * (j - 1)],
				yborder[(i + 0) + dr_size * (j - 1)],
				yborder[(i + 1) + dr_size * (j - 1)],
				yborder[(i - 1) + dr_size * (j + 0)]
			};
			const int16_t xvalues[4] = {
				xborder[(i - 1) + dr_size * (j - 1)],
				xborder[(i + 0) + dr_size * (j - 1)],
				xborder[(i + 1) + dr_size * (j - 1)],
				xborder[(i - 1) + dr_size * (j + 0)]
			};
			const bool masks[4] = {
				distance_map[(i - 1) + dr_size * (j - 1)] + rt_2 < distance_map[i + dr_size * j],
				distance_map[(i + 0) + dr_size * (j - 1)] + 1.0f < distance_map[i + dr_size * j],
				distance_map[(i + 1) + dr_size * (j - 1)] + rt_2 < distance_map[i + dr_size * j],
				distance_map[(i - 1) + dr_size * (j + 0)] + 1.0f < distance_map[i + dr_size * j]
			};
			if(masks[0]) {
				// inherit from previous (i - 1, j - 1)
				yborder[i + dr_size * j] = yvalues[0];
				xborder[i + dr_size * j] = xvalues[0];
				distance_map[i + dr_size * j] = math::sqrti(
					(i - xvalues[0]) * (i - xvalues[0])
					+ (j - yvalues[0]) * (j - yvalues[0])
				);
			} else if(masks[1]) {
				// inherit from previous (i, j - 1)
				yborder[i + dr_size * j] = yvalues[1];
				xborder[i + dr_size * j] = xvalues[1];
				distance_map[i + dr_size * j] = math::sqrti(
					(i - xvalues[1]) * (i - xvalues[1])
					+ (j - yvalues[1]) * (j - yvalues[1])
				);
			} else if(masks[2]) {
				// inherit from previous (i + 1, j - 1)
				yborder[i + dr_size * j] = yvalues[2];
				xborder[i + dr_size * j] = xvalues[2];
				distance_map[i + dr_size * j] = math::sqrti(
					(i - xvalues[2]) * (i - xvalues[2])
					+ (j - yvalues[2]) * (j - yvalues[2])
				);
			} else if(masks[3]) {
				// inherit from previous (i - 1, j)
				yborder[i + dr_size * j] = yvalues[3];
				xborder[i + dr_size * j] = xvalues[3];
				distance_map[i + dr_size * j] = math::sqrti(
					(i - xvalues[3]) * (i - xvalues[3])
					+ (j - yvalues[3]) * (j - yvalues[3])
				);
			}
		}
	}
	for(uint32_t j = dr_size - 2; j > 0; --j) {
		for(uint32_t i = dr_size - 2; i > 0; --i) {
			if(distance_map[(i + 1) + dr_size * (j)] + 1.0f < distance_map[(i) + dr_size * (j)]) {
				yborder[i + dr_size * j] = yborder[(i + 1) + dr_size * (j)];
				xborder[i + dr_size * j] = xborder[(i + 1) + dr_size * (j)];
				distance_map[(i) + dr_size * (j)] = math::sqrti((i - xborder[i + dr_size * j]) * (i - xborder[i + dr_size * j]) + (j - yborder[i + dr_size * j]) * (j - yborder[i + dr_size * j]));
			}
			if(distance_map[(i - 1) + dr_size * (j + 1)] + rt_2 < distance_map[(i) + dr_size * (j)]) {
				yborder[i + dr_size * j] = yborder[(i - 1) + dr_size * (j + 1)];
				xborder[i + dr_size * j] = xborder[(i - 1) + dr_size * (j + 1)];
				distance_map[(i) + dr_size * (j)] = math::sqrti((i - xborder[i + dr_size * j]) * (i - xborder[i + dr_size * j]) + (j - yborder[i + dr_size * j]) * (j - yborder[i + dr_size * j]));
			}
			if(distance_map[(i) + dr_size * (j + 1)] + 1.0f < distance_map[(i) + dr_size * (j)]) {
				yborder[i + dr_size * j] = yborder[(i) + dr_size * (j + 1)];
				xborder[i + dr_size * j] = xborder[(i) + dr_size * (j + 1)];
				distance_map[(i) + dr_size * (j)] = math::sqrti((i - xborder[i + dr_size * j]) * (i - xborder[i + dr_size * j]) + (j - yborder[i + dr_size * j]) * (j - yborder[i + dr_size * j]));
			}
			if(distance_map[(i + 1) + dr_size * (j + 1)] + rt_2 < distance_map[(i) + dr_size * (j)]) {
				yborder[i + dr_size * j] = yborder[(i + 1) + dr_size * (j + 1)];
				xborder[i + dr_size * j] = xborder[(i + 1) + dr_size * (j + 1)];
				distance_map[(i) + dr_size * (j)] = math::sqrti((i - xborder[i + dr_size * j]) * (i - xborder[i + dr_size * j]) + (j - yborder[i + dr_size * j]) * (j - yborder[i + dr_size * j]));
			}
		}
	}
	for(uint32_t i = 0; i < dr_size * dr_size; ++i) {
		if(in_map[i]) {
			distance_map[i] *= -1.0f;
		}
	}
}

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

	BENCHMARK_ADVANCED("dead-reckoning-1")(Catch::Benchmark::Chronometer meter) {
		alignas(64) float distance_map[dr_size * dr_size] = { 1.f };
		alignas(64) bool in_map[dr_size * dr_size] = { false };
		meter.measure([&] {
			dead_reckoning_1(distance_map, in_map);
			return distance_map;
		});
	};
	BENCHMARK_ADVANCED("dead-reckoning-2")(Catch::Benchmark::Chronometer meter) {
		alignas(64) float distance_map[dr_size * dr_size] = { 1.f };
		alignas(64) bool in_map[dr_size * dr_size] = { false };
		meter.measure([&] {
			dead_reckoning_2(distance_map, in_map);
			return distance_map;
		});
	};
}
