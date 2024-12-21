#pragma once
#include "system_state.hpp"

namespace province {

	template<typename T>
	auto is_overseas(sys::state const& state, T ids) {
		auto owners = state.world.province_get_nation_from_province_ownership(ids);
		auto owner_cap = state.world.nation_get_capital(owners);
		return (state.world.province_get_continent(ids) != state.world.province_get_continent(owner_cap)) &&
				 (state.world.province_get_connected_region_id(ids) != state.world.province_get_connected_region_id(owner_cap));
	}

	template<typename F>
	void for_each_land_province(sys::state& state, F const& func) {
		int32_t last = state.province_definitions.first_sea_province.index();
		for(int32_t i = 0; i < last; ++i) {
		dcon::province_id pid{dcon::province_id::value_base_t(i)};
			func(pid);
		}
	}

	template<typename F>
	void ve_for_each_land_province(sys::state& state, F const& func) {
		int32_t last = state.province_definitions.first_sea_province.index();
		ve::execute_serial<dcon::province_id>(uint32_t(last), func);
	}

	template<typename F>
	void for_each_sea_province(sys::state& state, F const& func) {
		int32_t first = state.province_definitions.first_sea_province.index();
		for(int32_t i = first; i < int32_t(state.world.province_size()); ++i) {
		dcon::province_id pid{dcon::province_id::value_base_t(i)};
			func(pid);
		}
	}

	template<typename F>
	void for_each_province_in_state_instance(sys::state const& state, dcon::state_instance_id s, F const& func) {
		auto const d = state.world.state_instance_get_definition(s);
		auto const o = state.world.state_instance_get_nation_from_state_ownership(s);
		for(auto const p : state.world.state_definition_get_abstract_state_membership(d)) {
			if(p.get_province().get_nation_from_province_ownership() == o) {
				func(p.get_province().id);
			}
		}
	}

	/* Checks if we can build a province building in a given province, however with a limit
		this is mainly used for can_build_in_province = { limit_to_max_world = yes/no } trigger */
	template<typename vector_type, typename primary_type, typename this_type>
	vector_type can_build_province_building_in_province_limit(sys::state& state, dcon::province_building_type_id pbt, primary_type prov, this_type n, bool limit, bool whole_state) {
		auto const level = state.world.province_get_building_level(prov, pbt);
		auto const max_level = state.world.nation_get_max_building_level(n, pbt);
		if(pbt == state.economy_definitions.naval_base_building) {
			auto const has_base = ve::apply([&state](dcon::state_instance_id i) {
				return !(military::state_has_naval_base(state, i));
			}, state.world.province_get_state_membership(prov));
			return state.world.province_get_is_coast(prov) && (level < max_level) && (level != 0 || has_base);
		} else if(pbt == state.economy_definitions.railroad_building) {
			auto const pmod = state.world.province_get_modifier_values(prov, sys::provincial_mod_offsets::min_build_railroad);
			return level + ve::to_int(pmod) < max_level;
		}
		return level < max_level;
	}
} // namespace province
