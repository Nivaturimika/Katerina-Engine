#include "gui_production_window.hpp"
#include "gui_factory_buttons_window.hpp"
#include "gui_invest_brow_window.hpp"
#include "gui_pop_sort_buttons_window.hpp"
#include "gui_commodity_filters_window.hpp"
#include "gui_projects_window.hpp"
#include "gui_build_factory_window.hpp"
#include "gui_project_investment_window.hpp"
#include "gui_foreign_investment_window.hpp"
#include "gui_element_templates.hpp"

namespace ui {
	void populate_production_states_list(sys::state& state, std::vector<dcon::state_instance_id>& row_contents, dcon::nation_id n, bool show_empty, production_sort_order sort_order) {
		for(auto const fat_id : state.world.nation_get_state_ownership(n)) {
			if(show_empty && !fat_id.get_state().get_capital().get_is_colonial()) {
				row_contents.push_back(fat_id.get_state());
			} else if(economy_factory::has_factory(state, fat_id.get_state().id)) {
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
				if(count > 0) {
					row_contents.push_back(fat_id.get_state());
				}
			}
		}

		auto sort_by_name = [&](dcon::state_instance_id a, dcon::state_instance_id b) {
			auto av = text::produce_simple_string(state, state.world.state_definition_get_name(state.world.state_instance_get_definition(a)));
			auto bv = text::produce_simple_string(state, state.world.state_definition_get_name(state.world.state_instance_get_definition(b)));
			if(av != bv)
				return av > bv;
			return a.index() < b.index();
		};
		auto sort_by_factories = [&](dcon::state_instance_id a, dcon::state_instance_id b) {
			auto av = economy_factory::state_factory_count(state, a);
			auto bv = economy_factory::state_factory_count(state, b);
			if(av != bv)
				return av > bv;
			return a.index() < b.index();
		};
		auto sort_by_primary_workers = [&](dcon::state_instance_id a, dcon::state_instance_id b) {
			auto av = state.world.state_instance_get_demographics(a, demographics::to_key(state, state.culture_definitions.primary_factory_worker));
			auto bv = state.world.state_instance_get_demographics(b, demographics::to_key(state, state.culture_definitions.primary_factory_worker));
			if(av != bv)
				return av > bv;
			return a.index() < b.index();
		};
		auto sort_by_secondary_workers = [&](dcon::state_instance_id a, dcon::state_instance_id b) {
			auto av = state.world.state_instance_get_demographics(a, demographics::to_key(state, state.culture_definitions.secondary_factory_worker));
			auto bv = state.world.state_instance_get_demographics(b, demographics::to_key(state, state.culture_definitions.secondary_factory_worker));
			if(av != bv)
				return av > bv;
			return a.index() < b.index();
		};
		auto sort_by_owners = [&](dcon::state_instance_id a, dcon::state_instance_id b) {
			auto atotal = state.world.state_instance_get_demographics(a, demographics::total);
			auto btotal = state.world.state_instance_get_demographics(b, demographics::total);
			auto acap = state.world.state_instance_get_demographics(a, demographics::to_key(state, state.culture_definitions.capitalists));
			auto bcap = state.world.state_instance_get_demographics(b, demographics::to_key(state, state.culture_definitions.capitalists));
			auto av = atotal > 0.f ? acap / atotal : 0.f;
			auto bv = btotal > 0.f ? bcap / btotal : 0.f;
			if(av != bv)
				return av > bv;
			return a.index() < b.index();
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
			auto av = ap_total > 0.f ? atotal / ap_total : 0.f;
			auto bv = bp_total > 0.f ? btotal / bp_total : 0.f;
			if(av != bv)
				return av > bv;
			return a.index() < b.index();
		};
		auto sort_by_focus = [&](dcon::state_instance_id a, dcon::state_instance_id b) {
			auto a_focus = state.world.state_instance_get_owner_focus(a);
			auto b_focus = state.world.state_instance_get_owner_focus(b);
			if(a_focus != b_focus)
				return a_focus.id.value > b_focus.id.value;
			return a.index() < b.index();
		};

		switch(sort_order) {
		case production_sort_order::name:
			pdqsort(row_contents.begin(), row_contents.end(), sort_by_name);
			break;
		case production_sort_order::factories:
			pdqsort(row_contents.begin(), row_contents.end(), sort_by_factories);
			break;
		case production_sort_order::primary_workers:
			pdqsort(row_contents.begin(), row_contents.end(), sort_by_primary_workers);
			break;
		case production_sort_order::secondary_workers:
			pdqsort(row_contents.begin(), row_contents.end(), sort_by_secondary_workers);
			break;
		case production_sort_order::owners:
			pdqsort(row_contents.begin(), row_contents.end(), sort_by_owners);
			break;
		case production_sort_order::infrastructure:
			pdqsort(row_contents.begin(), row_contents.end(), sort_by_infrastructure);
			break;
		case production_sort_order::focus:
			pdqsort(row_contents.begin(), row_contents.end(), sort_by_focus);
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

} // namespace ui
