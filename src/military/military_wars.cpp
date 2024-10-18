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
	bool are_at_war(sys::state const& state, dcon::nation_id a, dcon::nation_id b) {
		for(auto wa : state.world.nation_get_war_participant(a)) {
			auto is_attacker = wa.get_is_attacker();
			for(auto o : wa.get_war().get_war_participant()) {
				if(o.get_nation() == b)
					return o.get_is_attacker() != is_attacker;
			}
		}
		return false;
	}

	bool are_allied_in_war(sys::state const& state, dcon::nation_id a, dcon::nation_id b) {
		for(auto wa : state.world.nation_get_war_participant(a)) {
			auto is_attacker = wa.get_is_attacker();
			for(auto o : wa.get_war().get_war_participant()) {
				if(o.get_nation() == b)
					return o.get_is_attacker() == is_attacker;
			}
		}
		return false;
	}

	bool are_in_common_war(sys::state const& state, dcon::nation_id a, dcon::nation_id b) {
		for(auto wa : state.world.nation_get_war_participant(a)) {
			for(auto o : wa.get_war().get_war_participant()) {
				if(o.get_nation() == b)
					return true;
			}
		}
		return false;
	}

	void remove_from_common_allied_wars(sys::state& state, dcon::nation_id a, dcon::nation_id b) {
		std::vector<dcon::war_id> wars;
		for(auto wa : state.world.nation_get_war_participant(a)) {
			for(auto o : wa.get_war().get_war_participant()) {
				if(o.get_nation() == b && o.get_is_attacker() == wa.get_is_attacker()) {
					wars.push_back(wa.get_war());
				}
			}
		}
		for(const auto w : wars) {
			//military::remove_from_war(state, w, a, false);
			military::remove_from_war(state, w, b, false);
		}
	}

	participation internal_find_war_between(sys::state const& state, dcon::nation_id a, dcon::nation_id b) {
		for(auto wa : state.world.nation_get_war_participant(a)) {
			auto is_attacker = wa.get_is_attacker();
			for(auto o : wa.get_war().get_war_participant()) {
				if(o.get_nation() == b) {
					if(o.get_is_attacker() != is_attacker) {
						return participation{ wa.get_war().id, is_attacker ? war_role::attacker : war_role::defender };
					} else {
						return participation{};
					}
				}
			}
		}
		return participation{};
	}

	dcon::war_id find_war_between(sys::state const& state, dcon::nation_id a, dcon::nation_id b) {
		return internal_find_war_between(state, a, b).w;
	}

	bool joining_war_does_not_violate_constraints(sys::state const& state, dcon::nation_id a, dcon::war_id w, bool as_attacker) {
		auto target_war_participants = state.world.war_get_war_participant(w);
		for(auto wa : state.world.nation_get_war_participant(a)) {
			for(auto other_participants : wa.get_war().get_war_participant()) {
				if(other_participants.get_is_attacker() ==
					wa.get_is_attacker()) { // case: ally on same side -- must not be on opposite site
					for(auto tp : target_war_participants) {
						if(tp.get_nation() == other_participants.get_nation() && tp.get_is_attacker() != as_attacker) {
							return false;
						}
					}
				} else { // case opponent -- must not be in new war at all
					for(auto tp : target_war_participants) {
						if(tp.get_nation() == other_participants.get_nation()) {
							return false;
						}
					}
				}
			}
		}
		return true;
	}

	bool is_civil_war(sys::state const& state, dcon::war_id w) {
		for(auto wg : state.world.war_get_wargoals_attached(w)) {
			if((wg.get_wargoal().get_type().get_type_bits() & cb_flag::is_civil_war) != 0)
				return true;
		}
		return false;
	}

	bool is_defender_wargoal(sys::state const& state, dcon::war_id w, dcon::wargoal_id wg) {
		auto from = state.world.wargoal_get_added_by(wg);
		for(auto p : state.world.war_get_war_participant(w)) {
			if(p.get_nation() == from)
				return !p.get_is_attacker();
		}
		return false;
	}

	bool defenders_have_non_status_quo_wargoal(sys::state const& state, dcon::war_id w) {
		for(auto wg : state.world.war_get_wargoals_attached(w)) {
			if((wg.get_wargoal().get_type().get_type_bits() & cb_flag::po_status_quo) == 0 && is_defender_wargoal(state, w, wg.get_wargoal()))
				return true;
		}
		return false;
	}

	bool defenders_have_status_quo_wargoal(sys::state const& state, dcon::war_id w) {
		for(auto wg : state.world.war_get_wargoals_attached(w)) {
			if((wg.get_wargoal().get_type().get_type_bits() & cb_flag::po_status_quo) != 0 && is_defender_wargoal(state, w, wg.get_wargoal()))
				return true;
		}
		return false;
	}
	bool attackers_have_status_quo_wargoal(sys::state const& state, dcon::war_id w) {
		for(auto wg : state.world.war_get_wargoals_attached(w)) {
			if((wg.get_wargoal().get_type().get_type_bits() & cb_flag::po_status_quo) != 0 && !is_defender_wargoal(state, w, wg.get_wargoal()))
				return true;
		}
		return false;
	}

	bool joining_as_attacker_would_break_truce(sys::state& state, dcon::nation_id a, dcon::war_id w) {
		for(auto p : state.world.war_get_war_participant(w)) {
			if(p.get_is_attacker() == false) {
				if(has_truce_with(state, a, p.get_nation())) {
					return true;
				}
			}
		}
		return false;
	}
} // naesmapce military
