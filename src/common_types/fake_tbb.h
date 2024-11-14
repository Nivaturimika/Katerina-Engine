#pragma once

#include <ranges>

namespace concurrency {
	struct partitioner {};
	inline partitioner static_partitioner() {
		return partitioner{};
	}

	template<typename It, typename F>
	inline void parallel_for(It first, It last, It div, F&& func, partitioner p) {
		for(It it = first; it != last; ++it) {
			for(It offset = 0; offset < div; ++offset) {
				func(it + offset);
			}
		}
	}
	template<typename It, typename F>
	inline void parallel_for(It first, It last, It div, F&& func) {
		for(It it = first; it != last; ++it) {
			for(It offset = 0; offset < div; ++offset) {
				func(it + offset);
			}
		}
	}
	template<typename It, typename F>
	inline void parallel_for(It first, It last, F&& func) {
		for(It it = first; it != last; ++it)
			func(it);
	}

	template<typename T>
	struct combinable {
		T value;
		inline const T& local() const { return value; }
		inline T& local() { return value; }
		template<typename F>
		inline void combine_each(F&& func) {
			func(value);
		}
		template<typename F>
		inline T combine(F&&) { return value; }
	};
}
