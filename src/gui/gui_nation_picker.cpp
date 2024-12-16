#include "gui_common_elements.hpp"
#include "gui_nation_picker.hpp"
#include "gui_element_templates.hpp"
#include "gui_diplomacy_request_window.hpp"
#include "gui_message_window.hpp"
#include "gui_message_settings_window.hpp"
#include "gui_diplomacy_request_topbar.hpp"
#include "gui_leader_tooltip.hpp"
#include "gui_land_combat.hpp"
#include "gui_naval_combat.hpp"
#include "gui_event.hpp"
#include "simple_fs.hpp"
#include "serialization.hpp"

namespace ui {
	void save_flag::button_action(sys::state& state) noexcept {
		/* Do nothing */
	}
	void save_flag::on_update(sys::state& state) noexcept  {
		save_item* i = retrieve< save_item*>(state, parent);
		auto tag = i->save_flag;
		auto gov = i->as_gov;
		visible = !i->is_new_game && !i->is_bookmark();
		if(visible) {
			tag = tag ? tag : state.national_definitions.rebel_id;
			dcon::flag_type_id ft{};
			if(gov) {
				auto id = state.world.national_identity_get_government_flag_type(tag, gov);
				ft = id ? id : state.world.government_type_get_flag_type(gov);
			}
			flag_texture_handle = ogl::get_flag_handle(state, tag, ft);
		}
	}
	void save_flag::render(sys::state& state, int32_t x, int32_t y) noexcept {
		if(visible) {
			dcon::gfx_object_id gid;
			if(base_data.get_element_type() == element_type::image) {
				gid = base_data.data.image.gfx_object;
			} else if(base_data.get_element_type() == element_type::button) {
				gid = base_data.data.button.button_image;
			}
			if(gid && flag_texture_handle > 0) {
				auto& gfx_def = state.ui_defs.gfx[gid];
				if(gfx_def.type_dependent) {
					auto mask_handle = ogl::get_texture_handle(state, dcon::texture_id(gfx_def.type_dependent - 1), true);
					auto& mask_tex = state.open_gl.asset_textures[dcon::texture_id(gfx_def.type_dependent - 1)];
					ogl::render_masked_rect(state, get_color_modification(this == state.ui_state.under_mouse, disabled, interactable),
						float(x) + float(base_data.size.x - mask_tex.size_x) * 0.5f,
						float(y) + float(base_data.size.y - mask_tex.size_y) * 0.5f,
						float(mask_tex.size_x),
						float(mask_tex.size_y),
						flag_texture_handle, mask_handle, base_data.get_rotation(), gfx_def.is_vertically_flipped(),
						false);
				} else {
					ogl::render_textured_rect(state, get_color_modification(this == state.ui_state.under_mouse, disabled, interactable),
						float(x), float(y), float(base_data.size.x), float(base_data.size.y), flag_texture_handle, base_data.get_rotation(),
						gfx_def.is_vertically_flipped(),
						false);
				}
			}
			image_element_base::render(state, x, y);
		}
	}

	void save_name::on_update(sys::state& state) noexcept {
		save_item* i = retrieve< save_item*>(state, parent);
		if(i->is_new_game) {
			set_text(state, text::produce_simple_string(state, "fe_new_game"));
		} else if(i->is_bookmark()) {
			set_text(state, text::produce_simple_string(state, i->name));
		} else {
			auto name = state.world.national_identity_get_name(i->save_flag);
			if(auto gov_name = state.world.national_identity_get_government_name(i->save_flag, i->as_gov); state.key_is_localized(gov_name)) {
				name = gov_name;
			}
			if(!i->save_flag || i->save_flag == state.national_definitions.rebel_id) {
				set_text(state, text::produce_simple_string(state, "spectator_game"));
			} else {
				set_text(state, text::produce_simple_string(state, name));
			}
		}
	}

	void start_game_button::button_action(sys::state& state) noexcept {
		if(state.network_mode == sys::network_mode_type::client) {
			//clients cant start the game, only tell that they're "ready"
		} else {
			if(auto cap = state.world.nation_get_capital(state.local_player_nation); cap) {
				if(state.map_state.get_zoom() < map::zoom_very_close) {
					state.map_state.zoom = map::zoom_very_close;
				}
				state.map_state.center_map_on_province(state, cap);
			}
			command::notify_start_game(state, state.local_player_nation);
		}
	}

	void lobby_save_game_listbox::update_save_list(sys::state& state) noexcept {
		row_contents.clear();
		row_contents.push_back(std::make_shared<save_item>(save_item{ NATIVE(""), 0, sys::date(0), dcon::national_identity_id{ }, dcon::government_type_id{ }, true, std::string("") }));
		auto sdir = simple_fs::get_or_create_save_game_directory();
		for(auto& f : simple_fs::list_files(sdir, NATIVE(".bin"))) {
			if(auto of = simple_fs::open_file(f); of) {
				auto content = simple_fs::view_contents(*of);
				sys::save_header h;
				if(content.file_size > sys::sizeof_save_header(h)) {
					sys::read_save_header(reinterpret_cast<uint8_t const*>(content.data), h);
				}
				if(h.checksum.is_equal(state.scenario_checksum)) {
					row_contents.push_back(std::make_shared<save_item>(save_item{ simple_fs::get_file_name(f), h.timestamp, h.d, h.tag, h.cgov, false, std::string(h.save_name) }));
				}
			}
		}
		std::sort(row_contents.begin() + 1, row_contents.end(), [](std::shared_ptr<save_item> const& a, std::shared_ptr<save_item> const& b) {
			if(a->is_bookmark() != b->is_bookmark())
				return a->is_bookmark();
			return a->timestamp > b->timestamp;
		});
		update(state);
	}

	void lobby_save_game_listbox::on_update(sys::state& state) noexcept {
		if(state.save_list_updated.load(std::memory_order::acquire) == true) {
			state.save_list_updated.store(false, std::memory_order::release); // acknowledge update
			update_save_list(state);
		}
	}

	void lobby_select_save_game_button::on_create(sys::state& state) noexcept {
		button_element_base::on_create(state);
		disabled = state.network_mode == sys::network_mode_type::client;
	}

	std::unique_ptr<element_base> nation_details_window::make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept {
		if(name == "player_shield") {
			return make_element_by_type<flag_button>(state, id);
		} else if(name == "flag_overlay") {
			return make_element_by_type<nation_flag_frame>(state, id);
		} else if(name == "selected_nation_totalrank") {
			return make_element_by_type<nation_rank_text>(state, id);
		} else if(name == "selected_nation_label") {
			return make_element_by_type<generic_name_text<dcon::nation_id>>(state, id);
		} else if(name == "selected_countrystatus") {
			return make_element_by_type<nation_status_text>(state, id);
		} else if(name == "selected_fog") {
			return make_element_by_type<nation_government_type_text>(state, id);
		} else if(name == "selected_prestige_rank") {
			return make_element_by_type<nation_prestige_rank_text>(state, id);
		} else if(name == "selected_industry_rank") {
			return make_element_by_type<nation_industry_rank_text>(state, id);
		} else if(name == "selected_military_rank") {
			return make_element_by_type<nation_military_rank_text>(state, id);
		} else if(name == "selected_population_amount") {
			return make_element_by_type<nation_population_text>(state, id);
		} else if(name == "selected_population_chart") {
			// at left
			auto aptr = make_element_by_type<nation_picker_cultures_chart>(state, id);
			if(piechart_offset == 0)
				piechart_offset = aptr->base_data.size.x + 4;
			aptr->base_data.position.x -= piechart_offset;
			add_child_to_front(std::move(aptr));
			// at middle
			auto bptr = make_element_by_type<nation_picker_ideologies_chart>(state, id);
			add_child_to_front(std::move(bptr));
			// at right
			auto cptr = make_element_by_type<nation_picker_poptypes_chart>(state, id);
			cptr->base_data.position.x += piechart_offset;
			// bring overlays on top
			if(overlay1)
				move_child_to_front(overlay1);
			if(overlay2)
				move_child_to_front(overlay2);
			return cptr;
		} else if(name == "selected_population_chart_overlay") {
			// at left
			auto aptr = make_element_by_type<image_element_base>(state, id);
			if(piechart_offset == 0)
				piechart_offset = aptr->base_data.size.x + 4;
			aptr->base_data.position.x -= piechart_offset;
			overlay1 = aptr.get();
			add_child_to_front(std::move(aptr));
			// at middle
			auto bptr = make_element_by_type<image_element_base>(state, id);
			overlay2 = bptr.get();
			add_child_to_front(std::move(bptr));
			// at right
			auto cptr = make_element_by_type<image_element_base>(state, id);
			cptr->base_data.position.x += piechart_offset;
			return cptr;
		} else if(name == "wars_overlappingbox") {
			auto ptr = make_element_by_type<overlapping_enemy_flags>(state, id);
			ptr->base_data.position.y -= 8 - 1; // Nudge
			return ptr;
		} else if(name == "allies_overlappingbox") {
			auto ptr = make_element_by_type<overlapping_ally_flags>(state, id);
			ptr->base_data.position.y -= 8 - 1; // Nudge
			return ptr;
		} else if(name == "puppets_overlappingbox") {
			auto ptr = make_element_by_type<overlapping_puppet_flags>(state, id);
			ptr->base_data.position.y -= 8 - 1; // Nudge
			ptr->base_data.position.x += 20;
			return ptr;
		} else if(name == "puppets_label") {
			auto ptr = make_element_by_type<nation_puppet_list_label>(state, id);
			return ptr;
		}
		return nullptr;
	}

	void lobby_select_save_game_button::button_action(sys::state& state) noexcept {
		save_item* i = retrieve< save_item*>(state, parent);
		if(!i->is_new_game && i->file_name == state.loaded_save_file)
			return;

		window::change_cursor(state, window::cursor_type::busy); //show busy cursor so player doesn't question

		if(state.ui_state.request_window)
			static_cast<ui::diplomacy_request_window*>(state.ui_state.request_window)->messages.clear();
		if(state.ui_state.msg_window)
			static_cast<ui::message_window*>(state.ui_state.msg_window)->messages.clear();
		if(state.ui_state.request_topbar_listbox)
			static_cast<ui::diplomatic_message_topbar_listbox*>(state.ui_state.request_topbar_listbox)->messages.clear();
		if(state.ui_state.msg_log_window)
			static_cast<ui::message_log_window*>(state.ui_state.msg_log_window)->messages.clear();
		for(const auto& win : land_combat_end_popup::land_reports_pool)
			win->set_visible(state, false);
		for(const auto& win : naval_combat_end_popup::naval_reports_pool)
			win->set_visible(state, false);
		ui::clear_event_windows(state);

		// initiate request (first need to set names)
		state.load_file_name = i->file_name;
		state.load_is_new_game = i->is_new_game;
		state.network_state.save_slock.store(true, std::memory_order::release);
	}

	void lobby_select_save_game_button::on_update(sys::state& state) noexcept {
		save_item* i = retrieve< save_item*>(state, parent);
		frame = i->file_name == state.loaded_save_file ? 1 : 0;
	}

	std::unique_ptr<element_base> lobby_playable_nations_row::make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept {
		if(name == "playable_countries_button") {
			auto ptr = make_element_by_type<pick_nation_button>(state, id);
			ptr->base_data.position.x += 9; // Nudge
			ptr->base_data.position.y = 0; // Nudge
			return ptr;
		} else if(name == "playable_countries_flag") {
			auto ptr = make_element_by_type<flag_button>(state, id);
			ptr->base_data.position.x += 9; // Nudge
			ptr->base_data.position.y = 1; // Nudge
			return ptr;
		} else if(name == "country_name") {
			auto ptr = make_element_by_type<generic_name_text<dcon::nation_id>>(state, id);
			ptr->base_data.position.x += 9; // Nudge
			ptr->base_data.position.y = 5; // Nudge
			return ptr;
		} else if(name == "prestige_rank") {
			auto ptr = make_element_by_type<nation_prestige_rank_text>(state, id);
			ptr->base_data.position.x += 9; // Nudge
			ptr->base_data.position.y = 5; // Nudge
			return ptr;
		} else if(name == "industry_rank") {
			auto ptr = make_element_by_type<nation_industry_rank_text>(state, id);
			ptr->base_data.position.x += 9; // Nudge
			ptr->base_data.position.y = 5; // Nudge
			return ptr;
		} else if(name == "military_rank") {
			auto ptr = make_element_by_type<nation_military_rank_text>(state, id);
			ptr->base_data.position.x += 9; // Nudge
			ptr->base_data.position.y = 5; // Nudge
			return ptr;
		}
		return nullptr;
	}

	void lobby_playable_nations_listbox::on_update(sys::state& state) noexcept {
		row_contents.clear();
		for(auto n : state.world.in_nation) {
			if(n.get_owned_province_count() > 0) {
				row_contents.push_back(n);
			}
		}
		auto s = retrieve<picker_sort>(state, parent);
		auto is_asc = retrieve<bool>(state, parent);
		std::function<bool(dcon::nation_id a, dcon::nation_id b)> fn;
		switch(s) {
		case picker_sort::name:
			fn = ([&](dcon::nation_id a, dcon::nation_id b) {
				auto av = text::get_name_as_string(state, fatten(state.world, a));
				auto bv = text::get_name_as_string(state, fatten(state.world, b));
				if(av != bv)
					return av < bv;
				return a.index() < b.index();
			});
			break;
		case picker_sort::mil_rank:
			fn = ([&](dcon::nation_id a, dcon::nation_id b) {
				auto av = state.world.nation_get_military_rank(a);
				auto bv = state.world.nation_get_military_rank(b);
				if(av != bv)
					return av < bv;
				return a.index() < b.index();
			});
			break;
		case picker_sort::indust_rank:
			fn = ([&](dcon::nation_id a, dcon::nation_id b) {
				auto av = state.world.nation_get_industrial_rank(a);
				auto bv = state.world.nation_get_industrial_rank(b);
				if(av != bv)
					return av < bv;
				return a.index() < b.index();
			});
			break;
		case picker_sort::p_rank:
			fn = ([&](dcon::nation_id a, dcon::nation_id b) {
				auto av = state.world.nation_get_prestige_rank(a);
				auto bv = state.world.nation_get_prestige_rank(b);
				if(av != bv)
					return av < bv;
				return a.index() < b.index();
			});
			break;
		}
		auto it = std::unique(row_contents.begin(), row_contents.end(), [&](auto a, auto b) {
			return a.index() == b.index();
		});
		row_contents.erase(it, row_contents.end());
		sys::merge_sort(row_contents.begin(), row_contents.end(), fn);
		if(is_asc) {
			std::reverse(row_contents.begin(), row_contents.end());
		}
		update(state);
	}

	std::unique_ptr<element_base> playable_nations_window::make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept {
		if(name == "lobby_sort_countryname") {
			auto ptr = make_element_by_type<lobby_playable_nations_sort_button<picker_sort::name>>(state, id);
			ptr->base_data.position.y += 1; // Nudge
			return ptr;
		} else if(name == "lobby_sort_prestige") {
			return make_element_by_type<lobby_playable_nations_sort_button<picker_sort::p_rank>>(state, id);
		} else if(name == "lobby_sort_industry") {
			return make_element_by_type<lobby_playable_nations_sort_button<picker_sort::indust_rank>>(state, id);
		} else if(name == "lobby_sort_military") {
			return make_element_by_type<lobby_playable_nations_sort_button<picker_sort::mil_rank>>(state, id);
		} else if(name == "playable_countries_list") {
			return make_element_by_type<lobby_playable_nations_listbox>(state, id);
		}
		return nullptr;
	}

	message_result playable_nations_window::get(sys::state& state, Cyto::Any& payload) noexcept {
		if(payload.holds_type<picker_sort>()) {
			payload.emplace<picker_sort>(sort_order);
			return message_result::consumed;
		} else if(payload.holds_type<element_selection_wrapper<picker_sort>>()) {
			auto v = any_cast<element_selection_wrapper<picker_sort>>(payload);
			if(sort_order == v.data) {
				is_asc = !is_asc;
			}
			sort_order = v.data;
			impl_on_update(state);
			return message_result::consumed;
		}
		return window_element_base::get(state, payload);
	}

	void date_label::on_update(sys::state& state) noexcept {
		text::substitution_map m;
		auto myd = state.current_date.to_ymd(state.start_date);
		text::add_to_substitution_map(m, text::variable_type::year, int64_t(myd.year));
		set_text(state, text::resolve_string_substitution(state, "the_world_in", m));
	}

	void start_game_button::on_update(sys::state& state) noexcept {
		disabled = !bool(state.local_player_nation);
		if(state.network_mode == sys::network_mode_type::client) {
			if(state.network_state.save_stream) { //in the middle of a save stream
				disabled = true;
			}
		}
	}

	void start_game_button::render(sys::state& state, int32_t x, int32_t y) noexcept {
		if(state.network_mode == sys::network_mode_type::host) {
			bool old_disabled = disabled;
			for(auto const& client : state.network_state.clients) {
				if(client.is_active()) {
					disabled = disabled || !client.send_buffer.empty();
				}
			}
			button_element_base::render(state, x, y);
			disabled = old_disabled;
		} else if(state.network_mode == sys::network_mode_type::client) {
			if(state.network_state.save_stream) {
				set_button_text(state, text::format_percentage(float(state.network_state.recv_count) / float(state.network_state.save_data.size())));
			} else {
				set_button_text(state, text::produce_simple_string(state, "ready"));
			}
			button_element_base::render(state, x, y);
		} else {
			button_element_base::render(state, x, y);
		}
	}

	void start_game_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		if(state.network_mode == sys::network_mode_type::client) {
			auto box = text::open_layout_box(contents, 0);
			if(state.network_state.save_stream) {
				text::localised_format_box(state, contents, box, std::string_view("host_is_streaming_save"));
			}
			for(auto const& client : state.network_state.clients) {
				if(client.is_active()) {
					if(!client.send_buffer.empty()) {
						text::substitution_map sub;
						text::add_to_substitution_map(sub, text::variable_type::playername, client.playing_as);
						text::localised_format_box(state, contents, box, std::string_view("alice_play_pending_client"), sub);
					}
				}
			}
			text::close_layout_box(contents, box);
		}
	}

	void quit_game_button::button_action(sys::state& state) noexcept {
		window::close_window(state);
	}

	std::unique_ptr<element_base> nation_picker_multiplayer_entry::make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept {
		if(name == "player_shield") {
			auto ptr = make_element_by_type<flag_button>(state, id);
			ptr->base_data.position.x += 10; // Nudge
			ptr->base_data.position.y += 7; // Nudge
			return ptr;
		} else if(name == "name") {
			auto ptr = make_element_by_type<player_name_text>(state, id);
			ptr->base_data.position.x += 10; // Nudge
			ptr->base_data.position.y += 7; // Nudge
			return ptr;
		} else if(name == "save_progress") {
			auto ptr = make_element_by_type<multiplayer_status_text>(state, id);
			ptr->base_data.position.x += 10; // Nudge
			ptr->base_data.position.y += 7; // Nudge
			return ptr;
		} else if(name == "button_kick") {
			auto ptr = make_element_by_type<player_kick_button>(state, id);
			ptr->base_data.position.x += 10; // Nudge
			ptr->base_data.position.y += 7; // Nudge
			return ptr;
		} else if(name == "button_ban") {
			auto ptr = make_element_by_type<player_ban_button>(state, id);
			ptr->base_data.position.x += 10; // Nudge
			ptr->base_data.position.y += 7; // Nudge
			return ptr;
		} else if(name == "frontend_player_entry") {
			auto ptr = make_element_by_type<image_element_base>(state, id);
			ptr->base_data.position.x += 10; // Nudge
			ptr->base_data.position.y += 7; // Nudge
			return ptr;
		} else {
			return nullptr;
		}
	}

	void lobby_checksum_text::on_update(sys::state& state) noexcept {
		std::string s;
		s += std::string(__TIME__);
		s += "-";
		uint32_t i_checksum = 0;
		auto const& k = state.network_mode == sys::network_mode_type::host
		? state.network_state.current_save_checksum
		: state.session_host_checksum;
		for(uint32_t i = 0; i < k.key_size; i++) {
			i_checksum += k.key[i];
		}
		static const std::string_view alnum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		s += alnum[(i_checksum >> 0) % alnum.size()];
		s += alnum[(i_checksum >> 2) % alnum.size()];
		s += alnum[(i_checksum >> 4) % alnum.size()];
		s += alnum[(i_checksum >> 6) % alnum.size()];
		s += alnum[(i_checksum >> 8) % alnum.size()];
		s += alnum[(i_checksum >> 10) % alnum.size()];
		s += alnum[(i_checksum >> 12) % alnum.size()];
		s += alnum[(i_checksum >> 14) % alnum.size()];
		s += alnum[(i_checksum >> 16) % alnum.size()];
		s += alnum[(i_checksum >> 18) % alnum.size()];
		s += alnum[(i_checksum >> 20) % alnum.size()];
		s += alnum[(i_checksum >> 22) % alnum.size()];
		s += alnum[(i_checksum >> 24) % alnum.size()];
		s += alnum[(i_checksum >> 26) % alnum.size()];
		s += alnum[(i_checksum >> 28) % alnum.size()];
		s += alnum[(i_checksum >> 30) % alnum.size()];
		set_text(state, s);
	}

	std::unique_ptr<element_base> nation_picker_container::make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept {
		if(name == "lobby_chat_edit") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "newgame_tab") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "savedgame_tab") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "year_label") {
			return make_element_by_type<date_label>(state, id);
		} else if(name == "multiplayer") {
			return make_element_by_type<nation_picker_multiplayer_window>(state, id);
		} else if(name == "singleplayer") {
			return make_element_by_type<nation_details_window>(state, id);
		} else if(name == "save_games") {
			return make_element_by_type<saves_window>(state, id);
		} else if(name == "bookmarks") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "playable_countries_window") {
			return make_element_by_type<playable_nations_window>(state, id);
		} else if(name == "back_button") {
			return make_element_by_type<quit_game_button>(state, id);
		} else if(name == "play_button") {
			return make_element_by_type<start_game_button>(state, id);
		} else if(name == "chatlog") {
			auto ptr = make_element_by_type<lobby_readme_text>(state, state.ui_state.defs_by_name.find(state.lookup_key("alice_readme_text"))->second.definition);
			add_child_to_front(std::move(ptr));
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "frontend_chat_bg") {
			return make_element_by_type<partially_transparent_image>(state, id);
		} else if(name == "frontend_lobby_leftbg") {
			return make_element_by_type<partially_transparent_image>(state, id);
		} else if(name == "frontend_lobby_rightbg") {
			return make_element_by_type<partially_transparent_image>(state, id);
		} else if(name == "frontend_chat_bg") {
			return make_element_by_type<partially_transparent_image>(state, id);
		} else if(name == "frontend_lobby_cornerleft") {
			return make_element_by_type<partially_transparent_image>(state, id);
		} else if(name == "frontend_lobby_cornerright") {
			return make_element_by_type<partially_transparent_image>(state, id);
		}
		return nullptr;
	}
}
