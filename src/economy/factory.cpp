#include "factory.hpp"
#include "demographics.hpp"
#include "dcon_generated.hpp"
#include "ai.hpp"
#include "system_state.hpp"
#include "prng.hpp"
#include "math_fns.hpp"
#include "nations_templates.hpp"
#include "province_templates.hpp"
#include "triggers.hpp"
#include "factory_templates.hpp"
namespace factory {
	template void for_each_new_factory<std::function<void(new_factory)>>(sys::state&, dcon::state_instance_id, std::function<void(new_factory)>&&);
	template void for_each_upgraded_factory<std::function<void(upgraded_factory)>>(sys::state&, dcon::state_instance_id, std::function<void(upgraded_factory)>&&);

	float factory_build_cost_modifier(sys::state& state, dcon::nation_id n, bool pop_project) {
		float admin_eff = state.world.nation_get_administrative_efficiency(n);
		float factory_mod = std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::factory_cost) + 1.0f);
		float pop_factory_mod = std::max(0.001f, state.world.nation_get_modifier_values(n, sys::national_mod_offsets::factory_owner_cost));
		return (pop_project ? pop_factory_mod : (2.0f - admin_eff)) * factory_mod;
	}

	bool has_factory(sys::state const& state, dcon::state_instance_id si) {
		auto sdef = state.world.state_instance_get_definition(si);
		auto owner = state.world.state_instance_get_nation_from_state_ownership(si);
		auto crng = state.world.state_instance_get_state_building_construction(si);
		if(crng.begin() != crng.end())
			return true;

		for(auto p : state.world.state_definition_get_abstract_state_membership(sdef)) {
			if(p.get_province().get_nation_from_province_ownership() == owner) {
				auto rng = p.get_province().get_factory_location();
				if(rng.begin() != rng.end())
					return true;
			}
		}
		return false;
	}
	bool nation_is_constructing_factories(sys::state& state, dcon::nation_id n) {
		auto rng = state.world.nation_get_state_building_construction(n);
		return rng.begin() != rng.end();
	}
	bool nation_has_closed_factories(sys::state& state, dcon::nation_id n) { // TODO - should be "good" now
		auto nation_fat = dcon::fatten(state.world, n);
		for(auto prov_owner : nation_fat.get_province_ownership()) {
			auto prov = prov_owner.get_province();
			for(auto factloc : prov.get_factory_location()) {
				auto scale = factloc.get_factory().get_production_scale();
				if(scale < factory_closed_threshold) {
					return true;
				}
			}
		}
		return false;
	}
}
