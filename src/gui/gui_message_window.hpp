#pragma once

#include "gui_common_elements.hpp"
#include "gui_element_types.hpp"
#include "notifications.hpp"

namespace ui {
	template<bool Left>
	class message_lr_button : public button_element_base {
		public:
		void on_create(sys::state& state) noexcept override {
			button_element_base::on_create(state);
			frame = Left ? 0 : 1;
		}

		void button_action(sys::state& state) noexcept override {
			send(state, parent, element_selection_wrapper<bool>{Left});
		}
	};

	struct message_dismiss_notification {
		int dummy = 0;
	};

	class message_dismiss_button : public button_element_base {
	public:
		void button_action(sys::state& state) noexcept override {
			send(state, parent, message_dismiss_notification{});
		}
	};

	class message_count_text : public simple_text_element_base {
	public:
		void on_create(sys::state& state) noexcept override {
			simple_text_element_base::on_create(state);
			black_text = false;
		}
	};

	class message_body_text : public multiline_text_element_base {
		void populate_layout(sys::state& state, text::endless_layout& contents) noexcept;
	public:
		void on_create(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
	};

	class message_flag_button : public flag_button {
	public:
		void on_create(sys::state& state) noexcept override;
		void render(sys::state& state, int32_t x, int32_t y) noexcept override;
	};

	class message_title_text : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override;
	};

	class message_window : public window_element_base {
		simple_text_element_base* count_text = nullptr;
		int32_t index = 0;
		int32_t prev_size = 0;
		message_body_text* desc_text = nullptr;
		message_dismiss_button* dismiss_btn = nullptr;
		draggable_target* bg_elm = nullptr;
	public:
		std::vector<notification::message> messages;
		void on_create(sys::state& state) noexcept override;
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
		void on_update(sys::state& state) noexcept override;
		message_result get(sys::state& state, Cyto::Any& payload) noexcept override;
	};

} // namespace ui
