#pragma once
#include "system_state.hpp"
#include "economy_factory.hpp"

namespace economy_factory {
	template<typename F>
	void for_each_new_factory(sys::state& state, dcon::state_instance_id state_instance_id, F&& function) {
		for(auto c : state.world.state_instance_get_state_building_construction(state_instance_id)) {
			if(!c.get_is_upgrade()) {
				float total = c.get_type().get_construction_time();
				float value = c.get_remaining_construction_time();
				function(economy_factory::new_factory{ total > 0.f ? 1.f - (value / total) : 0.f, c.get_type().id });
			}
		}
	}

	template<typename F>
	void for_each_upgraded_factory(sys::state& state, dcon::state_instance_id state_instance_id, F&& function) {
		for(auto c : state.world.state_instance_get_state_building_construction(state_instance_id)) {
			if(c.get_is_upgrade()) {
				float total = c.get_type().get_construction_time();
				float value = c.get_remaining_construction_time();
				function(economy_factory::upgraded_factory{ total > 0.f ? 1.f - (value / total) : 0.f, c.get_type().id });
			}
		}
	}
}
