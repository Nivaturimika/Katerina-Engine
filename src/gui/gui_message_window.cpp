#include "gui_message_window.hpp"

namespace ui {
	void message_body_text::populate_layout(sys::state& state, text::endless_layout& contents) noexcept {
		if(auto msg = retrieve<notification::message*>(state, parent); msg) {
			msg->body(state, contents);
		}
	}
	void message_body_text::on_create(sys::state& state) noexcept {
		base_data.size.x = 500 - (base_data.position.x * 2) - 8;
		base_data.size.y = 18 * 6;
		multiline_text_element_base::on_create(state);
	}
	void message_body_text::on_update(sys::state& state) noexcept {
		text::alignment align = text::alignment::left;
		switch(base_data.data.text.get_alignment()) {
		case ui::alignment::right:
			align = text::alignment::right;
			break;
		case ui::alignment::centered:
			align = text::alignment::center;
			break;
		default:
			break;
		}
		auto border = base_data.data.text.border_size;
		auto content = retrieve<diplomatic_message::message>(state, parent);
		auto color = black_text ? text::text_color::black : text::text_color::white;
		auto container = text::create_endless_layout(state,
		internal_layout,
		text::layout_parameters{
			border.x,
			border.y,
			int16_t(base_data.size.x - border.x * 2),
			int16_t(base_data.size.y - border.y * 2),
			base_data.data.text.font_handle,
			0,
			align,
			color,
			false
		});
		populate_layout(state, container);
	}

	void message_title_text::on_update(sys::state& state) noexcept {
		if(auto msg = retrieve<notification::message*>(state, parent); msg) {
			set_text(state, text::produce_simple_string(state, msg->title));	
		}
	}

	void message_flag_button::on_create(sys::state& state) noexcept {
		base_data.position.x += 8;
		base_data.position.y -= 6;
		base_data.size.x -= 16;
		base_data.size.y += 32;
	}

	void message_flag_button::render(sys::state& state, int32_t x, int32_t y) noexcept {
		dcon::gfx_object_id gid;
		if(base_data.get_element_type() == element_type::image) {
			gid = base_data.data.image.gfx_object;
		} else if(base_data.get_element_type() == element_type::button) {
			gid = base_data.data.button.button_image;
		}
		if(gid && flag_texture_handle > 0) {
			auto& gfx_def = state.ui_defs.gfx[gid];
			auto mask_handle = ogl::get_texture_handle(state, dcon::texture_id(gfx_def.type_dependent - 1), true);
			auto& mask_tex = state.open_gl.asset_textures[dcon::texture_id(gfx_def.type_dependent - 1)];
			ogl::render_masked_rect(state, get_color_modification(this == state.ui_state.under_mouse, disabled, interactable), float(x),
			float(y), float(base_data.size.x), float(base_data.size.y), flag_texture_handle, mask_handle, base_data.get_rotation(),
			gfx_def.is_vertically_flipped(),
			false);
		}
		image_element_base::render(state, x, y);
	}

	void message_window::on_create(sys::state& state) noexcept {
		window_element_base::on_create(state);
		xy_pair cur_pos{0, 0};
		{
			auto ptr = make_element_by_type<message_lr_button<false>>(state, "alice_left_right_button");
			cur_pos.x = base_data.size.x - (ptr->base_data.size.x * 2);
			cur_pos.y = ptr->base_data.size.y * 1;
			ptr->base_data.position = cur_pos;
			add_child_to_front(std::move(ptr));
		}
		{
			auto ptr = make_element_by_type<message_count_text>(state, "alice_page_count");
			cur_pos.x -= ptr->base_data.size.x;
			ptr->base_data.position = cur_pos;
			count_text = static_cast<simple_text_element_base*>(ptr.get());
			add_child_to_front(std::move(ptr));
		}
		{
			auto ptr = make_element_by_type<message_lr_button<true>>(state, "alice_left_right_button");
			cur_pos.x -= ptr->base_data.size.x;
			ptr->base_data.position = cur_pos;
			add_child_to_front(std::move(ptr));
		}
		set_visible(state, false);
	}

	std::unique_ptr<element_base> message_window::make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept {
		if(name == "header") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "line1") {
			auto ptr = make_element_by_type<message_title_text>(state, id);
			ptr->base_data.size.x = base_data.size.x - (ptr->base_data.position.x * 2);
			ptr->base_data.size.y = 22;
			return ptr;
		} else if(name == "line3") {
			auto ptr = make_element_by_type<message_body_text>(state, id);
			desc_text = ptr.get();
			return ptr;
		} else if(name.substr(0, 4) == "line") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "agreebutton") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "declinebutton") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "centerok") {
			auto ptr = make_element_by_type<message_dismiss_button>(state, id);
			dismiss_btn = ptr.get();
			return ptr;
		} else if(name == "leftshield") {
			return make_element_by_type<message_flag_button>(state, id);
		} else if(name == "rightshield") {
			return make_element_by_type<message_flag_button>(state, id);
		} else if(name == "background") {
			auto ptr = make_element_by_type<draggable_target>(state, id);
			ptr->base_data.size = base_data.size;
			bg_elm = ptr.get();
			return ptr;
		} else {
			return nullptr;
		}
	}

	void message_window::on_update(sys::state& state) noexcept {
		if(messages.empty()) {
			state.ui_pause.store(false, std::memory_order_release);
			set_visible(state, false);
		} else {
			if(int32_t(messages.size()) > prev_size) {
				index = int32_t(messages.size()) - 1;
			} else {
				if(index >= int32_t(messages.size())) {
					index = 0;
				} else if(index < 0) {
					index = int32_t(messages.size()) - 1;
				}
			}
			desc_text->impl_on_update(state);
			// Automatically resize the message window :)
			auto const new_height = std::max(desc_text->line_height
				* float(desc_text->internal_layout.number_of_lines), 72.f);
			base_data.size.y = new_height + 196;
			bg_elm->base_data.size.y = new_height + 196;
			desc_text->base_data.size.y = new_height;
			dismiss_btn->base_data.position.y = desc_text->base_data.position.y + desc_text->base_data.size.y + 24;
			//
			prev_size = int32_t(messages.size());
			count_text->set_text(state, std::to_string(int32_t(index) + 1) + "/" + std::to_string(int32_t(messages.size())));
		}
	}

	message_result message_window::get(sys::state& state, Cyto::Any& payload) noexcept {
		if(index >= int32_t(messages.size())) {
			index = 0;
		} else if(index < 0) {
			index = int32_t(messages.size()) - 1;
		}
		if(payload.holds_type<dcon::nation_id>()) {
			if(messages.empty()) {
				payload.emplace<dcon::nation_id>(dcon::nation_id{});
			} else {
				payload.emplace<dcon::nation_id>(messages[index].source);
			}
			return message_result::consumed;
		} else if(payload.holds_type<dcon::national_identity_id>()) {
			if(messages.empty()) {
				payload.emplace<dcon::national_identity_id>(dcon::national_identity_id{});
			} else {
				payload.emplace<dcon::national_identity_id>(state.world.nation_get_identity_from_identity_holder(messages[index].source));
			}
			return message_result::consumed;
		} else if(payload.holds_type<element_selection_wrapper<bool>>()) {
			bool b = any_cast<element_selection_wrapper<bool>>(payload).data;
			index += b ? -1 : +1;
			impl_on_update(state);
			return message_result::consumed;
		} else if(payload.holds_type<message_dismiss_notification>()) {
			messages.clear();
			impl_on_update(state);
			return message_result::consumed;
		} else if(payload.holds_type<notification::message*>()) {
			if(messages.empty()) {
				payload.emplace<notification::message*>(nullptr);
			} else {
				payload.emplace<notification::message*>(&(messages[index]));
			}
		}
		return window_element_base::get(state, payload);
	}
}
