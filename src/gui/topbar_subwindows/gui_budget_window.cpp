#include "system_state.hpp"
#include "gui_element_templates.hpp"
#include "gui_budget_window.hpp"
#include "economy_templates.hpp"

dcon::text_key dcon::pop_satisfaction_wrapper_fat::names[5] = {};

namespace ui {
	void nation_loan_spending_text::on_update(sys::state& state) noexcept {
		set_text(state, text::format_money(economy::interest_payment(state, state.local_player_nation)));
	}

	void nation_diplomatic_balance_text::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto n = retrieve<dcon::nation_id>(state, parent);

		float w_subsidies_amount = economy_estimations::estimate_war_subsidies_income(state, n) - economy_estimations::estimate_war_subsidies_spending(state, n);
		float reparations_amount = economy_estimations::estimate_reparations_income(state, n) - economy_estimations::estimate_reparations_spending(state, n);

		if(w_subsidies_amount > 0.0f) {
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::val, text::fp_one_place{ w_subsidies_amount });
			auto box = text::open_layout_box(contents, 0);
			text::localised_format_box(state, contents, box, "warsubsidies_income", m);
			text::close_layout_box(contents, box);
		} else if(w_subsidies_amount < 0.0f) {
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::val, text::fp_one_place{ w_subsidies_amount });
			auto box = text::open_layout_box(contents, 0);
			text::localised_format_box(state, contents, box, "warsubsidies_expense", m);
			text::close_layout_box(contents, box);
		}

		if(reparations_amount > 0.0f) {
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::val, text::fp_one_place{ w_subsidies_amount });
			auto box = text::open_layout_box(contents, 0);
			text::localised_format_box(state, contents, box, "warindemnities_income", m);
			text::close_layout_box(contents, box);
		} else if(reparations_amount < 0.0f) {
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::val, text::fp_one_place{ w_subsidies_amount });
			auto box = text::open_layout_box(contents, 0);
			text::localised_format_box(state, contents, box, "warindemnities_expense", m);
			text::close_layout_box(contents, box);
		}
	}

	void nation_administrative_efficiency_text::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto n = state.local_player_nation;
		{
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::val,
				text::fp_percentage{ 1.0f + state.world.nation_get_modifier_values(n, sys::national_mod_offsets::administrative_efficiency_modifier) });
			auto box = text::open_layout_box(contents, 0);
			text::localised_format_box(state, contents, box, "admin_explain_1", m);
			text::close_layout_box(contents, box);
		}
		active_modifiers_description(state, contents, n, 15, sys::national_mod_offsets::administrative_efficiency_modifier, false);
		{
			auto non_colonial = state.world.nation_get_non_colonial_population(n);
			auto total = non_colonial > 0.0f ? state.world.nation_get_non_colonial_bureaucrats(n) / non_colonial : 0.0f;

			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::val, text::fp_percentage{ total });
			auto box = text::open_layout_box(contents, 0);
			text::localised_format_box(state, contents, box, "admin_explain_2", m);
			text::close_layout_box(contents, box);
		}
		{
			float issue_sum = 0.0f;
			for(auto i : state.culture_definitions.social_issues) {
				issue_sum = issue_sum + state.world.issue_option_get_administrative_multiplier(state.world.nation_get_issues(n, i));
			}
			auto from_issues = issue_sum * state.defines.bureaucracy_percentage_increment;

			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::val, text::fp_percentage{ (from_issues + state.defines.max_bureaucracy_percentage) });
			text::add_to_substitution_map(m, text::variable_type::x, text::fp_percentage{ state.defines.max_bureaucracy_percentage });
			text::add_to_substitution_map(m, text::variable_type::y, text::fp_percentage{ from_issues });
			auto box = text::open_layout_box(contents, 0);
			text::localised_format_box(state, contents, box, "admin_explain_3", m);
			text::close_layout_box(contents, box);
		}
	}

	template<culture::pop_strata Strata>
	class pop_satisfaction_piechart : public piechart<dcon::pop_satisfaction_wrapper_id> {
	protected:
		void on_update(sys::state& state) noexcept override {
			distribution.clear();
			if(parent == nullptr)
				return;
			auto total = 0.f;
			std::array<float, 5> sat_pool = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
			for(auto prov : state.world.nation_get_province_ownership(state.local_player_nation)) {
				for(auto pop_loc : prov.get_province().get_pop_location()) {
					auto pop_id = pop_loc.get_pop();
					auto pop_strata = state.world.pop_type_get_strata(state.world.pop_get_poptype(pop_id));
					auto pop_size = pop_strata == uint8_t(Strata) ? state.world.pop_get_size(pop_id) : 0.f;
					// All luxury needs
					// OR All everyday needs
					// OR All life needs
					// OR Some life needs
					// OR No needs fulfilled...
					sat_pool[(pop_id.get_luxury_needs_satisfaction() > 0.95f)             ? 4
								 : (pop_id.get_everyday_needs_satisfaction() > 0.95f) ? 3
								 : (pop_id.get_life_needs_satisfaction() > 0.95f)     ? 2
								 : (pop_id.get_life_needs_satisfaction() > 0.01f)     ? 1
								 : 0] += pop_size;
				}
			}
			for(uint8_t i = 0; i < 5; i++)
				distribution.emplace_back(dcon::pop_satisfaction_wrapper_id(i), sat_pool[i]);
			update_chart(state);
		}
	public:
		void on_create(sys::state& state) noexcept override {
			// Fill-in static information...
			static bool has_run = false;
			if(!has_run) {
				dcon::fatten(state.world, dcon::pop_satisfaction_wrapper_id{0})
					.set_name(text::find_or_add_key(state, "budget_strata_need", false));
				dcon::fatten(state.world, dcon::pop_satisfaction_wrapper_id{1})
					.set_name(text::find_or_add_key(state, "budget_strata_need", false));
				dcon::fatten(state.world, dcon::pop_satisfaction_wrapper_id{2})
					.set_name(text::find_or_add_key(state, "budget_strata_need", false));
				dcon::fatten(state.world, dcon::pop_satisfaction_wrapper_id{3})
					.set_name(text::find_or_add_key(state, "budget_strata_need", false));
				dcon::fatten(state.world, dcon::pop_satisfaction_wrapper_id{4})
					.set_name(text::find_or_add_key(state, "budget_strata_need", false));
				has_run = true;
			}
			piechart::on_create(state);
		}

		void populate_tooltip(sys::state& state, dcon::pop_satisfaction_wrapper_id psw, float percentage,
			text::columnar_layout& contents) noexcept override {
			static const std::string needs_types[5] = {"no_need", "some_life_needs", "life_needs", "everyday_needs", "luxury_needs"};
			auto fat_psw = dcon::fatten(state.world, psw);
			auto box = text::open_layout_box(contents, 0);
			auto sub = text::substitution_map{};
			auto needs_type = text::produce_simple_string(state, needs_types[psw.value]);
			text::add_to_substitution_map(sub, text::variable_type::val, text::fp_one_place{percentage * 100.f});
			text::add_to_substitution_map(sub, text::variable_type::type, std::string_view(needs_type));
			text::add_to_layout_box(state, contents, box, fat_psw.get_name(), sub);
			text::close_layout_box(contents, box);
		}
	};

	void budget_expenditure_projection_text::put_values(sys::state& state, std::array<float, size_t(budget_slider_target::target_count)>& vals) noexcept {
		vals[uint8_t(budget_slider_target::construction_stock)] = economy_estimations::estimate_construction_spending(state, state.local_player_nation);
		vals[uint8_t(budget_slider_target::army_stock)] = economy_estimations::estimate_land_spending(state, state.local_player_nation);
		vals[uint8_t(budget_slider_target::navy_stock)] = economy_estimations::estimate_naval_spending(state, state.local_player_nation);
		vals[uint8_t(budget_slider_target::social)] = economy_estimations::estimate_social_spending(state, state.local_player_nation);
		vals[uint8_t(budget_slider_target::education)] = economy_estimations::estimate_pop_payouts_by_income_type(state, state.local_player_nation, culture::income_type::education);
		vals[uint8_t(budget_slider_target::admin)] = economy_estimations::estimate_pop_payouts_by_income_type(state, state.local_player_nation, culture::income_type::administration);
		vals[uint8_t(budget_slider_target::military)] = economy_estimations::estimate_pop_payouts_by_income_type(state, state.local_player_nation, culture::income_type::military);
		vals[uint8_t(budget_slider_target::domestic_investment)] = economy_estimations::estimate_domestic_investment(state, state.local_player_nation);
		vals[uint8_t(budget_slider_target::subsidies)] = economy_estimations::estimate_subsidy_spending(state, state.local_player_nation);
		vals[uint8_t(budget_slider_target::overseas)] = economy_estimations::estimate_overseas_penalty_spending(state, state.local_player_nation);
		vals[uint8_t(budget_slider_target::stockpile_filling)] = economy_estimations::estimate_stockpile_filling_spending(state, state.local_player_nation);
		vals[uint8_t(budget_slider_target::interest)] = economy::interest_payment(state, state.local_player_nation);
	}

	void budget_balance_projection_text::put_values(sys::state& state, std::array<float, size_t(budget_slider_target::target_count)>& vals) noexcept {
		// income
		vals[uint8_t(budget_slider_target::poor_tax)] = economy_estimations::estimate_tax_income_by_strata(state, state.local_player_nation, culture::pop_strata::poor);
		vals[uint8_t(budget_slider_target::middle_tax)] = economy_estimations::estimate_tax_income_by_strata(state, state.local_player_nation, culture::pop_strata::middle);
		vals[uint8_t(budget_slider_target::rich_tax)] = economy_estimations::estimate_tax_income_by_strata(state, state.local_player_nation, culture::pop_strata::rich);
		vals[uint8_t(budget_slider_target::gold_income)] = economy_estimations::estimate_gold_income(state, state.local_player_nation);

		// spend
		vals[uint8_t(budget_slider_target::construction_stock)] = -economy_estimations::estimate_construction_spending(state, state.local_player_nation);
		vals[uint8_t(budget_slider_target::army_stock)] = -economy_estimations::estimate_land_spending(state, state.local_player_nation);
		vals[uint8_t(budget_slider_target::navy_stock)] = -economy_estimations::estimate_naval_spending(state, state.local_player_nation);
		vals[uint8_t(budget_slider_target::social)] = -economy_estimations::estimate_social_spending(state, state.local_player_nation);
		vals[uint8_t(budget_slider_target::education)] = -economy_estimations::estimate_pop_payouts_by_income_type(state, state.local_player_nation, culture::income_type::education);
		vals[uint8_t(budget_slider_target::admin)] = -economy_estimations::estimate_pop_payouts_by_income_type(state, state.local_player_nation, culture::income_type::administration);
		vals[uint8_t(budget_slider_target::military)] = -economy_estimations::estimate_pop_payouts_by_income_type(state, state.local_player_nation, culture::income_type::military);
		vals[uint8_t(budget_slider_target::subsidies)] = -economy_estimations::estimate_subsidy_spending(state, state.local_player_nation);
		vals[uint8_t(budget_slider_target::overseas)] = -economy_estimations::estimate_overseas_penalty_spending(state, state.local_player_nation);
		vals[uint8_t(budget_slider_target::stockpile_filling)] = -economy_estimations::estimate_stockpile_filling_spending(state, state.local_player_nation);
		vals[uint8_t(budget_slider_target::domestic_investment)] = -economy_estimations::estimate_domestic_investment(state, state.local_player_nation);
		// balance
		vals[uint8_t(budget_slider_target::diplomatic_interest)] = economy_estimations::estimate_diplomatic_balance(state, state.local_player_nation);
		vals[uint8_t(budget_slider_target::interest)] = -economy::interest_payment(state, state.local_player_nation);
		vals[uint8_t(budget_slider_target::tariffs)] = economy_estimations::estimate_tariff_income(state, state.local_player_nation);
	}

	void budget_window::on_create(sys::state& state) noexcept {
		window_element_base::on_create(state);

		auto win1337 = make_element_by_type<budget_take_loan_window>(state, state.ui_state.defs_by_name.find(state.lookup_key("take_loan_window"))->second.definition);
		budget_take_loan_win = win1337.get();
		win1337->base_data.position.y -= 66; // Nudge >w<
		win1337->set_visible(state, false);
		add_child_to_front(std::move(win1337));

		auto win101 = make_element_by_type<budget_repay_loan_window>(state, state.ui_state.defs_by_name.find(state.lookup_key("repay_loan_window"))->second.definition);
		budget_repay_loan_win = win101.get();
		win101->base_data.position.y -= 66; // Nudge >w<
		win101->set_visible(state, false);
		add_child_to_front(std::move(win101));

		set_visible(state, false);
	}

	std::unique_ptr<element_base> budget_window::make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept {
		if(name == "main_bg") {
			return make_element_by_type<image_element_base>(state, id);
		} else if(name == "debt_checkbox") {
			return make_element_by_type<enable_debt_toggle>(state, id);
		} else if(name == "domestic_investment_slider") {
			return make_element_by_type<domestic_investment_slider>(state, id);
		} else if(name == "domestic_investment_value") {
			return make_element_by_type<domestic_investment_estimated_text>(state, id);
		} else if(name == "overseas_maintenance_slider") {
			return make_element_by_type<overseas_maintenance_slider>(state, id);
		} else if(name == "overseas_maintenance_value") {
			return make_element_by_type<overseas_maintenance_estimated_text>(state, id);
		} else if(name == "bg_budget") {
			return make_element_by_type<opaque_element_base>(state, id);
		} else if(name == "tariff_mid") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "close_button") {
			return make_element_by_type<generic_close_button>(state, id);
		} else if(name == "tariffs_percent") {
			auto ptr = make_element_by_type<budget_tariff_percentage_text>(state, id);
			ptr->base_data.position.x += int16_t(10); //nudge
			return ptr;
		} else if(name == "total_funds_val") {
			return make_element_by_type<nation_budget_funds_text>(state, id);
		} else if(name == "national_bank_val") {
			return make_element_by_type<nation_budget_bank_text>(state, id);
		} else if(name == "debt_val") {
			return make_element_by_type<nation_budget_debt_text>(state, id);
		} else if(name == "interest_val") {
			return make_element_by_type<nation_budget_interest_text>(state, id);
		} else if(name == "tab_takenloans") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "tab_givenloans") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "givenloans_text") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "take_loan") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "repay_loan") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "gunboat_alert") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "chart_debt") {
			return make_element_by_type<debt_piechart>(state, id);
		} else if(name == "debt_listbox") {
			return make_element_by_type<debt_listbox>(state, id);
		} else if(name == "chart_0") {
			return make_element_by_type<pop_satisfaction_piechart<culture::pop_strata::poor>>(state, id);
		} else if(name == "chart_1") {
			return make_element_by_type<pop_satisfaction_piechart<culture::pop_strata::middle>>(state, id);
		} else if(name == "chart_2") {
			return make_element_by_type<pop_satisfaction_piechart<culture::pop_strata::rich>>(state, id) ;
		} else if(name == "nat_stock_val") {
			auto ptr = make_element_by_type<budget_actual_stockpile_spending_text>(state, id);
			ptr->expense = true;
			return ptr;
		} else if(name == "nat_stock_est") {
			auto ptr = make_element_by_type<budget_estimated_stockpile_spending_text>(state, id);
			ptr->expense = true;
			return ptr;
		} else if(name == "mil_cost_val") {
			auto ptr = make_element_by_type<budget_military_spending_text>(state, id);
			ptr->expense = true;
			return ptr;
		} else if(name == "overseas_cost_val") {
			auto ptr = make_element_by_type<budget_overseas_spending_text>(state, id);
			ptr->expense = true;
			return ptr;
		} else if(name == "tariff_val") {
			return make_element_by_type<budget_tariff_income_text>(state, id);
		} else if(name == "gold_inc") {
			return make_element_by_type<nation_gold_income_text>(state, id);
		} else if(name == "tax_0_inc") {
			return make_element_by_type<budget_stratified_tax_income_text<culture::pop_strata::poor, budget_slider_target::poor_tax>>( state, id);
		} else if(name == "tax_1_inc") {
			return make_element_by_type<budget_stratified_tax_income_text<culture::pop_strata::middle, budget_slider_target::middle_tax>>(state, id);
		} else if(name == "tax_2_inc") {
			return make_element_by_type<budget_stratified_tax_income_text<culture::pop_strata::rich, budget_slider_target::rich_tax>>( state, id);
		} else if(name == "exp_val_0") {
			auto ptr = make_element_by_type<budget_expenditure_text<culture::income_type::education, budget_slider_target::education>>( state, id);
			ptr->expense = true;
			return ptr;
		} else if(name == "exp_val_1") {
			auto ptr = make_element_by_type<budget_expenditure_text<culture::income_type::administration, budget_slider_target::admin>>(state, id);
			ptr->expense = true;
			return ptr;
		} else if(name == "exp_val_2") {
			auto ptr = make_element_by_type<budget_social_spending_text>(state, id);
			ptr->expense = true;
			return ptr;
		} else if(name == "exp_val_3") {
			auto ptr = make_element_by_type<budget_expenditure_text<culture::income_type::military, budget_slider_target::military>>(state, id);
			ptr->expense = true;
			return ptr;
		} else if(name == "admin_efficiency") {
			return make_element_by_type<nation_administrative_efficiency_text>(state, id);
		} else if(name == "interest_val") {
			return make_element_by_type<nation_loan_spending_text>(state, id);
		} else if(name == "ind_sub_val") {
			return make_element_by_type<nation_subsidy_spending_text>(state, id);
		} else if(name == "diplomatic_balance") {
			return make_element_by_type<nation_diplomatic_balance_text>(state, id);
		} else if(name == "total_inc") {
			return make_element_by_type<budget_income_projection_text>(state, id);
		} else if(name == "total_exp") {
			auto ptr = make_element_by_type<budget_expenditure_projection_text>(state, id);
			ptr->expense = true;
			return ptr;
		} else if(name == "balance") {
			return make_element_by_type<budget_balance_projection_text>(state, id);
		} else if(name == "tax_0_slider") {
			return make_element_by_type<budget_poor_tax_slider>(state, id);
		} else if(name == "tax_1_slider") {
			return make_element_by_type<budget_middle_tax_slider>(state, id);
		} else if(name == "tax_2_slider") {
			return make_element_by_type<budget_rich_tax_slider>(state, id);
		} else if(name == "land_stockpile_slider") {
			return make_element_by_type<budget_army_stockpile_slider>(state, id);
		} else if(name == "naval_stockpile_slider") {
			return make_element_by_type<budget_navy_stockpile_slider>(state, id);
		} else if(name == "projects_stockpile_slider") {
			return make_element_by_type<budget_construction_stockpile_slider>(state, id);
		} else if(name == "exp_0_slider") {
			return make_element_by_type<budget_education_slider>(state, id);
		} else if(name == "exp_1_slider") {
			return make_element_by_type<budget_administration_slider>(state, id);
		} else if(name == "exp_2_slider") {
			return make_element_by_type<budget_social_spending_slider>(state, id);
		} else if(name == "exp_3_slider") {
			return make_element_by_type<budget_military_spending_slider>(state, id);
		} else if(name == "tariff_slider") {
			return make_element_by_type<budget_tariff_slider>(state, id);
		} else if(name == "take_loan") {
			return make_element_by_type<budget_take_loan_button>(state, id);
		} else if(name == "repay_loan") {
			return make_element_by_type<budget_repay_loan_button>(state, id);
		} else if(name == "tax_0_pops") {
			return make_element_by_type<budget_pop_tax_list<culture::pop_strata::poor>>(state, id);
		} else if(name == "tax_1_pops") {
			return make_element_by_type<budget_pop_tax_list<culture::pop_strata::middle>>(state, id);
		} else if(name == "tax_2_pops") {
			return make_element_by_type<budget_pop_tax_list<culture::pop_strata::rich>>(state, id);
		} else if(name == "exp_0_pops") {
			return make_element_by_type<budget_small_pop_income_list<culture::income_type::education>>(state, id);
		} else if(name == "exp_1_pops") {
			return make_element_by_type<budget_small_pop_income_list<culture::income_type::administration>>(state, id);
		// } else if(name == "exp_2_pops") {   // intentionally unused
			// 	return make_element_by_type<budget_pop_income_list<culture::income_type::reforms>>(state, id);
		} else if(name == "exp_3_pops") {
			return make_element_by_type<budget_pop_income_list<culture::income_type::military>>(state, id);
		} else {
			return nullptr;
		}
	}

	message_result budget_window::get(sys::state& state, Cyto::Any& payload) noexcept {
		if(payload.holds_type<dcon::nation_id>()) {
			payload.emplace<dcon::nation_id>(state.local_player_nation);
			return message_result::consumed;
		}
		//=====================================================================
		else if(payload.holds_type<element_selection_wrapper<bool>>()) {
			bool type = any_cast<element_selection_wrapper<bool>>(payload).data;
			if(type) { // Take Loan Win.
				budget_take_loan_win->set_visible(state, true);
				move_child_to_front(budget_take_loan_win);
			} else { // Repay Loan Win.
				budget_repay_loan_win->set_visible(state, true);
				move_child_to_front(budget_repay_loan_win);
			}
			return message_result::consumed;
		} else if(payload.holds_type<budget_slider_signal>()) {
			impl_set(state, payload);
		}

		return message_result::unseen;
	}
}
