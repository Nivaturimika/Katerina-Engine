#pragma once

#include "container_types.hpp"
#include "dcon_generated.hpp"

namespace economy {
	// Demand more for construction than required (won't affect the intaken goods daily)
	// only will affect demand
	constexpr inline float excess_construction_demand = 1.f;
	// From the total construction demand, multiply the budget by this factor to obtain the actual
	// spending (and also to not get refunded for non-existing goods)
	constexpr inline float true_construction_demand = 1.f;
	constexpr inline float default_workforce = 40000.f;
	constexpr inline float pop_payout_factor = 1.f;
	inline constexpr float ln_2 = 0.30103f;
	constexpr float artisan_baseline_score = 6.25f;

	enum commodity_production_type {
		primary,
		derivative,
		both
	};

	struct profit_distribution {
		float per_primary_worker;
		float per_secondary_worker;
		float per_owner;
	};

	float get_artisan_distribution_slow(sys::state& state, dcon::nation_id n, dcon::commodity_id c);

	struct global_economy_state {
		float craftsmen_fraction = 0.8f; //4
		dcon::province_building_type_id railroad_building; //8,1
		dcon::province_building_type_id fort_building; //9,1
		dcon::province_building_type_id naval_base_building; //10,1
		dcon::province_building_type_id bank_building; //11,1
		dcon::province_building_type_id university_building; //12,1
		uint8_t padding[3] = { 0, 0, 0 }; //13,1
	};
	static_assert(sizeof(global_economy_state) ==
		sizeof(global_economy_state::railroad_building)
		+ sizeof(global_economy_state::fort_building)
		+ sizeof(global_economy_state::naval_base_building)
		+ sizeof(global_economy_state::craftsmen_fraction)
		+ sizeof(global_economy_state::bank_building)
		+ sizeof(global_economy_state::university_building)
		+ sizeof(global_economy_state::padding));

	enum class worker_effect : uint8_t {
		none = 0, input, output, throughput
	};

	template<typename T>
	auto desired_needs_spending(sys::state const& state, T pop_indices) {
		// TODO: gather pop types, extract cached needs sum, etc etc
		return 0.0f;
	}

	constexpr inline dcon::commodity_id money(0);
	inline constexpr uint32_t price_history_length = 256;
	inline constexpr float rgo_owners_cut = 0.05f;

	void presimulate(sys::state& state);

	float commodity_daily_production_amount(sys::state& state, dcon::commodity_id c);

	float full_spending_cost(sys::state& state, dcon::nation_id n);
	void populate_army_consumption(sys::state& state);
	void populate_navy_consumption(sys::state& state);
	void populate_construction_consumption(sys::state& state);

	//float subsistence_max_pseudoemployment(sys::state& state, dcon::nation_id n, dcon::province_id p);
	bool has_building(sys::state const& state, dcon::state_instance_id si, dcon::factory_type_id fac);
	bool is_bankrupt_debtor_to(sys::state& state, dcon::nation_id debt_holder, dcon::nation_id debtor);

	//void populate_effective_prices(sys::state& state, dcon::nation_id n);

	void initialize(sys::state& state);
	void regenerate_unsaved_values(sys::state& state);

	float pop_min_wage_factor(sys::state& state, dcon::nation_id n);
	float pop_farmer_min_wage(sys::state& state, dcon::nation_id n, float min_wage_factor);
	float pop_laborer_min_wage(sys::state& state, dcon::nation_id n, float min_wage_factor);

	struct rgo_workers_breakdown {
		float paid_workers = 0.f;
		float slaves = 0.f;
		float total = 0.f;
	};

	void daily_update(sys::state& state);
	void resolve_constructions(sys::state& state);
	void limit_pop_demand_to_production(sys::state& state);

	float base_artisan_profit(sys::state& state, dcon::nation_id n, dcon::commodity_id c);
	float artisan_scale_limit(sys::state& state, dcon::nation_id n, dcon::commodity_id c);

	float stockpile_commodity_daily_increase(sys::state& state, dcon::commodity_id c, dcon::nation_id n);
	float global_market_commodity_daily_increase(sys::state& state, dcon::commodity_id c);
	float government_consumption(sys::state& state, dcon::nation_id n, dcon::commodity_id c);
	float nation_pop_consumption(sys::state& state, dcon::nation_id n, dcon::commodity_id c);
	float nation_total_imports(sys::state& state, dcon::nation_id n);
	float pop_income(sys::state& state, dcon::pop_id p);

	struct construction_status {
		float progress = 0.0f; // in range [0,1)
		bool is_under_construction = false;
	};

	construction_status state_building_construction(sys::state& state, dcon::state_instance_id s, dcon::factory_type_id t);
	construction_status province_building_construction(sys::state& state, dcon::province_id, dcon::province_building_type_id t);

	float unit_construction_progress(sys::state& state, dcon::province_land_construction_id c);
	float unit_construction_progress(sys::state& state, dcon::province_naval_construction_id c);

	void bound_budget_settings(sys::state& state, dcon::nation_id n);

	int32_t most_recent_price_record_index(sys::state& state);
	int32_t previous_price_record_index(sys::state& state);

	void go_bankrupt(sys::state& state, dcon::nation_id n);
	bool can_take_loans(sys::state& state, dcon::nation_id n);
	float max_loan(sys::state& state, dcon::nation_id n);

	commodity_production_type get_commodity_production_type(sys::state& state, dcon::commodity_id c);

	float commodity_effective_price(sys::state& state, dcon::nation_id n, dcon::commodity_id c);
	void register_intermediate_demand(sys::state& state, dcon::nation_id n, dcon::commodity_id commodity_type, float amount);
	void register_domestic_supply(sys::state& state, dcon::nation_id n, dcon::commodity_id commodity_type, float amount);

	float commodity_set_effective_cost(sys::state& state, dcon::nation_id n, economy::commodity_set const& cset);
	float commodity_set_effective_cost(sys::state& state, dcon::nation_id n, economy::small_commodity_set const& cset);

	float commodity_set_total_satisfaction(sys::state& state, dcon::nation_id n, economy::commodity_set const& cset);
	float commodity_set_total_satisfaction(sys::state& state, dcon::nation_id n, economy::small_commodity_set const& cset);

	float commodity_market_activity(sys::state& state, dcon::nation_id n, dcon::commodity_id c);
} // namespace economy
