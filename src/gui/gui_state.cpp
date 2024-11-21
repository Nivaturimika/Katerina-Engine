#include <algorithm>
#include <functional>
#include <thread>

#include "system_state.hpp"
#include "dcon_generated.hpp"
#include "constants.hpp"
#include "opengl_wrapper.hpp"
#include "prng.hpp"
#include "demographics.hpp"
#include "blake2.h"
#include "reports.hpp"

#include "gui_element_base.hpp"
#include "gui_element_templates.hpp"
#include "gui_leader_select.hpp"
#include "gui_diplomacy_request_window.hpp"
#include "gui_console.hpp"
#include "gui_message_window.hpp"
#include "gui_event.hpp"
#include "gui_message_settings_window.hpp"
#include "gui_chat_window.hpp"
#include "gui_error_window.hpp"
#include "gui_diplomacy_request_topbar.hpp"
#include "gui_production_window.hpp"
#include "gui_diplomacy_window.hpp"
#include "gui_technology_window.hpp"
#include "gui_politics_window.hpp"
#include "gui_budget_window.hpp"
#include "gui_trade_window.hpp"
#include "gui_population_window.hpp"
#include "gui_military_window.hpp"
#include "gui_naval_combat.hpp"
#include "gui_land_combat.hpp"
#include "gui_nation_picker.hpp"
#include "gui_state_select.hpp"
#include "gui_minimap.hpp"
#include "gui_unit_panel.hpp"
#include "gui_topbar.hpp"
#include "gui_province_window.hpp"
#include "gui_outliner_window.hpp"
#include "gui_leader_tooltip.hpp"
#include "gui_end_window.hpp"
#include "gui_map_legend.hpp"
#include "gui_map_icons.hpp"


namespace ui {
	void create_in_game_windows(sys::state& state) {
		state.ui_state.lazy_load_in_game = true;
		//
		state.ui_state.select_states_legend = ui::make_element_by_type<ui::map_state_select_window>(state, "alice_select_legend_window");
		state.ui_state.end_screen = std::make_unique<ui::container_base>();
		{
			auto ewin = ui::make_element_by_type<ui::end_window>(state, "back_end");
			state.ui_state.end_screen->add_child_to_front(std::move(ewin));
		}

		/*
		state.world.for_each_province([&](dcon::province_id id) {
			if(state.world.province_get_port_to(id)) {
				auto ptr = ui::make_element_by_type<ui::port_window>(state, "alice_port_icon");
				static_cast<ui::port_window*>(ptr.get())->set_province(state, id);
				state.ui_state.units_root->add_child_to_front(std::move(ptr));
			}
		});
		*/
		province::for_each_land_province(state, [&](dcon::province_id id) {
			auto ptr = ui::make_element_by_type<ui::mobilization_counter_window>(state, "mobilization_mapicon");
			if(ptr.get()) {
				static_cast<ui::mobilization_counter_window*>(ptr.get())->prov = id;
				state.ui_state.units_root->add_child_to_front(std::move(ptr));
			}
		});
		for(const auto sdef : state.world.in_state_definition) {
			auto prange = sdef.get_abstract_state_membership();
			if(prange.begin() != prange.end()) {
				auto ptr = ui::make_element_by_type<ui::colonization_counter_window>(state, "colonization_mapicon");
				if(ptr.get()) {
					static_cast<ui::colonization_counter_window*>(ptr.get())->prov = (*prange.begin()).get_province();
					state.ui_state.colonizations_root->add_child_to_front(std::move(ptr));
				}
			}
		}
		province::for_each_land_province(state, [&](dcon::province_id id) {
			auto ptr = ui::make_element_by_type<ui::siege_counter_window>(state, "siege_mapicon");
			if(ptr.get()) {
				static_cast<ui::siege_counter_window*>(ptr.get())->prov = id;
				state.ui_state.units_root->add_child_to_front(std::move(ptr));
			}
		});
		province::for_each_land_province(state, [&](dcon::province_id id) {
			auto ptr = ui::make_element_by_type<ui::unit_counter_window<unit_counter_position_type::land>>(state, "unit_mapicon");
			if(ptr.get()) {
				static_cast<ui::unit_counter_window<unit_counter_position_type::land>*>(ptr.get())->prov = id;
				state.ui_state.units_root->add_child_to_front(std::move(ptr));
			}
		});
		province::for_each_sea_province(state, [&](dcon::province_id id) {
			auto ptr = ui::make_element_by_type<ui::unit_counter_window<unit_counter_position_type::sea>>(state, "unit_mapicon");
			if(ptr.get()) {
				static_cast<ui::unit_counter_window<unit_counter_position_type::sea>*>(ptr.get())->prov = id;
				state.ui_state.units_root->add_child_to_front(std::move(ptr));
			}
		});
		province::for_each_land_province(state, [&](dcon::province_id id) {
			auto ptr = ui::make_element_by_type<ui::unit_counter_window<unit_counter_position_type::land_move>>(state, "unit_mapicon");
			if(ptr.get()) {
				static_cast<ui::unit_counter_window<unit_counter_position_type::land_move>*>(ptr.get())->prov = id;
				state.ui_state.units_root->add_child_to_front(std::move(ptr));
			}
		});
		province::for_each_land_province(state, [&](dcon::province_id id) {
			if(state.world.province_get_port_to(id)) {
				auto ptr = ui::make_element_by_type<ui::unit_counter_window<unit_counter_position_type::port>>(state, "unit_mapicon");
				if(ptr.get()) {
					static_cast<ui::unit_counter_window<unit_counter_position_type::port>*>(ptr.get())->prov = id;
					state.ui_state.units_root->add_child_to_front(std::move(ptr));
				}
			}
		});
		state.world.for_each_province([&](dcon::province_id id) {
			auto ptr = ui::make_element_by_type<ui::battle_counter_window>(state, "combat_mapicon");
			if(ptr.get()) {
				static_cast<ui::battle_counter_window*>(ptr.get())->prov = id;
				state.ui_state.units_root->add_child_to_front(std::move(ptr));
			}
		});

		province::for_each_land_province(state, [&](dcon::province_id id) {
			auto ptr = ui::make_element_by_type<ui::rgo_icon>(state, "alice_rgo_mapicon");
			static_cast<ui::rgo_icon*>(ptr.get())->content = id;
			state.ui_state.rgos_root->add_child_to_front(std::move(ptr));
		});
		province::for_each_land_province(state, [&](dcon::province_id id) {
			auto ptr = ui::make_element_by_type<ui::province_details_container>(state, "alice_province_values");
			static_cast<ui::province_details_container*>(ptr.get())->prov = id;
			state.ui_state.province_details_root->add_child_to_front(std::move(ptr));
		});
		{
			auto new_elm = ui::make_element_by_type<ui::chat_message_listbox<false>>(state, "chat_list");
			if(new_elm.get()) {
				new_elm->base_data.position.x += 156; // nudge
				new_elm->base_data.position.y += 24; // nudge
				new_elm->impl_on_update(state);
				state.ui_state.tl_chat_list = new_elm.get();
				state.ui_state.root->add_child_to_front(std::move(new_elm));
			}
		}
		{
			auto new_elm = ui::make_element_by_type<ui::outliner_window>(state, "outliner");
			if(new_elm.get()) {
				state.ui_state.outliner_window = new_elm.get();
				new_elm->impl_on_update(state);
				state.ui_state.root->add_child_to_front(std::move(new_elm));
				// Has to be created AFTER the outliner window
				// The topbar has this button within, however since the button isn't properly displayed, it is better to make
				// it into an independent element of it's own, living freely on the UI root so it can be flexibly moved around when
				// the window is resized for example.
				for(size_t i = state.ui_defs.gui.size(); i-- > 0;) {
					auto gdef = dcon::gui_def_id(dcon::gui_def_id::value_base_t(i));
					if(state.to_string_view(state.ui_defs.gui[gdef].name) == "topbar_outlinerbutton_bg") {
						auto new_bg = ui::make_element_by_type<ui::outliner_button>(state, gdef);
						state.ui_state.root->add_child_to_front(std::move(new_bg));
						break;
					}
				}
				// Then create button atop
				for(size_t i = state.ui_defs.gui.size(); i-- > 0;) {
					auto gdef = dcon::gui_def_id(dcon::gui_def_id::value_base_t(i));
					if(state.to_string_view(state.ui_defs.gui[gdef].name) == "topbar_outlinerbutton") {
						auto new_btn = ui::make_element_by_type<ui::outliner_button>(state, gdef);
						new_btn->impl_on_update(state);
						state.ui_state.root->add_child_to_front(std::move(new_btn));
						break;
					}
				}
			}
		}
		{
			auto new_elm = ui::make_element_by_type<ui::topbar_window>(state, "topbar");
			if(new_elm.get()) {
				new_elm->impl_on_update(state);
				state.ui_state.root->add_child_to_front(std::move(new_elm));
			}
		}
		{
			auto new_elm = ui::make_element_by_type<ui::minimap_container_window>(state, "alice_menubar");
			state.ui_state.menubar_window = new_elm.get();
			state.ui_state.root->add_child_to_front(std::move(new_elm));
		}
		{
			auto new_elm = ui::make_element_by_type<ui::minimap_picture_window>(state, "minimap_pic");
			state.ui_state.root->add_child_to_front(std::move(new_elm));
		}
		{
			auto new_elm = ui::make_element_by_type<ui::province_view_window>(state, "province_view");
			state.ui_state.root->add_child_to_front(std::move(new_elm));
		}
		{
			auto new_elm_army = ui::make_element_by_type<ui::unit_details_window<dcon::army_id>>(state, "sup_unit_status");
			if(new_elm_army.get()) {
				state.ui_state.army_status_window = static_cast<ui::unit_details_window<dcon::army_id>*>(new_elm_army.get());
				new_elm_army->set_visible(state, false);
				state.ui_state.root->add_child_to_front(std::move(new_elm_army));
			}
			auto new_elm_navy = ui::make_element_by_type<ui::unit_details_window<dcon::navy_id>>(state, "sup_unit_status");
			if(new_elm_navy.get()) {
				state.ui_state.navy_status_window = static_cast<ui::unit_details_window<dcon::navy_id>*>(new_elm_navy.get());
				new_elm_navy->set_visible(state, false);
				state.ui_state.root->add_child_to_front(std::move(new_elm_navy));
			}
		}
		{
			auto mselection = ui::make_element_by_type<ui::mulit_unit_selection_panel>(state, "multi_unitpanel");
			if(mselection.get()) {
				state.ui_state.multi_unit_selection_window = mselection.get();
				mselection->set_visible(state, false);
				state.ui_state.root->add_child_to_front(std::move(mselection));
			}
		}
		{
			auto win = make_element_by_type<unit_reorg_window<dcon::army_id, dcon::regiment_id>>(state, "reorg_window");
			if(win.get()) {
				win->set_visible(state, false);
				state.ui_state.army_reorg_window = win.get();
				state.ui_state.root->add_child_to_front(std::move(win));
			}
		}
		{
			auto win = make_element_by_type<unit_reorg_window<dcon::navy_id, dcon::ship_id>>(state, "reorg_window");
			if(win.get()) {
				win->set_visible(state, false);
				state.ui_state.navy_reorg_window = win.get();
				state.ui_state.root->add_child_to_front(std::move(win));
			}
		}
		{
			auto new_elm = ui::make_element_by_type<ui::diplomacy_request_window>(state, "defaultdialog");
			state.ui_state.request_window = new_elm.get();
			state.ui_state.root->add_child_to_front(std::move(new_elm));
		}
		{
			auto new_elm = ui::make_element_by_type<ui::message_window>(state, "defaultpopup");
			state.ui_state.msg_window = new_elm.get();
			state.ui_state.root->add_child_to_front(std::move(new_elm));
		}
		{
			auto new_elm = ui::make_element_by_type<ui::leader_selection_window>(state, "alice_leader_selection_panel");
			if(new_elm.get()) {
				new_elm->set_visible(state, false);
				state.ui_state.change_leader_window = new_elm.get();
				state.ui_state.root->add_child_to_front(std::move(new_elm));
			}
		}
		{
			auto new_elm = ui::make_element_by_type<ui::naval_combat_end_popup>(state, "endofnavalcombatpopup");
			if(new_elm.get()) {
				new_elm->set_visible(state, false);
				state.ui_state.root->add_child_to_front(std::move(new_elm));
			}
		}
		{ // And the other on the normal in game UI
			auto new_elm = ui::make_element_by_type<ui::chat_window>(state, "ingame_lobby_window");
			if(new_elm.get()) {
				new_elm->set_visible(state, !(state.network_mode == sys::network_mode_type::single_player)); // Hidden in singleplayer by default
				state.ui_state.chat_window = new_elm.get(); // Default for singleplayer is the in-game one, lobby one is useless in sp
				state.ui_state.root->add_child_to_front(std::move(new_elm));
			}
		}
		{
			auto new_elm = make_element_by_type<news_icon_window>(state, "news_icon");
			state.ui_state.root->add_child_to_front(std::move(new_elm));
		}
		{
			auto new_elm = make_element_by_type<news_page_window>(state, "news_window_default");
			if(new_elm.get()) {
				state.ui_state.news_page_window = new_elm.get();
				new_elm->set_visible(state, false);
				state.ui_state.root->add_child_to_front(std::move(new_elm));
			}
		}
		state.ui_state.rgos_root->impl_on_update(state);
		state.ui_state.units_root->impl_on_update(state);
		state.ui_state.colonizations_root->impl_on_update(state);
	}
}
