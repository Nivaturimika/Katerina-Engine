#include "gui_production_window.hpp"
#include "gui_factory_buttons_window.hpp"
#include "gui_invest_brow_window.hpp"
#include "gui_pop_sort_buttons_window.hpp"
#include "gui_commodity_filters_window.hpp"
#include "gui_projects_window.hpp"
#include "gui_build_factory_window.hpp"
#include "gui_project_investment_window.hpp"
#include "gui_foreign_investment_window.hpp"

namespace ui {

void populate_production_states_list(sys::state& state, std::vector<dcon::state_instance_id>& row_contents, dcon::nation_id n, bool show_empty, production_sort_order sort_order) {

	for(auto const fat_id : state.world.nation_get_state_ownership(n)) {
		if(show_empty && !fat_id.get_state().get_capital().get_is_colonial()) {
			row_contents.push_back(fat_id.get_state());
		} else if(economy::has_factory(state, fat_id.get_state().id)) {
			// Then account for factories **hidden** by the filter from goods...
			size_t count = 0;
			province::for_each_province_in_state_instance(state, fat_id.get_state(), [&](dcon::province_id pid) {
				auto ffact_id = dcon::fatten(state.world, pid);
				ffact_id.for_each_factory_location_as_province([&](dcon::factory_location_id flid) {
					auto fid = state.world.factory_location_get_factory(flid);
					Cyto::Any payload = commodity_filter_query_data{
							state.world.factory_type_get_output(state.world.factory_get_building_type(fid)).id, false};
					state.ui_state.production_subwindow->impl_get(state, payload);
					auto content = any_cast<commodity_filter_query_data>(payload);
					count += content.filter ? 1 : 0;
				});
			});
			for(auto construct : fat_id.get_state().get_state_building_construction()) {
				Cyto::Any payload = commodity_filter_query_data{construct.get_type().get_output().id, false};
				state.ui_state.production_subwindow->impl_get(state, payload);
				auto content = any_cast<commodity_filter_query_data>(payload);
				count += content.filter ? 1 : 0;
			}
			if(count > 0)
				row_contents.push_back(fat_id.get_state());
		}
	}

	auto sort_by_name = [&](dcon::state_instance_id a, dcon::state_instance_id b) {
		auto a_name = text::produce_simple_string(state, state.world.state_definition_get_name(state.world.state_instance_get_definition(a)));
		auto b_name = text::produce_simple_string(state, state.world.state_definition_get_name(state.world.state_instance_get_definition(b)));
		return a_name < b_name;
	};
	auto sort_by_factories = [&](dcon::state_instance_id a, dcon::state_instance_id b) {
		auto acount = economy::state_factory_count(state, a);
		auto bcount = economy::state_factory_count(state, b);
		return acount > bcount;
	};
	auto sort_by_primary_workers = [&](dcon::state_instance_id a, dcon::state_instance_id b) {
		return state.world.state_instance_get_demographics(a,
							 demographics::to_key(state, state.culture_definitions.primary_factory_worker)) >
					 state.world.state_instance_get_demographics(b,
							 demographics::to_key(state, state.culture_definitions.primary_factory_worker));
	};
	auto sort_by_secondary_workers = [&](dcon::state_instance_id a, dcon::state_instance_id b) {
		return state.world.state_instance_get_demographics(a,
							 demographics::to_key(state, state.culture_definitions.secondary_factory_worker)) >
					 state.world.state_instance_get_demographics(b,
							 demographics::to_key(state, state.culture_definitions.secondary_factory_worker));
	};
	auto sort_by_owners = [&](dcon::state_instance_id a, dcon::state_instance_id b) {
		auto atotal = state.world.state_instance_get_demographics(a, demographics::total);
		auto btotal = state.world.state_instance_get_demographics(b, demographics::total);
		return state.world.state_instance_get_demographics(a, demographics::to_key(state, state.culture_definitions.capitalists)) /
							 atotal >
					 state.world.state_instance_get_demographics(b, demographics::to_key(state, state.culture_definitions.capitalists)) /
							 btotal;
	};
	auto sort_by_infrastructure = [&](dcon::state_instance_id a, dcon::state_instance_id b) {
		float atotal = 0.0f;
		float ap_total = 0.0f;
		province::for_each_province_in_state_instance(state, a, [&](dcon::province_id p) {
			atotal += float(state.world.province_get_building_level(p, economy::province_building_type::railroad));
			ap_total += 1.0f;
		});
		float btotal = 0.0f;
		float bp_total = 0.0f;
		province::for_each_province_in_state_instance(state, b, [&](dcon::province_id p) {
			btotal += float(state.world.province_get_building_level(p, economy::province_building_type::railroad));
			bp_total += 1.0f;
		});
		return atotal / ap_total > btotal / bp_total;
	};
	auto sort_by_focus = [&](dcon::state_instance_id a, dcon::state_instance_id b) {
		auto a_focus = state.world.state_instance_get_owner_focus(a);
		auto b_focus = state.world.state_instance_get_owner_focus(b);
		return a_focus.id.value > b_focus.id.value;
	};

	switch(sort_order) {
	case production_sort_order::name:
		std::sort(row_contents.begin(), row_contents.end(), sort_by_name);
		break;
	case production_sort_order::factories:
		std::sort(row_contents.begin(), row_contents.end(), sort_by_factories);
		break;
	case production_sort_order::primary_workers:
		std::sort(row_contents.begin(), row_contents.end(), sort_by_primary_workers);
		break;
	case production_sort_order::secondary_workers:
		std::sort(row_contents.begin(), row_contents.end(), sort_by_secondary_workers);
		break;
	case production_sort_order::owners:
		std::sort(row_contents.begin(), row_contents.end(), sort_by_owners);
		break;
	case production_sort_order::infrastructure:
		std::sort(row_contents.begin(), row_contents.end(), sort_by_infrastructure);
		break;
	case production_sort_order::focus:
		std::sort(row_contents.begin(), row_contents.end(), sort_by_focus);
		break;
	}
}

void open_foreign_investment(sys::state& state, dcon::nation_id n) {
	if(state.ui_state.topbar_subwindow->is_visible()) {
		state.ui_state.topbar_subwindow->set_visible(state, false);
	}
	state.ui_state.production_subwindow->set_visible(state, true);
	state.ui_state.root->move_child_to_front(state.ui_state.production_subwindow);
	state.ui_state.topbar_subwindow = state.ui_state.production_subwindow;

	send(state, state.ui_state.production_subwindow, open_investment_nation{n});
}

void open_build_factory(sys::state& state, dcon::state_instance_id st) {
	state.open_production();
	if(state.ui_state.production_subwindow) {
		auto owner = state.world.state_instance_get_nation_from_state_ownership(st);
		if(owner != state.local_player_nation) {
			send(state, state.ui_state.production_subwindow, open_investment_nation{ owner });
		} else {
			send(state, state.ui_state.production_subwindow, production_window_tab::factories);
		}
		send(state, state.ui_state.production_subwindow, production_selection_wrapper{ st, true, xy_pair{0, 0} });
	}
}

void production_window::on_create(sys::state& state) noexcept {
	generic_tabbed_window::on_create(state);

	// All filters enabled by default
	commodity_filters.resize(state.world.commodity_size(), true);

	for(curr_commodity_group = sys::commodity_group::military_goods; curr_commodity_group != sys::commodity_group::count;
			curr_commodity_group = static_cast<sys::commodity_group>(uint8_t(curr_commodity_group) + 1)) {

		bool is_empty = true;
		for(auto id : state.world.in_commodity) {
			if(sys::commodity_group(state.world.commodity_get_commodity_group(id)) != curr_commodity_group || !bool(id) || id == economy::money)
				continue;
			is_empty = false;
		}
		if(is_empty)
			continue;

		commodity_offset.x = base_commodity_offset.x;

		// Place legend for this category...
		auto ptr = make_element_by_type<production_goods_category_name>(state,
				state.ui_state.defs_by_name.find(state.lookup_key("production_goods_name"))->second.definition);
		ptr->base_data.position = commodity_offset;
		Cyto::Any payload = curr_commodity_group;
		ptr->impl_set(state, payload);
		ptr->set_visible(state, false);
		commodity_offset.y += ptr->base_data.size.y;
		good_elements.push_back(ptr.get());
		add_child_to_front(std::move(ptr));

		int16_t cell_height = 0;
		// Place infoboxes for each of the goods...
		for(auto id : state.world.in_commodity) {
			if(sys::commodity_group(state.world.commodity_get_commodity_group(id)) != curr_commodity_group || !bool(id) || id == economy::money)
				continue;

			auto info_ptr = make_element_by_type<production_good_info>(state,
					state.ui_state.defs_by_name.find(state.lookup_key("production_info"))->second.definition);
			info_ptr->base_data.position = commodity_offset;
			info_ptr->set_visible(state, false);

			int16_t cell_width = info_ptr->base_data.size.x;
			cell_height = info_ptr->base_data.size.y;

			commodity_offset.x += cell_width;
			if(commodity_offset.x + cell_width >= base_data.size.x) {
				commodity_offset.x = base_commodity_offset.x;
				commodity_offset.y += cell_height;
			}

			info_ptr.get()->commodity_id = id;

			good_elements.push_back(info_ptr.get());
			add_child_to_front(std::move(info_ptr));
		}
		// Has atleast 1 good on this row? skip to next row then...
		if(commodity_offset.x > base_commodity_offset.x)
			commodity_offset.y += cell_height;
	}

	{
		auto ptr = make_element_by_type<national_focus_window>(state, "state_focus_window");
		ptr->set_visible(state, false);
		nf_win = ptr.get();
		add_child_to_front(std::move(ptr));
	}

	auto win = make_element_by_type<factory_build_window>(state, state.ui_state.defs_by_name.find(state.lookup_key("build_factory"))->second.definition);
	build_win = win.get();
	add_child_to_front(std::move(win));

	auto win2 = make_element_by_type<project_investment_window>(state,
			state.ui_state.defs_by_name.find(state.lookup_key("invest_project_window"))->second.definition);
	win2->set_visible(state, false);
	project_window = win2.get();
	add_child_to_front(std::move(win2));

	show_output_commodity = std::unique_ptr<bool[]>(new bool[state.world.commodity_size()]);
	set_visible(state, false);
}

std::unique_ptr<element_base> production_window::make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept {
	if(name == "main_bg") {
		return make_element_by_type<image_element_base>(state, id);
	} else if(name == "prod_bg") {
		return make_element_by_type<opaque_element_base>(state, id);
	} else if(name == "close_button") {
		return make_element_by_type<generic_close_button>(state, id);
	} else if(name == "tab_factories") {
		auto ptr = make_element_by_type<generic_tab_button<production_window_tab>>(state, id);
		ptr->target = production_window_tab::factories;
		return ptr;
	} else if(name == "tab_invest") {
		auto ptr = make_element_by_type<generic_tab_button<production_window_tab>>(state, id);
		ptr->target = production_window_tab::investments;
		return ptr;
	} else if(name == "tab_popprojects") {
		auto ptr = make_element_by_type<generic_tab_button<production_window_tab>>(state, id);
		ptr->target = production_window_tab::projects;
		return ptr;
	} else if(name == "tab_goodsproduction") {
		auto ptr = make_element_by_type<generic_tab_button<production_window_tab>>(state, id);
		ptr->target = production_window_tab::goods;
		return ptr;
	} else if(name == "tab_factories_text") {
		return make_element_by_type<invisible_element>(state, id);
	} else if(name == "tab_invest_text") {
		return make_element_by_type<invisible_element>(state, id);
	} else if(name == "tab_goodsproduction_text") {
		return make_element_by_type<invisible_element>(state, id);
	} else if(name == "tab_popprojects_text") {
		return make_element_by_type<invisible_element>(state, id);
	} else if(name == "factory_buttons") {
		auto ptr = make_element_by_type<factory_buttons_window>(state, id);
		factory_elements.push_back(ptr.get());
		ptr->set_visible(state, true);
		return ptr;
	} else if(name == "invest_buttons") {
		auto ptr = make_element_by_type<production_foreign_investment_window>(state, id);
		foreign_invest_win = ptr.get();
		investment_nation.push_back(ptr.get());
		ptr->set_visible(state, false);
		return ptr;
	} else if(name == "state_listbox") {
		auto ptr = make_element_by_type<production_state_listbox>(state, id);
		state_listbox = ptr.get();
		factory_elements.push_back(ptr.get());
		ptr->set_visible(state, true);
		return ptr;
	} else if(name == "state_listbox_invest") {
		auto ptr = make_element_by_type<production_state_invest_listbox>(state, id);
		state_listbox_invest = ptr.get();
		investment_nation.push_back(ptr.get());
		ptr->set_visible(state, false);
		return ptr;
	} else if(name == "investment_browser") {
		auto ptr = make_element_by_type<invest_brow_window>(state, id);
		investment_brow_elements.push_back(ptr.get());
		ptr->set_visible(state, false);
		return ptr;
	} else if(name == "sort_by_state") {
		auto ptr = make_element_by_type<button_element_base>(state, id);
		project_elements.push_back(ptr.get());
		ptr->set_visible(state, false);
		return ptr;
	} else if(name == "sort_by_projects") {
		auto ptr = make_element_by_type<button_element_base>(state, id);
		project_elements.push_back(ptr.get());
		ptr->set_visible(state, false);
		return ptr;
	} else if(name == "sort_by_completion") {
		auto ptr = make_element_by_type<button_element_base>(state, id);
		project_elements.push_back(ptr.get());
		ptr->set_visible(state, false);
		return ptr;
	} else if(name == "sort_by_projecteers") {
		auto ptr = make_element_by_type<button_element_base>(state, id);
		project_elements.push_back(ptr.get());
		ptr->set_visible(state, false);
		return ptr;
	} else if(name == "pop_sort_buttons") {
		auto ptr = make_element_by_type<pop_sort_buttons_window>(state, id);
		factory_elements.push_back(ptr.get());
		ptr->set_visible(state, true);
		return ptr;
	} else if(name == "project_listbox") {
		auto ptr = make_element_by_type<production_project_listbox>(state, id);
		project_elements.push_back(ptr.get());
		ptr->set_visible(state, false);
		return ptr;
	} else {
		return nullptr;
	}
}

message_result production_window::get(sys::state& state, Cyto::Any& payload) noexcept {
	if(payload.holds_type<production_window_tab>()) {
		auto enum_val = any_cast<production_window_tab>(payload);
		active_tab = enum_val;
		hide_sub_windows(state);
		switch(enum_val) {
		case production_window_tab::factories:
			set_visible_vector_elements(state, factory_elements, true);
			break;
		case production_window_tab::investments:
			set_visible_vector_elements(state, investment_brow_elements, true);
			foreign_invest_win->set_visible(state, open_foreign_invest);
			break;
		case production_window_tab::projects:
			set_visible_vector_elements(state, project_elements, true);
			break;
		case production_window_tab::goods:
			set_visible_vector_elements(state, good_elements, true);
			break;
		}
		return message_result::consumed;
	} else if(payload.holds_type<production_foreign_invest_target>()) {
		payload.emplace<production_foreign_invest_target>(production_foreign_invest_target{ foreign_invest_win->curr_nation });
		return message_result::consumed;
	} else if(payload.holds_type<open_investment_nation>()) {
		hide_sub_windows(state);
		auto target = any_cast<open_investment_nation>(payload).id;
		active_tab = production_window_tab::investments;
		foreign_invest_win->curr_nation = target;
		set_visible_vector_elements(state, investment_nation, true);
		return message_result::consumed;
	} else if(payload.holds_type<production_sort_order>()) {
		auto sort_type = any_cast<production_sort_order>(payload);
		state_listbox->sort_order = sort_type;
		if(state_listbox->is_visible())
			state_listbox->impl_on_update(state);
		state_listbox_invest->sort_order = sort_type;
		if(state_listbox_invest->is_visible())
			state_listbox_invest->impl_on_update(state);
	} else if(payload.holds_type<dcon::state_instance_id>()) {
		payload.emplace<dcon::state_instance_id>(focus_state);
		return message_result::consumed;
	} else if(payload.holds_type<production_selection_wrapper>()) {
		auto data = any_cast<production_selection_wrapper>(payload);
		focus_state = data.data;
		if(data.is_build) {
			build_win->set_visible(state, true);
			move_child_to_front(build_win);
		} else {
			nf_win->set_visible(state, true);
			nf_win->base_data.position = data.focus_pos;
			move_child_to_front(nf_win);
		}
		impl_on_update(state);
		return message_result::consumed;
	} else if(payload.holds_type<bool>()) {
		payload.emplace<bool>(show_empty_states);
		return message_result::consumed;
	} else if(payload.holds_type<element_selection_wrapper<bool>>()) {
		show_empty_states = any_cast<element_selection_wrapper<bool>>(payload).data;
		impl_on_update(state);
		return message_result::consumed;
	} else if(payload.holds_type<commodity_filter_query_data>()) {
		auto content = any_cast<commodity_filter_query_data>(payload);
		content.filter = commodity_filters[content.cid.index()];
		payload.emplace<commodity_filter_query_data>(content);
		return message_result::consumed;
	} else if(payload.holds_type<commodity_filter_toggle_data>()) {
		auto content = any_cast<commodity_filter_toggle_data>(payload);
		commodity_filters[content.data.index()] = !commodity_filters[content.data.index()];
		impl_on_update(state);
		return message_result::consumed;
	} else if(payload.holds_type<element_selection_wrapper<production_action>>()) {
		auto content = any_cast<element_selection_wrapper<production_action>>(payload).data;
		switch(content) {
		case production_action::investment_window:
			project_window->is_visible() ? project_window->set_visible(state, false) : project_window->set_visible(state, true);
			break;
		case production_action::foreign_invest_window:
			foreign_invest_win->is_visible() ? foreign_invest_win->set_visible(state, false) : foreign_invest_win->set_visible(state, true);
			break;
		default:
			break;
		}
		impl_on_update(state);
		return message_result::consumed;
	} else if(payload.holds_type<element_selection_wrapper<dcon::nation_id>>()) {
		foreign_nation = any_cast<element_selection_wrapper<dcon::nation_id>>(payload).data;
		open_foreign_invest = true;
		foreign_invest_win->set_visible(state, true);
		return message_result::consumed;
	} else if(payload.holds_type<dcon::nation_id>()) {
		if(foreign_invest_win->is_visible())
			payload.emplace<dcon::nation_id>(foreign_nation);
		else
			payload.emplace<dcon::nation_id>(state.local_player_nation);
		return message_result::consumed;
	}
	return message_result::unseen;
}

void invest_brow_window::on_create(sys::state& state) noexcept {
	window_element_base::on_create(state);
	set_visible(state, false);
	country_listbox->list_scrollbar->base_data.position.x += 13;
}

std::unique_ptr<element_base> invest_brow_window::make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept {
	if(name == "country_listbox") {
		auto ptr = make_element_by_type<production_country_listbox>(state, id);
		ptr->base_data.position.x -= 8 + 5; // Nudge
		country_listbox = ptr.get();
		return ptr;
	} else if(name == "sort_by_my_flag") {
		auto ptr = make_element_by_type<production_sort_my_nation_flag>(state, id);
		ptr->base_data.position.y -= 2; // Nudge
		return ptr;
	} else if(name == "sort_by_country") {
		auto ptr = make_element_by_type<country_sort_button<country_list_sort::country>>(state, id);
		ptr->base_data.position.y -= 1; // Nudge
		return ptr;
	} else if(name == "sort_by_boss") {
		auto ptr = make_element_by_type<country_sort_button<country_list_sort::boss>>(state, id);
		ptr->base_data.position.y -= 1; // Nudge
		return ptr;
	} else if(name == "sort_by_prestige") {
		auto ptr = make_element_by_type<country_sort_button<country_list_sort::prestige_rank>>(state, id);
		ptr->base_data.position.y -= 1; // Nudge
		return ptr;
	} else if(name == "sort_by_economic") {
		auto ptr = make_element_by_type<country_sort_button<country_list_sort::economic_rank>>(state, id);
		ptr->base_data.position.y -= 1; // Nudge
		return ptr;
	} else if(name == "sort_by_military") {
		auto ptr = make_element_by_type<country_sort_button<country_list_sort::military_rank>>(state, id);
		ptr->base_data.position.y -= 1; // Nudge
		return ptr;
	} else if(name == "sort_by_total") {
		auto ptr = make_element_by_type<country_sort_button<country_list_sort::total_rank>>(state, id);
		ptr->base_data.position.y -= 1; // Nudge
		return ptr;
	} else if(name == "sort_by_relation") {
		auto ptr = make_element_by_type<country_sort_button<country_list_sort::relation>>(state, id);
		ptr->base_data.position.y -= 1; // Nudge
		return ptr;
	} else if(name == "sort_by_opinion") {
		return make_element_by_type<country_sort_button<country_list_sort::opinion>>(state, id);
	} else if(name == "sort_by_prio") {
		return make_element_by_type<country_sort_button<country_list_sort::priority>>(state, id);
	} else if(name == "filter_all") {
		return make_element_by_type<category_filter_button<country_list_filter::all>>(state, id);
	} else if(name == "filter_enemies") {
		return make_element_by_type<category_filter_button<country_list_filter::enemies>>(state, id);
	} else if(name == "filter_allies") {
		return make_element_by_type<category_filter_button<country_list_filter::allies>>(state, id);
	} else if(name == "filter_neighbours") {
		return make_element_by_type<category_filter_button<country_list_filter::neighbors>>(state, id);
	} else if(name == "filter_sphere") {
		return make_element_by_type<category_filter_button<country_list_filter::sphere>>(state, id);
	} else if(name == "sort_by_invest_factories") {
		auto ptr = make_element_by_type<country_sort_button<country_list_sort::factories>>(state, id);
		ptr->base_data.position.y -= 1; // Nudge
		return ptr;
	} else if(name.length() >= 7 && name.substr(0, 7) == "filter_") {
		auto const filter_name = name.substr(7);
		auto ptr = make_element_by_type<continent_filter_button>(state, id);
		auto k = state.lookup_key(name);
		if(k) {
			for(auto m : state.world.in_modifier) {
				if(m.get_name() == k) {
					ptr->continent = m;
					break;
				}
			}
		}
		return ptr;
	} else if(name.substr(0, 14) == "sort_by_gpflag") {
		auto ptr = make_element_by_type<production_sort_nation_gp_flag>(state, id);
		ptr->rank = uint16_t(std::stoi(std::string{ name.substr(14) }));
		ptr->base_data.position.y -= 2; // Nudge
		return ptr;
	} else if(name.substr(0, 10) == "sort_by_gp") {
		auto ptr = make_element_by_type<country_sort_button<country_list_sort::gp_investment>>(state, id);
		ptr->offset = uint8_t(std::stoi(std::string{ name.substr(10) }));
		return ptr;
	} else if(name == "sort_by_my_invest") {
		return make_element_by_type<country_sort_button<country_list_sort::player_investment>>(state, id);
	} else {
		return nullptr;
	}
}

message_result invest_brow_window::get(sys::state& state, Cyto::Any& payload) noexcept {
	if(payload.holds_type<country_sort_setting>()) {
		payload.emplace<country_sort_setting>(sort);
		return message_result::consumed;
	} else if(payload.holds_type< country_filter_setting>()) {
		payload.emplace<country_filter_setting>(filter);
		return message_result::consumed;
	} else if(payload.holds_type<element_selection_wrapper<country_list_sort>>()) {
		auto new_sort = any_cast<element_selection_wrapper<country_list_sort>>(payload).data;
		sort.sort_ascend = (new_sort == sort.sort) ? !sort.sort_ascend : true;
		sort.sort = new_sort;
		country_listbox->impl_on_update(state);
		return message_result::consumed;
	} else if(payload.holds_type<country_list_filter>()) {
		auto temp = any_cast<country_list_filter>(payload);
		filter.general_category = filter.general_category != temp ? temp : country_list_filter::all;
		country_listbox->impl_on_update(state);
		return message_result::consumed;
	} else if(payload.holds_type<dcon::modifier_id>()) {
		auto temp_c = any_cast<dcon::modifier_id>(payload);
		filter.continent = filter.continent == temp_c ? dcon::modifier_id{} : temp_c;
		country_listbox->impl_on_update(state);
		return message_result::consumed;
	}
	return message_result::unseen;
}

} // namespace ui
