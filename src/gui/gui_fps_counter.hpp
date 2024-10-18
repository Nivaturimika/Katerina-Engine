#pragma once

#include "gui_element_types.hpp"
namespace ui {
	class fps_counter_text_box : public simple_text_element_base {
	private:
		std::chrono::time_point<std::chrono::steady_clock> last_render_time{};
	public:
		void render(sys::state& state, int32_t x, int32_t y) noexcept override {
			std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
			if(last_render_time == std::chrono::time_point<std::chrono::steady_clock>{}) {
				last_render_time = now;
			}
			auto ms_delta = std::chrono::duration_cast<std::chrono::microseconds>(now - last_render_time).count();
			auto target_fps = 60.f;
			auto fps = 1.f / (ms_delta * target_fps / 1000.f);
			set_text(state, std::to_string(int32_t(fps)));
			last_render_time = now;
			simple_text_element_base::render(state, x, y);
		}
	};

} // namespace ui
