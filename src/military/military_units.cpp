#include "military.hpp"
#include "military_templates.hpp"
#include "dcon_generated.hpp"
#include "prng.hpp"
#include "effects.hpp"
#include "events.hpp"
#include "ai.hpp"
#include "demographics.hpp"
#include "politics.hpp"
#include "province_templates.hpp"
#include "rebels.hpp"
#include "triggers.hpp"
#include "container_types.hpp"
#include "pdqsort.h"

namespace military {
	int32_t total_regiments(sys::state& state, dcon::nation_id n) {
		return state.world.nation_get_active_regiments(n);
	}

	int32_t total_ships(sys::state& state, dcon::nation_id n) {
		int32_t total = 0;
		for(auto v : state.world.nation_get_navy_control(n)) {
			auto srange = v.get_navy().get_navy_membership();
			total += int32_t(srange.end() - srange.begin());
		}
		return total;
	}

	void reset_unit_stats(sys::state& state) {
		for(uint32_t i = 0; i < state.military_definitions.unit_base_definitions.size(); ++i) {
			dcon::unit_type_id uid = dcon::unit_type_id{ dcon::unit_type_id::value_base_t(i) };
			state.world.for_each_nation([&](dcon::nation_id nid) {
				state.world.nation_set_unit_stats(nid, uid, state.military_definitions.unit_base_definitions[uid]);
			});
		}
	}

	void apply_base_unit_stat_modifiers(sys::state& state) {
		assert(state.military_definitions.base_army_unit.index() < 2);
		assert(state.military_definitions.base_naval_unit.index() < 2);
		assert(state.military_definitions.base_army_unit.index() != -1);
		assert(state.military_definitions.base_naval_unit.index() != -1);
		for(uint32_t i = 2; i < state.military_definitions.unit_base_definitions.size(); ++i) {
			dcon::unit_type_id uid = dcon::unit_type_id{ dcon::unit_type_id::value_base_t(i) };
			auto base_id = state.military_definitions.unit_base_definitions[uid].is_land
				? state.military_definitions.base_army_unit
				: state.military_definitions.base_naval_unit;
			state.world.for_each_nation([&](dcon::nation_id nid) {
				auto& base_stats = state.world.nation_get_unit_stats(nid, base_id);
				auto& current_stats = state.world.nation_get_unit_stats(nid, uid);
				current_stats += base_stats;
			});
		}
	}

	int32_t regiments_created_from_province(sys::state& state, dcon::province_id p) {
		int32_t total = 0;
		for(auto pop : state.world.province_get_pop_location(p)) {
			if(pop.get_pop().get_poptype() == state.culture_definitions.soldiers) {
				auto regs = pop.get_pop().get_regiment_source();
				total += int32_t(regs.end() - regs.begin());
			}
		}
		return total;
	}
	int32_t mobilized_regiments_created_from_province(sys::state& state, dcon::province_id p) {
		/*
		Mobilized regiments come only from non-colonial provinces.
		*/
		if(fatten(state.world, p).get_is_colonial())
			return 0;

		int32_t total = 0;
		for(auto pop : state.world.province_get_pop_location(p)) {
			if(pop.get_pop().get_poptype() != state.culture_definitions.soldiers &&
				pop.get_pop().get_poptype().get_strata() == uint8_t(culture::pop_strata::poor)) {
				auto regs = pop.get_pop().get_regiment_source();
				total += int32_t(regs.end() - regs.begin());
			}
		}
		return total;
	}

	int32_t mobilized_regiments_possible_from_pop(sys::state& state, dcon::pop_id p) {
		/*
		The number of regiments these pops can provide is determined by pop-size x mobilization-size /
		define:POP_SIZE_PER_REGIMENT.
		*/
		float ps = state.world.pop_get_size(p) * mobilization_size(state, nations::owner_of_pop(state, p));
		float pa = ps / state.defines.pop_min_size_for_regiment;
		auto loc = state.world.pop_get_province_from_pop_location(p);
		auto sid = state.world.province_get_state_membership(loc);
		return int32_t(std::floor(pa));
	}

	int32_t professional_regiments_possible_from_pop(sys::state& state, dcon::pop_id p) {
		/*
		- A soldier pop must be at least define:POP_MIN_SIZE_FOR_REGIMENT to support any regiments
		- If it is at least that large, then it can support one regiment per define:POP_SIZE_PER_REGIMENT x
		define:POP_MIN_SIZE_FOR_REGIMENT_COLONY_MULTIPLIER (if it is located in a colonial province) x
		define:POP_MIN_SIZE_FOR_REGIMENT_NONCORE_MULTIPLIER (if it is non-colonial but uncored)
		*/
		auto location = state.world.pop_get_province_from_pop_location(p);
		if(state.world.province_get_is_colonial(location)) {
			float divisor = state.defines.pop_size_per_regiment * state.defines.pop_min_size_for_regiment_colony_multiplier;
			float minimum = state.defines.pop_min_size_for_regiment;
			if(state.world.pop_get_size(p) >= minimum) {
				return int32_t((state.world.pop_get_size(p) / divisor) + 1);
			}
		} else if(!state.world.province_get_is_owner_core(location)) {
			float divisor = state.defines.pop_size_per_regiment * state.defines.pop_min_size_for_regiment_noncore_multiplier;
			float minimum = state.defines.pop_min_size_for_regiment;
			if(state.world.pop_get_size(p) >= minimum) {
				return int32_t((state.world.pop_get_size(p) / divisor) + 1);
			}
		} else {
			float divisor = state.defines.pop_size_per_regiment;
			float minimum = state.defines.pop_min_size_for_regiment;
			if(state.world.pop_get_size(p) >= minimum) {
				return int32_t((state.world.pop_get_size(p) / divisor) + 1);
			}
		}
		return 0;
	}

	int32_t regiments_possible_from_pop(sys::state& state, dcon::pop_id p) {
		auto type = state.world.pop_get_poptype(p);
		if(type == state.culture_definitions.soldiers) {
			return professional_regiments_possible_from_pop(state, p);
		}
		// mobilized
		return mobilized_regiments_possible_from_pop(state, p);
	}

	int32_t regiments_max_possible_from_province(sys::state& state, dcon::province_id p) {
		int32_t total = 0;
		for(auto pop : state.world.province_get_pop_location(p)) {
			if(pop.get_pop().get_poptype() == state.culture_definitions.soldiers) {
				total += professional_regiments_possible_from_pop(state, pop.get_pop());
			}
		}
		return total;
	}
	int32_t main_culture_regiments_created_from_province(sys::state& state, dcon::province_id p) {
		int32_t total = 0;
		for(auto pop : state.world.province_get_pop_location(p)) {
			if(pop.get_pop().get_poptype() == state.culture_definitions.soldiers && pop.get_pop().get_is_primary_or_accepted_culture()) {
				auto regs = pop.get_pop().get_regiment_source();
				total += int32_t(regs.end() - regs.begin());
			}
		}
		return total;
	}
	int32_t main_culture_regiments_max_possible_from_province(sys::state& state, dcon::province_id p) {
		int32_t total = 0;
		for(auto pop : state.world.province_get_pop_location(p)) {
			if(pop.get_pop().get_poptype() == state.culture_definitions.soldiers
			&& pop.get_pop().get_is_primary_or_accepted_culture()) {
				total += professional_regiments_possible_from_pop(state, pop.get_pop());
			}
		}
		return total;
	}
	int32_t regiments_under_construction_in_province(sys::state& state, dcon::province_id p) {
		int32_t total = 0;
		for(auto pop : state.world.province_get_pop_location(p)) {
			if(pop.get_pop().get_poptype() == state.culture_definitions.soldiers) {
				auto regs = pop.get_pop().get_province_land_construction();
				total += int32_t(regs.end() - regs.begin());
			}
		}
		return total;
	}
	int32_t main_culture_regiments_under_construction_in_province(sys::state& state, dcon::province_id p) {
		int32_t total = 0;
		for(auto pop : state.world.province_get_pop_location(p)) {
			if(pop.get_pop().get_poptype() == state.culture_definitions.soldiers && pop.get_pop().get_is_primary_or_accepted_culture()) {
				auto regs = pop.get_pop().get_province_land_construction();
				total += int32_t(regs.end() - regs.begin());
			}
		}
		return total;
	}

	dcon::pop_id find_available_soldier(sys::state& state, dcon::province_id p, dcon::culture_id pop_culture) {
		if(state.world.province_get_is_colonial(p)) {
			float divisor = state.defines.pop_size_per_regiment * state.defines.pop_min_size_for_regiment_colony_multiplier;
			float minimum = state.defines.pop_min_size_for_regiment;

			for(auto pop : state.world.province_get_pop_location(p)) {
				if(pop.get_pop().get_poptype() == state.culture_definitions.soldiers) {
					if(pop.get_pop().get_size() >= minimum && pop.get_pop().get_culture() == pop_culture) {
						auto amount = int32_t((pop.get_pop().get_size() / divisor) + 1);
						auto regs = pop.get_pop().get_regiment_source();
						auto building = pop.get_pop().get_province_land_construction();

						if(amount > ((regs.end() - regs.begin()) + (building.end() - building.begin()))) {
							return pop.get_pop().id;
						}
					}
				}
			}
			return dcon::pop_id{};
		} else if(!state.world.province_get_is_owner_core(p)) {
			float divisor = state.defines.pop_size_per_regiment * state.defines.pop_min_size_for_regiment_noncore_multiplier;
			float minimum = state.defines.pop_min_size_for_regiment;

			for(auto pop : state.world.province_get_pop_location(p)) {
				if(pop.get_pop().get_poptype() == state.culture_definitions.soldiers) {
					if(pop.get_pop().get_size() >= minimum && pop.get_pop().get_culture() == pop_culture) {
						auto amount = int32_t((pop.get_pop().get_size() / divisor) + 1);
						auto regs = pop.get_pop().get_regiment_source();
						auto building = pop.get_pop().get_province_land_construction();

						if(amount > ((regs.end() - regs.begin()) + (building.end() - building.begin()))) {
							return pop.get_pop().id;
						}
					}
				}
			}
			return dcon::pop_id{};
		} else {
			float divisor = state.defines.pop_size_per_regiment;
			float minimum = state.defines.pop_min_size_for_regiment;

			for(auto pop : state.world.province_get_pop_location(p)) {
				if(pop.get_pop().get_poptype() == state.culture_definitions.soldiers) {
					if(pop.get_pop().get_size() >= minimum && pop.get_pop().get_culture() == pop_culture) {
						auto amount = int32_t((pop.get_pop().get_size() / divisor) + 1);
						auto regs = pop.get_pop().get_regiment_source();
						auto building = pop.get_pop().get_province_land_construction();

						if(amount > ((regs.end() - regs.begin()) + (building.end() - building.begin()))) {
							return pop.get_pop().id;
						}
					}
				}
			}
			return dcon::pop_id{};
		}
	}

	dcon::pop_id find_available_soldier(sys::state& state, dcon::province_id p, bool require_accepted) {
		if(state.world.province_get_is_colonial(p)) {
			float divisor = state.defines.pop_size_per_regiment * state.defines.pop_min_size_for_regiment_colony_multiplier;
			float minimum = state.defines.pop_min_size_for_regiment;

			dcon::pop_id non_preferred;
			for(auto pop : state.world.province_get_pop_location(p)) {
				if(pop.get_pop().get_poptype() == state.culture_definitions.soldiers) {
					if(pop.get_pop().get_size() >= minimum) {
						auto amount = int32_t((pop.get_pop().get_size() / divisor) + 1);
						auto regs = pop.get_pop().get_regiment_source();
						auto building = pop.get_pop().get_province_land_construction();

						if(amount > ((regs.end() - regs.begin()) + (building.end() - building.begin()))) {
							if(require_accepted == pop.get_pop().get_is_primary_or_accepted_culture())
								return pop.get_pop().id;
							else
								non_preferred = pop.get_pop().id;
						}
					}
				}
			}
			return non_preferred;
		} else if(!state.world.province_get_is_owner_core(p)) {
			float divisor = state.defines.pop_size_per_regiment * state.defines.pop_min_size_for_regiment_noncore_multiplier;
			float minimum = state.defines.pop_min_size_for_regiment;

			dcon::pop_id non_preferred;
			for(auto pop : state.world.province_get_pop_location(p)) {
				if(pop.get_pop().get_poptype() == state.culture_definitions.soldiers) {
					if(pop.get_pop().get_size() >= minimum) {
						auto amount = int32_t((pop.get_pop().get_size() / divisor) + 1);
						auto regs = pop.get_pop().get_regiment_source();
						auto building = pop.get_pop().get_province_land_construction();

						if(amount > ((regs.end() - regs.begin()) + (building.end() - building.begin()))) {
							if(require_accepted == pop.get_pop().get_is_primary_or_accepted_culture())
								return pop.get_pop().id;
							else
								non_preferred = pop.get_pop().id;
						}
					}
				}
			}
			return non_preferred;
		} else {
			float divisor = state.defines.pop_size_per_regiment;
			float minimum = state.defines.pop_min_size_for_regiment;

			dcon::pop_id non_preferred;
			for(auto pop : state.world.province_get_pop_location(p)) {
				if(pop.get_pop().get_poptype() == state.culture_definitions.soldiers) {
					if(pop.get_pop().get_size() >= minimum) {
						auto amount = int32_t((pop.get_pop().get_size() / divisor) + 1);
						auto regs = pop.get_pop().get_regiment_source();
						auto building = pop.get_pop().get_province_land_construction();

						if(amount > ((regs.end() - regs.begin()) + (building.end() - building.begin()))) {
							if(require_accepted == pop.get_pop().get_is_primary_or_accepted_culture())
								return pop.get_pop().id;
							else
								non_preferred = pop.get_pop().id;
						}
					}
				}
			}
			return non_preferred;
		}
	}

	int32_t mobilized_regiments_possible_from_province(sys::state& state, dcon::province_id p) {
		/*
		Mobilized regiments come only from unoccupied, non-colonial provinces, and they must be the state capital or national capital.
		*/
		auto fp = fatten(state.world, p);
		if(fp.get_is_colonial() || fp.get_nation_from_province_control() != fp.get_nation_from_province_ownership())
			return 0;

		if(fp.get_state_membership().get_capital() != p && fp.get_nation_from_province_ownership().get_capital() != p)
			return 0;

		int32_t total = 0;
		// Mobilization size = national-modifier-to-mobilization-size + technology-modifier-to-mobilization-size
		for(auto pop : state.world.province_get_pop_location(p)) {
			/*
			In those provinces, mobilized regiments come from non-soldier, non-slave, poor-strata pops with a culture that is either
			the primary culture of the nation or an accepted culture.
			*/
			if(pop_eligible_for_mobilization(state, pop.get_pop())) {
				/*
				The number of regiments these pops can provide is determined by pop-size x mobilization-size /
				define:POP_SIZE_PER_REGIMENT.
				*/
				total += mobilized_regiments_possible_from_pop(state, pop.get_pop());
			}
		}
		return total;
	}

	int32_t mobilized_regiments_pop_limit(sys::state& state, dcon::nation_id n) {
		auto real_regs = std::max(state.world.nation_get_active_regiments(n), uint16_t(state.defines.min_mobilize_limit));
		return int32_t(real_regs * state.world.nation_get_modifier_values(n, sys::national_mod_offsets::mobilization_impact));
	}

	void update_recruitable_regiments(sys::state& state, dcon::nation_id n) {
		int32_t count = 0;
		for(auto p : state.world.nation_get_province_ownership(n)) {
			count += regiments_max_possible_from_province(state, p.get_province());
		}
		state.world.nation_set_recruitable_regiments(n, uint16_t(count));
	}
	void update_all_recruitable_regiments(sys::state& state) {
		//state.world.execute_serial_over_nation([&](auto ids) { state.world.nation_set_recruitable_regiments(ids, ve::int_vector(0)); });
		state.world.execute_serial_over_nation([&](auto ids) {
			auto num_regs = ve::apply([&](dcon::nation_id n) {
				int32_t count = 0;
				for(const auto po : state.world.nation_get_province_ownership(n)) {
					count += regiments_max_possible_from_province(state, po.get_province());
				}
				return count;
			}, ids);
			state.world.nation_set_recruitable_regiments(ids, num_regs);
		});
	}
	void regenerate_total_regiment_counts(sys::state& state) {
		//state.world.execute_serial_over_nation([&](auto ids) { state.world.nation_set_active_regiments(ids, ve::int_vector(0)); });
		state.world.execute_serial_over_nation([&](auto ids) {
			auto num_regs = ve::apply([&](dcon::nation_id n) {
				int32_t count = 0;
				for(const auto al : state.world.nation_get_army_control(n)) {
					for(const auto reg : al.get_army().get_army_membership()) {
						if(reg.get_regiment().get_pop_from_regiment_source().get_poptype() == state.culture_definitions.soldiers) {
							++count;
						}
					}
				}
				return count;
			}, ids);
			state.world.nation_set_active_regiments(ids, num_regs);
		});
	}

	void regenerate_land_unit_average(sys::state& state) {
		/*
		We also need to know the average land unit score, which we define here as (attack + defense + national land attack modifier +
		national land defense modifier) x discipline
		*/
		auto const max = state.military_definitions.unit_base_definitions.size();
		state.world.for_each_nation([&](dcon::nation_id n) {
			float total = 0.f;
			float count = 0.f;

			auto lo_mod = state.world.nation_get_modifier_values(n, sys::national_mod_offsets::land_attack_modifier);
			auto ld_mod = state.world.nation_get_modifier_values(n, sys::national_mod_offsets::land_defense_modifier);

			for(uint32_t i = 2; i < max; ++i) {
				dcon::unit_type_id u{ dcon::unit_type_id::value_base_t(i) };
				if((state.world.nation_get_active_unit(n, u) || state.military_definitions.unit_base_definitions[u].active) && state.military_definitions.unit_base_definitions[u].is_land) {
					auto& reg_stats = state.world.nation_get_unit_stats(n, u);
					total += ((reg_stats.defence_or_hull + ld_mod) + (reg_stats.attack_or_gun_power + lo_mod))
						* state.military_definitions.unit_base_definitions[u].discipline_or_evasion;
					++count;
				}
			}
			state.world.nation_set_averge_land_unit_score(n, total / count);
		});
	}
} // naesmapce military

