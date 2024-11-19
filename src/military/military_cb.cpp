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
#include "military.hpp"

namespace military {
	bool can_use_cb_against(sys::state& state, dcon::nation_id from, dcon::nation_id target) {
		auto other_cbs = state.world.nation_get_available_cbs(from);
		for(auto const& cb : other_cbs) {
			if(cb.target == target && cb_conditions_satisfied(state, from, target, cb.cb_type)) {
				return true;
			}
		}
		for(auto cb : state.world.in_cb_type) {
			if((cb.get_type_bits() & military::cb_flag::always) != 0 && cb_conditions_satisfied(state, from, target, cb)) {
				return true;
			}
		}
		return false;
	}

	bool can_add_always_cb_to_war(sys::state& state, dcon::nation_id actor, dcon::nation_id target, dcon::cb_type_id cb, dcon::war_id w) {
		auto can_use = state.world.cb_type_get_can_use(cb);
		if(can_use && !trigger::evaluate(state, can_use, trigger::to_generic(target), trigger::to_generic(actor), trigger::to_generic(target))) {
			return false;
		}
		auto allowed_countries = state.world.cb_type_get_allowed_countries(cb);
		auto allowed_states = state.world.cb_type_get_allowed_states(cb);
		bool for_all_state = (state.world.cb_type_get_type_bits(cb) & military::cb_flag::all_allowed_states) != 0;
		if(!allowed_countries && allowed_states) {
			if(for_all_state) {
				for(auto wg : state.world.war_get_wargoals_attached(w)) {
					if(wg.get_wargoal().get_added_by() == actor
					&& wg.get_wargoal().get_type() == cb
					&& wg.get_wargoal().get_target_nation() == target) {
						return false;
					}
				}
			}
			bool any_allowed = false;
			for(auto si : state.world.nation_get_state_ownership(target)) {
				if(trigger::evaluate(state, allowed_states, trigger::to_generic(si.get_state().id), trigger::to_generic(actor), trigger::to_generic(actor))) {
					any_allowed = true;
					for(auto wg : state.world.war_get_wargoals_attached(w)) {
						if(wg.get_wargoal().get_added_by() == actor
						&& wg.get_wargoal().get_type() == cb
						&& wg.get_wargoal().get_associated_state() == si.get_state().get_definition()
						&& wg.get_wargoal().get_target_nation() == target) {
							any_allowed = false;
							break;
						}
					}
					if(any_allowed)
						break;
				}
			}
			if(!any_allowed)
				return false;
		}

		auto allowed_substates = state.world.cb_type_get_allowed_substate_regions(cb);
		if(allowed_substates) {
			bool any_allowed = [&]() {
				for(auto v : state.world.nation_get_overlord_as_ruler(target)) {
					if(v.get_subject().get_is_substate()) {
						for(auto si : state.world.nation_get_state_ownership(target)) {
							if(trigger::evaluate(state, allowed_substates, trigger::to_generic(si.get_state().id), trigger::to_generic(actor), trigger::to_generic(actor))) {
								return true;
							}
						}
					}
				}
				return false;
			}();
			if(!any_allowed) {
				return false;
			}
		}

		if(allowed_countries) {
			bool any_allowed = [&]() {
				for(auto n : state.world.in_nation) {
					if(n != actor) {
						if(trigger::evaluate(state, allowed_countries, trigger::to_generic(target), trigger::to_generic(actor), trigger::to_generic(n.id))) {
							if(allowed_states) { // check whether any state within the target is valid for free / liberate
								bool found_dup = false;
								if(for_all_state) {
									for(auto wg : state.world.war_get_wargoals_attached(w)) {
										if(wg.get_wargoal().get_added_by() == actor
										&& wg.get_wargoal().get_type() == cb
										&& (wg.get_wargoal().get_associated_tag() == n.get_identity_from_identity_holder()
											|| wg.get_wargoal().get_secondary_nation() == n)
										&& wg.get_wargoal().get_target_nation() == target) {

											found_dup = true;
										}
									}
								}

								if(!found_dup) {
									for(auto si : state.world.nation_get_state_ownership(target)) {
										if(trigger::evaluate(state, allowed_states, trigger::to_generic(si.get_state().id), trigger::to_generic(actor), trigger::to_generic(n.id))) {
											for(auto wg : state.world.war_get_wargoals_attached(w)) {
												if(wg.get_wargoal().get_added_by() == actor
												&& wg.get_wargoal().get_type() == cb
												&& (wg.get_wargoal().get_associated_tag() == n.get_identity_from_identity_holder()
												|| wg.get_wargoal().get_secondary_nation() == n)
												&& wg.get_wargoal().get_associated_state() == si.get_state().get_definition()
												&& wg.get_wargoal().get_target_nation() == target) {
													found_dup = true;
													break;
												}
											}
										}
									}
								}
								if(!found_dup) {
									return true;
								}
							} else { // no allowed states trigger -> nation is automatically a valid target
								bool found_dup = false;
								for(auto wg : state.world.war_get_wargoals_attached(w)) {
									if(wg.get_wargoal().get_added_by() == actor
									&& wg.get_wargoal().get_type() == cb
									&& (wg.get_wargoal().get_associated_tag() == n.get_identity_from_identity_holder()
									|| wg.get_wargoal().get_secondary_nation() == n)
									&& wg.get_wargoal().get_target_nation() == target) {
										found_dup = true;
									}
								}
								if(!found_dup) {
									return true;
								}
							}
						}
					}
				}
				return false;
			}();
			if(!any_allowed) {
				return false;
			}
		}
		return true;

	}

	bool cb_conditions_satisfied(sys::state& state, dcon::nation_id actor, dcon::nation_id target, dcon::cb_type_id cb) {
		auto can_use = state.world.cb_type_get_can_use(cb);
		if(can_use && !trigger::evaluate(state, can_use, trigger::to_generic(target), trigger::to_generic(actor), trigger::to_generic(target))) {
			return false;
		}

		auto allowed_countries = state.world.cb_type_get_allowed_countries(cb);
		auto allowed_states = state.world.cb_type_get_allowed_states(cb);

		if(!allowed_countries && allowed_states) {
			bool any_allowed = false;
			for(auto si : state.world.nation_get_state_ownership(target)) {
				if(trigger::evaluate(state, allowed_states, trigger::to_generic(si.get_state().id), trigger::to_generic(actor), trigger::to_generic(actor))) {
					any_allowed = true;
					break;
				}
			}
			if(!any_allowed)
				return false;
		}

		auto allowed_substates = state.world.cb_type_get_allowed_substate_regions(cb);
		if(allowed_substates) {
			bool any_allowed = [&]() {
				for(auto v : state.world.nation_get_overlord_as_ruler(target)) {
					if(v.get_subject().get_is_substate()) {
						for(auto si : state.world.nation_get_state_ownership(target)) {
							if(trigger::evaluate(state, allowed_substates, trigger::to_generic(si.get_state().id), trigger::to_generic(actor),
								trigger::to_generic(actor))) {
								return true;
							}
						}
					}
				}
				return false;
				}();
				if(!any_allowed)
					return false;
		}

		if(allowed_countries) {
			bool any_allowed = [&]() {
				for(auto n : state.world.in_nation) {
					if(n != actor) {
						if(trigger::evaluate(state, allowed_countries, trigger::to_generic(target), trigger::to_generic(actor), trigger::to_generic(n.id))) {
							if(allowed_states) { // check whether any state within the target is valid for free / liberate
								for(auto si : state.world.nation_get_state_ownership(target)) {
									if(trigger::evaluate(state, allowed_states, trigger::to_generic(si.get_state().id), trigger::to_generic(actor),
										trigger::to_generic(n.id))) {
										return true;
									}
								}
							} else { // no allowed states trigger -> nation is automatically a valid target
								return true;
							}
						}
					}
				}
				return false;
				}();
				if(!any_allowed)
					return false;
		}

		return true;
	}

	bool cb_instance_conditions_satisfied(sys::state& state, dcon::nation_id actor, dcon::nation_id target, dcon::cb_type_id cb,
		dcon::state_definition_id st, dcon::national_identity_id tag, dcon::nation_id secondary) {

		auto can_use = state.world.cb_type_get_can_use(cb);
		if(can_use && !trigger::evaluate(state, can_use, trigger::to_generic(target), trigger::to_generic(actor), trigger::to_generic(target))) {
			return false;
		}

		auto allowed_countries = state.world.cb_type_get_allowed_countries(cb);
		auto allowed_states = state.world.cb_type_get_allowed_states(cb);

		if(!allowed_countries && allowed_states) {
			bool any_allowed = false;
			if(cb_requires_selection_of_a_state(state, cb)) {
				for(auto si : state.world.nation_get_state_ownership(target)) {
					if(si.get_state().get_definition() == st &&
						trigger::evaluate(state, allowed_states, trigger::to_generic(si.get_state().id), trigger::to_generic(actor),
							trigger::to_generic(actor))) {
						any_allowed = true;
						break;
					}
				}
			} else {
				for(auto si : state.world.nation_get_state_ownership(target)) {
					if(trigger::evaluate(state, allowed_states, trigger::to_generic(si.get_state().id), trigger::to_generic(actor),
						trigger::to_generic(actor))) {
						any_allowed = true;
						break;
					}
				}
			}
			if(!any_allowed)
				return false;
		}

		auto allowed_substates = state.world.cb_type_get_allowed_substate_regions(cb);
		if(allowed_substates) {
			bool any_allowed = [&]() {
				for(auto v : state.world.nation_get_overlord_as_ruler(target)) {
					if(v.get_subject().get_is_substate()) {
						if(cb_requires_selection_of_a_state(state, cb)) {
							for(auto si : state.world.nation_get_state_ownership(target)) {
								if(si.get_state().get_definition() == st &&
									trigger::evaluate(state, allowed_substates, trigger::to_generic(si.get_state().id), trigger::to_generic(actor),
										trigger::to_generic(actor))) {
									return true;
								}
							}
						} else {
							for(auto si : state.world.nation_get_state_ownership(target)) {
								if(trigger::evaluate(state, allowed_substates, trigger::to_generic(si.get_state().id), trigger::to_generic(actor),
									trigger::to_generic(actor))) {
									return true;
								}
							}
						}
					}
				}
				return false;
				}();
				if(!any_allowed)
					return false;
		}

		if(allowed_countries) {
			auto secondary_nation = secondary ? secondary : state.world.national_identity_get_nation_from_identity_holder(tag);

			if(secondary_nation != actor && trigger::evaluate(state, allowed_countries, trigger::to_generic(target), trigger::to_generic(actor), trigger::to_generic(secondary_nation))) {
				bool validity = false;
				if(allowed_states) { // check whether any state within the target is valid for free / liberate
					if((state.world.cb_type_get_type_bits(cb) & cb_flag::all_allowed_states) != 0) {
						for(auto si : state.world.nation_get_state_ownership(target)) {
							if(trigger::evaluate(state, allowed_states, trigger::to_generic(si.get_state().id), trigger::to_generic(actor),
								trigger::to_generic(secondary_nation))) {
								validity = true;
								break;
							}
						}
					} else {
						for(auto si : state.world.nation_get_state_ownership(target)) {
							if(si.get_state().get_definition() == st &&
								trigger::evaluate(state, allowed_states, trigger::to_generic(si.get_state().id), trigger::to_generic(actor),
									trigger::to_generic(secondary_nation))) {
								validity = true;
								break;
							}
						}
					}
				} else { // no allowed states trigger -> nation is automatically a valid target
					validity = true;
				}
				if(!validity)
					return false;
			} else {
				return false;
			}
		}

		return true;
	}

	void update_cbs(sys::state& state) {
		for(auto n : state.world.in_nation) {
			if(state.current_date == n.get_reparations_until()) {
				for(auto urel : n.get_unilateral_relationship_as_source()) {
					urel.set_reparations(false);
				}
			}
			auto current_cbs = n.get_available_cbs();
			for(uint32_t i = current_cbs.size(); i-- > 0;) {
				if(current_cbs[i].expiration && current_cbs[i].expiration <= state.current_date) {
					current_cbs.remove_at(i);
				}
			}

			// check for cancellation
			if(n.get_constructing_cb_type()) {
				/*
				CBs that become invalid (the nations involved no longer satisfy the conditions or enter into a war with each other)
				are canceled (and the player should be notified in this event).
				*/
				auto target = n.get_constructing_cb_target();
				if(military::are_at_war(state, n, target) || state.world.nation_get_owned_province_count(target) == 0 ||
					!cb_conditions_satisfied(state, n, target, n.get_constructing_cb_type())) {
					notification::post(state, notification::message{
						[](sys::state& state, text::layout_base& contents) {
							text::add_line(state, contents, "msg_fab_canceled_1");
						},
						"msg_fab_canceled_title",
						n, dcon::nation_id{}, dcon::nation_id{},
						sys::message_base_type::cb_fab_cancelled
					});
					n.set_constructing_cb_is_discovered(false);
					n.set_constructing_cb_progress(0.0f);
					n.set_constructing_cb_target(dcon::nation_id{});
					n.set_constructing_cb_type(dcon::cb_type_id{});
				}
			}

			if(n.get_constructing_cb_type() && !nations::is_involved_in_crisis(state, n)) {
				/*
				CB fabrication by a nation is paused while that nation is in a crisis (nor do events related to CB fabrication
				happen). CB fabrication is advanced by points equal to: define:CB_GENERATION_BASE_SPEED x cb-type-construction-speed x
				(national-cb-construction-speed-modifiers + technology-cb-construction-speed-modifier + 1).
				*/

				auto eff_speed = state.defines.cb_generation_base_speed * n.get_constructing_cb_type().get_construction_speed() * (n.get_modifier_values(sys::national_mod_offsets::cb_generation_speed_modifier) + 1.0f);

				n.get_constructing_cb_progress() += std::max(eff_speed, 0.0f);

				/*
				Each day, a fabricating CB has a define:CB_DETECTION_CHANCE_BASE out of 1000 chance to be detected. If discovered, the
				fabricating country gains the infamy for that war goal x the fraction of fabrication remaining. If discovered
				relations between the two nations are changed by define:ON_CB_DETECTED_RELATION_CHANGE. If discovered, any states with
				a flashpoint in the target nation will have their tension increase by define:TENSION_ON_CB_DISCOVERED
				*/
				if(!n.get_constructing_cb_is_discovered() && eff_speed > 0.0f) {
					auto val = rng::get_random(state, uint32_t((n.id.index() << 3) + 5)) % 1000;
					if(val <= uint32_t(state.defines.cb_detection_chance_base)) {
						execute_cb_discovery(state, n);
						n.set_constructing_cb_is_discovered(true);
					}
				}

				/*
				When fabrication progress reaches 100, the CB will remain valid for define:CREATED_CB_VALID_TIME months (so x30 days
				for us). Note that pending CBs have their target nation fixed, but all other parameters are flexible.
				*/
				if(n.get_constructing_cb_progress() >= 100.0f) {
					add_cb(state, n, n.get_constructing_cb_type(), n.get_constructing_cb_target());

					if(n == state.local_player_nation) {
						notification::post(state, notification::message{
							[t = n.get_constructing_cb_target(), c = n.get_constructing_cb_type()](sys::state& state, text::layout_base& contents) {
								text::add_line(state, contents, "msg_fab_finished_1", text::variable_type::x, state.world.cb_type_get_name(c), text::variable_type::y, t);
							},
							"msg_fab_finished_title",
						n, dcon::nation_id{}, dcon::nation_id{},
							sys::message_base_type::cb_fab_finished
						});
					}

					n.set_constructing_cb_is_discovered(false);
					n.set_constructing_cb_progress(0.0f);
					n.set_constructing_cb_target(dcon::nation_id{});
					n.set_constructing_cb_type(dcon::cb_type_id{});
				}
			}
		}
	}

	void add_cb(sys::state& state, dcon::nation_id n, dcon::cb_type_id cb, dcon::nation_id target) {
		auto current_cbs = state.world.nation_get_available_cbs(n);
		current_cbs.push_back(military::available_cb{ state.current_date + int32_t(state.defines.created_cb_valid_time) * 30, target, cb });
	}

	float cb_infamy(sys::state const& state, dcon::cb_type_id t) {
		float total = 0.0f;
		auto bits = state.world.cb_type_get_type_bits(t);

		if((bits & cb_flag::po_clear_union_sphere) != 0) {
			total += state.defines.infamy_clear_union_sphere;
		}
		if((bits & cb_flag::po_gunboat) != 0) {
			total += state.defines.infamy_gunboat;
		}
		if((bits & cb_flag::po_annex) != 0) {
			total += state.defines.infamy_annex;
		}
		if((bits & cb_flag::po_demand_state) != 0) {
			total += state.defines.infamy_demand_state;
		}
		if((bits & cb_flag::po_add_to_sphere) != 0) {
			total += state.defines.infamy_add_to_sphere;
		}
		if((bits & cb_flag::po_disarmament) != 0) {
			total += state.defines.infamy_disarmament;
		}
		if((bits & cb_flag::po_reparations) != 0) {
			total += state.defines.infamy_reparations;
		}
		if((bits & cb_flag::po_transfer_provinces) != 0) {
			total += state.defines.infamy_transfer_provinces;
		}
		if((bits & cb_flag::po_remove_prestige) != 0) {
			total += state.defines.infamy_prestige;
		}
		if((bits & cb_flag::po_make_puppet) != 0) {
			total += state.defines.infamy_make_puppet;
		}
		if((bits & cb_flag::po_release_puppet) != 0) {
			total += state.defines.infamy_release_puppet;
		}
		if((bits & cb_flag::po_status_quo) != 0) {
			total += state.defines.infamy_status_quo;
		}
		if((bits & cb_flag::po_install_communist_gov_type) != 0) {
			total += state.defines.infamy_install_communist_gov_type;
		}
		if((bits & cb_flag::po_uninstall_communist_gov_type) != 0) {
			total += state.defines.infamy_uninstall_communist_gov_type;
		}
		if((bits & cb_flag::po_remove_cores) != 0) {
			total += state.defines.infamy_remove_cores;
		}
		if((bits & cb_flag::po_colony) != 0) {
			total += state.defines.infamy_colony;
		}
		if((bits & cb_flag::po_destroy_forts) != 0) {
			total += state.defines.infamy_destroy_forts;
		}
		if((bits & cb_flag::po_destroy_naval_bases) != 0) {
			total += state.defines.infamy_destroy_naval_bases;
		}

		return total * state.world.cb_type_get_badboy_factor(t);
	}

	float truce_break_cb_prestige_cost(sys::state& state, dcon::cb_type_id t) {
		float total = 0.0f;
		auto bits = state.world.cb_type_get_type_bits(t);

		if((bits & cb_flag::po_clear_union_sphere) != 0) {
			total += state.defines.breaktruce_prestige_clear_union_sphere;
		}
		if((bits & cb_flag::po_gunboat) != 0) {
			total += state.defines.breaktruce_prestige_gunboat;
		}
		if((bits & cb_flag::po_annex) != 0) {
			total += state.defines.breaktruce_prestige_annex;
		}
		if((bits & cb_flag::po_demand_state) != 0) {
			total += state.defines.breaktruce_prestige_demand_state;
		}
		if((bits & cb_flag::po_add_to_sphere) != 0) {
			total += state.defines.breaktruce_prestige_add_to_sphere;
		}
		if((bits & cb_flag::po_disarmament) != 0) {
			total += state.defines.breaktruce_prestige_disarmament;
		}
		if((bits & cb_flag::po_reparations) != 0) {
			total += state.defines.breaktruce_prestige_reparations;
		}
		if((bits & cb_flag::po_transfer_provinces) != 0) {
			total += state.defines.breaktruce_prestige_transfer_provinces;
		}
		if((bits & cb_flag::po_remove_prestige) != 0) {
			total += state.defines.breaktruce_prestige_prestige;
		}
		if((bits & cb_flag::po_make_puppet) != 0) {
			total += state.defines.breaktruce_prestige_make_puppet;
		}
		if((bits & cb_flag::po_release_puppet) != 0) {
			total += state.defines.breaktruce_prestige_release_puppet;
		}
		if((bits & cb_flag::po_status_quo) != 0) {
			total += state.defines.breaktruce_prestige_status_quo;
		}
		if((bits & cb_flag::po_install_communist_gov_type) != 0) {
			total += state.defines.breaktruce_prestige_install_communist_gov_type;
		}
		if((bits & cb_flag::po_uninstall_communist_gov_type) != 0) {
			total += state.defines.breaktruce_prestige_uninstall_communist_gov_type;
		}
		if((bits & cb_flag::po_remove_cores) != 0) {
			total += state.defines.breaktruce_prestige_remove_cores;
		}
		if((bits & cb_flag::po_colony) != 0) {
			total += state.defines.breaktruce_prestige_colony;
		}
		if((bits & cb_flag::po_destroy_forts) != 0) {
			total += state.defines.breaktruce_prestige_destroy_forts;
		}
		if((bits & cb_flag::po_destroy_naval_bases) != 0) {
			total += state.defines.breaktruce_prestige_destroy_naval_bases;
		}

		return total * state.world.cb_type_get_break_truce_prestige_factor(t);
	}
	float truce_break_cb_militancy(sys::state& state, dcon::cb_type_id t) {
		float total = 0.0f;
		auto bits = state.world.cb_type_get_type_bits(t);

		if((bits & cb_flag::po_clear_union_sphere) != 0) {
			total += state.defines.breaktruce_militancy_clear_union_sphere;
		}
		if((bits & cb_flag::po_gunboat) != 0) {
			total += state.defines.breaktruce_militancy_gunboat;
		}
		if((bits & cb_flag::po_annex) != 0) {
			total += state.defines.breaktruce_militancy_annex;
		}
		if((bits & cb_flag::po_demand_state) != 0) {
			total += state.defines.breaktruce_militancy_demand_state;
		}
		if((bits & cb_flag::po_add_to_sphere) != 0) {
			total += state.defines.breaktruce_militancy_add_to_sphere;
		}
		if((bits & cb_flag::po_disarmament) != 0) {
			total += state.defines.breaktruce_militancy_disarmament;
		}
		if((bits & cb_flag::po_reparations) != 0) {
			total += state.defines.breaktruce_militancy_reparations;
		}
		if((bits & cb_flag::po_transfer_provinces) != 0) {
			total += state.defines.breaktruce_militancy_transfer_provinces;
		}
		if((bits & cb_flag::po_remove_prestige) != 0) {
			total += state.defines.breaktruce_militancy_prestige;
		}
		if((bits & cb_flag::po_make_puppet) != 0) {
			total += state.defines.breaktruce_militancy_make_puppet;
		}
		if((bits & cb_flag::po_release_puppet) != 0) {
			total += state.defines.breaktruce_militancy_release_puppet;
		}
		if((bits & cb_flag::po_status_quo) != 0) {
			total += state.defines.breaktruce_militancy_status_quo;
		}
		if((bits & cb_flag::po_install_communist_gov_type) != 0) {
			total += state.defines.breaktruce_militancy_install_communist_gov_type;
		}
		if((bits & cb_flag::po_uninstall_communist_gov_type) != 0) {
			total += state.defines.breaktruce_militancy_uninstall_communist_gov_type;
		}
		if((bits & cb_flag::po_remove_cores) != 0) {
			total += state.defines.breaktruce_militancy_remove_cores;
		}
		if((bits & cb_flag::po_colony) != 0) {
			total += state.defines.breaktruce_militancy_colony;
		}
		if((bits & cb_flag::po_destroy_forts) != 0) {
			total += state.defines.breaktruce_militancy_destroy_forts;
		}
		if((bits & cb_flag::po_destroy_naval_bases) != 0) {
			total += state.defines.breaktruce_militancy_destroy_naval_bases;
		}

		return total * state.world.cb_type_get_break_truce_militancy_factor(t);
	}
	float truce_break_cb_infamy(sys::state& state, dcon::cb_type_id t) {
		float total = 0.0f;
		auto bits = state.world.cb_type_get_type_bits(t);

		if((bits & cb_flag::po_clear_union_sphere) != 0) {
			total += state.defines.breaktruce_infamy_clear_union_sphere;
		}
		if((bits & cb_flag::po_gunboat) != 0) {
			total += state.defines.breaktruce_infamy_gunboat;
		}
		if((bits & cb_flag::po_annex) != 0) {
			total += state.defines.breaktruce_infamy_annex;
		}
		if((bits & cb_flag::po_demand_state) != 0) {
			total += state.defines.breaktruce_infamy_demand_state;
		}
		if((bits & cb_flag::po_add_to_sphere) != 0) {
			total += state.defines.breaktruce_infamy_add_to_sphere;
		}
		if((bits & cb_flag::po_disarmament) != 0) {
			total += state.defines.breaktruce_infamy_disarmament;
		}
		if((bits & cb_flag::po_reparations) != 0) {
			total += state.defines.breaktruce_infamy_reparations;
		}
		if((bits & cb_flag::po_transfer_provinces) != 0) {
			total += state.defines.breaktruce_infamy_transfer_provinces;
		}
		if((bits & cb_flag::po_remove_prestige) != 0) {
			total += state.defines.breaktruce_infamy_prestige;
		}
		if((bits & cb_flag::po_make_puppet) != 0) {
			total += state.defines.breaktruce_infamy_make_puppet;
		}
		if((bits & cb_flag::po_release_puppet) != 0) {
			total += state.defines.breaktruce_infamy_release_puppet;
		}
		if((bits & cb_flag::po_status_quo) != 0) {
			total += state.defines.breaktruce_infamy_status_quo;
		}
		if((bits & cb_flag::po_install_communist_gov_type) != 0) {
			total += state.defines.breaktruce_infamy_install_communist_gov_type;
		}
		if((bits & cb_flag::po_uninstall_communist_gov_type) != 0) {
			total += state.defines.breaktruce_infamy_uninstall_communist_gov_type;
		}
		if((bits & cb_flag::po_remove_cores) != 0) {
			total += state.defines.breaktruce_infamy_remove_cores;
		}
		if((bits & cb_flag::po_colony) != 0) {
			total += state.defines.breaktruce_infamy_colony;
		}
		if((bits & cb_flag::po_destroy_forts) != 0) {
			total += state.defines.breaktruce_infamy_destroy_forts;
		}
		if((bits & cb_flag::po_destroy_naval_bases) != 0) {
			total += state.defines.breaktruce_infamy_destroy_naval_bases;
		}

		return total * state.world.cb_type_get_break_truce_infamy_factor(t);
	}

	float successful_cb_prestige(sys::state& state, dcon::cb_type_id t, dcon::nation_id actor) {
		float total = 0.0f;
		auto bits = state.world.cb_type_get_type_bits(t);
		float actor_prestige = nations::prestige_score(state, actor);

		if((bits & cb_flag::po_clear_union_sphere) != 0) {
			total += std::max(state.defines.prestige_clear_union_sphere * actor_prestige, state.defines.prestige_clear_union_sphere_base);
		}
		if((bits & cb_flag::po_gunboat) != 0) {
			total += std::max(state.defines.prestige_gunboat * actor_prestige, state.defines.prestige_gunboat_base);
		}
		if((bits & cb_flag::po_annex) != 0) {
			total += std::max(state.defines.prestige_annex * actor_prestige, state.defines.prestige_annex_base);
		}
		if((bits & cb_flag::po_demand_state) != 0) {
			total += std::max(state.defines.prestige_demand_state * actor_prestige, state.defines.prestige_demand_state_base);
		}
		if((bits & cb_flag::po_add_to_sphere) != 0) {
			total += std::max(state.defines.prestige_add_to_sphere * actor_prestige, state.defines.prestige_add_to_sphere_base);
		}
		if((bits & cb_flag::po_disarmament) != 0) {
			total += std::max(state.defines.prestige_disarmament * actor_prestige, state.defines.prestige_disarmament_base);
		}
		if((bits & cb_flag::po_reparations) != 0) {
			total += std::max(state.defines.prestige_reparations * actor_prestige, state.defines.prestige_reparations_base);
		}
		if((bits & cb_flag::po_transfer_provinces) != 0) {
			total += std::max(state.defines.prestige_transfer_provinces * actor_prestige, state.defines.prestige_transfer_provinces_base);
		}
		if((bits & cb_flag::po_remove_prestige) != 0) {
			total += std::max(state.defines.prestige_prestige * actor_prestige, state.defines.prestige_prestige_base);
		}
		if((bits & cb_flag::po_make_puppet) != 0) {
			total += std::max(state.defines.prestige_make_puppet * actor_prestige, state.defines.prestige_make_puppet_base);
		}
		if((bits & cb_flag::po_release_puppet) != 0) {
			total += std::max(state.defines.prestige_release_puppet * actor_prestige, state.defines.prestige_release_puppet_base);
		}
		if((bits & cb_flag::po_status_quo) != 0) {
			total += std::max(state.defines.prestige_status_quo * actor_prestige, state.defines.prestige_status_quo_base);
		}
		if((bits & cb_flag::po_install_communist_gov_type) != 0) {
			total += std::max(state.defines.prestige_install_communist_gov_type * actor_prestige, state.defines.prestige_install_communist_gov_type_base);
		}
		if((bits & cb_flag::po_uninstall_communist_gov_type) != 0) {
			total += std::max(state.defines.prestige_uninstall_communist_gov_type * actor_prestige, state.defines.prestige_uninstall_communist_gov_type_base);
		}
		if((bits & cb_flag::po_remove_cores) != 0) {
			total += std::max(state.defines.prestige_remove_cores * actor_prestige, state.defines.prestige_remove_cores_base);
		}
		if((bits & cb_flag::po_colony) != 0) {
			total += std::max(state.defines.prestige_colony * actor_prestige, state.defines.prestige_colony_base);
		}
		if((bits & cb_flag::po_destroy_forts) != 0) {
			total += std::max(state.defines.prestige_destroy_forts * actor_prestige, state.defines.prestige_destroy_forts_base);
		}
		if((bits & cb_flag::po_destroy_naval_bases) != 0) {
			total += std::max(state.defines.prestige_destroy_naval_bases * actor_prestige, state.defines.prestige_destroy_naval_bases_base);
		}
		return total * state.world.cb_type_get_prestige_factor(t);
	}

	float crisis_cb_addition_infamy_cost(sys::state& state, dcon::cb_type_id type, dcon::nation_id from, dcon::nation_id target) {
		if((state.world.cb_type_get_type_bits(type) & (military::cb_flag::always | military::cb_flag::is_not_constructing_cb)) != 0) {
			// not a constructible CB
			return 0.0f;
		}
		return cb_infamy(state, type);
	}

	float cb_addition_infamy_cost(sys::state& state, dcon::war_id war, dcon::cb_type_id type, dcon::nation_id from,
		dcon::nation_id target) {
		if((state.world.cb_type_get_type_bits(type) & (military::cb_flag::always | military::cb_flag::is_not_constructing_cb)) != 0) {
			// not a constructible CB
			return 0.0f;
		}
		auto other_cbs = state.world.nation_get_available_cbs(from);
		for(auto& cb : other_cbs) {
			if(cb.target == target && cb.cb_type == type && cb_conditions_satisfied(state, from, target, cb.cb_type)) {
				return 0.0f;
			}
		}
		if(state.world.war_get_is_great(war)) {
			return cb_infamy(state, type) * state.defines.gw_justify_cb_badboy_impact;
		}
		return cb_infamy(state, type);
	}

	bool cb_requires_selection_of_a_valid_nation(sys::state const& state, dcon::cb_type_id t) {
		auto allowed_nation = state.world.cb_type_get_allowed_countries(t);
		return bool(allowed_nation);
	}
	bool cb_requires_selection_of_a_liberatable_tag(sys::state const& state, dcon::cb_type_id t) {
		auto bits = state.world.cb_type_get_type_bits(t);
		return (bits & (cb_flag::po_transfer_provinces)) != 0;
	}
	bool cb_requires_selection_of_a_state(sys::state const& state, dcon::cb_type_id t) {
		auto bits = state.world.cb_type_get_type_bits(t);
		return (bits & (cb_flag::po_demand_state | cb_flag::po_transfer_provinces | cb_flag::po_destroy_naval_bases | cb_flag::po_destroy_forts)) != 0
			&& (bits & cb_flag::all_allowed_states) == 0;
	}

	/*	If discovered, the fabricating country gains the infamy for that war goal x the fraction of fabrication remaining. If
		discovered relations between the two nations are changed by define:ON_CB_DETECTED_RELATION_CHANGE. If discovered, any states
		with a flashpoint in the target nation will have their tension increase by define:TENSION_ON_CB_DISCOVERED */
	void execute_cb_discovery(sys::state& state, dcon::nation_id n) {
		auto infamy = cb_infamy(state, state.world.nation_get_constructing_cb_type(n));
		auto adj_infamy = std::max(0.0f, ((100.0f - state.world.nation_get_constructing_cb_progress(n)) / 100.0f) * infamy);
		state.world.nation_get_infamy(n) += adj_infamy;
		auto target = state.world.nation_get_constructing_cb_target(n);
		nations::adjust_relationship(state, n, target, state.defines.on_cb_detected_relation_change);
		for(auto si : state.world.nation_get_state_ownership(target)) {
			if(si.get_state().get_flashpoint_tag()) {
				si.get_state().set_flashpoint_tension(std::min(100.0f, si.get_state().get_flashpoint_tension() + state.defines.tension_on_cb_discovered));
			}
		}
		notification::post(state, notification::message{
			[n, target, adj_infamy](sys::state& state, text::layout_base& contents) {
				if(n == state.local_player_nation) {
					text::add_line(state, contents, "msg_fab_discovered_1", text::variable_type::x, text::fp_one_place{ adj_infamy });
				} else {
					text::add_line(state, contents, "msg_fab_discovered_2", text::variable_type::x, n, text::variable_type::y, target);
				}
			},
			"msg_fab_discovered_title",
			n, target, dcon::nation_id{},
			sys::message_base_type::cb_detected
		});
	}
} // naesmapce military
