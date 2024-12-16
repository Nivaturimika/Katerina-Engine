#pragma once

#include "gui_element_types.hpp"
#include "gui_chat_window.hpp"

namespace ui {
	class nation_picker_poptypes_chart : public piechart<dcon::pop_type_id> {
	protected:
		void on_update(sys::state& state) noexcept override {
			distribution.clear();
			auto n = retrieve<dcon::nation_id>(state, parent);
			for(auto pt : state.world.in_pop_type) {
				auto amount = state.world.nation_get_demographics(n, demographics::to_key(state, pt));
				distribution.emplace_back(pt.id, amount);
			}
			update_chart(state);
		}
	};

	class nation_picker_cultures_chart : public piechart<dcon::culture_id> {
	protected:
		void on_update(sys::state& state) noexcept override {
			distribution.clear();
			auto n = retrieve<dcon::nation_id>(state, parent);
			for(auto c : state.world.in_culture) {
				auto amount = state.world.nation_get_demographics(n, demographics::to_key(state, c));
				distribution.emplace_back(c.id, amount);
			}
			update_chart(state);
		}
	};

	class nation_picker_ideologies_chart : public piechart<dcon::ideology_id> {
	protected:
		void on_update(sys::state& state) noexcept override {
			distribution.clear();
			auto n = retrieve<dcon::nation_id>(state, parent);
			for(auto c : state.world.in_ideology) {
				auto amount = state.world.nation_get_demographics(n, demographics::to_key(state, c));
				distribution.emplace_back(c.id, amount);
			}
			update_chart(state);
		}
	};

	class nation_details_window : public window_element_base {
		int16_t piechart_offset = 0;
		element_base* overlay1 = nullptr;
		element_base* overlay2 = nullptr;
	public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
	};

	struct save_item {
		native_string file_name;
		uint64_t timestamp = 0;
		sys::date save_date;
		dcon::national_identity_id save_flag;
		dcon::government_type_id as_gov;
		bool is_new_game = false;
		std::string name = "fe_new_game";
		bool is_bookmark() const {
			return file_name.starts_with(NATIVE("bookmark_"));
		}
		bool operator==(save_item const& o) const {
			return save_flag == o.save_flag && as_gov == o.as_gov && save_date == o.save_date && is_new_game == o.is_new_game && file_name == o.file_name && timestamp == o.timestamp;
		}
		bool operator!=(save_item const& o) const {
			return !(*this == o);
		}
	};

	class lobby_select_save_game_button : public button_element_base {
	public:
		void on_create(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
	};

	class save_flag : public button_element_base {
	protected:
		GLuint flag_texture_handle = 0;
		bool visible = false;
	public:
		void button_action(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept  override;
		void render(sys::state& state, int32_t x, int32_t y) noexcept override;
	};

	class save_name : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override;
	};

	class save_date : public simple_text_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			save_item* i = retrieve< save_item*>(state, parent);
			set_text(state, text::date_to_string(state, i->save_date));
		}
	};

	class lobby_save_game_row : public listbox_row_element_base<std::shared_ptr<save_item>> {
	public:
		void on_create(sys::state& state) noexcept override {
			listbox_row_element_base<std::shared_ptr<save_item>>::on_create(state);
			base_data.position.x += 9; // Nudge
			base_data.position.y += 7; // Nudge
		}

		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "save_game") {
				return make_element_by_type<lobby_select_save_game_button>(state, id);
			} else if(name == "shield") {
				return make_element_by_type<save_flag>(state, id);
			} else if(name == "title") {
				return make_element_by_type<save_name>(state, id);
			} else if(name == "date") {
				return make_element_by_type<save_date>(state, id);
			}
			return nullptr;
		}

		message_result get(sys::state& state, Cyto::Any& payload) noexcept  override {
			if(payload.holds_type<save_item*>()) {
				payload.emplace<save_item*>(content.get());
				return message_result::consumed;
			}
			return listbox_row_element_base<std::shared_ptr<save_item>>::get(state, payload);
		}
	};

	class lobby_save_game_listbox : public listbox_element_base<lobby_save_game_row, std::shared_ptr<save_item>> {
	protected:
		std::string_view get_row_element_name() override {
			return "alice_savegameentry";
		}

		void update_save_list(sys::state& state) noexcept;
	public:
		void on_create(sys::state& state) noexcept override {
			base_data.size.x -= 20; //nudge
			base_data.size.y += base_data.position.y;
			base_data.position.y = 0;
			listbox_element_base<lobby_save_game_row, std::shared_ptr<save_item>>::on_create(state);
			update_save_list(state);
		}
		void on_update(sys::state& state) noexcept override;
	};

	class saves_window : public window_element_base {
	public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "save_games_list") {
				return make_element_by_type<lobby_save_game_listbox>(state, id);
			}
			return nullptr;
		}
	};

	class pick_nation_button : public button_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			auto n = retrieve<dcon::nation_id>(state, parent);
			if(state.network_mode == sys::network_mode_type::single_player) {
				disabled = n == state.local_player_nation;
			} else {
				// Prevent (via UI) the player from selecting a nation already selected by someone
				disabled = !command::can_notify_player_picks_nation(state, state.local_player_nation, n);
			}
		}

		void button_action(sys::state& state) noexcept override {
			auto n = retrieve<dcon::nation_id>(state, parent);
			if(state.network_mode == sys::network_mode_type::single_player) {
				state.world.nation_set_is_player_controlled(state.local_player_nation, false);
				state.local_player_nation = n;
				state.world.nation_set_is_player_controlled(state.local_player_nation, true);
				if(state.ui_state.nation_picker) {
					state.ui_state.nation_picker->impl_on_update(state);
				}
			} else if(command::can_notify_player_picks_nation(state, state.local_player_nation, n)) {
				command::notify_player_picks_nation(state, state.local_player_nation, n);
			}
		}
	};

	class lobby_playable_nations_row : public listbox_row_element_base<dcon::nation_id> {
	public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
	};

	enum class picker_sort {
		name, mil_rank, indust_rank, p_rank
	};

	class lobby_playable_nations_listbox : public listbox_element_base<lobby_playable_nations_row, dcon::nation_id> {
	protected:
		std::string_view get_row_element_name() override {
			return "playable_countries_list_item";
		}
	public:
		void on_update(sys::state& state) noexcept override;
	};

	template< picker_sort stype>
	class lobby_playable_nations_sort_button : public button_element_base {
		void button_action(sys::state& state) noexcept override {
			send(state, parent, element_selection_wrapper<picker_sort>{stype});
		}
	};

	class playable_nations_window : public window_element_base {
		picker_sort sort_order = picker_sort::name;
		bool is_asc = false;
	public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
		message_result get(sys::state& state, Cyto::Any& payload) noexcept override;
	};

	class date_label : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override;
	};


	class start_game_button : public button_element_base {
	public:
		void button_action(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
		void render(sys::state& state, int32_t x, int32_t y) noexcept override;
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
	};

	class quit_game_button : public button_element_base {
	public:
		void button_action(sys::state& state) noexcept override;
	};

	class multiplayer_status_text : public simple_text_element_base {
		void on_update(sys::state& state) noexcept override {
			auto n = retrieve<dcon::nation_id>(state, parent);
			if(state.network_mode == sys::network_mode_type::host) {
				// on render
			} else {
				set_visible(state, false);
			}
		}
		void render(sys::state& state, int32_t x, int32_t y) noexcept override {
			auto n = retrieve<dcon::nation_id>(state, parent);
			if(state.network_mode == sys::network_mode_type::host) {
				set_text(state, text::produce_simple_string(state, "ready")); // default
				for(auto const& c : state.network_state.clients) {
					if(c.is_active() && c.playing_as == n) {
						auto remaining = c.save_stream_offset - c.total_sent_bytes;
						auto total = c.save_stream_size;
						if(total > 0) {
							float progress = float(remaining) / float(total);
							if(progress < 1.f) {
								set_text(state, text::format_percentage(progress));
							}
						}
						break;
					}
				}
			}
			simple_text_element_base::render(state, x, y);
		}
	};

	class number_of_players_text : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			int32_t count = 0;
			if(state.network_mode == sys::network_mode_type::single_player) {
				count = 1;
			} else {
				state.world.for_each_nation([&](dcon::nation_id n) {
					if(state.world.nation_get_is_player_controlled(n)) {
						count++;
					}
				});
			}
			text::substitution_map sub{};
			text::add_to_substitution_map(sub, text::variable_type::num, count);
			set_text(state, text::resolve_string_substitution(state, "fe_num_players", sub));
		}
	};

	class nation_picker_multiplayer_entry : public listbox_row_element_base<dcon::nation_id> {
	public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
	};

	class nation_picker_multiplayer_listbox : public listbox_element_base<nation_picker_multiplayer_entry, dcon::nation_id> {
	protected:
		std::string_view get_row_element_name() override {
			return "multiplayer_entry_server";
		}
	public:
		void on_update(sys::state& state) noexcept override {
			row_contents.clear();
			if(state.network_mode == sys::network_mode_type::single_player) {
				row_contents.push_back(state.local_player_nation);
			} else {
				state.world.for_each_nation([&](dcon::nation_id n) {
					if(state.world.nation_get_is_player_controlled(n))
					row_contents.push_back(n);
				});
			}
			update(state);
		}
	};

	class lobby_checksum_text : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override;
	};

	class nation_picker_multiplayer_window : public window_element_base {
	public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "multiplayer_list") {
				return make_element_by_type<nation_picker_multiplayer_listbox>(state, id);
			} else if(name == "checksum") {
				return make_element_by_type<lobby_checksum_text>(state, id);
			} else if(name == "num_players") {
				return make_element_by_type<number_of_players_text>(state, id);
			}
			return nullptr;
		}
	};

	class lobby_readme_text : public scrollable_text {
		void populate_layout(sys::state& state, text::endless_layout& contents) noexcept {
			text::add_line(state, contents, "alice_readme");
			text::add_line_break_to_layout(state, contents);
			text::add_line(state, contents, "gc_desc");
		}
	public:
		void on_reset_text(sys::state& state) noexcept override {
			auto container = text::create_endless_layout(state, delegate->internal_layout,
			text::layout_parameters{ 0, 0, static_cast<int16_t>(base_data.size.x), static_cast<int16_t>(base_data.size.y),
				base_data.data.text.font_handle, 0, text::alignment::left,
				text::is_black_from_font_id(base_data.data.text.font_handle) ? text::text_color::black : text::text_color::white,
			false });
			populate_layout(state, container);
			calibrate_scrollbar(state);
		}
		void on_create(sys::state& state) noexcept override {
			scrollable_text::on_create(state);
			on_reset_text(state);
		}
	};

	class nation_picker_container : public window_element_base {
	public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
		message_result get(sys::state& state, Cyto::Any& payload) noexcept  override {
			if(payload.holds_type<dcon::nation_id>()) {
				payload.emplace<dcon::nation_id>(state.local_player_nation);
				return message_result::consumed;
			}
			return window_element_base::get(state, payload);
		}
	};

}
