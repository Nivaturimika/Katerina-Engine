#include "system_state.hpp"
#include "unit_tooltip.hpp"
#include "ai.hpp"
#include "rebels.hpp"

namespace ui {

	unitamounts calc_amounts_from_army(sys::state& state, dcon::army_fat_id army) {
		unitamounts amounts;
		for(auto n : army.get_army_membership()) {
			dcon::unit_type_id utid = n.get_regiment().get_type();
			auto result = state.military_definitions.unit_base_definitions[utid].type;
			if(result == military::unit_type::infantry) {
				amounts.type1++;
			} else if(result == military::unit_type::cavalry) {
				amounts.type2++;
			} else if(result == military::unit_type::support || result == military::unit_type::special) {
				amounts.type3++;
			}
		}
		return amounts;
	}

	unitamounts calc_amounts_from_navy(sys::state& state, dcon::navy_fat_id navy) {
		unitamounts amounts;
		for(auto n : navy.get_navy_membership()) {
			dcon::unit_type_id utid = n.get_ship().get_type();
			auto result = state.military_definitions.unit_base_definitions[utid].type;
			if(result == military::unit_type::big_ship) {
				amounts.type1++;
			} else if(result == military::unit_type::light_ship) {
				amounts.type2++;
			} else if(result == military::unit_type::transport) {
				amounts.type3++;
			}
		}
		return amounts;
	}

	void single_unit_tooltip(sys::state& state, text::columnar_layout& contents, dcon::army_id a) {
		auto army = dcon::fatten(state.world, a);
		unitamounts amounts = calc_amounts_from_army(state, army);

		auto controller = army.get_controller_from_army_control();
		if(!controller) {
			controller = dcon::fatten(state.world, state.world.national_identity_get_nation_from_identity_holder(state.national_definitions.rebel_id));
		}

		float total_cost = 0.f;
		for(const auto memb : army.get_army_membership()) {
			auto type = state.world.regiment_get_type(memb.get_regiment());
			float admin_eff = state.world.nation_get_administrative_efficiency(controller);
			float admin_cost_factor = 2.0f - admin_eff;
			auto o_sc_mod = std::max(0.01f, state.world.nation_get_modifier_values(controller, sys::national_mod_offsets::supply_consumption) + 1.0f);
			auto& supply_cost = state.military_definitions.unit_base_definitions[type].supply_cost;
			for(uint32_t i = 0; i < economy::commodity_set::set_size; ++i) {
				if(supply_cost.commodity_type[i]) {
					float cost = state.world.commodity_get_cost(supply_cost.commodity_type[i]);
					float amount = supply_cost.commodity_amounts[i] * state.world.nation_get_unit_stats(controller, type).supply_consumption * o_sc_mod * admin_cost_factor;
					total_cost += cost * amount;
				} else {
					break;
				}
			}
		}

		text::substitution_map sub;
		text::add_to_substitution_map(sub, text::variable_type::m, std::string_view{ "@(A)" });
		text::add_to_substitution_map(sub, text::variable_type::n, int64_t(amounts.type1));
		text::add_to_substitution_map(sub, text::variable_type::x, int64_t(amounts.type2));
		text::add_to_substitution_map(sub, text::variable_type::y, int64_t(amounts.type3));
		text::add_to_substitution_map(sub, text::variable_type::cost, text::fp_currency{ total_cost });
		text::add_to_substitution_map(sub, text::variable_type::attunit, text::fp_one_place{ ai::estimate_army_offensive_strength(state, a) });
		text::add_to_substitution_map(sub, text::variable_type::defunit, text::fp_one_place{ ai::estimate_army_defensive_strength(state, a) });
		auto box = text::open_layout_box(contents);
		text::add_to_layout_box(state, contents, box, text::embedded_flag{ controller.get_identity_from_identity_holder().id });
		if(army.get_arrival_time()) {
			text::add_to_substitution_map(sub, text::variable_type::prov, *(army.get_path().end() - 1));
			text::add_to_substitution_map(sub, text::variable_type::date, army.get_arrival_time());
			if(auto rf = army.get_controller_from_army_rebel_control(); rf) {
				std::string name = rebel::rebel_name(state, rf);
				text::add_to_layout_box(state, contents, box, std::string_view(name));
			}
			auto resolved = text::resolve_string_substitution(state, "unit_moving_text", sub);
			text::add_unparsed_text_to_layout_box(state, contents, box, resolved);
		} else {
			if(auto rf = army.get_controller_from_army_rebel_control(); rf) {
				std::string name = rebel::rebel_name(state, rf);
				text::add_to_layout_box(state, contents, box, std::string_view(name));
			}
			auto resolved = text::resolve_string_substitution(state, "unit_standing_text", sub);
			text::add_unparsed_text_to_layout_box(state, contents, box, resolved);
		}
		if(state.cheat_data.show_province_id_tooltip) {
			text::add_to_layout_box(state, contents, box, std::string("<"));
			text::add_to_layout_box(state, contents, box, state.world.army_get_ai_province(a));
			text::add_to_layout_box(state, contents, box, std::string(">"));
		}
		text::close_layout_box(contents, box);
		if(state.cheat_data.show_province_id_tooltip) {
			switch(ai::army_activity(state.world.army_get_ai_activity(a))) {
			case ai::army_activity::attacking:
				text::add_line(state, contents, "ai_activity_attacking");
				break;
			case ai::army_activity::attack_gathered:
				text::add_line(state, contents, "ai_activity_gathered");
				break;
			case ai::army_activity::attack_transport:
				text::add_line(state, contents, "ai_activity_transport");
				break;
			case ai::army_activity::merging:
				text::add_line(state, contents, "ai_activity_merging");
				break;
			case ai::army_activity::on_guard:
				text::add_line(state, contents, "ai_activity_on_guard");
				break;
			case ai::army_activity::transport_attack:
				text::add_line(state, contents, "ai_activity_transport_attack");
				break;
			case ai::army_activity::transport_guard:
				text::add_line(state, contents, "ai_activity_transport_guard");
				break;
			case ai::army_activity::unspecified:
				text::add_line(state, contents, "ai_activity_unspecified");
				break;
			default:
				text::add_line(state, contents, "ai_activity_default");
				break;
			}
		}
	}

	void single_unit_tooltip(sys::state& state, text::columnar_layout& contents, dcon::navy_id n) {
		auto navy = dcon::fatten(state.world, n);
		unitamounts amounts = calc_amounts_from_navy(state, navy);

		auto controller = navy.get_controller_from_navy_control();
		if(!controller) {
			controller = dcon::fatten(state.world, state.world.national_identity_get_nation_from_identity_holder(state.national_definitions.rebel_id));
		}

		float total_cost = 0.f;
		for(const auto memb : navy.get_navy_membership()) {
			auto type = state.world.ship_get_type(memb.get_ship());
			float admin_eff = state.world.nation_get_administrative_efficiency(controller);
			float admin_cost_factor = 2.0f - admin_eff;
			auto o_sc_mod = std::max(0.01f, state.world.nation_get_modifier_values(controller, sys::national_mod_offsets::supply_consumption) + 1.0f);
			auto& supply_cost = state.military_definitions.unit_base_definitions[type].supply_cost;
			for(uint32_t i = 0; i < economy::commodity_set::set_size; ++i) {
				if(supply_cost.commodity_type[i]) {
					float cost = state.world.commodity_get_cost(supply_cost.commodity_type[i]);
					float amount = supply_cost.commodity_amounts[i] * state.world.nation_get_unit_stats(controller, type).supply_consumption * o_sc_mod * admin_cost_factor;
					total_cost += cost * amount;
				} else {
					break;
				}
			}
		}

		text::substitution_map sub;
		text::add_to_substitution_map(sub, text::variable_type::m, std::string_view{ "@(N)" });
		text::add_to_substitution_map(sub, text::variable_type::n, int64_t(amounts.type1));
		text::add_to_substitution_map(sub, text::variable_type::x, int64_t(amounts.type2));
		text::add_to_substitution_map(sub, text::variable_type::y, int64_t(amounts.type3));
		text::add_to_substitution_map(sub, text::variable_type::cost, text::fp_currency{ total_cost });
		text::add_to_substitution_map(sub, text::variable_type::attunit, text::fp_one_place{ 1.f });
		text::add_to_substitution_map(sub, text::variable_type::defunit, text::fp_one_place{ 1.f });
		if(navy.get_arrival_time()) {
			text::add_to_substitution_map(sub, text::variable_type::prov, *(navy.get_path().end() - 1));
			text::add_to_substitution_map(sub, text::variable_type::date, navy.get_arrival_time());
			auto box = text::open_layout_box(contents);
			text::add_to_layout_box(state, contents, box, text::embedded_flag{ controller.get_identity_from_identity_holder().id });
			auto resolved = text::resolve_string_substitution(state, "unit_moving_text", sub);
			text::add_unparsed_text_to_layout_box(state, contents, box, resolved);
			text::close_layout_box(contents, box);
		} else {
			auto box = text::open_layout_box(contents);
			text::add_to_layout_box(state, contents, box, text::embedded_flag{ controller.get_identity_from_identity_holder().id });
			auto resolved = text::resolve_string_substitution(state, "unit_standing_text", sub);
			text::add_unparsed_text_to_layout_box(state, contents, box, resolved);
			text::close_layout_box(contents, box);
		}
	}

	void populate_armies(sys::state& state, text::columnar_layout& contents, dcon::province_id prov) {
		auto fat = dcon::fatten(state.world, prov);
		for(auto armyloc : fat.get_army_location()) {
			auto army = armyloc.get_army();
			single_unit_tooltip(state, contents, army);
		}
	}

	void populate_navies(sys::state& state, text::columnar_layout& contents, dcon::province_id prov) {
		auto fat = dcon::fatten(state.world, prov);
		for(auto navyloc : fat.get_navy_location()) {
			auto navy = navyloc.get_navy();
			single_unit_tooltip(state, contents, navy);
		}
	}

	void populate_unit_tooltip(sys::state& state, text::columnar_layout& contents, dcon::province_id prov) {
		populate_armies(state, contents, prov);
		populate_navies(state, contents, prov);
	}

}
