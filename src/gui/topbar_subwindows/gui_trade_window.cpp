#include "gui_trade_window.hpp"
#include "gui_element_templates.hpp"

namespace ui {
	void commodity_stockpile_indicator::on_update(sys::state& state) noexcept {
		auto const cid = retrieve<dcon::commodity_id>(state, parent);
		if(state.world.nation_get_drawing_on_stockpiles(state.local_player_nation, cid)) {
			if(state.world.nation_get_stockpiles(state.local_player_nation, cid) > 0) {
				frame = 2;
			} else {
				frame = 0;
			}
		} else if(state.world.nation_get_stockpiles(state.local_player_nation, cid)  < state.world.nation_get_stockpile_targets(state.local_player_nation, cid)) {
			frame = 1;
		} else {
			frame = 0;
		}
	}
	void commodity_stockpile_indicator::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto const cid = retrieve<dcon::commodity_id>(state, parent);
		if(state.world.nation_get_drawing_on_stockpiles(state.local_player_nation, cid)) {
			if(state.world.nation_get_stockpiles(state.local_player_nation, cid) > 0) {
				text::add_line(state, contents, "trade_setting_drawing");
			}
		} else if(state.world.nation_get_stockpiles(state.local_player_nation, cid) < state.world.nation_get_stockpile_targets(state.local_player_nation, cid)) {
			text::add_line(state, contents, "trade_setting_filling");
		}
	}

	void commodity_price_trend::on_update(sys::state& state) noexcept {
		auto const cid = retrieve<dcon::commodity_id>(state, parent);
		auto const current_price = state.world.commodity_get_price_record(cid, (state.ui_date.value >> 4) % economy::price_history_length);
		auto const previous_price = state.world.commodity_get_price_record(cid, ((state.ui_date.value >> 4) + economy::price_history_length - 1) % economy::price_history_length);
		if(current_price > previous_price) {
			frame = 0;
		} else if(current_price < previous_price) {
			frame = 2;
		} else {
			frame = 1;
		}
	}

	void trade_commodity_icon::on_update(sys::state& state) noexcept {
		frame = int32_t(state.world.commodity_get_icon(retrieve<dcon::commodity_id>(state, parent)));
	}

	void trade_commodity_icon::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto com = retrieve<dcon::commodity_id>(state, parent);
		if(com) {
			auto box = text::open_layout_box(contents, 0);
			text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, state.world.commodity_get_name(com)), text::text_color::yellow);
			text::close_layout_box(contents, box);
		}
	}

	void trade_commodity_icon::button_action(sys::state& state) noexcept {
		trade_details_select_commodity payload{ retrieve<dcon::commodity_id>(state, parent) };
		send<trade_details_select_commodity>(state, state.ui_state.trade_subwindow, payload);
		Cyto::Any dt_payload = trade_details_open_window{ retrieve<dcon::commodity_id>(state, parent) };
		state.ui_state.trade_subwindow->impl_get(state, dt_payload);
	}

	std::unique_ptr<element_base> trade_commodity_entry::make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept {
		if(name == "entry_button") {
			return make_element_by_type<trade_commodity_entry_button>(state, id);
		} else if(name == "goods_type") {
			return make_element_by_type<trade_commodity_icon>(state, id);
		} else if(name == "price") {
			return make_element_by_type<commodity_price_text>(state, id);
		} else if(name == "trend_indicator") {
			return make_element_by_type<commodity_price_trend>(state, id);
		} else if(name == "selling_indicator") {
			return make_element_by_type<commodity_stockpile_indicator>(state, id);
		} else if(name == "automation_indicator") {
			return make_element_by_type<commodity_automation_indicator>(state, id);
		} else {
			return nullptr;
		}
	}

	message_result trade_commodity_entry::get(sys::state& state, Cyto::Any& payload) noexcept {
		if(payload.holds_type<dcon::commodity_id>()) {
			payload.emplace<dcon::commodity_id>(commodity_id);
			return message_result::consumed;
		}
		return message_result::unseen;
	}

	std::unique_ptr<element_base> trade_flow_entry::make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept {
		if(name == "icon") {
			auto ptr = make_element_by_type<image_element_base>(state, id);
			icon = ptr.get();
			return ptr;
		} else if(name == "title") {
			auto ptr = make_element_by_type<simple_text_element_base>(state, id);
			title = ptr.get();
			return ptr;
		} else if(name == "value") {
			auto ptr = make_element_by_type<simple_text_element_base>(state, id);
			value = ptr.get();
			return ptr;
		} else if(name == "output_icon") {
			auto ptr = make_element_by_type<image_element_base>(state, id);
			output_icon = ptr.get();
			return ptr;
		} else {
			return nullptr;
		}
	}

	void trade_flow_entry::on_update(sys::state& state) noexcept {
		auto const cid = retrieve<dcon::commodity_id>(state, parent);
		icon->frame = int32_t(content.type);
		if(content.type == trade_flow_data::type::military_navy) {
			icon->frame = int32_t(trade_flow_data::type::military_army);
		}

		output_icon->set_visible(state, content.value_type != trade_flow_data::value_type::produced_by);
		value->set_visible(state, content.value_type != trade_flow_data::value_type::may_be_used_by);
		float amount = 0.f;
		switch(content.type) {
		case trade_flow_data::type::factory: {
			auto fid = content.data.factory_id;
			auto ftid = state.world.factory_get_building_type(fid);
			switch(content.value_type) {
			case trade_flow_data::value_type::produced_by: {
				amount += state.world.factory_get_actual_production(fid);
			} break;
			case trade_flow_data::value_type::used_by: {
				auto& inputs = state.world.factory_type_get_inputs(ftid);
				for(uint32_t i = 0; i < inputs.set_size; ++i) {
					if(inputs.commodity_type[i] == cid) {
						amount += inputs.commodity_amounts[i];
					}
				}
				output_icon->frame = state.world.commodity_get_icon(state.world.factory_type_get_output(ftid));
			} break;
			case trade_flow_data::value_type::may_be_used_by:
				output_icon->frame = state.world.commodity_get_icon(state.world.factory_type_get_output(ftid));
				break;
			default:
				break;
			}
			auto name = state.world.factory_type_get_name(ftid);
			title->set_text(state, text::produce_simple_string(state, name));
		} break;
		case trade_flow_data::type::province: {
			auto pid = content.data.province_id;
			switch(content.value_type) {
			case trade_flow_data::value_type::produced_by:
				amount += state.world.province_get_rgo_actual_production_per_good(pid, content.trade_good);
				break;
			case trade_flow_data::value_type::used_by:
			case trade_flow_data::value_type::may_be_used_by:
			default:
				break;
			}
			auto name = state.world.province_get_name(pid);
			title->set_text(state, text::produce_simple_string(state, name));
		} break;
		case trade_flow_data::type::pop:
		case trade_flow_data::type::military_army:
		case trade_flow_data::type::military_navy:
		default:
			break;
		}
		if(value->is_visible())
		value->set_text(state, text::format_float(amount, 2));
	}

	void trade_flow_produced_by_listbox::on_update(sys::state& state) noexcept {
		auto const cid = retrieve<dcon::commodity_id>(state, parent);
		row_contents.clear();
		populate_rows(
			state,
			[&](dcon::factory_id fid) -> bool {
				auto const ftid = state.world.factory_get_building_type(fid);
				return state.world.factory_type_get_output(ftid) == cid;
			},
			trade_flow_data::value_type::produced_by);
		update(state);
	}
	void trade_flow_used_by_listbox::on_update(sys::state& state) noexcept {
		auto cid = retrieve<dcon::commodity_id>(state, parent);

		row_contents.clear();
		populate_rows(state, [&](dcon::factory_id fid) -> bool {
			auto const ftid = state.world.factory_get_building_type(fid);
			auto const& inputs = state.world.factory_type_get_inputs(ftid);
			for(uint32_t i = 0; i < inputs.set_size; ++i) {
				if(inputs.commodity_type[i] == cid) {
					return inputs.commodity_amounts[i] > 0.f; // Some inputs taken
				}
			}
			return false;
		}, trade_flow_data::value_type::used_by);
		update(state);
	}
	void trade_flow_may_be_used_by_listbox::on_update(sys::state& state) noexcept {
		auto const cid = retrieve<dcon::commodity_id>(state, parent);
		row_contents.clear();
		populate_rows(state, [&](dcon::factory_id fid) -> bool {
			auto const ftid = state.world.factory_get_building_type(fid);
			auto const& inputs = state.world.factory_type_get_inputs(ftid);
			for(uint32_t i = 0; i < inputs.set_size; ++i) {
				if(inputs.commodity_type[i] == cid) {
					return inputs.commodity_amounts[i] == 0.f; // No inputs intaken
				}
			}
			return false;
		},
		trade_flow_data::value_type::may_be_used_by);
		update(state);
	}

	void trade_flow_producers_piechart::on_update(sys::state& state) noexcept {
		auto com = retrieve<dcon::commodity_id>(state, parent);
		distribution.clear();
		for(auto n : state.world.in_nation) {
			if(n.get_owned_province_count() != 0) {
				distribution.emplace_back(n.id, n.get_domestic_market_pool(com));
			}
		}
		update_chart(state);
	}
	void trade_flow_consumers_piechart::on_update(sys::state& state) noexcept {
		auto com = retrieve<dcon::commodity_id>(state, parent);
		distribution.clear();
		for(auto n : state.world.in_nation) {
			if(n.get_owned_province_count() != 0) {
				distribution.emplace_back(n.id, n.get_real_demand(com));
			}
		}
		update_chart(state);
	}
	void trade_flow_workers_piechart::on_update(sys::state& state) noexcept {
		auto com = retrieve<dcon::commodity_id>(state, parent);
		distribution.clear();
		float total = 0.f;
		{
			float amount = 0.f;
			for(const auto pc : state.world.nation_get_province_control(state.local_player_nation)) {
				for(const auto fl : pc.get_province().get_factory_location()) {
					if(fl.get_factory().get_building_type().get_output() == com) {
						amount += fl.get_factory().get_actual_production();
					}
				}
			}
			total += amount;
			distribution.emplace_back(state.culture_definitions.capitalists, amount);
		}
		{
			float amount = 0.f;
			for(const auto pc : state.world.nation_get_province_control(state.local_player_nation)) {
				amount += pc.get_province().get_rgo_actual_production_per_good(com);
			}
			total += amount;
			distribution.emplace_back(state.culture_definitions.aristocrat, amount);
		}
		{
			auto amount = state.world.nation_get_artisan_actual_production(state.local_player_nation, com);
			total += amount;
			distribution.emplace_back(state.culture_definitions.artisans, amount);
		}
		// remaining
		auto produced = state.world.nation_get_domestic_market_pool(state.local_player_nation, com);
		if(produced >= total) {
			distribution.emplace_back(state.culture_definitions.laborers, total - produced);
		}
		update_chart(state);
	}

	std::unique_ptr<element_base> trade_flow_price_graph_window::make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept {
		if(name == "current_price_label") {
			auto ptr = make_element_by_type<simple_text_element_base>(state, id);
			ptr->set_text(state, text::produce_simple_string(state, "alice_trade_flow_label"));
			ptr->base_data.size.x *= 2; // Nudge
			return ptr;
		} else if(name == "current_price_value"
		|| name == "price_linechart"
		|| name == "price_chart_low"
		|| name == "price_chart_time") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "price_chart_high") {
			auto ptr = make_element_by_type<invisible_element>(state, id);
			{
				auto ov_elm = make_element_by_type<image_element_base>(state, "generic_piechart_overlay");
				ov_elm->base_data.position.x = ptr->base_data.position.x;
				ov_elm->base_data.position.y = ptr->base_data.position.y;
				auto pc_elm = make_element_by_type<trade_flow_producers_piechart>(state, "generic_piechart");
				pc_elm->base_data.position.x += ov_elm->base_data.position.x;
				pc_elm->base_data.position.y += ov_elm->base_data.position.y;
				auto lg_elm = make_element_by_type<simple_text_element_base>(state, id);
				lg_elm->set_text(state, text::produce_simple_string(state, "alice_trade_flow_piechart_producers"));
				lg_elm->base_data.position.x = ptr->base_data.position.x;
				lg_elm->base_data.position.y = ptr->base_data.position.y - 8;
				add_child_to_front(std::move(lg_elm));
				add_child_to_front(std::move(pc_elm));
				add_child_to_front(std::move(ov_elm));
			}
			{
				auto ov_elm = make_element_by_type<image_element_base>(state, "generic_piechart_overlay");
				ov_elm->base_data.position.x = ptr->base_data.position.x + (ov_elm->base_data.size.x + 20) * 1;
				ov_elm->base_data.position.y = ptr->base_data.position.y;
				auto pc_elm = make_element_by_type<trade_flow_consumers_piechart>(state, "generic_piechart");
				pc_elm->base_data.position.x += ov_elm->base_data.position.x;
				pc_elm->base_data.position.y += ov_elm->base_data.position.y;
				auto lg_elm = make_element_by_type<simple_text_element_base>(state, id);
				lg_elm->set_text(state, text::produce_simple_string(state, "alice_trade_flow_piechart_consumers"));
				lg_elm->base_data.position.x = ptr->base_data.position.x + (ov_elm->base_data.size.x + 20) * 1;
				lg_elm->base_data.position.y = ptr->base_data.position.y - 8;
				add_child_to_front(std::move(lg_elm));
				add_child_to_front(std::move(pc_elm));
				add_child_to_front(std::move(ov_elm));
			}
			{
				auto ov_elm = make_element_by_type<image_element_base>(state, "generic_piechart_overlay");
				ov_elm->base_data.position.x = ptr->base_data.position.x + (ov_elm->base_data.size.x + 20) * 2;
				ov_elm->base_data.position.y = ptr->base_data.position.y;
				auto pc_elm = make_element_by_type<trade_flow_workers_piechart>(state, "generic_piechart");
				pc_elm->base_data.position.x += ov_elm->base_data.position.x;
				pc_elm->base_data.position.y += ov_elm->base_data.position.y;
				auto lg_elm = make_element_by_type<simple_text_element_base>(state, id);
				lg_elm->set_text(state, text::produce_simple_string(state, "alice_trade_flow_piechart_workforce"));
				lg_elm->base_data.position.x = ptr->base_data.position.x + (ov_elm->base_data.size.x + 20) * 2;
				lg_elm->base_data.position.y = ptr->base_data.position.y - 8;
				add_child_to_front(std::move(lg_elm));
				add_child_to_front(std::move(pc_elm));
				add_child_to_front(std::move(ov_elm));
			}
			return ptr;
		} else {
			return nullptr;
		}
	}

	std::unique_ptr<element_base> trade_flow_window::make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept {
		if(name == "close_button") {
			return make_element_by_type<generic_close_button>(state, id);
		} else if(name == "trade_flow_bg") {
			return make_element_by_type<draggable_target>(state, id);
		} else if(name == "material_name") {
			return make_element_by_type<generic_name_text<dcon::commodity_id>>(state, id);
		} else if(name == "material_icon_big") {
			return make_element_by_type<commodity_image>(state, id);
		} else if(name == "header_produced_by") {
			auto ptr = make_element_by_type<single_multiline_text_element_base>(state, id);
			ptr->text_id = text::find_or_add_key(state, "alice_trade_flow_produced", true);
			return ptr;
		} else if(name == "header_used_by") {
			auto ptr = make_element_by_type<single_multiline_text_element_base>(state, id);
			ptr->text_id = text::find_or_add_key(state, "alice_trade_flow_consumed", true);
			return ptr;
		} else if(name == "header_may_be_used_by") {
			auto ptr = make_element_by_type<single_multiline_text_element_base>(state, id);
			ptr->text_id = text::find_or_add_key(state, "trade_flow_may_be_used", true);
			return ptr;
		} else if(name == "total_produced_text") {
			auto ptr = make_element_by_type<single_multiline_text_element_base>(state, id);
			ptr->text_id = text::find_or_add_key(state, "trade_flow_total_produced", true);
			ptr->base_data.position.x += 48; // Nudge
			return ptr;
		} else if(name == "total_used_text") {
			auto ptr = make_element_by_type<single_multiline_text_element_base>(state, id);
			ptr->text_id = text::find_or_add_key(state, "trade_flow_total_used", true);
			ptr->base_data.position.x += 48; // Nudge
			return ptr;
		} else if(name == "total_produced_value") {
			return make_element_by_type<trade_flow_total_produced_text>(state, id);
		} else if(name == "total_used_value") {
			return make_element_by_type<trade_flow_total_used_text>(state, id);
		} else if(name == "price_graph") {
			return make_element_by_type<trade_flow_price_graph_window>(state, id);
		} else if(name == "produced_by_listbox") {
			return make_element_by_type<trade_flow_produced_by_listbox>(state, id);
		} else if(name == "used_by_listbox") {
			return make_element_by_type<trade_flow_used_by_listbox>(state, id);
		} else if(name == "may_be_used_by_listbox") {
			return make_element_by_type<trade_flow_may_be_used_by_listbox>(state, id);
		} else if(name == "part_of_world_production") {
			return make_element_by_type<trade_flow_world_production_text>(state, id);
		} else {
			return nullptr;
		}
	}

	std::unique_ptr<element_base> trade_details_window::make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept {
		if(name == "trade_flow_bg") {
			return make_element_by_type<image_element_base>(state, id);
		} else if(name == "goods_icon") {
			return make_element_by_type<commodity_image>(state, id);
		} else if(name == "goods_title") {
			return make_element_by_type<generic_name_text<dcon::commodity_id>>(state, id);
		} else if(name == "goods_price") {
			return make_element_by_type<commodity_price_text>(state, id);
		} else if(name == "automate_label") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "automate") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "price_linechart") {
			return make_element_by_type<prices_line_graph>(state, id);
		} else if(name == "price_chart_low") {
			return make_element_by_type<price_chart_low>(state, id);
		} else if(name == "price_chart_high") {
			return make_element_by_type<price_chart_high>(state, id);
		} else if(name == "price_chart_time") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "sell_stockpile") {
			return make_element_by_type<stockpile_sell_button>(state, id);
		} else if(name == "sell_stockpile_label") {
			return make_element_by_type<stockpile_sell_label>(state, id);
		} else if(name == "sell_slidier_desc") {
			return make_element_by_type<stockpile_slider_label>(state, id);
		} else if(name == "sell_slider") {
			return make_element_by_type<trade_slider>(state, id);
		} else if(name == "slider_value") {
			auto ptr = make_element_by_type<trade_slider_amount>(state, id);
			slider_value_display = ptr.get();
			return ptr;
		} else if(name == "confirm_trade") {
			return make_element_by_type<stockpile_amount_label>(state, id);
		} else if(name == "goods_details") {
			return make_element_by_type<trade_details_button>(state, id);
		} else if(name == "goods_need_gov_desc") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "goods_need_factory_desc") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "produced_detail_desc") {
			return make_element_by_type<detail_domestic_production>(state, id);
		} else if(name == "goods_need_pop_desc") {
			return make_element_by_type<invisible_element>(state, id);
		} else {
			return nullptr;
		}
	}

	message_result trade_details_window::get(sys::state& state, Cyto::Any& payload) noexcept {
		if(payload.holds_type<get_stockpile_target>()) {
			payload = get_stockpile_target{trade_amount};
			return message_result::consumed;
		} else if(payload.holds_type<stockpile_target_change>()) {
			trade_amount = any_cast<stockpile_target_change>(payload).value;
			slider_value_display->impl_on_update(state);
			return message_result::consumed;
		}
		return window_element_base::get(state, payload);
	}

	void trade_commodity_entry_button::on_update(sys::state& state) noexcept {
		auto com = retrieve<dcon::commodity_id>(state, parent);
		auto sat = state.world.nation_get_demand_satisfaction(state.local_player_nation, com);
		if(sat < 0.5f) { // shortage
			color = sys::pack_color(255, 196, 196);
			if(state.user_settings.color_blind_mode == sys::color_blind_mode::deutan || state.user_settings.color_blind_mode == sys::color_blind_mode::protan) {
				color = sys::pack_color(255, 100, 255); //remap to blue
			} else if(state.user_settings.color_blind_mode == sys::color_blind_mode::achroma) {
				color = sys::pack_color(196, 196, 196);
			}
		} else if(sat >= 1.f) { // full fulfillment
			color = sys::pack_color(196, 255, 196);
			if(state.user_settings.color_blind_mode == sys::color_blind_mode::deutan || state.user_settings.color_blind_mode == sys::color_blind_mode::protan) {
				color = sys::pack_color(114, 150, 77); //remap to yellow
			} else if(state.user_settings.color_blind_mode == sys::color_blind_mode::achroma) {
				color = sys::pack_color(128, 128, 128);
			}
		} else {
			color = sys::pack_color(255, 255, 255);
		}
	}

	void trade_commodity_entry_button::button_action(sys::state& state) noexcept {
		trade_details_select_commodity payload{ retrieve<dcon::commodity_id>(state, parent) };
		send<trade_details_select_commodity>(state, state.ui_state.trade_subwindow, payload);
	}

	void trade_commodity_entry_button::button_right_action(sys::state& state) noexcept {
		trade_details_select_commodity payload{ retrieve<dcon::commodity_id>(state, parent) };
		send<trade_details_select_commodity>(state, state.ui_state.trade_subwindow, payload);
		Cyto::Any dt_payload = trade_details_open_window{ retrieve<dcon::commodity_id>(state, parent) };
		state.ui_state.trade_subwindow->impl_get(state, dt_payload);
	}

	void trade_commodity_entry_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto com = retrieve<dcon::commodity_id>(state, parent);
		text::add_line(state, contents, state.world.commodity_get_name(com));
		if(com != economy::money) {
			auto sat = state.world.nation_get_demand_satisfaction(state.local_player_nation, com);
			if(sat < 0.5f) {
				text::add_line(state, contents, "commodity_shortage");
			} else if(sat >= 1.f) {
				text::add_line(state, contents, "commodity_surplus");
			}
			text::add_line(state, contents, "alice_commodity_cprice", text::variable_type::x, text::format_money(state.world.commodity_get_current_price(com)));
			text::add_line(state, contents, "alice_commodity_cost", text::variable_type::x, text::format_money(state.world.commodity_get_cost(com)));
			text::add_line(state, contents, "alice_commodity_eprice", text::variable_type::x, text::format_money(economy::commodity_effective_price(state, state.local_player_nation, com)));
			text::add_line_break_to_layout(state, contents);
		}
		text::add_line(state, contents, "trade_commodity_report_1", text::variable_type::x, text::fp_one_place{ state.world.commodity_get_total_real_demand(com) });
		text::add_line(state, contents, "trade_commodity_report_2", text::variable_type::x, text::fp_one_place{ state.world.commodity_get_total_production(com) });
		text::add_line(state, contents, "trade_commodity_report_4", text::variable_type::x, text::fp_one_place{ state.world.commodity_get_global_market_pool(com) });
		text::add_line_break_to_layout(state, contents);

		struct tagged_value {
			float v = 0.0f;
			dcon::nation_id n;
		};
		static std::vector<tagged_value> producers;

		producers.clear();
		for(auto n : state.world.in_nation) {
			if(n.get_domestic_market_pool(com) >= 0.05f) {
				producers.push_back(tagged_value{ n.get_domestic_market_pool(com), n.id });
			}
		}
		if(producers.size() > 0) {
			sys::merge_sort(producers.begin(), producers.end(), [](auto const& a, auto const& b) {
				return a.v > b.v;
			});
			{
				auto box = text::open_layout_box(contents, 0);
				text::substitution_map sub{};
				text::add_to_substitution_map(sub, text::variable_type::value, std::min(int32_t(state.defines.great_nations_count), int32_t(producers.size())));
				text::localised_format_box(state, contents, box, "alice_trade_top_producers", sub);
				text::close_layout_box(contents, box);
			}
			for(uint32_t i = 0; i < producers.size() && i < state.defines.great_nations_count; ++i) {
				auto box = text::open_layout_box(contents, 15);
				auto ident = state.world.nation_get_identity_from_identity_holder(producers[i].n);
				text::add_to_layout_box(state, contents, box, text::embedded_flag{ ident ? ident : state.national_definitions.rebel_id });
				text::add_space_to_layout_box(state, contents, box);
				text::add_to_layout_box(state, contents, box, text::get_name(state, producers[i].n));
				text::add_space_to_layout_box(state, contents, box);
				text::add_to_layout_box(state, contents, box, text::fp_one_place{ producers[i].v });
				text::close_layout_box(contents, box);
			}
			text::add_line_break_to_layout(state, contents);
			float r_total = 0.0f;
			for(auto p : state.world.in_province) {
				if(p.get_nation_from_province_ownership()) {
					r_total += p.get_rgo_actual_production_per_good(com);
				}
			}
			float a_total = 0.0f;
			for(auto n : state.world.in_nation) {
				a_total += state.world.nation_get_artisan_actual_production(n, com);
			}
			float f_total = 0.0f;
			for(auto f : state.world.in_factory) {
				if(f.get_building_type().get_output() == com)
					f_total += f.get_actual_production();
			}
			float total = r_total + a_total + f_total;
			if(r_total > 0.f) {
				if(com == economy::money) {
					text::add_line(state, contents, "rgo_trade_prod_3",
					text::variable_type::x, text::fp_one_place{ r_total },
					text::variable_type::y, text::fp_percentage{ r_total / total });
				} else if(state.world.commodity_get_is_mine(com)) {
					text::add_line(state, contents, "rgo_trade_prod_2",
					text::variable_type::x, text::fp_one_place{ r_total },
					text::variable_type::y, text::fp_percentage{ r_total / total });
				} else {
					text::add_line(state, contents, "rgo_trade_prod_1",
					text::variable_type::x, text::fp_one_place{ r_total },
					text::variable_type::y, text::fp_percentage{ r_total / total });
				}
			}
			if(a_total > 0.f) {
				text::add_line(state, contents, "alice_artisan_trade_prod",
				text::variable_type::x, text::fp_one_place{ a_total },
				text::variable_type::y, text::fp_percentage{ a_total / total });
				text::add_line(state, contents, "w_artisan_profit", text::variable_type::x, text::fp_one_place{ economy::base_artisan_profit(state, state.local_player_nation, com) * economy::artisan_scale_limit(state, state.local_player_nation, com) });
				text::add_line(state, contents, "w_artisan_distribution", text::variable_type::x, text::fp_one_place{ economy::get_artisan_distribution_slow(state, state.local_player_nation, com) * 100.f });
			}
			if(f_total > 0.f) {
				text::add_line(state, contents, "alice_factory_trade_prod",
				text::variable_type::x, text::fp_one_place{ f_total },
				text::variable_type::y, text::fp_percentage{ f_total / total });
			}
			text::add_line(state, contents, "alice_all_trade_prod", text::variable_type::x, text::fp_one_place{ total });
		} else {
			text::add_line(state, contents, "alice_trade_no_producers");
		}
	}

	void trade_commodity_group_window::on_create(sys::state& state) noexcept {
		window_element_base::on_create(state);
	}
	void trade_commodity_group_window::on_update(sys::state& state) noexcept {
		entries_element.clear();
		state.world.for_each_commodity([&](dcon::commodity_id id) {			
			if(state.world.commodity_get_commodity_group(id) != content) {
				return;
			}
			auto ptr = make_element_by_type<trade_commodity_entry>(state, state.ui_state.defs_by_name.find(state.lookup_key("goods_entry"))->second.definition);
			ptr->commodity_id = id;
			entries_element.push_back(ptr.get());
			add_child_to_front(std::move(ptr));
		});

		xy_pair cell_size = state.ui_defs.gui[state.ui_state.defs_by_name.find(state.lookup_key("goods_entry_offset"))->second.definition].position;
		xy_pair offset{ 0, 0 };
		for(const auto& e : entries_element) {
			dcon::commodity_id c = e->commodity_id ;
			auto kf = state.world.commodity_get_key_factory(c);
			bool is_active_globally = state.world.commodity_get_is_available_from_start(c);
			for(const auto n : state.world.in_nation) {
				if(kf && n.get_owned_province_count() > 0 && n.get_active_building(kf)) {
					is_active_globally = true;
					break;
				}
			}
			if(is_active_globally && !state.world.commodity_get_money_rgo(c)) {
				e->base_data.position = offset;
				offset.x += cell_size.x;
				if(offset.x + cell_size.x - 1 >= base_data.size.x) {
					offset.x = 0;
					offset.y += cell_size.y;
					if(offset.y + cell_size.y >= base_data.size.y) {
						offset.x += cell_size.x;
						offset.y = 0;
					}
				}
				e->set_visible(state, true);
			} else {
				e->set_visible(state, false);
			}
		}
	}

	void trade_window::on_create(sys::state& state) noexcept {
		window_element_base::on_create(state);
		auto ptr = make_element_by_type<trade_flow_window>(state, "trade_flow");
		if(ptr.get()) {
			trade_flow_win = static_cast<trade_flow_window*>(ptr.get());
			add_child_to_front(std::move(ptr));
		}
		set_visible(state, false);
	}

	std::unique_ptr<element_base> trade_window::make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept {
		if(name == "main_bg") {
			return make_element_by_type<image_element_base>(state, id);
		} else if(name == "bg_trade") {
			return make_element_by_type<opaque_element_base>(state, id);
		} else if(name == "close_button") {
			return make_element_by_type<generic_close_button>(state, id);
		} else if(name == "market_activity_list") {
			auto ptr = make_element_by_type<trade_market_activity_listbox>(state, id);
			list_ma = ptr.get();
			return ptr;
		} else if(name == "common_market_list") {
			auto ptr = make_element_by_type<trade_common_market_listbox>(state, id);
			list_cm = ptr.get();
			return ptr;
		} else if(name == "stockpile_list") {
			auto ptr = make_element_by_type<trade_stockpile_listbox>(state, id);
			list_sp = ptr.get();
			return ptr;
		} else if(name == "government_needs_list") {
			auto ptr = make_element_by_type<trade_government_needs_listbox>(state, id);
			list_gn = ptr.get();
			return ptr;
		} else if(name == "factory_needs_list") {
			auto ptr = make_element_by_type<trade_factory_needs_listbox>(state, id);
			list_fn = ptr.get();
			return ptr;
		} else if(name == "pop_needs_list") {
			auto ptr = make_element_by_type<trade_pop_needs_listbox>(state, id);
			list_pn = ptr.get();
			return ptr;
		} else if(name == "trade_details") {
			auto ptr = make_element_by_type<trade_details_window>(state, id);
			details_win = ptr.get();
			return ptr;
		} else if(name == "market_activity_sort_by_goods") {
			return make_element_by_type<trade_sort_button<trade_sort::commodity, trade_sort_assoc::market_activity>>(state, id);
		} else if(name == "market_activity_sort_by_activity") {
			return make_element_by_type<trade_sort_button<trade_sort::demand_satisfaction, trade_sort_assoc::market_activity>>(state, id);
		} else if(name == "market_activity_sort_by_cost") {
			return make_element_by_type<trade_sort_button<trade_sort::price, trade_sort_assoc::market_activity>>(state, id);
		} else if(name == "stockpile_sort_by_goods") {
			return make_element_by_type<trade_sort_button<trade_sort::commodity, trade_sort_assoc::stockpile>>(state, id);
		} else if(name == "stockpile_sort_by_value") {
			return make_element_by_type<trade_sort_button<trade_sort::stockpile, trade_sort_assoc::stockpile>>(state, id);
		} else if(name == "stockpile_sort_by_change") {
			return make_element_by_type<trade_sort_button<trade_sort::stockpile_change, trade_sort_assoc::stockpile>>(state, id);
		} else if(name == "common_market_sort_by_goods") {
			return make_element_by_type<trade_sort_button<trade_sort::commodity, trade_sort_assoc::common_market>>(state, id);
		} else if(name == "common_market_sort_by_produced") {
			return make_element_by_type<trade_sort_button<trade_sort::global_market_pool, trade_sort_assoc::common_market>>(state, id);
		} else if(name == "common_market_sort_by_diff") {
			return make_element_by_type<trade_sort_button<trade_sort::real_demand, trade_sort_assoc::common_market>>(state, id);
		} else if(name == "common_market_sort_by_exported") {
			return make_element_by_type<trade_sort_button<trade_sort::domestic_market, trade_sort_assoc::common_market>>(state, id);
		} else if(name == "needs_government_sort_by_goods") {
			return make_element_by_type<trade_sort_button<trade_sort::commodity, trade_sort_assoc::needs_government>>(state, id);
		} else if(name == "needs_government_sort_by_value") {
			return make_element_by_type<trade_sort_button<trade_sort::needs, trade_sort_assoc::needs_government>>(state, id);
		} else if(name == "needs_factories_sort_by_goods") {
			return make_element_by_type<trade_sort_button<trade_sort::commodity, trade_sort_assoc::needs_factories>>(state, id);
		} else if(name == "needs_factories_sort_by_value") {
			return make_element_by_type<trade_sort_button<trade_sort::needs, trade_sort_assoc::needs_factories>>(state, id);
		} else if(name == "needs_pops_sort_by_goods") {
			return make_element_by_type<trade_sort_button<trade_sort::commodity, trade_sort_assoc::needs_pops>>(state, id);
		} else if(name == "needs_pops_sort_by_value") {
			return make_element_by_type<trade_sort_button<trade_sort::needs, trade_sort_assoc::needs_pops>>(state, id);
			// Non-vanila
		} else if(name.substr(0, 6) == "group_") {
			auto ptr = make_element_by_type<trade_commodity_group_window>(state, id);
			auto cgname = name.substr(6, std::string::npos);
			for(uint32_t i = 0; i < uint32_t(state.commodity_group_names.size()); ++i) {
				auto const id = dcon::commodity_group_id(uint8_t(i));
				if(state.to_string_view(state.commodity_group_names[id]) == cgname) {
					ptr->content = id;
					break;
				}
			}
			return ptr;
		} else {
			return nullptr;
		}
	}

	message_result trade_window::get(sys::state& state, Cyto::Any& payload) noexcept {
		// Special message rebroadcasted by the details button from the hierarchy
		if(payload.holds_type<dcon::commodity_id>()) {
			payload.emplace<dcon::commodity_id>(commodity_id);
			return message_result::consumed;
		} else if(payload.holds_type<trade_details_open_window>()) {
			commodity_id = any_cast<trade_details_open_window>(payload).commodity_id;
			if(trade_flow_win) {
				trade_flow_win->set_visible(state, true);
				trade_flow_win->impl_on_update(state);
			}
			return message_result::consumed;
		} else if(payload.holds_type<trade_details_select_commodity>()) {
			commodity_id = any_cast<trade_details_select_commodity>(payload).commodity_id;
			if(commodity_id)
			details_win->trade_amount = state.world.nation_get_stockpile_targets(state.local_player_nation, commodity_id);
			details_win->impl_on_update(state);
			return message_result::consumed;
		} else if(payload.holds_type<dcon::nation_id>()) {
			payload.emplace<dcon::nation_id>(state.local_player_nation);
			return message_result::consumed;
		} else if(payload.holds_type<trade_sort_data>()) {
			auto d = any_cast<trade_sort_data>(payload);
			switch(d.assoc) {
			case trade_sort_assoc::market_activity:
				if(list_ma->sort == d.sort) {
					list_ma->sort_ascend = !list_ma->sort_ascend;
				} else {
					list_ma->sort_ascend = false;
				}
				list_ma->sort = d.sort;
				list_ma->impl_on_update(state);
				break;
			case trade_sort_assoc::stockpile:
				if(list_sp->sort == d.sort) {
					list_sp->sort_ascend = !list_sp->sort_ascend;
				} else {
					list_sp->sort_ascend = false;
				}
				list_sp->sort = d.sort;
				list_sp->impl_on_update(state);
				break;
			case trade_sort_assoc::common_market:
				if(list_cm->sort == d.sort)
					list_cm->sort_ascend = !list_cm->sort_ascend;
				else
					list_cm->sort_ascend = false;
				list_cm->sort = d.sort;
				list_cm->impl_on_update(state);
				break;
			case trade_sort_assoc::needs_government:
				if(list_gn->sort == d.sort)
					list_gn->sort_ascend = !list_gn->sort_ascend;
				else
					list_gn->sort_ascend = false;
				list_gn->sort = d.sort;
				list_gn->impl_on_update(state);
				break;
			case trade_sort_assoc::needs_factories:
				if(list_fn->sort == d.sort)
					list_fn->sort_ascend = !list_fn->sort_ascend;
				else
					list_fn->sort_ascend = false;
				list_fn->sort = d.sort;
				list_fn->impl_on_update(state);
				break;
			case trade_sort_assoc::needs_pops:
				if(list_pn->sort == d.sort)
					list_pn->sort_ascend = !list_pn->sort_ascend;
				else
					list_pn->sort_ascend = false;
				list_pn->sort = d.sort;
				list_pn->impl_on_update(state);
				break;
			default:
				break;
			}
			return message_result::consumed;
		}
		return message_result::unseen;
	}
}
