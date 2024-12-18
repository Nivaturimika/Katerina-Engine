#pragma once

#include "ai.hpp"
#include "demographics.hpp"
#include "system_state.hpp"

namespace ai {
	template<typename vector_type, typename tag_type>
	vector_type province_strategic_weight(sys::state& state, tag_type ids) {
		auto const sid = state.world.province_get_state_membership(ids);
		auto const owner = state.world.province_get_nation_from_province_ownership(ids);
		auto const sk = demographics::to_key(state, state.culture_definitions.soldiers);
		auto const total_soldiers = state.world.province_get_demographics(ids, sk);
		auto const soldiers_w = (1.f / state.defines.pop_size_per_regiment);
		auto const defense_mod = state.world.province_get_modifier_values(ids, sys::provincial_mod_offsets::defense);
		auto const fort_level = state.world.province_get_building_level(ids, state.economy_definitions.fort_building);
		auto const river_crossing = ve::apply([&](dcon::province_id p) {
			int32_t count = 0;
			for(auto padj : state.world.province_get_province_adjacency(p)) {
				if((padj.get_type() & province::border::river_crossing_bit) != 0) {
					++count; //river crossings count as 1
					if((padj.get_type() & province::border::coastal_bit) != 0) {
						++count; //sea crossings count as 2
					}
				}
			}
			return float(count);
		}, ids);
		/* Prioritize provinces with lots of soldiers -- mobilization centers and capitals */
		auto weight = vector_type(1.f);
		weight = weight * ve::max(defense_mod * 5.f, vector_type(1.f));
		weight = weight * ve::max(fort_level * 5.f, vector_type(1.f));
		weight = weight * ve::max(river_crossing * 5.f, vector_type(1.f));
		weight = weight * ve::select(state.world.state_instance_get_capital(sid) == ids, 1.5f, vector_type(1.f));
		weight = weight * ve::select(state.world.nation_get_capital(owner) == ids, 2.5f, vector_type(1.f));
		weight = weight * ve::max(total_soldiers * soldiers_w * 0.001f, vector_type(0.1f));
		return weight;
	}
}
