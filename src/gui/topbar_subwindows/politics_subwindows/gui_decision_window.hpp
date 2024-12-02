#pragma once

#include <vector>

#include "gui_element_types.hpp"
#include "triggers.hpp"
#include "text.hpp"

namespace ui {
	void produce_decision_substitutions(sys::state& state, text::substitution_map& m, dcon::nation_id n);

	class decision_requirements : public button_element_base {
	public:
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			auto id = retrieve<dcon::decision_id>(state, parent);
			if(auto condition = state.world.decision_get_allow(id); condition) {
				trigger_description(state, contents, condition, trigger::to_generic(state.local_player_nation), trigger::to_generic(state.local_player_nation), -1);
			}
		}
	};

	class decision_ai_will_do : public button_element_base {
	public:
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			auto id = retrieve<dcon::decision_id>(state, parent);
			text::add_line(state, contents, "alice_ai_decision");
			if(auto mkey = state.world.decision_get_ai_will_do(id); mkey) {
				multiplicative_value_modifier_description(state, contents, mkey, trigger::to_generic(state.local_player_nation), trigger::to_generic(state.local_player_nation), -1);
			}
		}

		void render(sys::state& state, int32_t x, int32_t y) noexcept override {
			if(state.user_settings.spoilers) {
				button_element_base::render(state, x, y);
			}
		}

		mouse_probe impl_probe_mouse(sys::state& state, int32_t x, int32_t y, mouse_probe_type type) noexcept override {
			if(state.user_settings.spoilers) {
				return button_element_base::impl_probe_mouse(state, x, y, type);
			}
			return mouse_probe{ nullptr, ui::xy_pair{} };
		}
	};

	class decision_potential : public button_element_base {
	public:
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			auto id = retrieve<dcon::decision_id>(state, parent);
			if(auto potential = state.world.decision_get_potential(id); potential) {
				trigger_description(state, contents, potential, trigger::to_generic(state.local_player_nation), trigger::to_generic(state.local_player_nation), -1);
			}
		}

		void render(sys::state& state, int32_t x, int32_t y) noexcept override {
			if(state.user_settings.spoilers) {
				button_element_base::render(state, x, y);
			}
		}

		mouse_probe impl_probe_mouse(sys::state& state, int32_t x, int32_t y, mouse_probe_type type) noexcept override {
			if(state.user_settings.spoilers) {
				return button_element_base::impl_probe_mouse(state, x, y, type);
			}
			return mouse_probe{ nullptr, ui::xy_pair{} };
		}
	};

	class make_decision : public button_element_base {
	public:
		sound::audio_instance& get_click_sound(sys::state& state) noexcept override {
			return sound::get_decision_sound(state);
		}

		void button_action(sys::state& state) noexcept override {
			auto content = retrieve<dcon::decision_id>(state, parent);
			command::take_decision(state, state.local_player_nation, content);
		}

		void on_update(sys::state& state) noexcept override {
			auto content = retrieve<dcon::decision_id>(state, parent);
			disabled = !command::can_take_decision(state, state.local_player_nation, content);
		}

		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			auto id = retrieve<dcon::decision_id>(state, parent);

			auto fat_id = dcon::fatten(state.world, id);
			auto box = text::open_layout_box(contents);
			text::substitution_map m;
			produce_decision_substitutions(state, m, state.local_player_nation);
			text::add_to_layout_box(state, contents, box, fat_id.get_name(), m);
			text::close_layout_box(contents, box);
			
			if(auto ef = fat_id.get_effect(); ef) {
				effect_description(state, contents, ef, trigger::to_generic(state.local_player_nation),
					trigger::to_generic(state.local_player_nation), -1, uint32_t(state.current_date.value),
					uint32_t(state.local_player_nation.index() << 4 ^ id.index()));
			}
		}

	};

	// -------------
	// Decision Name
	// -------------

	class decision_name : public multiline_text_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			auto id = retrieve<dcon::decision_id>(state, parent);
		auto contents = text::create_endless_layout(state, internal_layout, text::layout_parameters{ 0, 0, static_cast<int16_t>(base_data.size.x), static_cast<int16_t>(base_data.size.y), base_data.data.text.font_handle, 0, text::alignment::left, text::text_color::white, true });
			auto box = text::open_layout_box(contents);
			text::substitution_map m;
			produce_decision_substitutions(state, m, state.local_player_nation);
			text::add_to_layout_box(state, contents, box, state.world.decision_get_name(id), m);
			text::close_layout_box(contents, box);
		}

		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			auto const id = retrieve<dcon::decision_id>(state, parent);
			if(state.cheat_data.show_province_id_tooltip) {
				auto box = text::open_layout_box(contents);
				text::add_to_layout_box(state, contents, box, std::string_view("Decision ID:"));
				text::add_space_to_layout_box(state, contents, box);
				text::add_to_layout_box(state, contents, box, std::to_string(id.value));
				text::close_layout_box(contents, box);
			}
			auto const desc = state.world.decision_get_description(id);
			if(state.key_is_localized(desc)) {
				auto box = text::open_layout_box(contents);
				text::substitution_map m;
				produce_decision_substitutions(state, m, state.local_player_nation);
				text::add_to_layout_box(state, contents, box, desc, m);
				text::close_layout_box(contents, box);
			}
		}
	};

	// --------------
	// Decision Image
	// --------------

	class decision_image : public image_element_base {
	public:
		bool get_horizontal_flip(sys::state& state) noexcept override {
			return false; //never flip
		}
		void on_update(sys::state& state) noexcept override {
			auto id = retrieve<dcon::decision_id>(state, parent);
			base_data.data.image.gfx_object = state.world.decision_get_image(id);
		}
		void render(sys::state& state, int32_t x, int32_t y) noexcept override {
			if(auto gid = base_data.data.image.gfx_object; gid) {
				auto& gfx_def = state.ui_defs.gfx[gid];
				auto tid = ogl::get_texture_handle(state, gfx_def.primary_texture_handle, gfx_def.is_partially_transparent());
				if(!tid) {
					tid = ogl::get_texture_handle(state, ui::definitions::no_decision_image, false);
				}
				ogl::render_textured_rect(state, get_color_modification(this == state.ui_state.under_mouse, disabled, interactable),
					float(x), float(y), float(base_data.size.x), float(base_data.size.y),
					tid, base_data.get_rotation(), gfx_def.is_vertically_flipped(), get_horizontal_flip(state));
			} else {
				auto tid = ogl::get_texture_handle(state, ui::definitions::no_decision_image, false);
				ogl::render_textured_rect(state, get_color_modification(this == state.ui_state.under_mouse, disabled, interactable),
					float(x), float(y), float(base_data.size.x), float(base_data.size.y),
					tid, base_data.get_rotation(), false, get_horizontal_flip(state));
			}
		}
	};

	// --------------------
	// Decision Description
	// --------------------

	class decision_desc : public scrollable_text {
		private:
		dcon::text_key description;
		void populate_layout(sys::state& state, text::endless_layout& contents) noexcept {
			auto box = text::open_layout_box(contents);
			text::substitution_map m;
			produce_decision_substitutions(state, m, state.local_player_nation);
			text::add_to_layout_box(state, contents, box, description, m);
			text::close_layout_box(contents, box);
		}

		public:
		void on_create(sys::state& state) noexcept override {
			base_data.size.y = 74; //-2
			scrollable_text::on_create(state);
		}

		void on_update(sys::state& state) noexcept override {
			auto id = retrieve<dcon::decision_id>(state, parent);
			auto fat_id = dcon::fatten(state.world, id);
			description = fat_id.get_description();
			auto container = text::create_endless_layout(state, delegate->internal_layout,
			text::layout_parameters{0, 0, static_cast<int16_t>(base_data.size.x), static_cast<int16_t>(base_data.size.y),
				base_data.data.text.font_handle, 0, text::alignment::left,
				text::is_black_from_font_id(base_data.data.text.font_handle) ? text::text_color::black : text::text_color::white,
				false});
			populate_layout(state, container);
			calibrate_scrollbar(state);
		}
		message_result test_mouse(sys::state& state, int32_t x, int32_t y, mouse_probe_type type) noexcept override {
			// Ignore mouse wheel scrolls so people DO NOT get confused!
			if(type == mouse_probe_type::scroll)
			return message_result::unseen;
			return scrollable_text::test_mouse(state, x, y, type);
		}
	};

	// ---------------
	// Ignore Checkbox
	// ---------------

	class ignore_checkbox : public checkbox_button {
		public:
		void button_action(sys::state& state) noexcept override {
			auto id = retrieve<dcon::decision_id>(state, parent);
			if(id) {
				state.world.decision_set_hide_notification(id, !state.world.decision_get_hide_notification(id));
				state.game_state_updated.store(true, std::memory_order_release);
			}
		}

		bool is_active(sys::state& state) noexcept override {
			auto id = retrieve<dcon::decision_id>(state, parent);
			return !state.world.decision_get_hide_notification(id);
		}

		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			text::add_line(state, contents, "hide_decision");
		}
	};

	// -------------
	// Decision Item
	// -------------

	class decision_item : public listbox_row_element_base<dcon::decision_id> {
		public:
		std::unique_ptr<ui::element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "decision_name") {
				return make_element_by_type<decision_name>(state, id);
			} else if(name == "decision_image") {
				return make_element_by_type<decision_image>(state, id);
			} else if(name == "decision_desc") {
				return make_element_by_type<decision_desc>(state, id);
			} else if(name == "requirements") {
				// Extra button to tell if AI will do
				auto btn1 = make_element_by_type<decision_ai_will_do>(state, id);
				btn1->base_data.position.x -= btn1->base_data.size.x * 2;
				add_child_to_front(std::move(btn1));
				auto btn2 = make_element_by_type<decision_potential>(state, id);
				btn2->base_data.position.x -= btn2->base_data.size.x;
				add_child_to_front(std::move(btn2));
				return make_element_by_type<decision_requirements>(state, id);
			} else if(name == "ignore_checkbox") {
				return make_element_by_type<ignore_checkbox>(state, id);
			} else if(name == "make_decision") {
				return make_element_by_type<make_decision>(state, id);
			} else {
				return nullptr;
			}
		}

		message_result get(sys::state& state, Cyto::Any& payload) noexcept override {
			if(payload.holds_type<dcon::decision_id>()) {
				payload.emplace<dcon::decision_id>(content);
				return message_result::consumed;
			} else if(payload.holds_type<wrapped_listbox_row_content<dcon::decision_id>>()) {
				return listbox_row_element_base<dcon::decision_id>::get(state, payload);
			} else {
				return message_result::unseen;
			}
		}
	};

	// ----------------
	// Decision Listbox
	// ----------------

	class decision_listbox : public listbox_element_base<decision_item, dcon::decision_id> {
		protected:
		std::string_view get_row_element_name() override {
			return "decision_entry";
		}
		private:
		std::vector<dcon::decision_id> get_decisions(sys::state& state) {
			std::vector<dcon::decision_id> list;
			auto n = state.local_player_nation;
			for(uint32_t i = state.world.decision_size(); i-- > 0;) {
			dcon::decision_id did{ dcon::decision_id::value_base_t(i) };
				if(!state.cheat_data.always_potential_decisions) {
					auto lim = state.world.decision_get_potential(did);
					if(!lim || trigger::evaluate(state, lim, trigger::to_generic(n), trigger::to_generic(n), 0)) {
						list.push_back(did);
					}
				} else {
					list.push_back(did);
				}
			}

			std::sort(list.begin(), list.end(), [&](dcon::decision_id a, dcon::decision_id b) {
				auto allow_a = state.world.decision_get_allow(a);
				auto allow_b = state.world.decision_get_allow(b);
				auto a_res = !allow_a || trigger::evaluate(state, allow_a, trigger::to_generic(n), trigger::to_generic(n), 0);
				auto b_res = !allow_b || trigger::evaluate(state, allow_b, trigger::to_generic(n), trigger::to_generic(n), 0);
				if(a_res != b_res)
				return a_res;
				else
				return a.index() < b.index();
			});

			return list;
		}
		public:
		void on_update(sys::state& state) noexcept override {
			row_contents = get_decisions(state);
			update(state);
		}
	};

	// ----------------
	// Decision Window
	// ----------------

	class decision_window : public window_element_base {
		public:
		void on_create(sys::state& state) noexcept override {
			window_element_base::on_create(state);
			set_visible(state, false);
		}

		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "decision_listbox") {
				return make_element_by_type<decision_listbox>(state, id);
			} else {
				return nullptr;
			}
		}
	};

} // namespace ui
