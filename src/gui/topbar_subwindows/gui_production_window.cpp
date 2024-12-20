#include "gui_element_templates.hpp"
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
				atotal += float(state.world.province_get_building_level(p, state.economy_definitions.railroad_building));
				ap_total += 1.0f;
			});
			float btotal = 0.0f;
			float bp_total = 0.0f;
			province::for_each_province_in_state_instance(state, b, [&](dcon::province_id p) {
				btotal += float(state.world.province_get_building_level(p, state.economy_definitions.railroad_building));
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
			sys::merge_sort(row_contents.begin(), row_contents.end(), sort_by_name);
			break;
		case production_sort_order::factories:
			sys::merge_sort(row_contents.begin(), row_contents.end(), sort_by_factories);
			break;
		case production_sort_order::primary_workers:
			sys::merge_sort(row_contents.begin(), row_contents.end(), sort_by_primary_workers);
			break;
		case production_sort_order::secondary_workers:
			sys::merge_sort(row_contents.begin(), row_contents.end(), sort_by_secondary_workers);
			break;
		case production_sort_order::owners:
			sys::merge_sort(row_contents.begin(), row_contents.end(), sort_by_owners);
			break;
		case production_sort_order::infrastructure:
			sys::merge_sort(row_contents.begin(), row_contents.end(), sort_by_infrastructure);
			break;
		case production_sort_order::focus:
			sys::merge_sort(row_contents.begin(), row_contents.end(), sort_by_focus);
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

	void normal_factory_background::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto const fid = retrieve<dcon::factory_id>(state, parent);
		if(!fid)
			return;
		auto const n = retrieve<dcon::nation_id>(state, parent);
		auto const p = state.world.factory_get_province_from_factory_location(retrieve<dcon::factory_id>(state, parent));
		auto const p_fat = fatten(state.world, p);
		auto const sdef = state.world.abstract_state_membership_get_state(state.world.province_get_abstract_state_membership(p));
		dcon::state_instance_id s{};
		for(auto const so : state.world.nation_get_state_ownership(n)) {
			if(so.get_state().get_definition() == sdef) {
				s = so.get_state();
				break;
			}
		}

		// nation data

		float mobilization_impact = state.world.nation_get_is_mobilized(n) ? military::mobilization_impact(state, n) : 1.0f;
		auto cap_prov = state.world.nation_get_capital(n);
		auto cap_continent = state.world.province_get_continent(cap_prov);
		auto cap_region = state.world.province_get_connected_region_id(cap_prov);


		auto fac = fatten(state.world, fid);
		auto type = state.world.factory_get_building_type(fid);

		auto& inputs = type.get_inputs();
		auto& einputs = type.get_efficiency_inputs();

		//inputs

		float input_total = economy_factory::factory_input_total_cost(state, n, type);
		float min_input_available = economy_factory::factory_min_input_available(state, n, type);
		float e_input_total = economy_factory::factory_efficiency_input_total_cost(state, n, type);
		float min_e_input_available = economy_factory::factory_min_efficiency_input_available(state, n, type);

		//modifiers

		float input_multiplier = economy_factory::factory_input_multiplier(state, fac, n, p, s);
		float throughput_multiplier = economy_factory::factory_throughput_multiplier(state, type, n, p, s);
		float output_multiplier = economy_factory::factory_output_multiplier(state, fac, n, p);

		float max_production_scale = economy_factory::factory_max_production_scale(state, fac, mobilization_impact, p_fat.get_nation_from_province_control() != n);
		float effective_production_scale = std::min(fac.get_production_scale() * fac.get_level(), max_production_scale);
		auto amount = (0.75f + 0.25f * min_e_input_available) * min_input_available * state.world.factory_get_production_scale(fid);

		text::add_line(state, contents, "factory_stats_1", text::variable_type::val, text::fp_percentage{ amount });
		text::add_line(state, contents, "factory_stats_2", text::variable_type::val, text::fp_percentage{ state.world.factory_get_production_scale(fid) });
		text::add_line(state, contents, "factory_stats_3", text::variable_type::val, text::fp_one_place{ state.world.factory_get_actual_production(fid) }, text::variable_type::x, type.get_output().get_name());
		text::add_line(state, contents, "factory_stats_4", text::variable_type::val, text::fp_currency{ state.world.factory_get_full_profit(fid) });
		text::add_line_break_to_layout(state, contents);
		text::add_line(state, contents, "factory_stats_5");

		int position_demand_sat = 100;
		int position_amount = 180;
		int position_cost = 250;

		auto input_cost_line = [&](dcon::commodity_id cid, float base_amount) {
			auto box = text::open_layout_box(contents);
			text::layout_box name_entry = box;
			text::layout_box demand_satisfaction = box;
			text::layout_box amount_box = box;
			text::layout_box cost_box = box;

			demand_satisfaction.x_position += position_demand_sat;
			amount_box.x_position += position_amount;
			cost_box.x_position += position_cost;

			name_entry.x_size /= 10;
			text::add_to_layout_box(state, contents, name_entry, state.world.commodity_get_name(cid));

			auto sat = state.world.nation_get_demand_satisfaction(n, cid);
			text::add_to_layout_box(state, contents,
			demand_satisfaction,
			text::fp_percentage{ sat },
			sat >= 0.9f ? text::text_color::green : text::text_color::red
			);

			float amount = base_amount * input_multiplier * throughput_multiplier * min_input_available * effective_production_scale;
			float cost = economy::commodity_effective_price(state, n, cid) * amount;
			text::add_to_layout_box(state, contents, amount_box, text::fp_two_places{ amount });
			text::add_to_layout_box(state, contents, cost_box, text::fp_currency{ -cost }, text::text_color::red);

			text::add_to_layout_box(state, contents, box, std::string(" "));
			text::close_layout_box(contents, box);
		};

		auto e_input_cost_line = [&](dcon::commodity_id cid, float base_amount) {
			auto box = text::open_layout_box(contents);
			text::layout_box name_entry = box;
			text::layout_box demand_satisfaction = box;
			text::layout_box amount_box = box;
			text::layout_box cost_box = box;

			demand_satisfaction.x_position += position_demand_sat;
			amount_box.x_position += position_amount;
			cost_box.x_position += position_cost;

			name_entry.x_size /= 10;
			text::add_to_layout_box(state, contents, name_entry, state.world.commodity_get_name(cid));

			auto sat = state.world.nation_get_demand_satisfaction(n, cid);
			text::add_to_layout_box(state, contents,
				demand_satisfaction,
				text::fp_percentage{ sat },
				sat >= 0.9f ? text::text_color::green : text::text_color::red
			);

			float amount = base_amount * input_multiplier * min_e_input_available * min_input_available * effective_production_scale;
			float cost = economy::commodity_effective_price(state, n, cid) * amount;
			text::add_to_layout_box(state, contents, amount_box, text::fp_two_places{ amount });
			text::add_to_layout_box(state, contents, cost_box, text::fp_currency{ -cost }, text::text_color::red);
			text::add_to_layout_box(state, contents, box, std::string(" "));
			text::close_layout_box(contents, box);
		};
		auto named_money_line = [&](std::string_view loc, float value) {
			auto box = text::open_layout_box(contents);
			text::layout_box name_entry = box;
			text::layout_box cost = box;
			cost.x_position += position_cost;
			name_entry.x_size /= 10;
			text::localised_format_box(state, contents, name_entry, loc);
			text::add_to_layout_box(state, contents, cost, text::fp_currency{ value }, value >= 0.f ? text::text_color::green : text::text_color::red);
			text::add_to_layout_box(state, contents, box, std::string(" "));
			text::close_layout_box(contents, box);
		};
		auto output_cost_line = [&](dcon::commodity_id cid, float base_amount) {
			auto box = text::open_layout_box(contents);
			text::layout_box name_entry = box;
			text::layout_box amount = box;
			text::layout_box cost = box;
			amount.x_position += position_amount;
			cost.x_position += position_cost;
			name_entry.x_size /= 10;
			text::add_to_layout_box(state, contents, name_entry, state.world.commodity_get_name(cid));
			float output_amount = base_amount * (0.75f + 0.25f * min_e_input_available) * throughput_multiplier * output_multiplier * min_input_available * effective_production_scale;
			float output_cost = economy::commodity_effective_price(state, n, cid) * output_amount;
			text::add_to_layout_box(state, contents, amount, text::fp_two_places{ output_amount });
			text::add_to_layout_box(state, contents, cost, text::fp_currency{ output_cost }, text::text_color::green);
			text::add_to_layout_box(state, contents, box, std::string(" "));
			text::close_layout_box(contents, box);
		};
		for(uint32_t i = 0; i < economy::commodity_set::set_size; ++i) {
			if(inputs.commodity_type[i]) {
				input_cost_line(inputs.commodity_type[i], inputs.commodity_amounts[i]);
			} else {
				break;
			}
		}
		text::add_line_break_to_layout(state, contents);
		text::add_line(state, contents, "factory_stats_6");
		for(uint32_t i = 0; i < economy::small_commodity_set::set_size; ++i) {
			if(einputs.commodity_type[i]) {
				e_input_cost_line(einputs.commodity_type[i], einputs.commodity_amounts[i]);
			} else {
				break;
			}
		}
		text::add_line_break_to_layout(state, contents);
		auto const min_wage_factor = economy::pop_min_wage_factor(state, n);
		float expected_min_wage = economy_factory::pop_factory_min_wage(state, n, min_wage_factor)
			* state.world.factory_type_get_base_workforce(type);

		float spendings = expected_min_wage
			+ input_multiplier * throughput_multiplier * input_total * min_input_available
			+ input_multiplier * e_input_total * min_e_input_available * min_input_available;

		named_money_line("factory_stats_wage", -expected_min_wage);
		text::add_line_break_to_layout(state, contents);
		named_money_line("factory_stats_expenses", -spendings);
		output_cost_line(type.get_output(), type.get_output_amount());
		auto const desired_income = economy_factory::factory_desired_raw_profit(state, fac.get_building_type(), fac.get_level());
		named_money_line("factory_stats_desired_income", desired_income);
	}

	void factory_input_icon::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto const n = retrieve<dcon::nation_id>(state, parent);
		auto const p = state.world.factory_get_province_from_factory_location(retrieve<dcon::factory_id>(state, parent));
		//auto com = retrieve<dcon::commodity_id>(state, parent);
		if(!com)
			return;

		auto box = text::open_layout_box(contents, 0);
		text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, state.world.commodity_get_name(com)), text::text_color::yellow);
		text::close_layout_box(contents, box);

		auto commodity_mod_description = [&](float value, std::string_view locale_base_name, std::string_view locale_farm_base_name) {
			if(value == 0.f)
				return;
			auto box = text::open_layout_box(contents, 0);
			text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, state.world.commodity_get_name(com)), text::text_color::white);
			text::add_space_to_layout_box(state, contents, box);
			text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, state.world.commodity_get_is_mine(com) ? locale_base_name : locale_farm_base_name), text::text_color::white);
			text::add_to_layout_box(state, contents, box, std::string{ ":" }, text::text_color::white);
			text::add_space_to_layout_box(state, contents, box);
			auto color = value > 0.f ? text::text_color::green : text::text_color::red;
			text::add_to_layout_box(state, contents, box, (value > 0.f ? "+" : "") + text::format_percentage(value, 1), color);
			text::close_layout_box(contents, box);
		};
		commodity_mod_description(state.world.nation_get_factory_goods_output(n, com), "tech_output", "tech_output");
		commodity_mod_description(state.world.nation_get_rgo_goods_output(n, com), "tech_mine_output", "tech_farm_output");
		commodity_mod_description(state.world.nation_get_rgo_size(n, com), "tech_mine_size", "tech_farm_size");
		if(state.world.commodity_get_key_factory(com)) {
			active_modifiers_description(state, contents, n, 0, sys::national_mod_offsets::factory_output, true);
			active_modifiers_description(state, contents, p, 0, sys::provincial_mod_offsets::local_factory_output, true);
			active_modifiers_description(state, contents, n, 0, sys::national_mod_offsets::factory_throughput, true);
			active_modifiers_description(state, contents, p, 0, sys::provincial_mod_offsets::local_factory_throughput, true);
		} else {
			if(state.world.commodity_get_is_mine(com)) {
				active_modifiers_description(state, contents, n, 0, sys::national_mod_offsets::mine_rgo_eff, true);
				active_modifiers_description(state, contents, p, 0, sys::provincial_mod_offsets::mine_rgo_eff, true);
				active_modifiers_description(state, contents, n, 0, sys::national_mod_offsets::mine_rgo_size, true);
				active_modifiers_description(state, contents, p, 0, sys::provincial_mod_offsets::mine_rgo_size, true);
			} else {
				active_modifiers_description(state, contents, n, 0, sys::national_mod_offsets::farm_rgo_eff, true);
				active_modifiers_description(state, contents, p, 0, sys::provincial_mod_offsets::farm_rgo_eff, true);
				active_modifiers_description(state, contents, n, 0, sys::national_mod_offsets::farm_rgo_size, true);
				active_modifiers_description(state, contents, p, 0, sys::provincial_mod_offsets::farm_rgo_size, true);
			}
		}
		auto const f = retrieve<dcon::factory_id>(state, parent);
		auto const ft = state.world.factory_get_building_type(f);
		auto const sid = state.world.province_get_state_membership(p);
		if(auto mod_k = state.world.factory_type_get_throughput_bonus(ft); mod_k) {
			ui::additive_value_modifier_description(state, contents, mod_k, trigger::to_generic(sid), trigger::to_generic(n), 0);
		}
		if(auto mod_k = state.world.factory_type_get_input_bonus(ft); mod_k) {
			ui::additive_value_modifier_description(state, contents, mod_k, trigger::to_generic(sid), trigger::to_generic(n), 0);
		}
	}

	std::unique_ptr<element_base> production_factory_info::make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept {
		if(name == "prod_factory_bg") {
			return make_element_by_type<normal_factory_background>(state, id);
		} else if(name == "level") {
			auto ptr = make_element_by_type<factory_level_text>(state, id);
			factory_elements.push_back(ptr.get());
			return ptr;
		} else if(name == "income") {
			auto ptr = make_element_by_type<factory_profit_text>(state, id);
			factory_elements.push_back(ptr.get());
			return ptr;
		} else if(name == "income_icon") {
			auto ptr = make_element_by_type<factory_income_image>(state, id);
			factory_elements.push_back(ptr.get());
			return ptr;
		} else if(name == "output") {
			return make_element_by_type<production_factory_output>(state, id);
		} else if(name == "closed_overlay") {
			auto ptr = make_element_by_type<image_element_base>(state, id);
			closed_elements.push_back(ptr.get());
			return ptr;
		} else if(name == "factory_closed_text") {
			auto ptr = make_element_by_type<simple_text_element_base>(state, id);
			closed_elements.push_back(ptr.get());
			return ptr;
		} else if(name == "prod_factory_inprogress_bg") {
			auto ptr = make_element_by_type<image_element_base>(state, id);
			build_elements.push_back(ptr.get());
			return ptr;
		} else if(name == "build_factory_progress") {
			auto ptr = make_element_by_type<factory_build_progress_bar>(state, id);
			build_elements.push_back(ptr.get());
			return ptr;
		} else if(name == "prod_cancel_progress") {
			auto ptr = make_element_by_type<factory_cancel_new_const_button>(state, id);
			build_elements.push_back(ptr.get());
			return ptr;
		} else if(name == "upgrade_factory_progress") {
			auto ptr = make_element_by_type<factory_upgrade_progress_bar>(state, id);
			upgrade_elements.push_back(ptr.get());
			return ptr;
		} else if(name == "progress_overlay_16_64") {
			auto ptr = make_element_by_type<image_element_base>(state, id);
			upgrade_elements.push_back(ptr.get());
			return ptr;
		} else if(name == "employment_ratio") {
			auto ptr = make_element_by_type<factory_employment_image>(state, id);
			factory_elements.push_back(ptr.get());
			return ptr;
		} else if(name == "priority") {
			auto ptr = make_element_by_type<factory_priority_button>(state, id);
			factory_elements.push_back(ptr.get());
			return ptr;
		} else if(name == "upgrade") {
			auto ptr = make_element_by_type<factory_upgrade_button>(state, id);
			factory_elements.push_back(ptr.get());
			return ptr;
		} else if(name == "subsidise") {
			auto ptr = make_element_by_type<factory_subsidise_button>(state, id);
			factory_elements.push_back(ptr.get());
			return ptr;
		} else if(name == "delete_factory") {
			auto ptr = make_element_by_type<factory_delete_button>(state, id);
			closed_elements.push_back(ptr.get());
			return ptr;
		} else if(name == "open_close") {
			auto ptr = make_element_by_type<factory_reopen_button>(state, id);
			closed_elements.push_back(ptr.get());
			auto ptrb = make_element_by_type<factory_close_and_delete_button>(state, id);
			factory_elements.push_back(ptrb.get());
			add_child_to_front(std::move(ptrb));
			return ptr;
		} else if(name.substr(0, 6) == "input_") {
			auto input_index = size_t(std::stoi(std::string(name.substr(6))));
			if(name.ends_with("_lack2")) {
				auto ptr = make_element_by_type<image_element_base>(state, id);
				input_lack_icons[input_index] = ptr.get();
				return ptr;
			} else {
				auto ptr = make_element_by_type<factory_input_icon>(state, id);
				input_icons[input_index] = static_cast<factory_input_icon*>(ptr.get());
				return ptr;
			}
		} else {
			return nullptr;
		}
	}
} // namespace ui
