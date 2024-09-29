#pragma once
#include "system_state.hpp"

namespace economy_factory {
	template<typename F>
	void for_each_new_factory(sys::state& state, dcon::state_instance_id state_instance_id, F&& function) {
		for(auto state_building_construction_fat_id : state.world.state_instance_get_state_building_construction(state_instance_id)) {
			if(!state_building_construction_fat_id.get_is_upgrade()) {
				float administrative_efficiency = state.world.nation_get_administrative_efficiency(state_building_construction_fat_id.get_nation());
				float factory_modifier_values = state.world.nation_get_modifier_values(state_building_construction_fat_id.get_nation(), sys::national_mod_offsets::factory_cost) + 1.0f;
				float pop_factory_modifiers = std::max(0.1f, state.world.nation_get_modifier_values(state_building_construction_fat_id.get_nation(), sys::national_mod_offsets::factory_owner_cost));
				float administrative_cost_factor = (state_building_construction_fat_id.get_is_pop_project() ? pop_factory_modifiers : (2.0f - administrative_efficiency)) * factory_modifier_values;
	
				float total = 0.0f;
				float purchased = 0.0f;
				auto& goods = state.world.factory_type_get_construction_costs(state_building_construction_fat_id.get_type());
	
				for(uint32_t i = 0; i < economy::commodity_set::set_size; ++i) {
					total += goods.commodity_amounts[i] * administrative_cost_factor;
					purchased += state_building_construction_fat_id.get_purchased_goods().commodity_amounts[i];
				}
	
				function(economy_factory::new_factory{ total > 0.0f ? purchased / total : 0.0f, state_building_construction_fat_id.get_type().id });
			}
		}
	}

	template<typename F>
	void for_each_upgraded_factory(sys::state& state, dcon::state_instance_id state_instance_id, F&& function) {
		for(auto state_building_construction_fat_id : state.world.state_instance_get_state_building_construction(state_instance_id)) {
			if(state_building_construction_fat_id.get_is_upgrade()) {
				float administrative_eff = state.world.nation_get_administrative_efficiency(state_building_construction_fat_id.get_nation());
				float factory_modifier_values = state.world.nation_get_modifier_values(state_building_construction_fat_id.get_nation(), sys::national_mod_offsets::factory_cost) + 1.0f;
				float pop_factory_modifier_values = std::max(0.1f, state.world.nation_get_modifier_values(state_building_construction_fat_id.get_nation(), sys::national_mod_offsets::factory_owner_cost));
				float administrative_cost_factor = (state_building_construction_fat_id.get_is_pop_project() ? pop_factory_modifier_values : (2.0f - administrative_eff)) * factory_modifier_values;
	
				float total = 0.0f;
				float purchased = 0.0f;
				auto& goods = state.world.factory_type_get_construction_costs(state_building_construction_fat_id.get_type());
	
				for(uint32_t i = 0; i < economy::commodity_set::set_size; ++i) {
					total += goods.commodity_amounts[i] * administrative_cost_factor;
					purchased += state_building_construction_fat_id.get_purchased_goods().commodity_amounts[i];
				}
	
				function(economy_factory::upgraded_factory{ total > 0.0f ? purchased / total : 0.0f, state_building_construction_fat_id.get_type().id });
			}
		}
	}
}
