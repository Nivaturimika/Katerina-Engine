#pragma once

#include "container_types.hpp"
#include "dcon_generated.hpp"

namespace economy {

	constexpr float default_workforce = 40000.f;

	constexpr float pop_payout_factor = 1.f;
	constexpr float consumption_factor = 1.f;

	constexpr float aristocrat_investment_ratio = 0.60f;
	constexpr float capitalist_investment_ratio = 0.85f;

	constexpr float satisfaction_delay_factor = 0.1f;
	constexpr float artisan_buff_factor = 1.25f;

	inline constexpr float ln_2 = 0.30103f;

	constexpr float artisan_baseline_score = 6.25f;

	enum commodity_production_type {
		primary,
		derivative,
		both
	};

	struct building_information {
		economy::commodity_set cost;
		int32_t naval_capacity = 1;
		int32_t colonial_points[8] = { 30, 50, 70, 90, 110, 130, 150, 170 };
		int32_t colonial_range = 50;
		int32_t max_level = 6;
		int32_t time = 1080;
		float infrastructure = 0.16f;
		dcon::text_key name;
		dcon::modifier_id province_modifier;
		uint16_t padding2 = 0;
		bool defined = false;
		uint8_t padding[3] = { 0 };
	};

	struct profit_distribution {
		float per_primary_worker;
		float per_secondary_worker;
		float per_owner;
	};

	static_assert(sizeof(building_information) == 104);
	static_assert(sizeof(building_information::cost) == 40);
	static_assert(sizeof(building_information::colonial_points) == 32);
	static_assert(sizeof(building_information::province_modifier) == 2);
	static_assert(sizeof(building_information::name) == 4);
	static_assert(sizeof(building_information::cost)
		+ sizeof(building_information::naval_capacity)
		+ sizeof(building_information::colonial_range)
		+ sizeof(building_information::colonial_points)
		+ sizeof(building_information::max_level)
		+ sizeof(building_information::time)
		+ sizeof(building_information::infrastructure)
		== 92);
	static_assert(sizeof(building_information) ==
		sizeof(building_information::cost)
		+ sizeof(building_information::naval_capacity)
		+ sizeof(building_information::colonial_range)
		+ sizeof(building_information::colonial_points)
		+ sizeof(building_information::max_level)
		+ sizeof(building_information::time)
		+ sizeof(building_information::infrastructure)
		+ sizeof(building_information::province_modifier)
		+ sizeof(building_information::name)
		+ sizeof(building_information::defined)
		+ sizeof(building_information::padding)
		+ sizeof(building_information::padding2));

	inline std::string_view province_building_type_get_name(economy::province_building_type v) {
		switch(v) {
		case economy::province_building_type::railroad:
			return "railroad";
		case economy::province_building_type::fort:
			return "fort";
		case economy::province_building_type::naval_base:
			return "naval_base";
		case economy::province_building_type::bank:
			return "bank";
		case economy::province_building_type::university:
			return "university";
		case economy::province_building_type::urban_center:
			return "urban_center";
		case economy::province_building_type::farmland:
			return "farmland";
		case economy::province_building_type::mine:
			return "mine";
		default:
			return "???";
		}
	}

	inline std::string_view province_building_type_get_level_text(economy::province_building_type v) {
		switch(v) {
		case economy::province_building_type::railroad:
			return "railroad_level";
		case economy::province_building_type::fort:
			return "fort_level";
		case economy::province_building_type::naval_base:
			return "naval_base_level";
		case economy::province_building_type::bank:
			return "bank_level";
		case economy::province_building_type::university:
			return "university_level";
		case economy::province_building_type::urban_center:
			return "urban_center_level";
		case economy::province_building_type::farmland:
			return "farmland_level";
		case economy::province_building_type::mine:
			return "mine_level";
		default:
			return "???";
		}
	}

	float get_artisan_distribution_slow(sys::state& state, dcon::nation_id n, dcon::commodity_id c);

	// base subsistence
	inline constexpr float subsistence_factor = 10.0f;
	inline constexpr float subsistence_score_life = 30.0f;
	inline constexpr float subsistence_score_everyday = 50.0f;
	inline constexpr float subsistence_score_luxury = 80.0f;
	inline constexpr float subsistence_score_total = subsistence_score_life + subsistence_score_everyday + subsistence_score_luxury;

	struct global_economy_state {
		building_information building_definitions[max_building_types];
		float craftsmen_fraction = 0.8f;
		dcon::modifier_id selector_modifier{};
		dcon::modifier_id immigrator_modifier{};
	};
	static_assert(sizeof(global_economy_state) ==
	sizeof(global_economy_state::building_definitions)
	+ sizeof(global_economy_state::selector_modifier)
	+ sizeof(global_economy_state::immigrator_modifier)
	+ sizeof(global_economy_state::craftsmen_fraction));

	enum class worker_effect : uint8_t {
		none = 0, input, output, throughput
	};

	template<typename T>
	auto desired_needs_spending(sys::state const& state, T pop_indices) {
		// TODO: gather pop types, extract cached needs sum, etc etc
		return 0.0f;
	}

	constexpr inline dcon::commodity_id money(0);

	inline constexpr float production_scale_delta = 0.1f;
	inline constexpr uint32_t price_history_length = 256;
	inline constexpr uint32_t gdp_history_length = 128;
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

	void daily_update(sys::state& state, bool initiate_building);
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
	construction_status province_building_construction(sys::state& state, dcon::province_id, province_building_type t);

	float unit_construction_progress(sys::state& state, dcon::province_land_construction_id c);
	float unit_construction_progress(sys::state& state, dcon::province_naval_construction_id c);

	void bound_budget_settings(sys::state& state, dcon::nation_id n);

	int32_t most_recent_price_record_index(sys::state& state);
	int32_t previous_price_record_index(sys::state& state);

	void go_bankrupt(sys::state& state, dcon::nation_id n);
	dcon::modifier_id get_province_selector_modifier(sys::state& state);
	dcon::modifier_id get_province_immigrator_modifier(sys::state& state);
	bool can_take_loans(sys::state& state, dcon::nation_id n);
	float interest_payment(sys::state& state, dcon::nation_id n);
	float max_loan(sys::state& state, dcon::nation_id n);

	commodity_production_type get_commodity_production_type(sys::state& state, dcon::commodity_id c);

	//void update_land_ownership(sys::state& state);
	//void update_local_subsistence_factor(sys::state& state);
	float commodity_effective_price(sys::state& state, dcon::nation_id n, dcon::commodity_id c);
	void register_intermediate_demand(sys::state& state, dcon::nation_id n, dcon::commodity_id commodity_type, float amount);
	void register_domestic_supply(sys::state& state, dcon::nation_id n, dcon::commodity_id commodity_type, float amount);

	float commodity_set_effective_cost(sys::state& state, dcon::nation_id n, economy::commodity_set const& cset);
	float commodity_set_effective_cost(sys::state& state, dcon::nation_id n, economy::small_commodity_set const& cset);

	float commodity_set_total_satisfaction(sys::state& state, dcon::nation_id n, economy::commodity_set const& cset);
	float commodity_set_total_satisfaction(sys::state& state, dcon::nation_id n, economy::small_commodity_set const& cset);

	float commodity_market_activity(sys::state& state, dcon::nation_id n, dcon::commodity_id c);
} // namespace economy
