#include "gui_province_window.hpp"

namespace ui {

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

}
