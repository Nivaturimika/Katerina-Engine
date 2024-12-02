#include "system_state.hpp"
#include "gui_element_types.hpp"
#include "gui_element_templates.hpp"
#include "gui_topbar.hpp"
#include "economy_templates.hpp"

namespace ui {
	void topbar_nation_prestige_text::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto nation_id = retrieve<dcon::nation_id>(state, parent);
		auto box = text::open_layout_box(contents, 0);
		text::localised_format_box(state, contents, box, std::string_view("rank_prestige"), text::substitution_map{});
		text::add_line_break_to_layout_box(state, contents, box);
		switch(nations::get_status(state, nation_id)) {
		case(nations::status::great_power):
			text::localised_format_box(state, contents, box, std::string_view("diplomacy_greatnation_status"), text::substitution_map{});
			break;
		case(nations::status::secondary_power):
			text::localised_format_box(state, contents, box, std::string_view("diplomacy_colonialnation_status"), text::substitution_map{});
			break;
		case(nations::status::civilized):
			text::localised_format_box(state, contents, box, std::string_view("diplomacy_civilizednation_status"), text::substitution_map{});
			break;
		case(nations::status::westernizing):
			text::localised_format_box(state, contents, box, std::string_view("diplomacy_almost_western_nation_status"), text::substitution_map{});
			break;
		case(nations::status::uncivilized):
			text::localised_format_box(state, contents, box, std::string_view("diplomacy_uncivilizednation_status"), text::substitution_map{});
			break;
		case(nations::status::primitive):
			text::localised_format_box(state, contents, box, std::string_view("diplomacy_primitivenation_status"), text::substitution_map{});
			break;
		default:
			break;
		};
		text::close_layout_box(contents, box);
		nation_prestige_text::update_tooltip(state, x, y, contents);
	}

	void topbar_nation_industry_score_text::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto nation_id = retrieve<dcon::nation_id>(state, parent);
		auto box = text::open_layout_box(contents, 0);
		text::localised_format_box(state, contents, box, std::string_view("rank_industry"), text::substitution_map{});
		text::add_divider_to_layout_box(state, contents, box);
		text::localised_format_box(state, contents, box, std::string_view("rank_industry_d"), text::substitution_map{});
		text::close_layout_box(contents, box);
		nation_industry_score_text::update_tooltip(state, x, y, contents);
	}

	void topbar_nation_military_score_text::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto nation_id = retrieve<dcon::nation_id>(state, parent);
		auto box = text::open_layout_box(contents, 0);
		text::localised_format_box(state, contents, box, std::string_view("rank_military"), text::substitution_map{});
		text::add_divider_to_layout_box(state, contents, box);
		text::localised_format_box(state, contents, box, std::string_view("rank_military_d"), text::substitution_map{});
		text::close_layout_box(contents, box);
		nation_military_score_text::update_tooltip(state, x, y, contents);
	}

	void topbar_nation_colonial_power_text::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto n = retrieve<dcon::nation_id>(state, parent);
		text::substitution_map sub;
		std::string value = text::prettify(nations::free_colonial_points(state, n));
		text::add_to_substitution_map(sub, text::variable_type::value, std::string_view(value));
		auto box = text::open_layout_box(contents, 0);
		text::localised_format_box(state, contents, box, std::string_view("colonial_points"), sub);
		text::add_line_break_to_layout_box(state, contents, box);
		text::localised_format_box(state, contents, box, std::string_view("explain_colonial_points"), sub);
		text::add_divider_to_layout_box(state, contents, box);
		text::localised_format_box(state, contents, box, std::string_view("available_colonial_power"), sub);
		text::close_layout_box(contents, box);
		nation_colonial_power_text::update_tooltip(state, x, y, contents);
	}

	void topbar_nation_literacy_text::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto n = retrieve<dcon::nation_id>(state, parent);
		auto box = text::open_layout_box(contents, 0);
		text::substitution_map sub;
		auto literacy_change = demographics::get_estimated_literacy_change(state, n);
		auto lit_change = text::format_float(demographics::get_estimated_literacy_change(state, n), 8);
		text::add_to_substitution_map(sub, text::variable_type::val, std::string_view(lit_change));
		auto total = state.world.nation_get_demographics(n, demographics::total);
		auto avg_literacy = text::format_float(total != 0.f ? (state.world.nation_get_demographics(n, demographics::literacy) / total) : 0.f, 1);
		text::add_to_substitution_map(sub, text::variable_type::avg, std::string_view(avg_literacy));
		text::localised_format_box(state, contents, box, std::string_view("topbar_avg_literacy"), sub);
		text::add_line_break_to_layout_box(state, contents, box);
		text::localised_format_box(state, contents, box, std::string_view("topbar_avg_change"), sub);
		text::close_layout_box(contents, box);
		active_modifiers_description(state, contents, n, 0, sys::national_mod_offsets::education_efficiency, true);
		active_modifiers_description(state, contents, n, 0, sys::national_mod_offsets::education_efficiency_modifier, true);
	}

	void topbar_nation_infamy_text::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto nation_id = retrieve<dcon::nation_id>(state, parent);
		auto box = text::open_layout_box(contents, 0);
		auto fat_id = dcon::fatten(state.world, nation_id);
		text::localised_format_box(state, contents, box, "infamy");
		text::add_to_layout_box(state, contents, box, std::string_view(":"));
		text::add_space_to_layout_box(state, contents, box);
		text::add_to_layout_box(state, contents, box, text::fp_two_places{fat_id.get_infamy()});
		text::add_to_layout_box(state, contents, box, std::string_view("/"));
		text::add_to_layout_box(state, contents, box, text::fp_two_places{state.defines.badboy_limit});
		text::add_line_break_to_layout_box(state, contents, box);
		text::localised_format_box(state, contents, box, "badboy_dro_1");
		text::close_layout_box(contents, box);
		active_modifiers_description(state, contents, nation_id, 0, sys::national_mod_offsets::badboy, false);
	}

	void topbar_nation_diplomatic_points_text::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto n = retrieve<dcon::nation_id>(state, parent);
		text::substitution_map sub;
		text::add_to_substitution_map(sub, text::variable_type::curr, text::fp_one_place{ nations::diplomatic_points(state, n) });
		text::add_to_substitution_map(sub, text::variable_type::value, text::fp_one_place{ nations::monthly_diplomatic_points(state, n) });
		text::substitution_map sub_base;
		text::add_to_substitution_map(sub_base, text::variable_type::value, text::fp_one_place{ state.defines.base_monthly_diplopoints });
		auto box = text::open_layout_box(contents, 0);
		text::localised_format_box(state, contents, box, std::string_view("topbar_diplopoints"), sub);
		text::add_divider_to_layout_box(state, contents, box);
		text::localised_format_box(state, contents, box, std::string_view("topbar_diplopoints_gain"), sub);
		text::add_line_break_to_layout_box(state, contents, box);
		text::localised_format_box(state, contents, box, std::string_view("topbar_diplopoints_basegain"), sub_base);
		text::close_layout_box(contents, box);
		active_modifiers_description(state, contents, n, 0, sys::national_mod_offsets::diplomatic_points_modifier, false);
	}

	void topbar_nation_navy_allocation_text::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto n = retrieve<dcon::nation_id>(state, parent);
		{
			auto box = text::open_layout_box(contents, 0);
			text::substitution_map sub;
			int32_t num_ships = 0;
			for(auto nv : state.world.nation_get_navy_control(n)) {
				num_ships += int32_t(nv.get_navy().get_navy_membership().end() - nv.get_navy().get_navy_membership().begin());
			}
			text::add_to_substitution_map(sub, text::variable_type::curr, num_ships);
			text::add_to_substitution_map(sub, text::variable_type::tot, text::pretty_integer{ military::naval_supply_points(state, n) });
			text::add_to_substitution_map(sub, text::variable_type::req, text::pretty_integer{ military::naval_supply_points_used(state, n) });
			text::localised_format_box(state, contents, box, std::string_view("alice_navy_allocation_tt"), sub);
			text::close_layout_box(contents, box);
		}
		for(const auto nv : state.world.nation_get_navy_control_as_controller(n)) {
			int32_t total = 0;
			for(const auto memb : nv.get_navy().get_navy_membership()) {
				total += state.military_definitions.unit_base_definitions[memb.get_ship().get_type()].supply_consumption_score;
			}
			auto box = text::open_layout_box(contents, 0);
			text::substitution_map sub;
			text::add_to_substitution_map(sub, text::variable_type::name, state.to_string_view(nv.get_navy().get_name()));
			text::add_to_substitution_map(sub, text::variable_type::value, total);
			text::add_to_substitution_map(sub, text::variable_type::perc, text::fp_percentage{ float(total) / military::naval_supply_points(state, n) });
			text::localised_format_box(state, contents, box, std::string_view("alice_navy_allocation_2"), sub);
			text::close_layout_box(contents, box);
		}
		active_modifiers_description(state, contents, n, 0, sys::national_mod_offsets::supply_range, true);
	}

	void topbar_nation_population_text::on_update(sys::state& state) noexcept {
		auto n = retrieve<dcon::nation_id>(state, parent);
		auto total_pop = state.world.nation_get_demographics(n, demographics::total);

		auto pop_amount = state.player_data_cache.population_record[state.ui_date.value % 32];
		auto pop_change = state.ui_date.value <= 32
			? (state.ui_date.value <= 2 ? 0.0f : pop_amount - state.player_data_cache.population_record[2])
			: (pop_amount - state.player_data_cache.population_record[(state.ui_date.value - 30) % 32]);

		text::text_color color = pop_change < 0 ? text::text_color::red : text::text_color::green;
		if(pop_change == 0)
			color = text::text_color::white;

		auto layout = text::create_endless_layout(state, internal_layout, text::layout_parameters{ 0, 0, int16_t(base_data.size.x), int16_t(base_data.size.y), base_data.data.text.font_handle, 0, text::alignment::left, text::text_color::black, false });
		auto box = text::open_layout_box(layout, 0);
		text::add_to_layout_box(state, layout, box, text::prettify(int32_t(total_pop)), text::text_color::yellow);
		text::add_to_layout_box(state, layout, box, std::string(" ("));
		if(pop_change > 0) {
			text::add_to_layout_box(state, layout, box, std::string("+"), text::text_color::green);
		}
		text::add_to_layout_box(state, layout, box, text::pretty_integer{ int64_t(pop_change) }, color);
		text::add_to_layout_box(state, layout, box, std::string(")"));
		text::close_layout_box(layout, box);
	}
	void topbar_nation_population_text::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto n = retrieve<dcon::nation_id>(state, parent);
		auto pop_amount = state.player_data_cache.population_record[state.ui_date.value % 32];
		auto pop_change = state.ui_date.value <= 30 ? 0.0f : (pop_amount - state.player_data_cache.population_record[(state.ui_date.value - 30) % 32]);
		text::add_line(state, contents, "pop_growth_topbar_3", text::variable_type::curr, text::pretty_integer{ int64_t(state.world.nation_get_demographics(n, demographics::total)) });
		text::add_line(state, contents, "pop_growth_topbar_2", text::variable_type::x, text::pretty_integer{ int64_t(pop_change) });
		text::add_line(state, contents, "pop_growth_topbar", text::variable_type::x, text::pretty_integer{ int64_t(nations::get_monthly_pop_increase_of_nation(state, n)) });
		text::add_line(state, contents, "pop_growth_topbar_4", text::variable_type::val, text::pretty_integer{ int64_t(state.world.nation_get_demographics(n, demographics::total) * 4.f) });
		text::add_line_break_to_layout(state, contents);
		active_modifiers_description(state, contents, n, 0, sys::national_mod_offsets::pop_growth, true);
	}

	void topbar_treasury_text::on_update(sys::state& state) noexcept {
		auto n = retrieve<dcon::nation_id>(state, parent);
		auto layout = text::create_endless_layout(state, internal_layout,
		text::layout_parameters{ 0, 0, int16_t(base_data.size.x), int16_t(base_data.size.y), base_data.data.text.font_handle, 0, text::alignment::center, text::text_color::black, false });
		auto box = text::open_layout_box(layout, 0);
		auto current_day_record = state.player_data_cache.treasury_record[state.ui_date.value % 32];
		auto previous_day_record = state.player_data_cache.treasury_record[(state.ui_date.value + 31) % 32];
		auto change = current_day_record - previous_day_record;
		text::add_to_layout_box(state, layout, box, text::prettify_currency(nations::get_treasury(state, n)), text::text_color::yellow);
		text::add_to_layout_box(state, layout, box, std::string(" ("));
		if(change > 0) {
			text::add_to_layout_box(state, layout, box, std::string("+"), text::text_color::green);
			text::add_to_layout_box(state, layout, box, text::prettify_currency( change ), text::text_color::green);
		} else if(change == 0) {
			text::add_to_layout_box(state, layout, box, text::prettify_currency( change ), text::text_color::white);
		} else {
			text::add_to_layout_box(state, layout, box, text::prettify_currency(change), text::text_color::red);
		}
		text::add_to_layout_box(state, layout, box, std::string(")"));
		text::close_layout_box(layout, box);
		for(uint32_t i = 0; i < uint32_t(budget_slider_target::target_count); i++) {
			multipliers[i] = 1.f;
			switch(budget_slider_target(i)) {
			case budget_slider_target::poor_tax:
				multipliers[i] = float(state.world.nation_get_poor_tax(state.local_player_nation)) / 100.f;
				break;
			case budget_slider_target::middle_tax:
				multipliers[i] = float(state.world.nation_get_middle_tax(state.local_player_nation)) / 100.f;
				break;
			case budget_slider_target::rich_tax:
				multipliers[i] = float(state.world.nation_get_rich_tax(state.local_player_nation)) / 100.f;
				break;
			case budget_slider_target::construction_stock:
				multipliers[i] = float(state.world.nation_get_construction_spending(state.local_player_nation)) / 100.f;
				break;
			case budget_slider_target::social:
				multipliers[i] = float(state.world.nation_get_social_spending(state.local_player_nation)) / 100.f;
				break;
			case budget_slider_target::military:
				multipliers[i] = float(state.world.nation_get_military_spending(state.local_player_nation)) / 100.f;
				break;
			case budget_slider_target::education:
				multipliers[i] = float(state.world.nation_get_education_spending(state.local_player_nation)) / 100.f;
				break;
			case budget_slider_target::tariffs:
				multipliers[i] = float(state.world.nation_get_tariffs(state.local_player_nation)) / 100.f;
				break;
			case budget_slider_target::admin:
				multipliers[i] = float(state.world.nation_get_administrative_spending(state.local_player_nation)) / 100.f;
				break;
			case budget_slider_target::domestic_investment:
				multipliers[i] = float(state.world.nation_get_domestic_investment_spending(state.local_player_nation)) / 100.f;
				break;
			case budget_slider_target::army_stock:
				multipliers[i] = float(state.world.nation_get_land_spending(state.local_player_nation)) / 100.f;
				break;
			case budget_slider_target::navy_stock:
				multipliers[i] = float(state.world.nation_get_naval_spending(state.local_player_nation)) / 100.f;
				break;
			}
		}
		// income
		values[uint8_t(budget_slider_target::poor_tax)] = economy_estimations::estimate_tax_income_by_strata(state, state.local_player_nation, culture::pop_strata::poor);
		values[uint8_t(budget_slider_target::middle_tax)] = economy_estimations::estimate_tax_income_by_strata(state, state.local_player_nation, culture::pop_strata::middle);
		values[uint8_t(budget_slider_target::rich_tax)] = economy_estimations::estimate_tax_income_by_strata(state, state.local_player_nation, culture::pop_strata::rich);
		values[uint8_t(budget_slider_target::gold_income)] = economy_estimations::estimate_gold_income(state, state.local_player_nation);
		// spend
		values[uint8_t(budget_slider_target::construction_stock)] = -economy_estimations::estimate_construction_spending(state, state.local_player_nation);
		values[uint8_t(budget_slider_target::army_stock)] = -economy_estimations::estimate_land_spending(state, state.local_player_nation);
		values[uint8_t(budget_slider_target::navy_stock)] = -economy_estimations::estimate_naval_spending(state, state.local_player_nation);
		values[uint8_t(budget_slider_target::social)] = -economy_estimations::estimate_social_spending(state, state.local_player_nation);
		values[uint8_t(budget_slider_target::education)] = -economy_estimations::estimate_pop_payouts_by_income_type(state, state.local_player_nation, culture::income_type::education);
		values[uint8_t(budget_slider_target::admin)] = -economy_estimations::estimate_pop_payouts_by_income_type(state, state.local_player_nation, culture::income_type::administration);
		values[uint8_t(budget_slider_target::military)] = -economy_estimations::estimate_pop_payouts_by_income_type(state, state.local_player_nation, culture::income_type::military);
		values[uint8_t(budget_slider_target::subsidies)] = -economy_estimations::estimate_subsidy_spending(state, state.local_player_nation);
		values[uint8_t(budget_slider_target::overseas)] = -economy_estimations::estimate_overseas_penalty_spending(state, state.local_player_nation);
		values[uint8_t(budget_slider_target::stockpile_filling)] = -economy_estimations::estimate_stockpile_filling_spending(state, state.local_player_nation);
		values[uint8_t(budget_slider_target::domestic_investment)] = -economy_estimations::estimate_domestic_investment(state, state.local_player_nation);
		// balance
		values[uint8_t(budget_slider_target::diplomatic_interest)] = economy_estimations::estimate_diplomatic_balance(state, state.local_player_nation);
		values[uint8_t(budget_slider_target::interest)] = -economy::interest_payment(state, state.local_player_nation);
		values[uint8_t(budget_slider_target::tariffs)] = economy_estimations::estimate_tariff_income(state, state.local_player_nation);
	}
	void topbar_treasury_text::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto n = retrieve<dcon::nation_id>(state, parent);
		auto box = text::open_layout_box(contents, 0);

		float total = 0.f;
		float total_exp = 0.f;
		float total_inc = 0.f;
		for(uint8_t i = 0; i < uint8_t(budget_slider_target::target_count); ++i) {
			float v = values[i];
			if(v < 0.f)
				total_exp += v;
			else
				total_inc += v;
			total += v;
		}
		if(total_inc != 0.f) {
			text::localised_single_sub_box(state, contents, box, "alice_budget_scaled_inc", text::variable_type::value, text::fp_currency{ total_inc });
			text::add_line_break_to_layout_box(state, contents, box);
			for(uint8_t i = 0; i < uint8_t(budget_slider_target::target_count); ++i) {
				float v = values[i] * multipliers[i];
				if(v > 0.f) {
					switch(budget_slider_target(i)) {
					case budget_slider_target::poor_tax:
						text::localised_single_sub_box(state, contents, box, "alice_budget_scaled_1", text::variable_type::value, text::fp_currency{ v });
						break;
					case budget_slider_target::middle_tax:
						text::localised_single_sub_box(state, contents, box, "alice_budget_scaled_2", text::variable_type::value, text::fp_currency{ v });
						break;
					case budget_slider_target::rich_tax:
						text::localised_single_sub_box(state, contents, box, "alice_budget_scaled_3", text::variable_type::value, text::fp_currency{ v });
						break;
					case budget_slider_target::tariffs:
						text::localised_single_sub_box(state, contents, box, "alice_budget_scaled_4", text::variable_type::value, text::fp_currency{ v });
						break;
					case budget_slider_target::gold_income:
						text::localised_single_sub_box(state, contents, box, "alice_budget_scaled_17", text::variable_type::value, text::fp_currency{ v });
						break;
					case budget_slider_target::diplomatic_interest:
						text::localised_single_sub_box(state, contents, box, "alice_budget_scaled_16", text::variable_type::value, text::fp_currency{ v });
						break;
					default:
						break;
					}
					text::add_line_break_to_layout_box(state, contents, box);
				}
			}
		}
		if(total_exp != 0.f) {
			text::localised_single_sub_box(state, contents, box, "alice_budget_scaled_exp", text::variable_type::value, text::fp_currency{ total_exp });
			text::add_line_break_to_layout_box(state, contents, box);
			for(uint8_t i = 0; i < uint8_t(budget_slider_target::target_count); ++i) {
				float v = values[i] * multipliers[i];
				if(v < 0.f) {
					switch(budget_slider_target(i)) {
					case budget_slider_target::army_stock:
						text::localised_single_sub_box(state, contents, box, "alice_budget_scaled_5", text::variable_type::value, text::fp_currency{ v });
						break;
					case budget_slider_target::navy_stock:
						text::localised_single_sub_box(state, contents, box, "alice_budget_scaled_6", text::variable_type::value, text::fp_currency{ v });
						break;
					case budget_slider_target::construction_stock:
						text::localised_single_sub_box(state, contents, box, "alice_budget_scaled_7", text::variable_type::value, text::fp_currency{ v });
						break;
					case budget_slider_target::education:
						text::localised_single_sub_box(state, contents, box, "alice_budget_scaled_8", text::variable_type::value, text::fp_currency{ v });
						break;
					case budget_slider_target::admin:
						text::localised_single_sub_box(state, contents, box, "alice_budget_scaled_9", text::variable_type::value, text::fp_currency{ v });
						break;
					case budget_slider_target::social:
						text::localised_single_sub_box(state, contents, box, "alice_budget_scaled_10", text::variable_type::value, text::fp_currency{ v });
						break;
					case budget_slider_target::military:
						text::localised_single_sub_box(state, contents, box, "alice_budget_scaled_11", text::variable_type::value, text::fp_currency{ v });
						break;
					case budget_slider_target::domestic_investment:
						text::localised_single_sub_box(state, contents, box, "alice_budget_scaled_12", text::variable_type::value, text::fp_currency{ v });
						break;
					case budget_slider_target::overseas:
						text::localised_single_sub_box(state, contents, box, "alice_budget_scaled_13", text::variable_type::value, text::fp_currency{ v });
						break;
					case budget_slider_target::stockpile_filling:
						text::localised_single_sub_box(state, contents, box, "alice_budget_scaled_14", text::variable_type::value, text::fp_currency{ v });
						break;
					case budget_slider_target::subsidies:
						text::localised_single_sub_box(state, contents, box, "alice_budget_scaled_15", text::variable_type::value, text::fp_currency{ v });
						break;
					default:
						break;
					}
					text::add_line_break_to_layout_box(state, contents, box);
				}
			}
		}
		if(total != 0.f) {
			text::localised_single_sub_box(state, contents, box, "alice_budget_scaled_net", text::variable_type::value, text::fp_currency{ total });
			text::add_line_break_to_layout_box(state, contents, box);
		}
		text::close_layout_box(contents, box);
	}

	void topbar_nation_focus_allocation_text::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto n = retrieve<dcon::nation_id>(state, parent);
		auto box = text::open_layout_box(contents, 0);
		text::substitution_map sub1;
		float relevant_pop = 0.f;
		for(auto ac : state.world.in_culture) {
			if(nations::nation_accepts_culture(state, n, ac)) {
				relevant_pop += state.world.nation_get_demographics(n, demographics::to_key(state, ac));
			}
		}
		text::add_to_substitution_map(sub1, text::variable_type::num, text::pretty_integer{ (int64_t)relevant_pop });
		auto f_points = relevant_pop / state.defines.national_focus_divider;
		text::add_to_substitution_map(sub1, text::variable_type::focus, text::fp_two_places{ f_points });
		text::localised_format_box(state, contents, box, std::string_view("tb_nationalfocus_culture"), sub1);
		text::add_line_break_to_layout_box(state, contents, box);
		text::substitution_map sub2;
		text::add_to_substitution_map(sub2, text::variable_type::focus, nations::max_national_focuses(state, n));
		text::localised_format_box(state, contents, box, std::string_view("tb_max_focus"), sub2);
		if(nations::national_focuses_in_use(state, n) > 0) {
			text::add_divider_to_layout_box(state, contents, box);
			state.world.nation_for_each_state_ownership(n, [&](dcon::state_ownership_id so) {
				auto sid = state.world.state_ownership_get_state(so);
				auto nf = state.world.state_instance_get_owner_focus(sid);
				if(nf) {
					text::add_to_layout_box(state, contents, box, state.world.national_focus_get_name(nf));
					text::add_line_break_to_layout_box(state, contents, box);
				}
			});
		} else {
			text::add_divider_to_layout_box(state, contents, box);
			text::localised_format_box(state, contents, box, std::string_view("tb_nationalfocus_none"), text::substitution_map{});
		}
		text::close_layout_box(contents, box);
		active_modifiers_description(state, contents, n, 0, sys::national_mod_offsets::max_national_focus, true);
	}

	void topbar_nation_militancy_text::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto nation_id = retrieve<dcon::nation_id>(state, parent);
		auto box = text::open_layout_box(contents, 0);
		text::substitution_map sub;
		auto mil_change = demographics::get_estimated_mil_change(state, nation_id);
		auto total = state.world.nation_get_demographics(nation_id, demographics::total);
		text::add_to_substitution_map(sub, text::variable_type::avg,
			text::fp_two_places{total != 0.f ? state.world.nation_get_demographics(nation_id, demographics::militancy) / total : 0.f});
		text::add_to_substitution_map(sub, text::variable_type::val,
			text::fp_four_places{mil_change});
		text::localised_format_box(state, contents, box, std::string_view("topbar_avg_mil"), sub);
		text::add_line_break_to_layout_box(state, contents, box);
		text::localised_format_box(state, contents, box, std::string_view("topbar_avg_change"), sub);
		text::close_layout_box(contents, box);

		active_modifiers_description(state, contents, nation_id, 0, sys::national_mod_offsets::global_pop_militancy_modifier, true);
		active_modifiers_description(state, contents, nation_id, 0, sys::national_mod_offsets::core_pop_militancy_modifier, true);
		active_modifiers_description(state, contents, nation_id, 0, sys::national_mod_offsets::non_accepted_pop_militancy_modifier, true);
	}

	void topbar_nation_consciousness_text::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto nation_id = retrieve<dcon::nation_id>(state, parent);

		auto box = text::open_layout_box(contents, 0);
		text::substitution_map sub;
		auto con_change = demographics::get_estimated_con_change(state, nation_id);
		auto total = state.world.nation_get_demographics(nation_id, demographics::total);
		text::add_to_substitution_map(sub, text::variable_type::avg,
			text::fp_two_places{ total != 0.f ? (state.world.nation_get_demographics(nation_id, demographics::consciousness) / total) : 0.f });
		text::add_to_substitution_map(sub, text::variable_type::val, text::fp_four_places{ con_change });
		text::localised_format_box(state, contents, box, std::string_view("topbar_avg_con"), sub);
		text::add_line_break_to_layout_box(state, contents, box);
		text::localised_format_box(state, contents, box, std::string_view("topbar_avg_change"), sub);
		text::close_layout_box(contents, box);

		active_modifiers_description(state, contents, nation_id, 0, sys::national_mod_offsets::global_pop_consciousness_modifier, true);
		active_modifiers_description(state, contents, nation_id, 0, sys::national_mod_offsets::core_pop_consciousness_modifier, true);
		active_modifiers_description(state, contents, nation_id, 0, sys::national_mod_offsets::non_accepted_pop_consciousness_modifier, true);
	}

	void topbar_nation_mobilization_size_text::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto n = retrieve<dcon::nation_id>(state, parent);

		auto issue_id = politics::get_issue_by_name(state, std::string_view("war_policy"));
		auto fat_id = dcon::fatten(state.world, n);
		auto box = text::open_layout_box(contents, 0);
		text::substitution_map sub2;
		text::add_to_substitution_map(sub2, text::variable_type::curr, military::total_regiments(state, fat_id));
		// TODO - we (might) want to give the value the current war policy provides, though its more transparent perhaps to
		// just give the NV + Mob. Impact Modifier?
		text::add_to_substitution_map(sub2, text::variable_type::impact, text::fp_percentage{ state.world.nation_get_modifier_values(n, sys::national_mod_offsets::mobilization_impact) });
		text::add_to_substitution_map(sub2, text::variable_type::policy, fat_id.get_issues(issue_id).get_name());
		text::add_to_substitution_map(sub2, text::variable_type::units, military::mobilized_regiments_pop_limit(state, fat_id));
		text::localised_single_sub_box(state, contents, box, std::string_view("topbar_mobilize_tooltip"), text::variable_type::curr, military::mobilized_regiments_pop_limit(state, fat_id));
		text::add_line_break_to_layout_box(state, contents, box);
		text::localised_format_box(state, contents, box, std::string_view("mobilization_impact_limit_desc"), sub2);
		text::add_line_break_to_layout_box(state, contents, box);
		text::localised_format_box(state, contents, box, std::string_view("mobilization_impact_limit_desc2"), sub2);
		text::close_layout_box(contents, box);

		active_modifiers_description(state, contents, n, 0, sys::national_mod_offsets::mobilization_impact, true);
		active_modifiers_description(state, contents, n, 0, sys::national_mod_offsets::mobilization_size, true);
	}

	void topbar_nation_leadership_points_text::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto n = retrieve<dcon::nation_id>(state, parent);

		auto box = text::open_layout_box(contents, 0);
		text::substitution_map sub;
		text::add_to_substitution_map(sub, text::variable_type::poptype,
		state.world.pop_type_get_name(state.culture_definitions.officers));
		text::add_to_substitution_map(sub, text::variable_type::value,
		text::fp_two_places{getResearchPointsFromPop(state, state.culture_definitions.officers, n)});
		text::add_to_substitution_map(sub, text::variable_type::fraction,
		text::fp_percentage{
			state.world.nation_get_demographics(n, demographics::to_key(state, state.culture_definitions.officers)) /
			state.world.nation_get_demographics(n, demographics::total) });
		text::add_to_substitution_map(sub, text::variable_type::optimal,
		text::fp_two_places{(state.world.pop_type_get_research_optimum(state.culture_definitions.officers) * 100)});
		text::localised_format_box(state, contents, box, std::string_view("alice_daily_leadership_tt"), sub);
		text::close_layout_box(contents, box);

		active_modifiers_description(state, contents, n, 0, sys::national_mod_offsets::leadership, true);
		active_modifiers_description(state, contents, n, 0, sys::national_mod_offsets::leadership_modifier, true);
	}

	void topbar_pause_button::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		text::add_line(state, contents, "topbar_pause_speed");
		if(state.last_nation_that_paused) {
			text::add_line(state, contents, "last_player_that_paused", text::variable_type::x, state.last_nation_that_paused);
		}

		if(state.network_mode == sys::network_mode_type::host) {
			auto box = text::open_layout_box(contents);
			for(auto& client : state.network_state.clients) {
				if(client.is_active()) {
					auto const n = client.playing_as.index();
					std::string player_name = std::string(state.network_state.map_of_player_names[n].to_string_view());
					text::add_to_layout_box(state, contents, box, player_name, text::text_color::yellow);
					text::add_space_to_layout_box(state, contents, box);
					text::add_to_layout_box(state, contents, box, client.last_game_date, text::text_color::white);
					text::add_line_break_to_layout_box(state, contents, box);
				}
			}
			text::close_layout_box(contents, box);
		}

		/*
		auto ymd = state.current_date.to_ymd(state.start_date);
		if(sys::is_leap_year(ymd.year)) {
			text::add_line(state, contents, "date_is_leap");
		} else {
			text::add_line(state, contents, "date_is_not_leap");
		}
		float nh_temp = 15.f;
		std::string nh_season;
		if(ymd.month == 12 || ymd.month <= 2) {
			nh_season = text::produce_simple_string(state, "winter");
		} else if(ymd.month >= 3 && ymd.month <= 5) {
			nh_season = text::produce_simple_string(state, "spring");
		} else if(ymd.month >= 6 && ymd.month <= 8) {
			nh_season = text::produce_simple_string(state, "summer");
		} else if(ymd.month >= 9 && ymd.month <= 11) {
			nh_season = text::produce_simple_string(state, "autumn");
		}
		text::add_line(state, contents, "topbar_date_season_nh", text::variable_type::x, std::string_view(nh_season));
		std::string sh_season;
		if(ymd.month >= 6 && ymd.month <= 8) {
			sh_season = text::produce_simple_string(state, "winter");
		} else if(ymd.month >= 9 && ymd.month <= 11) {
			sh_season = text::produce_simple_string(state, "spring");
		} else if(ymd.month == 12 || ymd.month <= 2) {
			sh_season = text::produce_simple_string(state, "summer");
		} else if(ymd.month >= 3 && ymd.month <= 5) {
			sh_season = text::produce_simple_string(state, "autumn");
		}
		text::add_line(state, contents, "topbar_date_season_sh", text::variable_type::x, std::string_view(sh_season));
		*/

		//auto r = ((float(rng::reduce(state.game_seed, 4096)) / 4096.f) * 8.f) - 4.f;
		//float avg_temp = (nh_temp + sh_temp + r) / 2.f;
		//text::add_line(state, contents, "topbar_date_temperature", text::variable_type::x, text::fp_two_places{ avg_temp }, text::variable_type::y, text::fp_two_places{ (avg_temp * (9.f / 5.f)) + 32.f });
		//topbar_date_temperature;Average temperature: §Y$x$°C§! / §Y$y$°F§!
	}

	void topbar_closed_factories_icon::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto nation_id = retrieve<dcon::nation_id>(state, parent);
		auto box = text::open_layout_box(contents, 0);
		if(!economy_factory::nation_has_closed_factories(state, nation_id)) {
			text::localised_format_box(state, contents, box, "remove_countryalert_no_hasclosedfactories", text::substitution_map{});
		} else if(economy_factory::nation_has_closed_factories(state, nation_id)) {
			text::localised_format_box(state, contents, box, "remove_countryalert_hasclosedfactories", text::substitution_map{});
			text::add_line_break_to_layout_box(state, contents, box);
			auto nation_fat = dcon::fatten(state.world, nation_id);
			for(auto staat_owner : nation_fat.get_state_ownership()) {
				auto staat = staat_owner.get_state().get_definition();
				bool new_staat = true;
				for(auto abstract : staat.get_abstract_state_membership()) {
					auto prov = abstract.get_province();
					for(auto factloc : prov.get_factory_location()) {
						auto scale = factloc.get_factory().get_production_scale();
						if(scale < 0.05f) {
							if(new_staat) {
								text::add_to_layout_box(state, contents, box, staat.get_name(), text::text_color::yellow);
								// text::add_divider_to_layout_box(state, contents, box);		// TODO - Parity needed!
								text::add_line_break_to_layout_box(state, contents, box);
								new_staat = false;
							}
							text::add_to_layout_box(state, contents, box, std::string_view(" - "));
							text::add_to_layout_box(state, contents, box, factloc.get_factory().get_building_type().get_name(), text::text_color::yellow);
							text::add_line_break_to_layout_box(state, contents, box);
						}
					}
				}
			}
		}
		text::close_layout_box(contents, box);
	}

	void topbar_unemployment_icon::on_update(sys::state& state) noexcept {
		auto nation_id = retrieve<dcon::nation_id>(state, parent);
		bool has_unemployed = false;
		// Only care about factory workers for displaying the red alert
		std::array<dcon::pop_type_id, 2> factory_workers{
			state.culture_definitions.primary_factory_worker,
			state.culture_definitions.secondary_factory_worker
		};
		for(auto pt : factory_workers) {
			for(auto si : state.world.nation_get_state_ownership(nation_id)) {
				auto state_instance = si.get_state();
				auto total = state_instance.get_demographics(demographics::to_key(state, pt));
				auto unemployed = total - state_instance.get_demographics(demographics::to_employment_key(state, pt));
				if(unemployed >= 1.0f) {
					has_unemployed = true;
					break;
				}
			}
			if(has_unemployed)
				break;
		}
		frame = has_unemployed ? 0 : 1;
	}

	void topbar_unemployment_icon::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto nation_id = retrieve<dcon::nation_id>(state, parent);
		std::array<dcon::pop_type_id, 2> factory_workers{
			state.culture_definitions.primary_factory_worker,
			state.culture_definitions.secondary_factory_worker
		};
		struct unemployed_data {
			float amount = 0.f;
			float ratio = 0.f;
			dcon::state_instance_id sid;
			dcon::pop_type_id pt;
		};
		std::vector<unemployed_data> data;
		float rem_unemp = 0.f;
		for(auto si : state.world.nation_get_state_ownership(nation_id)) {
			for(auto pt : factory_workers) {
				auto total = si.get_state().get_demographics(demographics::to_key(state, pt));
				auto unemployed = total - si.get_state().get_demographics(demographics::to_employment_key(state, pt));
				if(unemployed >= 1.f) {
					unemployed_data unemp;
					unemp.amount = unemployed;
					unemp.sid = si.get_state();
					unemp.pt = pt;
					unemp.ratio = unemployed / total;
					data.emplace_back(unemp);
					rem_unemp += unemployed;
				}
			}
		}
		if(!data.empty()) {
			text::add_line(state, contents, "remove_countryalert_hasunemployedworkers");
			sys::merge_sort(data.begin(), data.end(), [&](auto const& a, auto const& b) {
				if(a.amount != b.amount)
					return a.amount < b.amount;
				if(a.sid.index() != b.sid.index())
					return a.sid.index() < b.sid.index();
				return a.pt.index() < b.pt.index();
			});
			std::reverse(data.begin(), data.end());
			for(uint32_t i = 0; i < uint32_t(data.size()) && i < 10; i++) {
				text::substitution_map sub;
				text::add_to_substitution_map(sub, text::variable_type::num, int64_t(data[i].amount));
				text::add_to_substitution_map(sub, text::variable_type::type, state.world.pop_type_get_name(data[i].pt));
				text::add_to_substitution_map(sub, text::variable_type::state, data[i].sid);
				text::add_to_substitution_map(sub, text::variable_type::perc, text::fp_percentage_one_place{ data[i].ratio });
				auto box = text::open_layout_box(contents);
				text::localised_format_box(state, contents, box, "topbar_unemployed_pop", sub);
				text::close_layout_box(contents, box);
				rem_unemp -= data[i].amount;
			}
			if(rem_unemp > 0.f) {
				text::substitution_map sub;
				text::add_to_substitution_map(sub, text::variable_type::x, text::int_wholenum{ int32_t(rem_unemp) });
				auto box = text::open_layout_box(contents);
				text::localised_format_box(state, contents, box, "topbar_more_unemployed", sub);
				text::close_layout_box(contents, box);
			}
		} else {
			// TODO: remove alice_rgo_unemployment_country_alert
			text::add_line(state, contents, "countryalert_no_hasunemployedworkers");
		}
	}

	void topbar_available_reforms_icon::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto nation_id = retrieve<dcon::nation_id>(state, parent);
		auto box = text::open_layout_box(contents, 0);
		if(!nations::has_reform_available(state, nation_id)) {
			text::localised_format_box(state, contents, box, std::string_view("countryalert_no_candoreforms"), text::substitution_map{});
		} else if(nations::has_reform_available(state, nation_id)) {
			text::localised_format_box(state, contents, box, std::string_view("countryalert_candoreforms"), text::substitution_map{});
			text::add_divider_to_layout_box(state, contents, box);
			// Display Avaliable Reforms
			// Mostly a copy of nations::has_reform_avaliable
			auto last_date = state.world.nation_get_last_issue_or_reform_change(nation_id);
			if(bool(last_date) && (last_date + int32_t(state.defines.min_delay_between_reforms * 30.0f)) > state.current_date) {
				text::close_layout_box(contents, box);
				return;
			}
			if(state.world.nation_get_is_civilized(nation_id)) {
				for(auto i : state.culture_definitions.political_issues) {
					for(auto o : state.world.issue_get_options(i)) {
						if(o && politics::can_enact_political_reform(state, nation_id, o)) {
							auto fat_id = dcon::fatten(state.world, o);
							text::add_to_layout_box(state, contents, box, fat_id.get_parent_issue().get_name());
							text::add_to_layout_box(state, contents, box, std::string_view(": "));
							text::add_to_layout_box(state, contents, box, fat_id.get_name());
							text::add_line_break_to_layout_box(state, contents, box);
						}
					}
				}

				for(auto i : state.culture_definitions.social_issues) {
					for(auto o : state.world.issue_get_options(i)) {
						if(o && politics::can_enact_social_reform(state, nation_id, o)) {
							auto fat_id = dcon::fatten(state.world, o);
							text::add_to_layout_box(state, contents, box, fat_id.get_parent_issue().get_name());
							text::add_to_layout_box(state, contents, box, std::string_view(": "));
							text::add_to_layout_box(state, contents, box, fat_id.get_name());
							text::add_line_break_to_layout_box(state, contents, box);
						}
					}
				}

				text::close_layout_box(contents, box);
				return;
			} else {
				for(auto i : state.culture_definitions.military_issues) {
					for(auto o : state.world.reform_get_options(i)) {
						if(o && politics::can_enact_military_reform(state, nation_id, o)) {
							auto fat_id = dcon::fatten(state.world, o);
							text::add_to_layout_box(state, contents, box, fat_id.get_parent_reform().get_name());
							text::add_to_layout_box(state, contents, box, std::string_view(": "));
							text::add_to_layout_box(state, contents, box, fat_id.get_name());
							text::add_line_break_to_layout_box(state, contents, box);
						}
					}
				}

				for(auto i : state.culture_definitions.economic_issues) {
					for(auto o : state.world.reform_get_options(i)) {
						if(o && politics::can_enact_economic_reform(state, nation_id, o)) {
							auto fat_id = dcon::fatten(state.world, o);
							text::add_to_layout_box(state, contents, box, fat_id.get_parent_reform().get_name());
							text::add_to_layout_box(state, contents, box, std::string_view(": "));
							text::add_to_layout_box(state, contents, box, fat_id.get_name());
							text::add_line_break_to_layout_box(state, contents, box);
						}
					}
				}
			}
		}
		text::close_layout_box(contents, box);
	}

	void topbar_available_decisions_icon::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto nation_id = retrieve<dcon::nation_id>(state, parent);
		if(!nations::has_decision_available(state, nation_id)) {
			text::add_line(state, contents, "countryalert_no_candodecisions");
		} else {
			text::add_line(state, contents, "countryalert_candodecisions");
			text::substitution_map m;
			produce_decision_substitutions(state, m, state.local_player_nation);
			state.world.for_each_decision([&](dcon::decision_id di) {
				if(nation_id != state.local_player_nation || !state.world.decision_get_hide_notification(di)) {
					auto lim = state.world.decision_get_potential(di);
					if(!lim || trigger::evaluate(state, lim, trigger::to_generic(nation_id), trigger::to_generic(nation_id), 0)) {
						auto allow = state.world.decision_get_allow(di);
						if(!allow || trigger::evaluate(state, allow, trigger::to_generic(nation_id), trigger::to_generic(nation_id), 0)) {
							auto fat_id = dcon::fatten(state.world, di);
							auto box = text::open_layout_box(contents);
							text::add_to_layout_box(state, contents, box, fat_id.get_name(), m);
							text::close_layout_box(contents, box);
						}
					}
				}
			});
		}
	}

	void topbar_crisis_icon::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		text::substitution_map sub;
		text::add_to_substitution_map(sub, text::variable_type::temperature, text::fp_two_places{state.crisis_temperature});
		text::add_to_substitution_map(sub, text::variable_type::attacker, state.primary_crisis_attacker);
		text::add_to_substitution_map(sub, text::variable_type::defender, state.primary_crisis_defender);
		text::add_to_substitution_map(sub, text::variable_type::date, state.last_crisis_end_date);
		text::add_to_substitution_map(sub, text::variable_type::time, int32_t(state.defines.crisis_cooldown_months));
		if(state.current_crisis_mode == sys::crisis_mode::inactive) {
			auto box = text::open_layout_box(contents);
			text::localised_format_box(state, contents, box, std::string_view("alice_crisis_inactive"), sub);
			text::close_layout_box(contents, box);
		} else if(state.current_crisis_mode == sys::crisis_mode::finding_attacker) {
			auto box = text::open_layout_box(contents);
			text::localised_format_box(state, contents, box, std::string_view("alice_crisis_finding_attacker"), sub);
			text::close_layout_box(contents, box);
		} else if(state.current_crisis_mode == sys::crisis_mode::finding_defender) {
			auto box = text::open_layout_box(contents);
			text::localised_format_box(state, contents, box, std::string_view("alice_crisis_finding_defender"), sub);
			text::close_layout_box(contents, box);
		} else if(state.current_crisis_mode == sys::crisis_mode::heating_up) {
			auto box = text::open_layout_box(contents);
			text::localised_format_box(state, contents, box, std::string_view("alice_crisis_heating_up"), sub);
			text::close_layout_box(contents, box);
			//atackers
			text::add_line(state, contents, std::string_view("alice_crisis_par_1"));
			for(const auto par : state.crisis_participants) {
				if(!par.merely_interested && par.supports_attacker) {
					text::add_line(state, contents, text::get_name(state, par.id));
				}
			}
			//defenders
			text::add_line(state, contents, std::string_view("alice_crisis_par_2"));
			for(const auto par : state.crisis_participants) {
				if(!par.merely_interested && !par.supports_attacker) {
					text::add_line(state, contents, text::get_name(state, par.id));
				}
			}
			//merely interested
			text::add_line(state, contents, std::string_view("alice_crisis_par_3"));
			for(const auto par : state.crisis_participants) {
				if(par.merely_interested) {
					text::add_line(state, contents, text::get_name(state, par.id));
				}
			}
		}
		if(state.last_crisis_end_date) {
			auto box = text::open_layout_box(contents);
			text::localised_format_box(state, contents, box, std::string_view("alice_last_crisis"), sub);
			text::close_layout_box(contents, box);
		}
	}

	void topbar_sphere_icon::button_action(sys::state& state) noexcept {
		auto n = retrieve<dcon::nation_id>(state, parent);
		std::vector<dcon::nation_id> targets;
		if(nations::is_great_power(state, n)) {
			for(auto it : state.world.nation_get_gp_relationship_as_great_power(n)) {
				if((it.get_status() & nations::influence::is_banned) == 0) {
					if(it.get_influence() >= state.defines.increaseopinion_influence_cost
					&& (nations::influence::level_mask & it.get_status()) != nations::influence::level_in_sphere
					&& (nations::influence::level_mask & it.get_status()) != nations::influence::level_friendly) {
						targets.push_back(it.get_influence_target());
					} else if(!(it.get_influence_target().get_in_sphere_of()) && it.get_influence() >= state.defines.addtosphere_influence_cost) {
						targets.push_back(it.get_influence_target());
					} else if(it.get_influence_target().get_in_sphere_of()
					&& (nations::influence::level_mask & it.get_status()) == nations::influence::level_friendly
					&& it.get_influence() >= state.defines.removefromsphere_influence_cost) {
						targets.push_back(it.get_influence_target());
					}
				}
			}
		}
		if(targets.empty()) {
			state.open_diplomacy(dcon::nation_id{});
		} else {
			bool is_match = false;
			for(const auto n1 : targets) {
				is_match = false;
				for(const auto n2 : visited_nations) {
					if(n1 == n2) {
						is_match = true;
						break;
					}
				}
				if(!is_match) { //not on list, add it
					state.open_diplomacy(n1);
					visited_nations.push_back(n1);
					break;
				}
			}
			if(is_match) {
				state.open_diplomacy(dcon::nation_id{});
				visited_nations.clear();
			}
		}
	}

	void topbar_sphere_icon::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto n = retrieve<dcon::nation_id>(state, parent);
		if(!nations::is_great_power(state, n)) {
			text::add_line(state, contents, std::string_view("countryalert_no_gpstatus"));
		} else {
			bool added_increase_header = false;

			for(auto it : state.world.nation_get_gp_relationship_as_great_power(n)) {
				if((it.get_status() & nations::influence::is_banned) == 0) {
					if(it.get_influence() >= state.defines.increaseopinion_influence_cost
					&& (nations::influence::level_mask & it.get_status()) != nations::influence::level_in_sphere
					&& (nations::influence::level_mask & it.get_status()) != nations::influence::level_friendly) {
						if(!added_increase_header)
							text::add_line(state, contents, std::string_view("countryalert_canincreaseopinion"));
						added_increase_header = true;
						text::nation_name_and_flag(state, it.get_influence_target(), contents, 15);
					} else if(!(it.get_influence_target().get_in_sphere_of()) && it.get_influence() >= state.defines.addtosphere_influence_cost) {
						if(!added_increase_header)
							text::add_line(state, contents, std::string_view("countryalert_canincreaseopinion"));
						added_increase_header = true;
						text::nation_name_and_flag(state, it.get_influence_target(), contents, 15);
					} else if(it.get_influence_target().get_in_sphere_of()
					&& (nations::influence::level_mask & it.get_status()) == nations::influence::level_friendly
					&& it.get_influence() >= state.defines.removefromsphere_influence_cost) {
						if(!added_increase_header)
							text::add_line(state, contents, std::string_view("countryalert_canincreaseopinion"));
						added_increase_header = true;
						text::nation_name_and_flag(state, it.get_influence_target(), contents, 15);
					}
				}
			}
			bool added_reb_header = false;
			for(auto m : state.world.in_nation) {
				if(state.world.nation_get_in_sphere_of(m) == n) {
					[&]() {
						for(auto fac : state.world.nation_get_rebellion_within(m)) {
							if(rebel::get_faction_brigades_active(state, fac.get_rebels()) > 0) {
								if(!added_reb_header)
									text::add_line(state, contents, std::string_view("a_alert_reb"));
								added_reb_header = true;
								text::nation_name_and_flag(state, m, contents, 15);
								return;
							}
						}
					}();
				}
			}

			if(!added_increase_header && !added_reb_header)
			text::add_line(state, contents, std::string_view("alice_ca_cant_influence"));
		}
	}

	void topbar_window::on_create(sys::state& state) noexcept {
		window_element_base::on_create(state);
		base_data.position.y += 1;

		auto bg_pic = make_element_by_type<background_image>(state, "bg_main_menus");
		background_pic = bg_pic.get();
		background_pic->base_data.position.y -= 1;
		add_child_to_back(std::move(bg_pic));

		auto dpi_win = make_element_by_type<ui::diplomatic_message_topbar_listbox>(state, "alice_diplomessageicons_window");
		state.ui_state.request_topbar_listbox = dpi_win.get();
		add_child_to_front(std::move(dpi_win));

		state.ui_state.topbar_window = this;
		on_update(state);
	}

	std::unique_ptr<element_base> topbar_window::make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept {
		if(name == "topbar_bg") {
			return partially_transparent_image::make_element_by_type_alias(state, id);
		} else if(name == "topbar_paper") {
			return partially_transparent_image::make_element_by_type_alias(state, id);
		} else if(name == "topbarbutton_production") {
			return make_element_by_type<topbar_production_tab_button>(state, id);
		} else if(name == "topbarbutton_budget") {
			return make_element_by_type<topbar_budget_tab_button>(state, id);
		} else if(name == "topbarbutton_tech") {
			return make_element_by_type<topbar_technology_tab_button>(state, id);
		} else if(name == "topbarbutton_politics") {
			return make_element_by_type<topbar_politics_tab_button>(state, id);
		} else if(name == "topbarbutton_pops") {
			return make_element_by_type<topbar_population_view_button>(state, id);
		} else if(name == "topbarbutton_trade") {
			return make_element_by_type<topbar_trade_tab_button>(state, id);
		} else if(name == "topbarbutton_diplomacy") {
			return make_element_by_type<topbar_diplomacy_tab_button>(state, id);
		} else if(name == "topbarbutton_military") {
			return make_element_by_type<topbar_military_tab_button>(state, id);
		} else if(name == "button_speedup") {
			return make_element_by_type<topbar_speedup_button>(state, id);
		} else if(name == "button_speeddown") {
			return make_element_by_type<topbar_speeddown_button>(state, id);
		} else if(name == "pause_bg") {
			return make_element_by_type<topbar_pause_button>(state, id);
		} else if(name == "speed_indicator") {
			return make_element_by_type<topbar_speed_indicator>(state, id);
		} else if(name == "datetext") {
			return make_element_by_type<topbar_date_text>(state, id);
		} else if(name == "countryname") {
			return make_element_by_type<topbar_nation_name>(state, id);
		} else if(name == "player_flag") {
			return make_element_by_type<flag_button>(state, id);
		} else if(name == "country_prestige") {
			return make_element_by_type<topbar_nation_prestige_text>(state, id);
		} else if(name == "country_economic") {
			return make_element_by_type<topbar_nation_industry_score_text>(state, id);
		} else if(name == "country_military") {
			return make_element_by_type<topbar_nation_military_score_text>(state, id);
		} else if(name == "country_total") {
			return make_element_by_type<nation_total_score_text>(state, id);
		} else if(name == "country_colonial_power") {
			return make_element_by_type<topbar_nation_colonial_power_text>(state, id);
		} else if(name == "selected_prestige_rank") {
			return make_element_by_type<nation_prestige_rank_text>(state, id);
		} else if(name == "selected_industry_rank") {
			return make_element_by_type<nation_industry_rank_text>(state, id);
		} else if(name == "selected_military_rank") {
			return make_element_by_type<nation_military_rank_text>(state, id);
		} else if(name == "nation_totalrank") {
			return make_element_by_type<nation_rank_text>(state, id);
		} else if(name == "topbar_flag_overlay") {
			return make_element_by_type<nation_flag_frame>(state, id);
		} else if(name == "alert_building_factories") {
			return make_element_by_type<topbar_building_factories_icon>(state, id);
		} else if(name == "alert_closed_factories") {
			return make_element_by_type<topbar_closed_factories_icon>(state, id);
		} else if(name == "alert_unemployed_workers") {
			return make_element_by_type<topbar_unemployment_icon>(state, id);
		} else if(name == "budget_linechart") {
			return make_element_by_type<topbar_budget_line_graph>(state, id);
		} else if(name == "budget_warning") {
			return make_element_by_type<topbar_budget_warning>(state, id);
		} else if(name == "budget_funds") {
			return make_element_by_type<topbar_treasury_text>(state, id);
		} else if(name == "topbar_tech_progress") {
			return make_element_by_type<nation_technology_research_progress>(state, id);
		} else if(name == "tech_current_research") {
			return make_element_by_type<nation_current_research_text>(state, id);
		} else if(name == "topbar_researchpoints_value") {
			return make_element_by_type<topbar_nation_daily_research_points_text>(state, id);
		} else if(name == "tech_literacy_value") {
			return make_element_by_type<topbar_nation_literacy_text>(state, id);
		} else if(name == "politics_party_icon") {
			return make_element_by_type<nation_ruling_party_ideology_plupp>(state, id);
		} else if(name == "politics_ruling_party") {
			return make_element_by_type<nation_ruling_party_text>(state, id);
		} else if(name == "politics_supressionpoints_value") {
			return make_element_by_type<nation_suppression_points_text>(state, id);
		} else if(name == "politics_infamy_value") {
			return make_element_by_type<topbar_nation_infamy_text>(state, id);
		} else if(name == "alert_can_do_reforms") {
			return make_element_by_type<topbar_available_reforms_icon>(state, id);
		} else if(name == "alert_can_do_decisions") {
			return make_element_by_type<topbar_available_decisions_icon>(state, id);
		} else if(name == "alert_is_in_election") {
			return make_element_by_type<topbar_ongoing_election_icon>(state, id);
		} else if(name == "alert_have_rebels") {
			return make_element_by_type<topbar_rebels_icon>(state, id);
		} else if(name == "population_total_value") {
			return make_element_by_type<topbar_nation_population_text>(state, id);
		} else if(name == "topbar_focus_value") {
			return make_element_by_type<topbar_nation_focus_allocation_text>(state, id);
		} else if(name == "population_avg_mil_value") {
			return make_element_by_type<topbar_nation_militancy_text>(state, id);
		} else if(name == "population_avg_con_value") {
			return make_element_by_type<topbar_nation_consciousness_text>(state, id);
		} else if(name == "diplomacy_status") {
			auto ptr = make_element_by_type<topbar_at_peace_text>(state, id);
			atpeacetext = ptr.get();
			return ptr;
		} else if(name == "diplomacy_at_war") {
			auto ptr = make_element_by_type<overlapping_enemy_flags>(state, id);
			ptr->base_data.position.y -= ptr->base_data.position.y / 4;
			ptr->base_data.size.x /= 2;
			return ptr;
		} else if(name == "diplomacy_diplopoints_value") {
			return make_element_by_type<topbar_nation_diplomatic_points_text>(state, id);
		} else if(name == "alert_colony") {
			return make_element_by_type<topbar_colony_icon>(state, id);
		} else if(name == "alert_crisis") {
			return make_element_by_type<topbar_crisis_icon>(state, id);
		} else if(name == "alert_can_increase_opinion") {
			return make_element_by_type<topbar_sphere_icon>(state, id);
		} else if(name == "alert_loosing_gp") {
			return make_element_by_type<topbar_losing_gp_status_icon>(state, id);
		} else if(name == "military_army_value") {
			return make_element_by_type<topbar_nation_brigade_allocation_text>(state, id);
		} else if(name == "military_navy_value") {
			return make_element_by_type<topbar_nation_navy_allocation_text>(state, id);
		} else if(name == "military_manpower_value") {
			return make_element_by_type<topbar_nation_mobilization_size_text>(state, id);
		} else if(name == "military_leadership_value") {
			return make_element_by_type<topbar_nation_leadership_points_text>(state, id);
		} else if(name == "topbar_outlinerbutton" || name == "topbar_outlinerbutton_bg") {
			// Fake button isn't used - we create it manually instead...
			return make_element_by_type<invisible_element>(state, id);
		} else if(name.substr(0, 13) == "topbar_import") {
			auto ptr = make_element_by_type<topbar_commodity_xport_icon>(state, id);
			std::string var = std::string{name.substr(13)};
			var.empty() ? ptr->slot = uint8_t(0) : ptr->slot = uint8_t(std::stoi(var));
			import_icons.push_back(ptr.get());
			return ptr;
		} else if(name.substr(0, 13) == "topbar_export") {
			auto ptr = make_element_by_type<topbar_commodity_xport_icon>(state, id);
			std::string var = std::string{name.substr(13)};
			var.empty() ? ptr->slot = uint8_t(0) : ptr->slot = uint8_t(std::stoi(var));
			export_icons.push_back(ptr.get());
			return ptr;
		} else if(name.substr(0, 15) == "topbar_produced") {
			auto ptr = make_element_by_type<topbar_commodity_xport_icon>(state, id);
			std::string var = std::string{name.substr(15)};
			var.empty() ? ptr->slot = uint8_t(0) : ptr->slot = uint8_t(std::stoi(var));
			produced_icons.push_back(ptr.get());
			return ptr;
		} else if(name == "selected_military_icon") {
			return make_element_by_type<military_score_icon>(state, id);
		} else if(name == "text_politics") {
			auto txt = make_element_by_type<simple_text_element_base>(state, id);
			txt->set_text(state, "?s" + text::produce_simple_string(state, "politics"));
			return txt;
		} else if(name == "text_pops") {
			auto txt = make_element_by_type<simple_text_element_base>(state, id);
			txt->set_text(state, "?s" + text::produce_simple_string(state, "population"));
			return txt;
		} else if(name == "text_trade") {
			auto txt = make_element_by_type<simple_text_element_base>(state, id);
			txt->set_text(state, "?s" + text::produce_simple_string(state, "trade"));
			return txt;
		} else if(name == "text_diplomacy") {
			auto txt = make_element_by_type<simple_text_element_base>(state, id);
			txt->set_text(state, "?s" + text::produce_simple_string(state, "diplomacy"));
			return txt;
		} else if(name == "text_military") {
			auto txt = make_element_by_type<simple_text_element_base>(state, id);
			txt->set_text(state, "?s" + text::produce_simple_string(state, "military"));
			return txt;
		} else if(name == "text_production") {
			auto txt = make_element_by_type<simple_text_element_base>(state, id);
			txt->set_text(state, "?s" + text::produce_simple_string(state, "production"));
			return txt;
		} else if(name == "text_tech") {
			auto txt = make_element_by_type<simple_text_element_base>(state, id);
			txt->set_text(state, "?s" + text::produce_simple_string(state, "technology"));
			return txt;
		} else if(name == "text_budget") {
			auto txt = make_element_by_type<simple_text_element_base>(state, id);
			txt->set_text(state, "?s" + text::produce_simple_string(state, "budget"));
			return txt;
		} else {
			return nullptr;
		}
	}

	void topbar_window::on_update(sys::state& state) noexcept {
		if(atpeacetext) {
			atpeacetext->set_visible(state, !state.world.nation_get_is_at_war(state.local_player_nation));
		}
		if(state.local_player_nation != current_nation) {
			current_nation = state.local_player_nation;
			Cyto::Any payload = current_nation;
			impl_set(state, payload);
		}

		for(auto& e : export_icons) {
			e->set_visible(state, false);
		}
		for(auto& e : import_icons) {
			e->set_visible(state, false);
		}
		for(auto& e : produced_icons) {
			e->set_visible(state, false);
		}

		{
			std::map<float, int32_t> v;
			for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
				dcon::commodity_id c{ dcon::commodity_id::value_base_t(i) };
				float produced = state.world.nation_get_domestic_market_pool(state.local_player_nation, c);
				float consumed = state.world.nation_get_real_demand(state.local_player_nation, c)
					* state.world.nation_get_demand_satisfaction(state.local_player_nation, c);
				v.insert({produced - consumed, c.index()});
			}

			uint8_t slot = 0;
			for(auto it = std::rbegin(v); it != std::rend(v); it++) {
				for(auto const& e : export_icons) {
					if(e->slot == slot) {
						dcon::commodity_id cid = dcon::commodity_id(dcon::commodity_id::value_base_t(it->second));
						e->frame = state.world.commodity_get_icon(cid);
						e->commodity_id = cid;
						e->amount = it->first;
						e->set_visible(state, true);
					}
				}
				++slot;
			}
			slot = 0;
			for(auto it = v.begin(); it != v.end(); it++) {
				for(auto const& e : import_icons) {
					if(e->slot == slot) {
						dcon::commodity_id cid = dcon::commodity_id(dcon::commodity_id::value_base_t(it->second));
						e->frame = state.world.commodity_get_icon(cid);
						e->commodity_id = cid;
						e->amount = it->first;
						e->set_visible(state, true);
					}
				}
				++slot;
			}
		}

		{
			std::map<float, int32_t> v;
			for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
				dcon::commodity_id c{ dcon::commodity_id::value_base_t(i) };
				v.insert({state.world.nation_get_domestic_market_pool(state.local_player_nation, c), c.index()});
			}

			uint8_t slot = 0;
			for(auto it = std::rbegin(v); it != std::rend(v); it++) {
				for(auto const& e : produced_icons) {
					if(e->slot == slot) {
						dcon::commodity_id cid = dcon::commodity_id(dcon::commodity_id::value_base_t(it->second));
						e->frame = state.world.commodity_get_icon(cid);
						e->commodity_id = cid;
						e->amount = it->first;
						e->set_visible(state, true);
					}
				}
				++slot;
			}
		}
	}
}
