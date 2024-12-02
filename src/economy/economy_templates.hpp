#pragma once
#include "system_state.hpp"
//#include "economy_factory.hpp"

namespace economy {
	template<typename T>
	bool valid_artisan_good(sys::state& state, dcon::nation_id n, T cid) {
		auto kf = state.world.commodity_get_key_factory(cid);
		return (state.world.commodity_get_artisan_output_amount(cid) > 0.0f
			&& (state.world.commodity_get_is_available_from_start(cid)
			|| (kf && state.world.nation_get_active_building(n, kf))));
	}

	/*	Every day, a nation must pay its creditors. It must pay:
		national-modifier-to-loan-interest x debt-amount x interest-to-debt-holder-rate / 30
		When a nation takes a loan, the interest-to-debt-holder-rate is set at:
		nation-taking-the-loan-technology-loan-interest-modifier + define:LOAN_BASE_INTEREST, with a minimum of 0.01. */
	template<typename T>
	float interest_payment(sys::state& state, T n) {
		auto debt = state.world.nation_get_stockpiles(n, economy::money);
		auto nmod = ve::max(0.01f, (state.world.nation_get_modifier_values(n, sys::national_mod_offsets::loan_interest) + 1.0f)
			* state.defines.loan_base_interest);
		return ve::select(debt >= 0, 0.0f, -debt * nmod / 30.0f);
	}
}
