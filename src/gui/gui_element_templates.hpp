#pragma once

#include "gui_element_types.hpp"
#include "pdqsort.h"
#include "dcon_generated.hpp"
#include "gui_graphics.hpp"
#include "gui_element_base.hpp"
#include "opengl_wrapper.hpp"
#include "sound.hpp"
#include "system_state.hpp"
#include "text.hpp"
#include "texture.hpp"
#include "demographics.hpp"
#include "color.hpp"

namespace dcon {
	class pop_satisfaction_wrapper_id {
	public:
		using value_base_t = uint8_t;
		value_base_t value = 0;
		pop_satisfaction_wrapper_id() { }
		pop_satisfaction_wrapper_id(uint8_t v) : value(v) { }
		value_base_t index() {
			return value;
		}
	};
	class pop_satisfaction_wrapper_fat {
		static dcon::text_key names[5];
	public:
		uint8_t value = 0;
		void set_name(dcon::text_key text) noexcept {
			names[value] = text;
		}
		dcon::text_key get_name() noexcept {
			switch(value) {
				case 0: // No needs fulfilled
				case 1: // Some life needs
				case 2: // All life needs, some everyday
				case 3: // All everyday, some luxury
				case 4: // All luxury
					return names[value];
			}
			return dcon::text_key{0};
		}
	};
	inline pop_satisfaction_wrapper_fat fatten(data_container const& c, pop_satisfaction_wrapper_id id) noexcept {
		return pop_satisfaction_wrapper_fat{id.value};
	}
} // namespace dcon
namespace ogl {
	template<>
	inline uint32_t get_ui_color(sys::state& state, dcon::pop_satisfaction_wrapper_id id) {
		switch(id.value) {
			case 0: // red
				return sys::pack_color(1.0f, 0.1f, 0.1f);
			case 1: // yellow
				return sys::pack_color(1.0f, 1.0f, 0.1f);
			case 2: // green
				return sys::pack_color(0.1f, 1.0f, 0.1f);
			case 3: // blue
				return sys::pack_color(0.1f, 0.1f, 1.0f);
			case 4: // light blue
				return sys::pack_color(0.1f, 1.0f, 1.0f);
		}
		return 0;
	}
} // namespace ogl

namespace ui {
	template<class T>
	void piechart<T>::render(sys::state& state, int32_t x, int32_t y) noexcept {
		if(distribution.size() > 0)
			ogl::render_piechart(state, ogl::color_modification::none, float(x), float(y), float(base_data.size.x), data_texture);
	}

	template<class T>
	void piechart<T>::on_create(sys::state& state) noexcept {
		base_data.position.x -= base_data.size.x;
		radius = float(base_data.size.x);
		base_data.size.x *= 2;
		base_data.size.y *= 2;
	}

	template<class T>
	void piechart<T>::update_chart(sys::state& state) {
		pdqsort(distribution.begin(), distribution.end(), [](auto const& a, auto const& b) {
			return a.value > b.value;
		});

		float total = 0.0f;
		for(auto& e : distribution) {
			total += e.value;
		}
		int32_t int_total = 0;

		if(total != 0.0f) {
			for(auto& e : distribution) {
				auto ivalue = int32_t(e.value * float(resolution) / total);
				e.slices = uint8_t(ivalue);
				e.value /= total;
				int_total += ivalue;
			}
		} else {
			distribution.clear();
		}

		if(int_total < resolution && distribution.size() > 0) {
			auto rem = resolution - int_total;
			while(rem > 0) {
				for(auto& e : distribution) {
					e.slices += uint8_t(1);
					rem -= 1;
					if(rem == 0) {
						break;
					}
				}
			}
		} else if(int_total > resolution) {
			assert(false);
		}

		size_t i = 0;
		for(auto& e : distribution) {
			uint32_t color = ogl::get_ui_color<T>(state, e.key);
			auto slice_count = size_t(e.slices);

			for(size_t j = 0; j < slice_count; j++) {
				data_texture.data[(i + j) * channels] = uint8_t(color & 0xFF);
				data_texture.data[(i + j) * channels + 1] = uint8_t(color >> 8 & 0xFF);
				data_texture.data[(i + j) * channels + 2] = uint8_t(color >> 16 & 0xFF);
			}

			i += slice_count;
		}
		for(; i < resolution; i++) {
			data_texture.data[i * channels] = uint8_t(0);
			data_texture.data[i * channels + 1] = uint8_t(0);
			data_texture.data[i * channels + 2] = uint8_t(0);
		}

		data_texture.data_updated = true;
	}

	template<class T>
	void piechart<T>::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		float const PI = 3.141592653589793238463f;
		float dx = float(x) - radius;
		float dy = float(y) - radius;
		size_t index = 0;
		if(dx != 0.0f || dy != 0.0f) {
			float dist = std::sqrt(dx * dx + dy * dy);
			float angle = std::acos(-dx / dist);
			if(dy > 0.f) {
				angle = PI + (PI - angle);
			}
			index = size_t(angle / (2.f * PI) * float(resolution));
		}
		for(auto const& e : distribution) {
			if(index < size_t(e.slices)) {
				populate_tooltip(state, e.key, e.value, contents);
				return;
			}
			index -= size_t(e.slices);
		}
	}

	template<class T>
	void piechart<T>::populate_tooltip(sys::state& state, T t, float percentage, text::columnar_layout& contents) noexcept {
		auto fat_t = dcon::fatten(state.world, t);
		auto box = text::open_layout_box(contents, 0);
		if constexpr(!std::is_same_v<dcon::nation_id, T>)
		text::add_to_layout_box(state, contents, box, fat_t.get_name(), text::substitution_map{});
		else
		text::add_to_layout_box(state, contents, box, text::get_name(state, t), text::substitution_map{});
		text::add_to_layout_box(state, contents, box, std::string(":"), text::text_color::white);
		text::add_space_to_layout_box(state, contents, box);
		text::add_to_layout_box(state, contents, box, text::format_percentage(percentage, 1), text::text_color::white);
		text::close_layout_box(contents, box);
	}

	template<class SrcT, class DemoT>
	void demographic_piechart<SrcT, DemoT>::on_update(sys::state& state) noexcept {
		this->distribution.clear();

		Cyto::Any obj_id_payload = SrcT{};
		size_t i = 0;
		if(this->parent) {
			this->parent->impl_get(state, obj_id_payload);
			float total_pops = 0.f;

			for_each_demo(state, [&](DemoT demo_id) {
				float volume = 0.f;
				if(obj_id_payload.holds_type<dcon::province_id>()) {
					auto demo_key = demographics::to_key(state, demo_id);
					auto prov_id = any_cast<dcon::province_id>(obj_id_payload);
					volume = state.world.province_get_demographics(prov_id, demo_key);
				} else if(obj_id_payload.holds_type<dcon::nation_id>()) {
					auto demo_key = demographics::to_key(state, demo_id);
					auto nat_id = any_cast<dcon::nation_id>(obj_id_payload);
					volume = state.world.nation_get_demographics(nat_id, demo_key);
				}

				if constexpr(std::is_same_v<SrcT, dcon::pop_id>) {
					if(obj_id_payload.holds_type<dcon::pop_id>()) {
						auto demo_key = pop_demographics::to_key(state, demo_id);
						auto pop_id = any_cast<dcon::pop_id>(obj_id_payload);
						volume = state.world.pop_get_demographics(pop_id, demo_key);
					}
				}
				if(volume > 0)
				this->distribution.emplace_back(demo_id, volume);
			});
		}

		this->update_chart(state);
	}

	template<class RowWinT, class RowConT>
	void standard_listbox_scrollbar<RowWinT, RowConT>::on_value_change(sys::state& state, int32_t v) noexcept {
		static_cast<listbox_element_base<RowWinT, RowConT>*>(parent)->update(state);
	}

	template<class RowConT>
	message_result listbox_row_element_base<RowConT>::get(sys::state& state, Cyto::Any& payload) noexcept {
		if(payload.holds_type<RowConT>()) {
			payload.emplace<RowConT>(content);
			return message_result::consumed;
		} else if(payload.holds_type<wrapped_listbox_row_content<RowConT>>()) {
			content = any_cast<wrapped_listbox_row_content<RowConT>>(payload).content;
			impl_on_update(state);
			return message_result::consumed;
		}
		return message_result::unseen;
	}

	template<class RowConT>
	message_result listbox_row_button_base<RowConT>::get(sys::state& state, Cyto::Any& payload) noexcept {
		if(payload.holds_type<RowConT>()) {
			payload.emplace<RowConT>(content);
			return message_result::consumed;
		} else if(payload.holds_type<wrapped_listbox_row_content<RowConT>>()) {
			content = any_cast<wrapped_listbox_row_content<RowConT>>(payload).content;
			update(state);
			return message_result::consumed;
		}
		return message_result::unseen;
	}

	template<class RowWinT, class RowConT>
	void listbox_element_base<RowWinT, RowConT>::update(sys::state& state) {
		auto content_off_screen = int32_t(row_contents.size() - row_windows.size());
		int32_t scroll_pos = list_scrollbar->raw_value();
		if(content_off_screen <= 0) {
			list_scrollbar->set_visible(state, false);
			scroll_pos = 0;
		} else {
		list_scrollbar->change_settings(state, mutable_scrollbar_settings{0, content_off_screen, 0, 0, false});
			list_scrollbar->set_visible(state, true);
			scroll_pos = std::min(scroll_pos, content_off_screen);
		}

		if(is_reversed()) {
			auto i = int32_t(row_contents.size()) - scroll_pos - 1;
			for(int32_t rw_i = int32_t(row_windows.size()) - 1; rw_i >= 0; rw_i--) {
				RowWinT* row_window = row_windows[size_t(rw_i)];
				if(i >= 0) {
					row_window->set_visible(state, true);
					auto prior_content = retrieve<RowConT>(state, row_window);
					auto new_content = row_contents[i--];

					if(prior_content != new_content) {
					send(state, row_window, wrapped_listbox_row_content<RowConT>{ new_content });
						if(!row_window->is_visible()) {
							row_window->set_visible(state, true);
						} else {
							row_window->impl_on_update(state);
						}
					} else {
						row_window->set_visible(state, true);
					}
				} else {
					row_window->set_visible(state, false);
				}
			}
		} else {
			auto i = size_t(scroll_pos);
			for(RowWinT* row_window : row_windows) {
				if(i < row_contents.size()) {
					auto prior_content = retrieve<RowConT>(state, row_window);
					auto new_content = row_contents[i++];

					if(prior_content != new_content) {
					send(state, row_window, wrapped_listbox_row_content<RowConT>{ new_content });
						if(!row_window->is_visible()) {
							row_window->set_visible(state, true);
						} else {
							row_window->impl_on_update(state);
						}
					} else {
						row_window->set_visible(state, true);
					}
				} else {
					row_window->set_visible(state, false);
				}
			}
		}
	}

	template<class RowWinT, class RowConT>
	message_result listbox_element_base<RowWinT, RowConT>::on_scroll(sys::state& state, int32_t x, int32_t y, float amount, sys::key_modifiers mods) noexcept {
		if(row_contents.size() > row_windows.size()) {
			amount = is_reversed() ? -amount : amount;
			list_scrollbar->update_raw_value(state, list_scrollbar->raw_value() + (amount < 0 ? 1 : -1));
			state.ui_state.last_tooltip = nullptr; //force update of tooltip
			update(state);
			return message_result::consumed;
		}
		return message_result::unseen;
	}

	template<class RowWinT, class RowConT>
	void listbox_element_base<RowWinT, RowConT>::scroll_to_bottom(sys::state& state) {
		uint32_t list_size = 0;
		for(auto rc : row_contents) {
			list_size++;
		}
		list_scrollbar->update_raw_value(state, list_size);
		state.ui_state.last_tooltip = nullptr; //force update of tooltip
		update(state);
	}

	template<typename contents_type>
	message_result listbox2_base<contents_type>::on_scroll(sys::state& state, int32_t x, int32_t y, float amount, sys::key_modifiers mods) noexcept {
		if(int32_t(row_contents.size()) > visible_row_count) {
			//amount = is_reversed() ? -amount : amount;
			list_scrollbar->update_raw_value(state, list_scrollbar->raw_value() + (amount < 0 ? 1 : -1));
			impl_on_update(state);
			return message_result::consumed;
		}
		return message_result::unseen;
	}

	template<typename contents_type>
	void listbox2_base<contents_type>::on_update(sys::state& state) noexcept {
		auto content_off_screen = int32_t(row_contents.size()) - visible_row_count;
		int32_t scroll_pos = list_scrollbar->raw_value();

		if(content_off_screen <= 0) {
			list_scrollbar->set_visible(state, false);
			list_scrollbar->update_raw_value(state, 0);

			int32_t i = 0;
			for(; i < int32_t(row_contents.size()); ++i) {
				row_windows[i]->set_visible(state, true);
			}
			for(; i < int32_t(row_windows.size()); ++i) {
				row_windows[i]->set_visible(state, false);
			}
		} else {
		list_scrollbar->change_settings(state, mutable_scrollbar_settings{ 0, content_off_screen, 0, 0, false });
			list_scrollbar->set_visible(state, true);
			scroll_pos = std::min(scroll_pos, content_off_screen);

			int32_t i = 0;
			for(; i < visible_row_count; ++i) {
				row_windows[i]->set_visible(state, true);
			}
			for(; i < int32_t(row_windows.size()); ++i) {
				row_windows[i]->set_visible(state, false);
			}
		}
	}

	template<typename contents_type>
	message_result listbox2_base<contents_type>::get(sys::state& state, Cyto::Any& payload) noexcept  {
		if(payload.holds_type<listbox2_scroll_event>()) {
			impl_on_update(state);
			return message_result::consumed;
		} else if(payload.holds_type<listbox2_row_view>()) {
			listbox2_row_view ptr = any_cast<listbox2_row_view>(payload);
			for(int32_t index = 0; index < int32_t(row_windows.size()); ++index) {
				if(row_windows[index] == ptr.row) {
					stored_index = index + list_scrollbar->raw_value();
					return message_result::consumed;
				}
			}
			stored_index = -1;
			return message_result::consumed;
		} else if(payload.holds_type<contents_type>()) {
			if(0 <= stored_index && stored_index < int32_t(row_contents.size())) {
				payload = row_contents[stored_index];
			}
			return message_result::consumed;
		} else {
			return message_result::unseen;
		}
	}

	template<typename contents_type>
	void listbox2_base<contents_type>::resize(sys::state& state, int32_t height) {
		int32_t row_height = row_windows[0]->base_data.position.y + row_windows[0]->base_data.size.y;
		int32_t height_covered = int32_t(row_windows.size()) * row_height;
		int32_t required_rows = (height - height_covered) / row_height;

		while(required_rows > 0) {
			auto new_row = make_row(state);
			row_windows.push_back(new_row.get());
			new_row->base_data.position.y += int16_t(height_covered);
			add_child_to_back(std::move(new_row));

			height_covered += row_height;
			--required_rows;
		}

		visible_row_count = height / row_height;
		base_data.size.y = int16_t(row_height * visible_row_count);

		if(visible_row_count != 0) {
			if(scrollbar_is_internal)
			base_data.size.x -= 16;
			list_scrollbar->scale_to_parent();
			if(scrollbar_is_internal)
			base_data.size.x += 16;
		}
	}

	template<typename contents_type>
	void listbox2_base<contents_type>::on_create(sys::state& state) noexcept {
		auto ptr = make_element_by_type<listbox2_scrollbar>(state, "standardlistbox_slider");
		list_scrollbar = static_cast<listbox2_scrollbar*>(ptr.get());
		add_child_to_back(std::move(ptr));

		auto base_row = make_row(state);
		row_windows.push_back(base_row.get());
		add_child_to_back(std::move(base_row));

		resize(state, base_data.size.y);
	}
	template<typename contents_type>
	void listbox2_base<contents_type>::render(sys::state& state, int32_t x, int32_t y) noexcept {
		dcon::gfx_object_id gid = base_data.data.list_box.background_image;
		if(gid) {
			auto const& gfx_def = state.ui_defs.gfx[gid];
			if(gfx_def.primary_texture_handle) {
				if(gfx_def.get_object_type() == ui::object_type::bordered_rect) {
					ogl::render_bordered_rect(state, get_color_modification(false, false, true), gfx_def.type_dependent, float(x), float(y),
					float(base_data.size.x), float(base_data.size.y),
					ogl::get_texture_handle(state, gfx_def.primary_texture_handle, gfx_def.is_partially_transparent()),
					base_data.get_rotation(), gfx_def.is_vertically_flipped(),
					state.world.locale_get_native_rtl(state.font_collection.get_current_locale()));
				} else {
					ogl::render_textured_rect(state, get_color_modification(false, false, true), float(x), float(y), float(base_data.size.x),
					float(base_data.size.y),
					ogl::get_texture_handle(state, gfx_def.primary_texture_handle, gfx_def.is_partially_transparent()),
					base_data.get_rotation(), gfx_def.is_vertically_flipped(),
					state.world.locale_get_native_rtl(state.font_collection.get_current_locale()));
				}
			}
		}
		container_base::render(state, x, y);
	}

	template<class RowWinT, class RowConT>
	void listbox_element_base<RowWinT, RowConT>::on_create(sys::state& state) noexcept {
		int16_t current_y = 0;
		int16_t subwindow_y_size = 0;
		while(current_y + subwindow_y_size <= base_data.size.y) {
			auto ptr = make_element_by_type<RowWinT>(state, get_row_element_name());
			if(ptr.get() == nullptr) {
				break; //unable to make children
			}
			row_windows.push_back(static_cast<RowWinT*>(ptr.get()));
			int16_t offset = ptr->base_data.position.y;
			ptr->base_data.position.y += current_y;
			subwindow_y_size = ptr->base_data.size.y;
			current_y += ptr->base_data.size.y + offset;
			add_child_to_front(std::move(ptr));
		}
		auto ptr = make_element_by_type<standard_listbox_scrollbar<RowWinT, RowConT>>(state, "standardlistbox_slider");
		if(ptr.get()) {
			list_scrollbar = static_cast<standard_listbox_scrollbar<RowWinT, RowConT>*>(ptr.get());
			add_child_to_front(std::move(ptr));
			list_scrollbar->scale_to_parent();
		}
		update(state);
	}

	template<class RowWinT, class RowConT>
	void listbox_element_base<RowWinT, RowConT>::render(sys::state& state, int32_t x, int32_t y) noexcept {
		dcon::gfx_object_id gid = base_data.data.list_box.background_image;
		if(gid) {
			auto const& gfx_def = state.ui_defs.gfx[gid];
			if(gfx_def.primary_texture_handle) {
				if(gfx_def.get_object_type() == ui::object_type::bordered_rect) {
					ogl::render_bordered_rect(state, get_color_modification(false, false, true), gfx_def.type_dependent, float(x), float(y),
					float(base_data.size.x), float(base_data.size.y),
					ogl::get_texture_handle(state, gfx_def.primary_texture_handle, gfx_def.is_partially_transparent()),
					base_data.get_rotation(), gfx_def.is_vertically_flipped(),
					state.world.locale_get_native_rtl(state.font_collection.get_current_locale()));
				} else {
					ogl::render_textured_rect(state, get_color_modification(false, false, true), float(x), float(y), float(base_data.size.x),
					float(base_data.size.y),
					ogl::get_texture_handle(state, gfx_def.primary_texture_handle, gfx_def.is_partially_transparent()),
					base_data.get_rotation(), gfx_def.is_vertically_flipped(),
					state.world.locale_get_native_rtl(state.font_collection.get_current_locale()));
				}
			}
		}
		container_base::render(state, x, y);
	}
	template<class ItemWinT, class ItemConT>
	void overlapping_listbox_element_base<ItemWinT, ItemConT>::update(sys::state& state) {
		auto spacing = int16_t(base_data.data.overlapping.spacing);
		if(base_data.get_element_type() == element_type::overlapping) {
			while(row_contents.size() > windows.size()) {
				auto ptr = make_element_by_type<ItemWinT>(state, get_row_element_name());
				if(subwindow_width <= 0) {
					subwindow_width = ptr->base_data.size.x;
				}
				windows.push_back(static_cast<ItemWinT*>(ptr.get()));
				add_child_to_front(std::move(ptr));
			}

			float size_ratio = float(row_contents.size() * (subwindow_width + spacing)) / float(base_data.size.x);
			int16_t offset = spacing + subwindow_width;
			if(size_ratio > 1.f) {
				offset = int16_t(float(subwindow_width) / size_ratio);
			}
			int16_t current_x = 0;
			if(base_data.data.overlapping.image_alignment == alignment::right) {
				current_x = base_data.size.x - subwindow_width - offset * int16_t(row_contents.size() - 1);
			}
			for(size_t i = 0; i < windows.size(); i++) {
				if(i < row_contents.size()) {
					update_subwindow(state, *windows[i], row_contents[i]);
					windows[i]->base_data.position.x = current_x;
					current_x += offset;
					windows[i]->set_visible(state, true);
				} else {
					windows[i]->set_visible(state, false);
				}
			}
		}
	}
}
