#include "gui_province_window.hpp"
#include "nations.hpp"
#include "gui_element_templates.hpp"

namespace ui {
	void province_liferating::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		dcon::province_id prov_id = retrieve<dcon::province_id>(state, parent);

		text::add_line(state, contents, "provinceview_liferating", text::variable_type::value, int64_t(state.world.province_get_life_rating(prov_id)));
		text::add_line_break_to_layout(state, contents);
		text::add_line(state, contents, "col_liferate_techs");
		for(auto i : state.world.in_invention) {
			auto mod = i.get_modifier();
			for(uint32_t j = 0; j < sys::national_modifier_definition::modifier_definition_size; j++) {
				if(mod.get_national_values().offsets[j] == sys::national_mod_offsets::colonial_life_rating) {
					auto box = text::open_layout_box(contents);
					text::add_to_layout_box(state, contents, box, i.get_name(), state.world.nation_get_active_inventions(state.local_player_nation, i) ? text::text_color::green : text::text_color::red);

					dcon::technology_id containing_tech;
					auto lim_trigger_k = i.get_limit();
					trigger::recurse_over_triggers(state.trigger_data.data() + state.trigger_data_indices[lim_trigger_k.index() + 1],
					[&](uint16_t* tval) {
						if((tval[0] & trigger::code_mask) == trigger::technology)
							containing_tech = trigger::payload(tval[1]).tech_id;
					});

					if(containing_tech) {
					text::add_to_layout_box(state, contents, box, std::string_view{ " (" });
						text::add_to_layout_box(state, contents, box, state.world.technology_get_name(containing_tech), state.world.nation_get_active_technologies(state.local_player_nation, containing_tech) ? text::text_color::green : text::text_color::red);
					text::add_to_layout_box(state, contents, box, std::string_view{ ")" });
					}
					text::close_layout_box(contents, box);
					break;
				}
			}
		}
	}

	void province_pop_button::button_action(sys::state& state) noexcept {
		auto const province_id = retrieve<dcon::province_id>(state, parent);
		state.open_population();
		if(state.ui_state.population_subwindow) {
			Cyto::Any fl_payload = pop_list_filter(province_id);
			state.ui_state.population_subwindow->impl_set(state, fl_payload);
		}
	}
	void province_pop_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto const province_id = retrieve<dcon::province_id>(state, parent);
		text::substitution_map sub_map;
		text::add_to_substitution_map(sub_map, text::variable_type::loc, state.world.province_get_name(province_id));
		auto box = text::open_layout_box(contents, 0);
		text::localised_format_box(state, contents, box, std::string_view("pw_open_popscreen"), sub_map);
		text::close_layout_box(contents, box);
	}

	void province_terrain_image::update_tooltip(sys::state& state, int32_t x, int32_t t, text::columnar_layout& contents) noexcept {
		dcon::province_id province_id = retrieve<dcon::province_id>(state, parent);
		auto fat_id = dcon::fatten(state.world, province_id);
		//terrain
		if(auto name = fat_id.get_terrain().get_name(); name) {
			auto box = text::open_layout_box(contents, 0);
			text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, name), text::text_color::yellow);
			text::close_layout_box(contents, box);
		}
		if(auto mod_id = fat_id.get_terrain().id; mod_id) {
			modifier_description(state, contents, mod_id);
		}
		//climate
		if(auto name = fat_id.get_climate().get_name(); name) {
			auto box = text::open_layout_box(contents, 0);
			text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, name), text::text_color::yellow);
			text::close_layout_box(contents, box);
		}
		if(auto mod_id = fat_id.get_climate().id; mod_id) {
			modifier_description(state, contents, mod_id);
		}
		//continent
		if(auto name = fat_id.get_continent().get_name(); name) {
			auto box = text::open_layout_box(contents, 0);
			text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, name), text::text_color::yellow);
			text::close_layout_box(contents, box);
		}
		if(auto mod_id = fat_id.get_continent().id; mod_id) {
			modifier_description(state, contents, mod_id);
		}
	}

	void province_owner_rgo_draw_tooltip(sys::state& state, text::columnar_layout& contents, dcon::province_id prov_id) noexcept {
		auto rgo_good = state.world.province_get_rgo(prov_id);
		auto nat_id = state.world.province_get_nation_from_province_ownership(prov_id);

		state.world.for_each_commodity([&](dcon::commodity_id c) {
			auto production = province::rgo_production_quantity(state, prov_id, c);
			auto profit = state.world.province_get_rgo_profit_per_good(prov_id, c);

			if(production < 0.0001f) {
				return;
			}

			auto base_box = text::open_layout_box(contents);
			auto name_box = base_box;
			name_box.x_size = 75;
			auto production_box = base_box;
			production_box.x_position += 90.f;

			auto profit_box = base_box;
			profit_box.x_position += 180.f;

			text::add_to_layout_box(state, contents, name_box, text::get_name_as_string(state, dcon::fatten(state.world, c)));
		text::add_to_layout_box(state, contents, production_box, text::fp_two_places{ production });
			text::add_to_layout_box(state, contents, profit_box, text::format_money(profit));
			text::add_to_layout_box(state, contents, base_box, std::string(" "));
			text::close_layout_box(contents, base_box);
		});

		if(rgo_good) {

			text::add_line(state, contents, "provinceview_goodsincome", text::variable_type::goods, rgo_good.get_name(), text::variable_type::value,
					text::fp_three_places{ province::rgo_income(state, prov_id) });

			{
				auto box = text::open_layout_box(contents, 0);
				text::substitution_map sub_map;
				auto const production = province::rgo_production_quantity(state, prov_id, rgo_good);

			text::add_to_substitution_map(sub_map, text::variable_type::curr, text::fp_two_places{ production });
				text::localised_format_box(state, contents, box, std::string_view("production_output_goods_tooltip2"), sub_map);
				text::localised_format_box(state, contents, box, std::string_view("production_output_explanation"));
				text::close_layout_box(contents, box);
			}

			text::add_line_break_to_layout(state, contents);

			{
				auto const production = province::rgo_production_quantity(state, prov_id, rgo_good);
				auto const base_size = economy_rgo::rgo_effective_size(state, nat_id, prov_id, rgo_good) * state.world.commodity_get_rgo_amount(rgo_good);
			text::add_line(state, contents, std::string_view("production_base_output_goods_tooltip"), text::variable_type::base, text::fp_two_places{ base_size });
			}

			{
				auto box = text::open_layout_box(contents, 0);
				bool const is_mine = state.world.commodity_get_is_mine(rgo_good);
				auto const efficiency = 1.0f +
				state.world.province_get_modifier_values(prov_id,
						is_mine ? sys::provincial_mod_offsets::mine_rgo_eff : sys::provincial_mod_offsets::farm_rgo_eff) +
				state.world.nation_get_modifier_values(nat_id,
						is_mine ? sys::national_mod_offsets::mine_rgo_eff : sys::national_mod_offsets::farm_rgo_eff);
				text::localised_format_box(state, contents, box, std::string_view("production_output_efficiency_tooltip"));
			text::add_to_layout_box(state, contents, box, text::fp_percentage{ efficiency }, efficiency >= 0.0f ? text::text_color::green : text::text_color::red);
				text::close_layout_box(contents, box);
			}

			text::add_line_break_to_layout(state, contents);

			{
				auto box = text::open_layout_box(contents, 0);
				auto const throughput = state.world.province_get_rgo_employment(prov_id);
				text::localised_format_box(state, contents, box, std::string_view("production_throughput_efficiency_tooltip"));
			text::add_to_layout_box(state, contents, box, text::fp_percentage{ throughput }, throughput >= 0.0f ? text::text_color::green : text::text_color::red);

				text::close_layout_box(contents, box);
			}
		}
	}

	void province_national_focus_button::button_action(sys::state& state) noexcept {
		auto province_window = static_cast<province_view_window*>(state.ui_state.province_window);
		province_window->nf_win->set_visible(state, !province_window->nf_win->is_visible());
	}

	void province_national_focus_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto box = text::open_layout_box(contents, 0);

		auto sid = retrieve<dcon::state_instance_id>(state, parent);
		auto fat_si = dcon::fatten(state.world, sid);
		text::add_to_layout_box(state, contents, box, sid);
		text::add_line_break_to_layout_box(state, contents, box);
		auto content = state.world.state_instance_get_owner_focus(sid);
		if(bool(content)) {
			auto fat_nf = dcon::fatten(state.world, content);
			text::add_to_layout_box(state, contents, box, state.world.national_focus_get_name(content), text::substitution_map{});
			text::add_line_break_to_layout_box(state, contents, box);
			auto color = text::text_color::white;
			if(fat_nf.get_promotion_type()) {
				if(nations::national_focus_is_unoptimal(state, state.local_player_nation, sid, content)) {
					color = text::text_color::red;
				}
				auto full_str = text::format_percentage(fat_si.get_demographics(demographics::to_key(state, fat_nf.get_promotion_type())) / fat_si.get_demographics(demographics::total));
				text::add_to_layout_box(state, contents, box, std::string_view(full_str), color);
			}
		}
		text::close_layout_box(contents, box);
		if(auto mid = state.world.national_focus_get_modifier(content);  mid) {
			modifier_description(state, contents, mid, 15);
		}
		text::add_line(state, contents, "alice_nf_controls");
	}

	void province_modifier_icon::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		sys::dated_modifier mod = retrieve< sys::dated_modifier>(state, parent);
		if(mod.mod_id) {
			auto p = retrieve<dcon::province_id>(state, parent);
			auto n = state.world.province_get_nation_from_province_ownership(p);
			auto box = text::open_layout_box(contents, 0);
			text::add_to_layout_box(state, contents, box, state.world.modifier_get_name(mod.mod_id), text::text_color::yellow);
			text::add_line_break_to_layout_box(state, contents, box);
			if(auto desc = state.world.modifier_get_desc(mod.mod_id); state.key_is_localized(desc)) {
				text::substitution_map sub{};
				text::add_to_substitution_map(sub, text::variable_type::country, n);
				text::add_to_substitution_map(sub, text::variable_type::country_adj, text::get_adjective(state, n));
				text::add_to_substitution_map(sub, text::variable_type::capital, state.world.nation_get_capital(n));
				text::add_to_substitution_map(sub, text::variable_type::continentname, state.world.modifier_get_name(state.world.province_get_continent(state.world.nation_get_capital(n))));
				text::add_to_substitution_map(sub, text::variable_type::provincename, p);
				text::add_to_layout_box(state, contents, box, desc, sub);
			}
			text::close_layout_box(contents, box);
			modifier_description(state, contents, mod.mod_id, 15);
		}
		if(mod.expiration) {
			text::add_line(state, contents, "expires_on", text::variable_type::date, mod.expiration);
		}
	}
}
