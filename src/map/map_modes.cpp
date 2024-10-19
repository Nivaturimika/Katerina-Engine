#include "map_modes.hpp"
#include "color.hpp"
#include "demographics.hpp"
#include "system_state.hpp"
#include "dcon_generated.hpp"
#include "province.hpp"
#include "nations.hpp"
#include "economy_factory.hpp"
#include "pdqsort.hpp"
#include "unordered_dense.h"
#include "gui_map_legend.hpp"

std::vector<uint32_t> admin_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;

	std::vector<uint32_t> prov_color(texture_size * 2);
	dcon::province_id selected_province = state.map_state.get_selected_province();
	dcon::nation_id selected_nation = selected_province
		? state.world.province_get_nation_from_province_ownership(selected_province)
		: state.local_player_nation;
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto i = province::to_map_id(prov_id);
		auto fat_id = dcon::fatten(state.world, prov_id);
		if(!selected_nation || fat_id.get_nation_from_province_ownership() == selected_nation) {
			auto admin_efficiency = province::state_admin_efficiency(state, fat_id.get_state_membership());
			uint32_t color = ogl::color_gradient(admin_efficiency,
				sys::pack_color(46, 247, 15), // red
				sys::pack_color(247, 15, 15) // green
			);
			prov_color[i] = color;
			prov_color[i + texture_size] = color;
		} else {
			prov_color[i] = 0;
			prov_color[i + texture_size] = 0;
		}
	});
	return prov_color;
}

std::vector<uint32_t> civilization_level_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		auto status = nations::get_status(state, nation);
		uint32_t color;
		// if it is uncolonized
		if(!nation) {
			color = sys::pack_color(250, 5, 5); // red
		} else if(state.world.nation_get_is_civilized(nation)) {
			color = sys::pack_color(53, 196, 53); // green
		} else {
			float civ_level = state.world.nation_get_modifier_values(nation, sys::national_mod_offsets::civilization_progress_modifier);
			// gray <-> yellow
			color = ogl::color_gradient(civ_level * (1 + (1 - civ_level)), sys::pack_color(250, 250, 5), sys::pack_color(64, 64, 64));
		}
		auto i = province::to_map_id(prov_id);
		prov_color[i] = color;
		if(!state.world.province_get_is_colonial(prov_id)) {
			prov_color[i + texture_size] = color;
		} else {
			prov_color[i + texture_size] = sys::pack_color(53, 53, 250);
		}
	});
	return prov_color;
}

std::vector<uint32_t> colonial_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;

	std::vector<uint32_t> prov_color(texture_size * 2);
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto fat_id = dcon::fatten(state.world, prov_id);
		auto i = province::to_map_id(prov_id);

		if(!(fat_id.get_nation_from_province_ownership())) {
			if(province::is_colonizing(state, state.local_player_nation, fat_id.get_state_from_abstract_state_membership())) {
				if(province::can_invest_in_colony(state, state.local_player_nation, fat_id.get_state_from_abstract_state_membership())) {
					prov_color[i] = sys::pack_color(140, 247, 15);
					prov_color[i + texture_size] = sys::pack_color(140, 247, 15);
				} else {
					prov_color[i] = sys::pack_color(250, 250, 5);
					prov_color[i + texture_size] = sys::pack_color(250, 250, 5);
				}
			} else if(province::can_start_colony(state, state.local_player_nation, fat_id.get_state_from_abstract_state_membership())) {
				prov_color[i] = sys::pack_color(46, 247, 15);
				prov_color[i + texture_size] = sys::pack_color(46, 247, 15);
			} else {
				prov_color[i] = sys::pack_color(247, 15, 15);
				prov_color[i + texture_size] = sys::pack_color(247, 15, 15);
			}
		}
	});
	return prov_color;
}

std::vector<uint32_t> crisis_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;

	std::vector<uint32_t> prov_color(texture_size * 2);
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto fat_id = dcon::fatten(state.world, prov_id);
		auto nation = fat_id.get_nation_from_province_ownership();
		auto i = province::to_map_id(prov_id);

		if(nation) {
			auto color = ogl::color_gradient(fat_id.get_state_membership().get_flashpoint_tension() / 100.0f, sys::pack_color(247, 15, 15), sys::pack_color(46, 247, 15));
			prov_color[i] = color;
			prov_color[i + texture_size] = color;
		}
	});
	return prov_color;
}

std::vector<uint32_t> diplomatic_map_from(sys::state& state) {
	/**
	 * Color:
	 *	- Yellorange -> Casus belli TODO: How do I get the casus belli?
	 *  - Skyblue -> Ally -> X
	 *  - Green stripes -> Cores X
	 *  - White stripes -> Not cores X
	 *  - Yellow stripes -> Nation cultural union X
	 *  - Red -> War X
	 *  - Green -> Selected X
	 *  - Light Green -> Sphereling X
	 *  - Dark green -> Puppet or puppet master X
	 */
	 // This could be stored in an other place
	uint32_t causus_belli_color = 0x00AAFF;
	uint32_t ally_color = 0xFFAA00;
	uint32_t selected_color = 0x00FF00; // Also unclaimed cores stripes color
	uint32_t sphere_color = 0x55AA55;
	uint32_t puppet_color = 0x009900;
	uint32_t non_cores_color = 0xFFFFFF;
	uint32_t war_color = 0x0000FF;
	uint32_t cultural_union_color = 0x00FFFF;

	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);

	auto fat_selected_id = dcon::fatten(state.world, state.map_state.get_selected_province());
	auto selected_nation = fat_selected_id.get_nation_from_province_ownership();

	if(!bool(selected_nation)) {
		selected_nation = state.local_player_nation;
	}

	std::vector<dcon::nation_id> enemies, allies, sphere;
	// Get all enemies
	for(auto wa : state.world.nation_get_war_participant(selected_nation)) {
		bool is_attacker = wa.get_is_attacker();
		for(auto o : wa.get_war().get_war_participant()) {
			if(o.get_is_attacker() != is_attacker) {
				enemies.push_back(o.get_nation().id);
			}
		}
	}
	// Get all allies
	selected_nation.for_each_diplomatic_relation([&](dcon::diplomatic_relation_fat_id relation_id) {
		if(relation_id.get_are_allied()) {
			dcon::nation_id ally_id = relation_id.get_related_nations(0).id != selected_nation.id
				? relation_id.get_related_nations(0).id
				: relation_id.get_related_nations(1).id;
			allies.push_back(ally_id);
		}
	});
	auto selected_primary_culture = selected_nation.get_primary_culture();
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto fat_id = dcon::fatten(state.world, prov_id);
		auto i = province::to_map_id(prov_id);
		uint32_t color = 0x222222;
		uint32_t stripe_color = 0x222222;
		auto fat_owner = fat_id.get_province_ownership().get_nation();
		if(bool(fat_owner)) {
			// Selected nation
			if(fat_owner.id == selected_nation.id) {
				color = selected_color;
				// By default asume its not a core
				stripe_color = non_cores_color;
			} else {
				// Cultural Union
				auto cultural_union_identity = selected_primary_culture.get_culture_group_membership().get_group().get_identity_from_cultural_union_of();
				fat_id.for_each_core([&](dcon::core_fat_id core_id) {
					if(core_id.get_identity().id == cultural_union_identity.id) {
						stripe_color = cultural_union_color;
					}
				});
				// War
				if(std::find(enemies.begin(), enemies.end(), fat_owner.id) != enemies.end()) {
					color = war_color;
				}
				// Allies
				if(std::find(allies.begin(), allies.end(), fat_owner.id) != allies.end()) {
					color = ally_color;
				}
				// Sphere
				if(fat_owner.get_in_sphere_of() == selected_nation) {
					color = sphere_color;
				}
				// Puppets
				auto province_overlord_id = fat_id.get_province_ownership().get_nation().get_overlord_as_subject();
				if(bool(province_overlord_id) && province_overlord_id.get_ruler().id == selected_nation.id) {
					color = puppet_color;
				} else {
					auto selected_overlord_id = selected_nation.get_overlord_as_subject();
					if(bool(selected_overlord_id) && selected_overlord_id.get_ruler().id == fat_owner.id) {
						color = puppet_color;
					}
				}
			}
			// Core
			fat_id.for_each_core([&](dcon::core_fat_id core_id) {
				if(core_id.get_identity().get_nation_from_identity_holder().id == selected_nation.id) {
					stripe_color = selected_color;
				}
			});
		}
		// If no stripe has been set, use the prov color
		if(stripe_color == 0x222222) {
			stripe_color = color;
		}
		prov_color[i] = color;
		prov_color[i + texture_size] = stripe_color;
	});
	return prov_color;
}

std::vector<uint32_t> infrastructure_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	int32_t max_rails_lvl = state.economy_definitions.building_definitions[int32_t(economy::province_building_type::railroad)].max_level;
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		int32_t current_rails_lvl = state.world.province_get_building_level(prov_id, economy::province_building_type::railroad);
		int32_t max_local_rails_lvl = state.world.nation_get_max_building_level(state.local_player_nation, economy::province_building_type::railroad);
		bool party_allows_building_railroads =
			(nation == state.local_player_nation && (state.world.nation_get_combined_issue_rules(nation) & issue_rule::build_railway) != 0) ||
			(nation != state.local_player_nation && (state.world.nation_get_combined_issue_rules(nation) & issue_rule::allow_foreign_investment) != 0);
		uint32_t color;
		if(party_allows_building_railroads) {
			if(province::can_build_railroads(state, prov_id, state.local_player_nation)) {
				color = ogl::color_gradient(
					float(current_rails_lvl) / float(max_rails_lvl),
					sys::pack_color(14, 240, 44), // green
					sys::pack_color(41, 5, 245) // blue
				);
			} else if(current_rails_lvl == max_local_rails_lvl) {
				color = sys::pack_color(232, 228, 111); // yellow
			} else {
				color = sys::pack_color(222, 7, 46); // red
			}
		} else {
			color = sys::pack_color(222, 7, 46); // red
		}
		auto i = province::to_map_id(prov_id);
		prov_color[i] = color;
		if(province::has_railroads_being_built(state, prov_id)) {
			prov_color[i + texture_size] = sys::pack_color(232, 228, 111); // yellow
		} else {
			prov_color[i + texture_size] = color;
		}
	});
	return prov_color;
}



std::vector<uint32_t> migration_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);

	auto selected = state.map_state.selected_province;
	auto for_nation = state.world.province_get_nation_from_province_ownership(selected);
	if(for_nation) {
		float mx = 0.0f;
		float mn = 0.0f;
		for(auto p : state.world.nation_get_province_ownership(for_nation)) {
			auto v = p.get_province().get_daily_net_migration();
			mn = std::min(mn, v);
			mx = std::max(mx, v);
		}
		if(mx > mn) {
			for(auto p : state.world.nation_get_province_ownership(for_nation)) {
				auto v = p.get_province().get_daily_net_migration();
				uint32_t color = ogl::color_gradient((v - mn) / (mx - mn),
					sys::pack_color(46, 247, 15),	// to green
					sys::pack_color(247, 15, 15)	// from red
				);
				auto i = province::to_map_id(p.get_province());
				prov_color[i] = color;
				prov_color[i + texture_size] = color;
			}
		}
	} else {
		static auto last_checked_date = sys::date{};
		static std::vector<float> nation_pos;
		static std::vector<float> nation_neg;

		if(state.ui_date != last_checked_date) {
			last_checked_date = state.ui_date;

			auto sz = state.world.nation_size();
			if(uint32_t(nation_pos.size()) < sz) {
				nation_pos.resize(sz);
			}
			for(uint32_t i = 0; i < sz; ++i) {
				nation_pos[i] = 0.0f;
			}
			//
			if(uint32_t(nation_neg.size()) < sz) {
				nation_neg.resize(sz);
			}
			for(uint32_t i = 0; i < sz; ++i) {
				nation_neg[i] = 0.0f;
			}

			float least_neg = -1.0f;
			float greatest_pos = 1.0f;
			for(auto p : state.world.in_province) {
				auto owner = p.get_nation_from_province_ownership();
				if(owner && uint32_t(owner.id.index()) < sz) {
					auto v = p.get_daily_net_immigration();
					if(v > 0.f) {
						nation_pos[owner.id.index()] += v;
					} else {
						nation_neg[owner.id.index()] += v;
					}
				}
			}
			for(uint32_t i = 0; i < sz; ++i) {
				least_neg = std::min(nation_neg[i], least_neg);
				greatest_pos = std::max(nation_pos[i], greatest_pos);
			}
			for(uint32_t i = 0; i < sz; ++i) {
				if(nation_neg[i] != 0.0f)
					nation_neg[i] = nation_neg[i] / least_neg;
				if(nation_pos[i] != 0.0f)
					nation_pos[i] = nation_pos[i] / greatest_pos;
			}
		}
		for(auto p : state.world.in_province) {
			auto owner = p.get_nation_from_province_ownership();
			if(owner) {
				auto i = province::to_map_id(p);
				if(uint32_t(owner.id.index()) < nation_pos.size()
				&& uint32_t(owner.id.index()) < nation_neg.size()) {
					uint32_t in_color = ogl::color_gradient(nation_pos[owner.id.index()],
						sys::pack_color(15, 255, 15),
						sys::pack_color(15, 64, 15)
					);
					uint32_t em_color = ogl::color_gradient(nation_neg[owner.id.index()],
						sys::pack_color(255, 15, 15),
						sys::pack_color(64, 15, 15)
					);
					if(nation_pos[owner.id.index()] != 0.f) { //has im
						prov_color[i] = in_color;
						if(nation_neg[owner.id.index()] != 0.f) { //has im and em
							prov_color[i + texture_size] = em_color;
						} else { //has only im
							prov_color[i + texture_size] = in_color;
						}
					} else if(nation_neg[owner.id.index()] != 0.f) { //only em -- else has no em or im
						prov_color[i] = em_color;
						prov_color[i + texture_size] = em_color;
					}
				}
			}
		}
	}
	return prov_color;
}



std::vector<uint32_t> get_nationality_global_color(sys::state& state) {
	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;

	std::vector<uint32_t> prov_color(texture_size * 2);
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto id = province::to_map_id(prov_id);
		float total_pops = state.world.province_get_demographics(prov_id, demographics::total);

		dcon::culture_id primary_culture, secondary_culture;
		float primary_culture_percent = 0.f, secondary_culture_percent = 0.f;

		state.world.for_each_culture([&](dcon::culture_id culture_id) {
			auto demo_key = demographics::to_key(state, culture_id);
			auto volume = state.world.province_get_demographics(prov_id, demo_key);
			float percent = volume / total_pops;
			if(percent > primary_culture_percent) {
				secondary_culture = primary_culture;
				secondary_culture_percent = primary_culture_percent;
				primary_culture = culture_id;
				primary_culture_percent = percent;
			} else if(percent > secondary_culture_percent) {
				secondary_culture = culture_id;
				secondary_culture_percent = percent;
			}
		});

		dcon::culture_fat_id fat_primary_culture = dcon::fatten(state.world, primary_culture);

		uint32_t primary_culture_color = fat_primary_culture.get_color();
		uint32_t secondary_culture_color = 0xFFAAAAAA; // This color won't be reached

		if(bool(secondary_culture)) {
			dcon::culture_fat_id fat_secondary_culture = dcon::fatten(state.world, secondary_culture);
			secondary_culture_color = fat_secondary_culture.get_color();
		}

		if(secondary_culture_percent >= .35) {
			prov_color[id] = primary_culture_color;
			prov_color[id + texture_size] = secondary_culture_color;
		} else {
			prov_color[id] = primary_culture_color;
			prov_color[id + texture_size] = primary_culture_color;
		}
	});

	return prov_color;
}

std::vector<uint32_t> get_nationality_diaspora_color(sys::state& state) {
	auto fat_selected_id = dcon::fatten(state.world, state.map_state.get_selected_province());
	auto culture_id = fat_selected_id.get_dominant_culture();
	auto culture_key = demographics::to_key(state, culture_id.id);

	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;

	std::vector<uint32_t> prov_color(texture_size * 2);

	if(bool(culture_id)) {
		uint32_t full_color = culture_id.get_color();
		uint32_t empty_color = 0xDDDDDD;
		// Make the other end of the gradient dark if the color is bright and vice versa.
		// This should make it easier to see cultures that would otherwise be problematic.
		if((full_color & 0xFF) + (full_color >> 8 & 0xFF) + (full_color >> 16 & 0xFF) > 140 * 3) {
			empty_color = 0x222222;
		}
		province::for_each_land_province(state, [&](dcon::province_id prov_id) {
			auto i = province::to_map_id(prov_id);
			auto fat_id = dcon::fatten(state.world, prov_id);
			auto total_pop = state.world.province_get_demographics(prov_id, demographics::total);
			auto culture_pop = state.world.province_get_demographics(prov_id, culture_key);
			auto ratio = culture_pop / total_pop;

			auto color = ogl::color_gradient(ratio, full_color, empty_color);

			prov_color[i] = color;
			prov_color[i + texture_size] = color;
		});
	}
	return prov_color;
}

std::vector<uint32_t> nationality_map_from(sys::state& state) {
	std::vector<uint32_t> prov_color;
	if(state.map_state.get_selected_province()) {
		prov_color = get_nationality_diaspora_color(state);
	} else {
		prov_color = get_nationality_global_color(state);
	}

	return prov_color;
}



std::vector<uint32_t> national_focus_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;

	std::vector<uint32_t> prov_color(texture_size * 2);
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto fat_id = dcon::fatten(state.world, prov_id);
		auto nation = fat_id.get_nation_from_province_ownership();
		auto i = province::to_map_id(prov_id);

		if(nation == state.local_player_nation && fat_id.get_state_membership().get_owner_focus()) {
			prov_color[i] = sys::pack_color(46, 247, 15);
			prov_color[i + texture_size] = sys::pack_color(46, 247, 15);
		}
	});
	return prov_color;
}



#include <vector>

std::vector<uint32_t> naval_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;

	std::vector<uint32_t> prov_color(texture_size * 2);

	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto fat_id = dcon::fatten(state.world, prov_id);
		auto nation = fat_id.get_nation_from_province_ownership();

		if(nation == state.local_player_nation) {
			uint32_t color = 0x222222;
			uint32_t stripe_color = 0x222222;

			if(province::has_naval_base_being_built(state, prov_id)) {
				color = 0x00FF00;
				stripe_color = 0x005500;
			} else if(province::can_build_naval_base(state, prov_id, state.local_player_nation)) {
				if(state.world.province_get_building_level(prov_id, economy::province_building_type::naval_base) != 0) {
					color = 0x00FF00;
					stripe_color = 0x00FF00;
				} else {
					color = sys::pack_color(50, 150, 200);
					stripe_color = sys::pack_color(50, 150, 200);
				}
			} else if(state.world.province_get_building_level(prov_id, economy::province_building_type::naval_base) != 0) {
				color = 0x005500;
				stripe_color = 0x005500;
			} else { // no naval base, not build target

			}

			auto i = province::to_map_id(prov_id);
			prov_color[i] = color;
			prov_color[i + texture_size] = stripe_color;
		}
	});

	return prov_color;
}

#include <algorithm>

struct party_info {
	dcon::ideology_id ideology;
	float loyalty;
	uint32_t color;
};

std::vector<party_info> get_sorted_parties_info(sys::state& state, dcon::province_id prov_id) {
	std::vector<party_info> result;
	state.world.for_each_ideology([&](dcon::ideology_id ideology) {
		auto loyalty = state.world.province_get_party_loyalty(prov_id, ideology);
		if(loyalty > 0.f) {
			result.push_back({ ideology, loyalty, state.world.ideology_get_color(ideology) });
		}
	});
	pdqsort(result.begin(), result.end(), [&](party_info a, party_info b) {
		if(a.loyalty != b.loyalty) {
			return a.loyalty > b.loyalty;
		}
		return a.ideology.index() < b.ideology.index();
	});
	return result;
}

std::vector<uint32_t> party_loyalty_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);

	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto parties_info = get_sorted_parties_info(state, prov_id);
		auto i = province::to_map_id(prov_id);
		if(parties_info.size() == 0) {
			prov_color[i] = 0xFFFFFF;
			prov_color[i + texture_size] = 0xFFFFFF;
		} else if(parties_info.size() == 1) {
			uint32_t color = parties_info[0].color;
			prov_color[i] = color;
			prov_color[i + texture_size] = color;
		} else {
			party_info a = parties_info[0];
			party_info b = parties_info[1];
			prov_color[i] = a.color;
			if(b.loyalty >= a.loyalty * 0.75) {
				prov_color[i + texture_size] = b.color;
			} else {
				prov_color[i + texture_size] = a.color;
			}
		}
	});

	return prov_color;
}


#include "prng.hpp"

uint32_t derive_color_from_ol_color(sys::state& state, uint32_t ol_color, dcon::nation_id n) {
	auto base = sys::rgb_to_hsv(ol_color);
	auto roff = rng::get_random_pair(state, uint32_t(n.index()), uint32_t(n.index()));
	base.h = fmod(base.h + (float(roff.low & 0x1F) - 15.5f), 360.0f);
	base.s = std::clamp(base.s + (float((roff.low >> 8) & 0xFF) / 255.0f) * 0.2f - 0.1f, 0.0f, 1.0f);
	base.v = std::clamp(base.v + (float((roff.high >> 4) & 0xFF) / 255.0f) * 0.2f - 0.1f, 0.0f, 1.0f);
	return sys::hsv_to_rgb(base);
}

std::vector<uint32_t> political_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;

	std::vector<uint32_t> prov_color(texture_size * 2);

	std::vector<uint32_t> nation_color(state.world.nation_size() + 1);
	state.world.for_each_nation([&](dcon::nation_id n) {
		nation_color[n.value] = state.world.nation_get_color(n);
		auto olr = state.world.nation_get_overlord_as_subject(n);
		auto ol = state.world.overlord_get_ruler(olr);
		auto ol_temp = ol;

		while(ol) {
			olr = state.world.nation_get_overlord_as_subject(ol);
			ol_temp = ol;
			ol = state.world.overlord_get_ruler(olr);
		} ol = ol_temp;

		if(ol) {
			auto ol_color = state.world.nation_get_color(ol);
			switch(state.user_settings.vassal_color) {
			case sys::map_vassal_color_mode::inherit:
				nation_color[n.value] = derive_color_from_ol_color(state, ol_color, n);
				break;
			case sys::map_vassal_color_mode::same:
				nation_color[n.value] = ol_color;
				break;
			case sys::map_vassal_color_mode::none:
				break;
			}
		}
	});

	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto fat_id = dcon::fatten(state.world, prov_id);
		auto i = province::to_map_id(prov_id);
		if(prov_id.index() >= state.province_definitions.first_sea_province.index()) {
			prov_color[i] = 0;
			prov_color[i + texture_size] = 0;

			dcon::nation_id first_n{};
			dcon::nation_id second_n{};
			for(const auto adj : fat_id.get_province_adjacency_as_connected_provinces()) {
				auto p2 = adj.get_connected_provinces(adj.get_connected_provinces(0) == prov_id ? 1 : 0);
				if(p2.get_is_coast()) {
					auto n = p2.get_province_control_as_province().get_nation();
					if(!n || second_n == n || first_n == n)
						continue;
					if(!bool(second_n) || n.get_rank() > state.world.nation_get_rank(second_n)) {
						if(!bool(first_n) || n.get_rank() > state.world.nation_get_rank(first_n)) {
							second_n = first_n;
							first_n = n;
						} else {
							second_n = n;
						}
					}
				}
			}

			if(first_n) {
				prov_color[i] = nation_color[first_n.value];
				prov_color[i] |= 0xff000000;
				if(second_n) {
					prov_color[i + texture_size] = nation_color[second_n.value];
					prov_color[i + texture_size] |= 0xff000000;
				}
			}
		} else {
			auto id = fat_id.get_nation_from_province_ownership();
			uint32_t color = 0;
			if(bool(id)) {
				color = nation_color[id.id.value];
			} else { // If no owner use default color
				color = 255 << 16 | 255 << 8 | 255;
			}
			auto occupier = fat_id.get_nation_from_province_control();
			uint32_t color_b = occupier ? nation_color[occupier.id.value] :
				(id ? sys::pack_color(127, 127, 127) : sys::pack_color(255, 255, 255));

			prov_color[i] = color;
			prov_color[i + texture_size] = color_b;
		}
	});

	return prov_color;
}


std::vector<uint32_t> get_global_population_color(sys::state& state) {
	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);

	auto continent_max_pop = state.world.modifier_make_vectorizable_float_buffer();
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto fat_id = dcon::fatten(state.world, prov_id);
		float population = state.world.province_get_demographics(prov_id, demographics::total);
		auto cid = fat_id.get_continent().id;
		continent_max_pop.set(cid, std::max(continent_max_pop.get(cid), population));
		auto i = province::to_map_id(prov_id);
	});
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto fat_id = dcon::fatten(state.world, prov_id);
		auto cid = fat_id.get_continent().id.index();
		auto i = province::to_map_id(prov_id);
		float gradient_index = state.world.province_get_demographics(prov_id, demographics::total) / continent_max_pop[cid];
		auto color = ogl::color_gradient(gradient_index, 210, 100 << 8);
		prov_color[i] = color;
		prov_color[i + texture_size] = color;
	});
	return prov_color;
}

std::vector<uint32_t> get_national_population_color(sys::state& state) {
	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	std::fill(prov_color.begin(), prov_color.end(), 0xFFAAAAAA);

	auto nat_id = state.world.province_get_nation_from_province_ownership(state.map_state.get_selected_province());
	float max_population = 0.f;
	for(const auto pc : state.world.nation_get_province_ownership(nat_id)) {
		auto i = province::to_map_id(pc.get_province());
		float population = state.world.province_get_demographics(pc.get_province(), demographics::total);
		max_population = std::max(max_population, population);
	}
	for(const auto pc : state.world.nation_get_province_ownership(nat_id)) {
		auto i = province::to_map_id(pc.get_province());
		float gradient_index = state.world.province_get_demographics(pc.get_province(), demographics::total) / max_population;
		auto color = ogl::color_gradient(gradient_index, 210, 100 << 8);
		prov_color[i] = color;
		prov_color[i + texture_size] = color;
	}
	return prov_color;
}

std::vector<uint32_t> population_map_from(sys::state& state) {
	std::vector<uint32_t> prov_color;
	if(state.map_state.get_selected_province()
	&& state.world.province_get_nation_from_province_ownership(state.map_state.get_selected_province())) {
		prov_color = get_national_population_color(state);
	} else {
		prov_color = get_global_population_color(state);
	}
	return prov_color;
}


std::vector<uint32_t> rank_map_from(sys::state& state) {
	// These colors are arbitrary
	// 1 to 8 -> green #30f233
	// 9 to 16 -> blue #242fff
	// under 16 but civilized -> yellow #eefc26
	// under 16 but uncivilized -> red #ff2626

	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;

	std::vector<uint32_t> prov_color(texture_size * 2);

	auto num_nations = state.world.nation_size();
	auto unciv_rank = num_nations;
	for(uint32_t i = 0; i < num_nations; ++i) {
		if(!state.world.nation_get_is_civilized(state.nations_by_rank[i])) {
			unciv_rank = i;
			break;
		}
	}

	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto fat_id = dcon::fatten(state.world, prov_id);
		auto nation_id = fat_id.get_nation_from_province_ownership();
		auto status = nations::get_status(state, nation_id);
		float darkness = 0.0f;
		if(status == nations::status::great_power)
			darkness = 1.0f - 0.7f * (state.world.nation_get_rank(nation_id)) / state.defines.great_nations_count;
		else if(status == nations::status::secondary_power)
			darkness = 1.0f - 0.7f * (state.world.nation_get_rank(nation_id) - state.defines.great_nations_count) /
			(state.defines.colonial_rank - state.defines.great_nations_count);
		else if(status == nations::status::civilized)
			darkness = 1.0f - 0.7f * (state.world.nation_get_rank(nation_id) - state.defines.colonial_rank) /
			std::max(1.0f, (float(unciv_rank) - state.defines.colonial_rank));
		else
			darkness = 1.0f - 0.7f * (state.world.nation_get_rank(nation_id) - unciv_rank) /
			std::max(1.0f, (float(num_nations) - float(unciv_rank)));

		uint32_t color;
		if(bool(nation_id)) {
			switch(status) {
			case nations::status::great_power:
				color = sys::pack_color(int32_t(48 * darkness), int32_t(242 * darkness), int32_t(51 * darkness));
				break;

			case nations::status::secondary_power:
				color = sys::pack_color(int32_t(36 * darkness), int32_t(47 * darkness), int32_t(255 * darkness));
				break;

			case nations::status::civilized:
				color = sys::pack_color(int32_t(238 * darkness), int32_t(252 * darkness), int32_t(38 * darkness));
				break;

				// primitive, uncivilized and westernized
			default:
				color = sys::pack_color(int32_t(250 * darkness), int32_t(5 * darkness), int32_t(5 * darkness));
				break;
			}
		} else { // If no owner use default color
			color = 255 << 16 | 255 << 8 | 255;
		}

		auto i = province::to_map_id(prov_id);

		prov_color[i] = color;
		prov_color[i + texture_size] = color;
	});
	return prov_color;
}


std::vector<uint32_t> recruitment_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;

	std::vector<uint32_t> prov_color(texture_size * 2);

	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto fat_id = dcon::fatten(state.world, prov_id);
		auto nation = fat_id.get_nation_from_province_ownership();
		if(nation == state.local_player_nation) {
			auto max_regiments = military::regiments_max_possible_from_province(state, prov_id);
			auto created_regiments = military::regiments_created_from_province(state, prov_id);

			uint32_t color;
			if(max_regiments == 0) {
				// grey
				color = sys::pack_color(155, 156, 149);
			} else if(created_regiments >= max_regiments) {
				// yellow
				color = sys::pack_color(212, 214, 62);
			} else {
				// green
				color = sys::pack_color(53, 196, 53);
			}
			auto i = province::to_map_id(prov_id);

			prov_color[i] = color;
			if(state.world.province_get_land_rally_point(prov_id) || state.world.province_get_naval_rally_point(prov_id)) {
				prov_color[i + texture_size] = sys::pack_color(128, 128, 255);
			} else {
				prov_color[i + texture_size] = color;
			}
		}
	});

	return prov_color;
}


std::vector<uint32_t> region_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto fat_id = dcon::fatten(state.world, prov_id);
		auto id = fat_id.get_abstract_state_membership();
		uint32_t color = ogl::color_from_hash(id.get_state().id.index());
		auto i = province::to_map_id(prov_id);
		prov_color[i] = color;
		prov_color[i + texture_size] = color;
	});
	return prov_color;
}


std::vector<uint32_t> relation_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;

	std::vector<uint32_t> prov_color(texture_size * 2);

	auto selected_province = state.map_state.get_selected_province();
	auto fat_id = dcon::fatten(state.world, selected_province);
	auto selected_nation = fat_id.get_nation_from_province_ownership();

	if(!selected_nation) {
		selected_nation = state.local_player_nation;
	}

	auto relations = selected_nation.get_diplomatic_relation_as_related_nations();
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto other_nation = state.world.province_get_nation_from_province_ownership(prov_id);

		// if the province has no owners
		if(!other_nation) {
			return;
		}

		uint32_t color;

		if(other_nation == selected_nation.id) {
			// the selected nation should be blue
			color = sys::pack_color(66, 106, 227);
		} else {
			auto diplo_relation = state.world.get_diplomatic_relation_by_diplomatic_pair(other_nation, selected_nation);
			auto relation_value = state.world.diplomatic_relation_get_value(diplo_relation);

			float interpolation = (200 + relation_value) / 400.f;

			color = ogl::color_gradient(interpolation, sys::pack_color(46, 247, 15), // green
					sys::pack_color(247, 15, 15)	// red
			);
		}

		auto i = province::to_map_id(prov_id);

		prov_color[i] = color;
		prov_color[i + texture_size] = color;
	});

	return prov_color;
}


std::vector<uint32_t> get_religion_global_color(sys::state& state) {
	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;

	std::vector<uint32_t> prov_color(texture_size * 2);
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto id = province::to_map_id(prov_id);
		float total_pops = state.world.province_get_demographics(prov_id, demographics::total);

		dcon::religion_id primary_religion, secondary_religion;
		float primary_religion_percent = 0.f, secondary_religion_percent = 0.f;

		state.world.for_each_religion([&](dcon::religion_id religion_id) {
			auto demo_key = demographics::to_key(state, religion_id);
			auto volume = state.world.province_get_demographics(prov_id, demo_key);
			float percent = volume / total_pops;

			if(percent > primary_religion_percent) {
				secondary_religion = primary_religion;
				secondary_religion_percent = primary_religion_percent;
				primary_religion = religion_id;
				primary_religion_percent = percent;
			} else if(percent > secondary_religion_percent) {
				secondary_religion = religion_id;
				secondary_religion_percent = percent;
			}
		});

		dcon::religion_fat_id fat_primary_religion = dcon::fatten(state.world, primary_religion);

		uint32_t primary_religion_color = fat_primary_religion.get_color();
		uint32_t secondary_religion_color = 0xFFAAAAAA; // This color won't be reached

		if(bool(secondary_religion)) {
			dcon::religion_fat_id fat_secondary_religion = dcon::fatten(state.world, secondary_religion);
			secondary_religion_color = fat_secondary_religion.get_color();
		}

		if(secondary_religion_percent >= .35) {
			prov_color[id] = primary_religion_color;
			prov_color[id + texture_size] = secondary_religion_color;
		} else {
			prov_color[id] = primary_religion_color;
			prov_color[id + texture_size] = primary_religion_color;
		}
	});

	return prov_color;
}

std::vector<uint32_t> get_religion_diaspora_color(sys::state& state) {
	auto fat_selected_id = dcon::fatten(state.world, state.map_state.get_selected_province());
	auto religion_id = fat_selected_id.get_dominant_religion();
	auto religion_key = demographics::to_key(state, religion_id.id);

	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;

	std::vector<uint32_t> prov_color(texture_size * 2);

	if(bool(religion_id)) {
		uint32_t full_color = religion_id.get_color();
		uint32_t empty_color = 0xDDDDDD;
		// Make the other end of the gradient dark if the color is bright and vice versa.
		// This should make it easier to see religions that would otherwise be problematic.
		if((full_color & 0xFF) + (full_color >> 8 & 0xFF) + (full_color >> 16 & 0xFF) > 140 * 3) {
			empty_color = 0x222222;
		}
		province::for_each_land_province(state, [&](dcon::province_id prov_id) {
			auto i = province::to_map_id(prov_id);
			auto total_pop = state.world.province_get_demographics(prov_id, demographics::total);
			auto religion_pop = state.world.province_get_demographics(prov_id, religion_key);
			auto ratio = religion_pop / total_pop;
			auto color = ogl::color_gradient(ratio, full_color, empty_color);
			prov_color[i] = color;
			prov_color[i + texture_size] = color;
		});
	}
	return prov_color;
}

std::vector<uint32_t> religion_map_from(sys::state& state) {
	std::vector<uint32_t> prov_color;
	if(state.map_state.get_selected_province()) {
		prov_color = get_religion_diaspora_color(state);
	} else {
		prov_color = get_religion_global_color(state);
	}
	return prov_color;
}



std::vector<uint32_t> revolt_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	auto sel_nation = state.world.province_get_nation_from_province_ownership(state.map_state.get_selected_province());
	ankerl::unordered_dense::map<uint16_t, float> rebels_in_province = {};
	ankerl::unordered_dense::map<int32_t, float> continent_max_rebels = {};
	state.world.for_each_rebel_faction([&](dcon::rebel_faction_id id) {
		auto rebellion = dcon::fatten(state.world, id);
		if((sel_nation && sel_nation == rebellion.get_ruler_from_rebellion_within().id) || !sel_nation) {
			for(auto members : state.world.rebel_faction_get_pop_rebellion_membership(id)) {
				rebels_in_province[province::to_map_id(members.get_pop().get_province_from_pop_location().id)] += members.get_pop().get_size();
			}
		}
	});
	for(auto& [p, value] : rebels_in_province) {
		auto pid = province::from_map_id(p);
		auto cid = dcon::fatten(state.world, pid).get_continent().id.index();
		continent_max_rebels[cid] = std::max(continent_max_rebels[cid], value);
	}
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			auto fat_id = dcon::fatten(state.world, prov_id);
			auto i = province::to_map_id(prov_id);
			auto cid = fat_id.get_continent().id.index();

			uint32_t color = 0xDDDDDD; // white
			if(rebels_in_province[i] > 0.0f) {
				float gradient_index = (continent_max_rebels[cid] == 0.f ? 0.f : (rebels_in_province[i] / continent_max_rebels[cid]));
				color = ogl::color_gradient(gradient_index,
						sys::pack_color(247, 15, 15), // red
						sys::pack_color(46, 247, 15) // green
				);
			}
			prov_color[i] = color;
			prov_color[i + texture_size] = color;
		}
	});
	return prov_color;
}

std::vector<uint32_t> rgo_output_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;

	std::vector<uint32_t> prov_color(texture_size * 2);

	auto selected_province = state.map_state.get_selected_province();
	if(selected_province) {
		auto searched_rgo = state.world.province_get_rgo(selected_province);
		float max_rgo_size = 0.f;
		province::for_each_land_province(state, [&](dcon::province_id prov_id) {
			auto n = state.world.province_get_nation_from_province_ownership(prov_id);
			max_rgo_size = std::max(max_rgo_size, state.world.province_get_rgo_actual_production_per_good(prov_id, searched_rgo));
		});
		province::for_each_land_province(state, [&](dcon::province_id prov_id) {
			auto i = province::to_map_id(prov_id);
			auto n = state.world.province_get_nation_from_province_ownership(prov_id);
			uint32_t color = ogl::color_gradient(state.world.province_get_rgo_actual_production_per_good(prov_id, searched_rgo) / max_rgo_size,
				sys::pack_color(46, 247, 15), // red
				sys::pack_color(247, 15, 15) // green
			);
			prov_color[i] = color;
			prov_color[i + texture_size] = color;
		});
	} else {
		province::for_each_land_province(state, [&](dcon::province_id prov_id) {
			auto const color = state.world.commodity_get_color(state.world.province_get_rgo(prov_id));
			auto i = province::to_map_id(prov_id);
			prov_color[i] = color;
			prov_color[i + texture_size] = color;
		});
	}
	return prov_color;
}


std::vector<uint32_t> get_global_sphere_color(sys::state& state) {

	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;

	std::vector<uint32_t> prov_color(texture_size * 2);

	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto fat_id = dcon::fatten(state.world, prov_id);
		auto i = province::to_map_id(prov_id);
		auto owner = fat_id.get_nation_from_province_ownership();
		uint32_t color = 0x222222;
		if(bool(owner)) {
			if(nations::is_great_power(state, owner.id)) {
				color = owner.get_color();
			} else {
				if(auto master = owner.get_in_sphere_of(); master) {
					color = master.get_color();
				}
			}
		}
		prov_color[i] = color;
		prov_color[i + texture_size] = color;
	});
	return prov_color;
}

std::vector<uint32_t> get_selected_sphere_color(sys::state& state) {
	/**
	 * Color logic
	 *	- GP -> Green
	 *  - In Sphere -> Yellow
	 *	- Is influenced -> Blue
	 *  - Is influenced by more than 1 -> Add Red Stripes
	 *
	 *  The concept of influenced is weird, if a tag has cordial relations with a gp but the influence is 0, then its not
	 *influenced Only the influence value should be taken into account
	 *
	 * If country is in sphere or is sphere master
	 *	Paint only the tags influenced by the master, including spherearlings
	 * Else
	 *	Paint the tags that influence the country + the country
	 **/

	 // Todo: Get better colors?
	uint32_t gp_color = 0x00FF00;				 // Green
	uint32_t inf_color = 0xFF0000;			 // Blue
	uint32_t sphere_color = 0x00FFFF;		 // Yellow
	uint32_t other_inf_color = 0x0000FF; // Red

	// Province color vector init
	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);

	auto fat_selected_id = dcon::fatten(state.world, state.map_state.get_selected_province());
	auto selected_nation = fat_selected_id.get_nation_from_province_ownership();

	// Get sphere master if exists
	auto master = selected_nation.get_in_sphere_of();
	if(!bool(master) && nations::is_great_power(state, selected_nation)) {
		master = selected_nation;
	}

	// Paint only sphere countries
	if(bool(master)) {
		province::for_each_land_province(state, [&](dcon::province_id prov_id) {
			auto fat_id = dcon::fatten(state.world, prov_id);
			auto i = province::to_map_id(prov_id);

			uint32_t color = 0x222222;
			uint32_t stripe_color = 0x222222;

			auto owner = fat_id.get_nation_from_province_ownership();

			if(bool(owner)) {
				if(owner.id == master.id) {
					color = gp_color;
				} else {
					auto master_rel_id = state.world.get_gp_relationship_by_gp_influence_pair(owner, master);
					if(bool(master_rel_id)) {
						auto master_rel_status = state.world.gp_relationship_get_status(master_rel_id);
						auto master_rel_inf = state.world.gp_relationship_get_influence(master_rel_id);

						if(master_rel_status == nations::influence::level_in_sphere || master_rel_inf != 0) {
							owner.for_each_gp_relationship_as_influence_target([&](dcon::gp_relationship_id rel_id) {
								// Has more than one influencer
								if(rel_id != master_rel_id && state.world.gp_relationship_get_influence(rel_id) != 0 &&
										state.world.gp_relationship_get_influence(rel_id) != 0) {
									stripe_color = other_inf_color;
								}
							});

							if(master_rel_status == nations::influence::level_in_sphere) {
								color = sphere_color;
							} else if(master_rel_inf != 0) {
								color = inf_color;
							}
						}
					}
				}
			}

			// If no stripe has been set, use the prov color
			if(stripe_color == 0x222222) {
				stripe_color = color;
			}

			prov_color[i] = color;
			prov_color[i + texture_size] = stripe_color;
		});
	} else { // Paint selected country and influencers
		province::for_each_land_province(state, [&](dcon::province_id prov_id) {
			auto fat_id = dcon::fatten(state.world, prov_id);
			auto i = province::to_map_id(prov_id);

			uint32_t color = 0x222222;
			uint32_t stripe_color = 0x222222;

			auto owner = fat_id.get_nation_from_province_ownership();
			if(bool(owner)) {
				bool is_gp = false;
				uint8_t rel_count = 0;

				selected_nation.for_each_gp_relationship_as_influence_target([&](dcon::gp_relationship_fat_id relation_id) {
					if(!is_gp && relation_id.get_influence() > 0) {
						if(owner.id == selected_nation.id) {
							rel_count++;
						}
						if(relation_id.get_great_power() == owner.id) {
							is_gp = true;
						}
					}
				});

				if(is_gp) {
					color = gp_color;
				} else if(rel_count >= 1) {
					color = inf_color;
					if(rel_count > 1) {
						stripe_color = other_inf_color;
					}
				}
			}

			// If no stripe has been set, use the prov color
			if(stripe_color == 0x222222) {
				stripe_color = color;
			}

			prov_color[i] = color;
			prov_color[i + texture_size] = stripe_color;
		});
	}

	return prov_color;
}

std::vector<uint32_t> sphere_map_from(sys::state& state) {
	std::vector<uint32_t> prov_color;
	if(state.map_state.get_selected_province()) {
		prov_color = get_selected_sphere_color(state);
	} else {
		prov_color = get_global_sphere_color(state);
	}
	return prov_color;
}

std::vector<uint32_t> supply_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto fat_id = dcon::fatten(state.world, prov_id);
		auto nation = fat_id.get_nation_from_province_ownership();
		int32_t supply_limit = military::supply_limit_in_province(state, nation, prov_id);
		float interpolation = (supply_limit < 50 ? supply_limit : 50) / 50.f;
		uint32_t color = ogl::color_gradient(interpolation,
			sys::pack_color(46, 247, 15), // red
			sys::pack_color(247, 15, 15) // green
		);
		auto i = province::to_map_id(prov_id);
		prov_color[i] = color;
		prov_color[i + texture_size] = color;
	});
	return prov_color;
}


//
// EXTRA MAP MODES
//
std::vector<uint32_t> ideology_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	if(state.map_state.get_selected_province()) {
		auto fat_id = state.world.province_get_dominant_ideology(state.map_state.get_selected_province());
		if(bool(fat_id)) {
			uint32_t full_color = fat_id.get_color();
			uint32_t empty_color = 0xDDDDDD;
			// Make the other end of the gradient dark if the color is bright and vice versa.
			if((full_color & 0xFF) + (full_color >> 8 & 0xFF) + (full_color >> 16 & 0xFF) > 140 * 3) {
				empty_color = 0x222222;
			}
			auto const pkey = pop_demographics::to_key(state, fat_id.id);
			province::for_each_land_province(state, [&](dcon::province_id prov_id) {
				auto i = province::to_map_id(prov_id);
				float total = 0.f;
				float value = 0.f;
				for(const auto pl : state.world.province_get_pop_location_as_province(prov_id)) {
					value += state.world.pop_get_demographics(pl.get_pop(), pkey);
					total += 1.f;
				}
				auto ratio = value / total;
				auto color = ogl::color_gradient(ratio, full_color, empty_color);
				prov_color[i] = color;
				prov_color[i + texture_size] = color;
			});
		}
	} else {
		province::for_each_land_province(state, [&](dcon::province_id prov_id) {
			auto id = province::to_map_id(prov_id);
			float total_pops = state.world.province_get_demographics(prov_id, demographics::total);
			dcon::ideology_id primary_id;
			dcon::ideology_id secondary_id;
			float primary_percent = 0.f;
			float secondary_percent = 0.f;
			state.world.for_each_ideology([&](dcon::ideology_id id) {
				auto demo_key = demographics::to_key(state, id);
				auto volume = state.world.province_get_demographics(prov_id, demo_key);
				float percent = volume / total_pops;
				if(percent > primary_percent) {
					secondary_id = primary_id;
					secondary_percent = primary_percent;
					primary_id = id;
					primary_percent = percent;
				} else if(percent > secondary_percent) {
					secondary_id = id;
					secondary_percent = percent;
				}
			});
			uint32_t primary_color = dcon::fatten(state.world, primary_id).get_color();
			uint32_t secondary_color = 0xFFAAAAAA; // This color won't be reached
			if(bool(secondary_id)) {
				secondary_color = dcon::fatten(state.world, secondary_id).get_color();
			}
			if(secondary_percent >= primary_percent * 0.75f) {
				prov_color[id] = primary_color;
				prov_color[id + texture_size] = secondary_color;
			} else {
				prov_color[id] = primary_color;
				prov_color[id + texture_size] = primary_color;
			}
		});
	}
	return prov_color;
}

std::vector<uint32_t> issue_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	if(state.map_state.get_selected_province()) {
		auto fat_id = state.world.province_get_dominant_issue_option(state.map_state.get_selected_province());
		if(bool(fat_id)) {
			uint32_t full_color = ogl::get_ui_color(state, fat_id.id);
			uint32_t empty_color = 0xDDDDDD;
			// Make the other end of the gradient dark if the color is bright and vice versa.
			if((full_color & 0xFF) + (full_color >> 8 & 0xFF) + (full_color >> 16 & 0xFF) > 140 * 3) {
				empty_color = 0x222222;
			}
			auto const pkey = pop_demographics::to_key(state, fat_id.id);
			province::for_each_land_province(state, [&](dcon::province_id prov_id) {
				auto i = province::to_map_id(prov_id);
				float total = 0.f;
				float value = 0.f;
				for(const auto pl : state.world.province_get_pop_location_as_province(prov_id)) {
					value += state.world.pop_get_demographics(pl.get_pop(), pkey);
					total += 1.f;
				}
				auto ratio = value / total;
				auto color = ogl::color_gradient(ratio, full_color, empty_color);
				prov_color[i] = color;
				prov_color[i + texture_size] = color;
			});
		}
	} else {
		province::for_each_land_province(state, [&](dcon::province_id prov_id) {
			auto id = province::to_map_id(prov_id);
			float total_pops = state.world.province_get_demographics(prov_id, demographics::total);
			dcon::issue_option_id primary_id;
			dcon::issue_option_id secondary_id;
			float primary_percent = 0.f;
			float secondary_percent = 0.f;
			state.world.for_each_issue_option([&](dcon::issue_option_id id) {
				auto demo_key = demographics::to_key(state, id);
				auto volume = state.world.province_get_demographics(prov_id, demo_key);
				float percent = volume / total_pops;
				if(percent > primary_percent) {
					secondary_id = primary_id;
					secondary_percent = primary_percent;
					primary_id = id;
					primary_percent = percent;
				} else if(percent > secondary_percent) {
					secondary_id = id;
					secondary_percent = percent;
				}
			});
			uint32_t primary_color = ogl::get_ui_color(state, primary_id);
			uint32_t secondary_color = 0xFFAAAAAA; // This color won't be reached
			if(bool(secondary_id)) {
				secondary_color = ogl::get_ui_color(state, secondary_id);
			}
			if(secondary_percent >= primary_percent * 0.75f) {
				prov_color[id] = primary_color;
				prov_color[id + texture_size] = secondary_color;
			} else {
				prov_color[id] = primary_color;
				prov_color[id + texture_size] = primary_color;
			}
		});
	}
	return prov_color;
}

std::vector<uint32_t> fort_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	int32_t max_lvl = state.economy_definitions.building_definitions[int32_t(economy::province_building_type::fort)].max_level;
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		int32_t current_lvl = state.world.province_get_building_level(prov_id, economy::province_building_type::fort);
		int32_t max_local_lvl = state.world.nation_get_max_building_level(state.local_player_nation, economy::province_building_type::fort);
		uint32_t color = 0x222222;
		uint32_t stripe_color = 0x222222;

		if(current_lvl > 0) {
			color = ogl::color_gradient(
				float(current_lvl) / float(max_lvl),
				sys::pack_color(14, 240, 44), // green
				sys::pack_color(41, 5, 245) // blue
			);
		}
		if(province::can_build_fort(state, prov_id, state.local_player_nation)) {
			stripe_color = sys::pack_color(232, 228, 111); // yellow
		} else if(nation == state.local_player_nation && province::has_fort_being_built(state, prov_id)) {
			stripe_color = sys::pack_color(247, 15, 15); // yellow
		} else {
			stripe_color = color;
		}
		auto i = province::to_map_id(prov_id);
		prov_color[i] = color;
		prov_color[i + texture_size] = stripe_color;
	});
	return prov_color;
}

std::vector<uint32_t> factory_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);

	auto sel_nation = state.world.province_get_nation_from_province_ownership(state.map_state.get_selected_province());
	// get state with most factories
	int32_t max_built_total = 1;
	int32_t max_total = 1;
	state.world.for_each_state_instance([&](dcon::state_instance_id sid) {
		auto sdef = state.world.state_instance_get_definition(sid);
		if(!sel_nation || sel_nation == state.world.state_instance_get_nation_from_state_ownership(sid)) {
			int32_t built = economy_factory::state_built_factory_count(state, sid);
			if(built > max_built_total)
				max_built_total = built;
			int32_t total = economy_factory::state_factory_count(state, sid);
			if(total > max_total)
				max_total = total;
		}
	});
	state.world.for_each_state_instance([&](dcon::state_instance_id sid) {
		int32_t total = economy_factory::state_factory_count(state, sid);
		if(total == 0)
			return;
		int32_t built = economy_factory::state_built_factory_count(state, sid);
		auto sdef = state.world.state_instance_get_definition(sid);
		for(const auto abm : state.world.state_definition_get_abstract_state_membership(sdef)) {
			if((sel_nation && abm.get_province().get_province_ownership().get_nation() != sel_nation)
				|| !(abm.get_province().get_nation_from_province_ownership())
				|| abm.get_province().get_nation_from_province_ownership() != state.world.state_instance_get_nation_from_state_ownership(sid))
				continue;
			auto i = province::to_map_id(abm.get_province());
			if(total > 0) {
				prov_color[i + texture_size] = ogl::color_gradient(float(total) / float(max_total),
					sys::pack_color(46, 247, 15), // green
					sys::pack_color(247, 15, 15) // red
				);
				if(built > 0) {
					prov_color[i] = ogl::color_gradient(float(built) / float(max_built_total),
						sys::pack_color(46, 247, 15), // green
						sys::pack_color(247, 15, 15) // red
					);
				}
			}
		}
	});
	return prov_color;
}

std::vector<uint32_t> con_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	auto sel_nation = state.world.province_get_nation_from_province_ownership(state.map_state.get_selected_province());
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			auto scale = 1.f / 10.f;
			auto value = scale * (state.world.province_get_demographics(prov_id, demographics::consciousness) / state.world.province_get_demographics(prov_id, demographics::total));
			uint32_t color = ogl::color_gradient(1.f - value,
				sys::pack_color(46, 247, 15), // green
				sys::pack_color(247, 15, 15) // red
			);
			auto i = province::to_map_id(prov_id);
			prov_color[i] = color;
			prov_color[i + texture_size] = color;
		}
	});
	return prov_color;
}

std::vector<uint32_t> literacy_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	auto sel_nation = state.world.province_get_nation_from_province_ownership(state.map_state.get_selected_province());
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			auto value = (state.world.province_get_demographics(prov_id, demographics::literacy) / state.world.province_get_demographics(prov_id, demographics::total));
			uint32_t color = ogl::color_gradient(value,
				sys::pack_color(46, 247, 15), // green
				sys::pack_color(247, 15, 15) // red
			);
			auto i = province::to_map_id(prov_id);
			prov_color[i] = color;
			prov_color[i + texture_size] = color;
		}
	});
	return prov_color;
}
std::vector<uint32_t> growth_map_from(sys::state& state) {
	std::vector<float> prov_population_change(state.world.province_size() + 1);
	ankerl::unordered_dense::map<int32_t, float> continent_max_growth = {};
	ankerl::unordered_dense::map<int32_t, float> continent_min_growth = {};
	auto sel_nation = state.world.province_get_nation_from_province_ownership(state.map_state.get_selected_province());
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			auto fat_id = dcon::fatten(state.world, prov_id);
			float population_change = float(demographics::get_monthly_pop_increase(state, prov_id));
			auto cid = fat_id.get_continent().id.index();
			continent_max_growth[cid] = std::max(continent_max_growth[cid], population_change);
			continent_min_growth[cid] = std::min(continent_min_growth[cid], population_change);
			auto i = province::to_map_id(prov_id);
			prov_population_change[i] = population_change;
		}
	});
	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			auto fat_id = dcon::fatten(state.world, prov_id);
			auto cid = fat_id.get_continent().id.index();
			auto i = province::to_map_id(prov_id);
			auto color = sys::pack_color(232, 228, 111); // yellow
			if(prov_population_change[i] > 0.f) {
				float gradient_index = (continent_max_growth[cid] == 0.f ? 0.f : (prov_population_change[i] / continent_max_growth[cid]));
				color = ogl::color_gradient(gradient_index,
					sys::pack_color(46, 247, 15), // green
					sys::pack_color(232, 228, 111) // yellow
				);
			} else if(prov_population_change[i] < 0.f) {
				float gradient_index = (continent_min_growth[cid] == 0.f ? 0.f : (prov_population_change[i] / continent_min_growth[cid]));
				color = ogl::color_gradient(gradient_index,
					sys::pack_color(247, 15, 15), // red
					sys::pack_color(232, 228, 111) // yellow
				);
			}
			prov_color[i] = color;
			prov_color[i + texture_size] = color;
		}
	});
	return prov_color;
}
std::vector<uint32_t> income_map_from(sys::state& state) {
	std::vector<float> prov_population(state.world.province_size() + 1);
	ankerl::unordered_dense::map<int32_t, float> continent_max_pop = {};
	auto sel_nation = state.world.province_get_nation_from_province_ownership(state.map_state.get_selected_province());
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			auto fat_id = dcon::fatten(state.world, prov_id);
			float population = 0.f;
			for(const auto pl : state.world.province_get_pop_location_as_province(prov_id))
				population += pl.get_pop().get_savings();
			auto cid = fat_id.get_continent().id.index();
			continent_max_pop[cid] = std::max(continent_max_pop[cid], population);
			auto i = province::to_map_id(prov_id);
			prov_population[i] = population;
		}
	});
	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			auto fat_id = dcon::fatten(state.world, prov_id);
			auto cid = fat_id.get_continent().id.index();
			auto i = province::to_map_id(prov_id);
			float gradient_index = 1.f - (prov_population[i] / continent_max_pop[cid]);
			auto color = ogl::color_gradient(gradient_index, 210, 100 << 8);
			prov_color[i] = color;
			prov_color[i + texture_size] = color;
		}
	});
	return prov_color;
}
std::vector<uint32_t> employment_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	auto sel_nation = state.world.province_get_nation_from_province_ownership(state.map_state.get_selected_province());
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			float pw_total = 0.f;
			float pw_employed = 0.f;
			float sw_total = 0.f;
			float sw_employed = 0.f;
			for(const auto pl : state.world.province_get_pop_location(prov_id)) {
				if(pl.get_pop().get_poptype() == state.culture_definitions.primary_factory_worker) {
					pw_total += pl.get_pop().get_size();
					pw_employed += pl.get_pop().get_employment();
				} else if(pl.get_pop().get_poptype() == state.culture_definitions.secondary_factory_worker) {
					sw_total += pl.get_pop().get_size();
					sw_employed += pl.get_pop().get_employment();
				}
			}
			auto i = province::to_map_id(prov_id);
			if(pw_total > 0.f) {
				uint32_t color = ogl::color_gradient(pw_employed / pw_total,
					sys::pack_color(46, 247, 15), // green
					sys::pack_color(247, 15, 15) // red
				);
				prov_color[i] = color;
			}
			if(sw_total > 0.f) {
				uint32_t color = ogl::color_gradient(sw_employed / sw_total,
					sys::pack_color(46, 247, 15), // green
					sys::pack_color(247, 15, 15) // red
				);
				prov_color[i + texture_size] = color;
			}
		}
	});
	return prov_color;
}

std::vector<uint32_t> militancy_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;

	std::vector<uint32_t> prov_color(texture_size * 2);
	auto sel_nation = state.world.province_get_nation_from_province_ownership(state.map_state.get_selected_province());
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto fat_id = dcon::fatten(state.world, prov_id);
		auto nation = fat_id.get_nation_from_province_ownership();
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			float revolt_risk = province::revolt_risk(state, prov_id) / 10;
			uint32_t color = ogl::color_gradient(revolt_risk,
				sys::pack_color(247, 15, 15), // green
				sys::pack_color(46, 247, 15) // red
			);
			auto i = province::to_map_id(prov_id);
			prov_color[i] = color;
			prov_color[i + texture_size] = color;
		}
	});
	return prov_color;
}

//
// Even newer mapmodes!
//
std::vector<uint32_t> life_needs_map_from(sys::state& state) {
	std::vector<float> prov_population(state.world.province_size() + 1);
	ankerl::unordered_dense::map<int32_t, float> continent_max_pop = {};
	auto sel_nation = state.world.province_get_nation_from_province_ownership(state.map_state.get_selected_province());
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			auto fat_id = dcon::fatten(state.world, prov_id);
			float population = 0.f;
			for(const auto pl : state.world.province_get_pop_location_as_province(prov_id))
				population += pl.get_pop().get_life_needs_satisfaction();
			auto cid = fat_id.get_continent().id.index();
			continent_max_pop[cid] = std::max(continent_max_pop[cid], population);
			auto i = province::to_map_id(prov_id);
			prov_population[i] = population;
		}
	});
	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			auto fat_id = dcon::fatten(state.world, prov_id);
			auto cid = fat_id.get_continent().id.index();
			auto i = province::to_map_id(prov_id);
			float gradient_index = 1.f - (prov_population[i] / continent_max_pop[cid]);
			auto color = ogl::color_gradient(gradient_index, 210, 100 << 8);
			prov_color[i] = color;
			prov_color[i + texture_size] = color;
		}
	});
	return prov_color;
}
std::vector<uint32_t> everyday_needs_map_from(sys::state& state) {
	std::vector<float> prov_population(state.world.province_size() + 1);
	ankerl::unordered_dense::map<int32_t, float> continent_max_pop = {};
	auto sel_nation = state.world.province_get_nation_from_province_ownership(state.map_state.get_selected_province());
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			auto fat_id = dcon::fatten(state.world, prov_id);
			float population = 0.f;
			for(const auto pl : state.world.province_get_pop_location_as_province(prov_id))
				population += pl.get_pop().get_everyday_needs_satisfaction();
			auto cid = fat_id.get_continent().id.index();
			continent_max_pop[cid] = std::max(continent_max_pop[cid], population);
			auto i = province::to_map_id(prov_id);
			prov_population[i] = population;
		}
	});
	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			auto fat_id = dcon::fatten(state.world, prov_id);
			auto cid = fat_id.get_continent().id.index();
			auto i = province::to_map_id(prov_id);
			float gradient_index = 1.f - (prov_population[i] / continent_max_pop[cid]);
			auto color = ogl::color_gradient(gradient_index, 210, 100 << 8);
			prov_color[i] = color;
			prov_color[i + texture_size] = color;
		}
	});
	return prov_color;
}
std::vector<uint32_t> luxury_needs_map_from(sys::state& state) {
	std::vector<float> prov_population(state.world.province_size() + 1);
	ankerl::unordered_dense::map<int32_t, float> continent_max_pop = {};
	auto sel_nation = state.world.province_get_nation_from_province_ownership(state.map_state.get_selected_province());
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			auto fat_id = dcon::fatten(state.world, prov_id);
			float population = 0.f;
			for(const auto pl : state.world.province_get_pop_location_as_province(prov_id))
				population += pl.get_pop().get_luxury_needs_satisfaction();
			auto cid = fat_id.get_continent().id.index();
			continent_max_pop[cid] = std::max(continent_max_pop[cid], population);
			auto i = province::to_map_id(prov_id);
			prov_population[i] = population;
		}
	});
	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			auto fat_id = dcon::fatten(state.world, prov_id);
			auto cid = fat_id.get_continent().id.index();
			auto i = province::to_map_id(prov_id);
			float gradient_index = 1.f - (prov_population[i] / continent_max_pop[cid]);
			auto color = ogl::color_gradient(gradient_index, 210, 100 << 8);
			prov_color[i] = color;
			prov_color[i + texture_size] = color;
		}
	});
	return prov_color;
}
std::vector<uint32_t> life_rating_map_from(sys::state& state) {
	std::vector<float> prov_population(state.world.province_size() + 1);
	ankerl::unordered_dense::map<int32_t, float> continent_max_pop = {};
	auto sel_nation = state.world.province_get_nation_from_province_ownership(state.map_state.get_selected_province());
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			auto fat_id = dcon::fatten(state.world, prov_id);
			auto cid = fat_id.get_continent().id.index();
			continent_max_pop[cid] = std::max(continent_max_pop[cid], float(fat_id.get_life_rating()));
			auto i = province::to_map_id(prov_id);
			prov_population[i] = float(fat_id.get_life_rating());
		}
	});
	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			auto fat_id = dcon::fatten(state.world, prov_id);
			auto cid = fat_id.get_continent().id.index();
			auto i = province::to_map_id(prov_id);
			float gradient_index = 1.f - (prov_population[i] / continent_max_pop[cid]);
			auto color = ogl::color_gradient(gradient_index, 210, 100 << 8);
			prov_color[i] = color;
			prov_color[i + texture_size] = color;
		}
	});
	return prov_color;
}
std::vector<uint32_t> officers_map_from(sys::state& state) {
	std::vector<float> prov_population(state.world.province_size() + 1);
	ankerl::unordered_dense::map<int32_t, float> continent_max_pop = {};
	auto sel_nation = state.world.province_get_nation_from_province_ownership(state.map_state.get_selected_province());
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			auto fat_id = dcon::fatten(state.world, prov_id);
			auto cid = fat_id.get_continent().id.index();
			float total_officers = fat_id.get_demographics(demographics::to_key(state, state.culture_definitions.officers));
			continent_max_pop[cid] = std::max(continent_max_pop[cid], total_officers);
			auto i = province::to_map_id(prov_id);
			prov_population[i] = total_officers;
		}
	});
	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			auto fat_id = dcon::fatten(state.world, prov_id);
			auto cid = fat_id.get_continent().id.index();
			auto i = province::to_map_id(prov_id);
			float gradient_index = 1.f - (prov_population[i] / continent_max_pop[cid]);
			auto color = ogl::color_gradient(gradient_index, 210, 100 << 8);
			prov_color[i] = color;
			prov_color[i + texture_size] = color;
		}
	});
	return prov_color;
}
std::vector<uint32_t> ctc_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	auto sel_nation = state.world.province_get_nation_from_province_ownership(state.map_state.get_selected_province());
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			auto total_pw = state.world.province_get_demographics(prov_id, demographics::to_key(state, state.culture_definitions.primary_factory_worker));
			auto total_sw = state.world.province_get_demographics(prov_id, demographics::to_key(state, state.culture_definitions.secondary_factory_worker));
			auto total = total_pw + total_sw;
			auto value = (total_pw == 0.f || total_sw == 0.f) ? 0.f : total_pw / (total_pw + total_sw);
			value = 1.f - (state.economy_definitions.craftsmen_fraction - value);
			uint32_t color = ogl::color_gradient(value,
				sys::pack_color(46, 247, 15), // green
				sys::pack_color(247, 15, 15) // red
			);
			auto i = province::to_map_id(prov_id);
			prov_color[i] = color;
			prov_color[i + texture_size] = color;
		}
	});
	return prov_color;
}
std::vector<uint32_t> crime_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		dcon::crime_id cmp_crime;
		if(state.map_state.get_selected_province()) {
			cmp_crime = state.world.province_get_crime(state.map_state.get_selected_province());
		}
		auto i = province::to_map_id(prov_id);
		if(auto crime = state.world.province_get_crime(prov_id); crime && (!cmp_crime || crime == cmp_crime)) {
			prov_color[i] = ogl::get_ui_color(state, crime);
			prov_color[i + texture_size] = ogl::get_ui_color(state, crime);
		} else {
			prov_color[i] = 0;
			prov_color[i + texture_size] = 0;
		}
	});
	return prov_color;
}
std::vector<uint32_t> rally_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		auto i = province::to_map_id(prov_id);
		prov_color[i] = state.world.province_get_land_rally_point(prov_id) ? sys::pack_color(46, 247, 15) : 0;
		prov_color[i + texture_size] = state.world.province_get_naval_rally_point(prov_id) ? sys::pack_color(46, 15, 247) : 0;
	});
	return prov_color;
}
std::vector<uint32_t> mobilization_map_from(sys::state& state) {
	std::vector<float> prov_population(state.world.province_size() + 1);
	ankerl::unordered_dense::map<int32_t, float> continent_max_pop = {};
	auto sel_nation = state.world.province_get_nation_from_province_ownership(state.map_state.get_selected_province());
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			auto fat_id = dcon::fatten(state.world, prov_id);
			auto cid = fat_id.get_continent().id.index();
			float total_regs = float(military::regiments_max_possible_from_province(state, prov_id));
			continent_max_pop[cid] = std::max(continent_max_pop[cid], total_regs);
			auto i = province::to_map_id(prov_id);
			prov_population[i] = total_regs;
		}
	});
	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	province::for_each_land_province(state, [&](dcon::province_id prov_id) {
		auto nation = state.world.province_get_nation_from_province_ownership(prov_id);
		if((sel_nation && nation == sel_nation) || !sel_nation) {
			auto fat_id = dcon::fatten(state.world, prov_id);
			auto cid = fat_id.get_continent().id.index();
			auto i = province::to_map_id(prov_id);
			if(prov_population[i] == 0.f) {
				auto color = sys::pack_color(0, 0, 0);
				prov_color[i] = color;
				prov_color[i + texture_size] = color;
			} else {
				float gradient_index = 1.f - (prov_population[i] / continent_max_pop[cid]);
				auto color = ogl::color_gradient(gradient_index, 210, 100 << 8);
				prov_color[i] = color;
				prov_color[i + texture_size] = color;
			}
		}
	});
	return prov_color;
}
std::vector<uint32_t> workforce_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	if(state.map_state.get_selected_province()) {
		dcon::pop_type_fat_id fat_id = dcon::fatten(state.world, dcon::pop_type_id{});
		float pt_max = 0.f;
		for(const auto pt : state.world.in_pop_type) {
			auto total = state.world.province_get_demographics(state.map_state.get_selected_province(), demographics::to_key(state, pt));
			if(total > pt_max) {
				fat_id = pt;
				total = pt_max;
			}
		}
		if(bool(fat_id)) {
			uint32_t full_color = fat_id.get_color();
			uint32_t empty_color = 0xDDDDDD;
			// Make the other end of the gradient dark if the color is bright and vice versa.
			if((full_color & 0xFF) + (full_color >> 8 & 0xFF) + (full_color >> 16 & 0xFF) > 140 * 3) {
				empty_color = 0x222222;
			}
			province::for_each_land_province(state, [&](dcon::province_id prov_id) {
				auto i = province::to_map_id(prov_id);
				float total = state.world.province_get_demographics(state.map_state.get_selected_province(), demographics::total);
				float value = state.world.province_get_demographics(state.map_state.get_selected_province(), demographics::to_key(state, fat_id));
				auto ratio = value / total;
				auto color = ogl::color_gradient(ratio, full_color, empty_color);
				prov_color[i] = color;
				prov_color[i + texture_size] = color;
			});
		}
	} else {
		province::for_each_land_province(state, [&](dcon::province_id prov_id) {
			auto id = province::to_map_id(prov_id);
			float total_pops = state.world.province_get_demographics(prov_id, demographics::total);
			dcon::pop_type_id primary_id;
			dcon::pop_type_id secondary_id;
			float primary_percent = 0.f;
			float secondary_percent = 0.f;
			state.world.for_each_pop_type([&](dcon::pop_type_id id) {
				float volume = 0.f;
				for(const auto pl : state.world.province_get_pop_location_as_province(prov_id)) {
					if(pl.get_pop().get_poptype() == id) {
						volume += pl.get_pop().get_size();
					}
				}
				float percent = volume / total_pops;
				if(percent > primary_percent) {
					secondary_id = primary_id;
					secondary_percent = primary_percent;
					primary_id = id;
					primary_percent = percent;
				} else if(percent > secondary_percent) {
					secondary_id = id;
					secondary_percent = percent;
				}
			});
			uint32_t primary_color = dcon::fatten(state.world, primary_id).get_color();
			uint32_t secondary_color = 0xFFAAAAAA; // This color won't be reached
			if(bool(secondary_id)) {
				secondary_color = dcon::fatten(state.world, secondary_id).get_color();
			}
			if(secondary_percent >= primary_percent * 0.75f) {
				prov_color[id] = primary_color;
				prov_color[id + texture_size] = secondary_color;
			} else {
				prov_color[id] = primary_color;
				prov_color[id + texture_size] = primary_color;
			}
		});
	}
	return prov_color;
}
std::vector<uint32_t> players_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size() + 1;
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2);
	for(const auto n : state.world.in_nation) {
		if(n.get_is_player_controlled()) {
			for(const auto po : state.world.nation_get_province_ownership_as_nation(n)) {
				auto id = province::to_map_id(po.get_province());
				prov_color[id] = n.get_color();
			}
			for(const auto po : state.world.nation_get_province_control_as_nation(n)) {
				auto id = province::to_map_id(po.get_province());
				prov_color[id + texture_size] = n.get_color();
			}
		}
	}
	return prov_color;
}

#include "gui_element_types.hpp"

std::vector<uint32_t> select_states_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;
	std::vector<uint32_t> prov_color(texture_size * 2, 0);

	assert(state.state_selection.has_value());
	if(state.state_selection) {
		for(const auto s : state.state_selection->selectable_states) {
			uint32_t color = ogl::color_from_hash(s.index());

			for(const auto m : state.world.state_definition_get_abstract_state_membership_as_state(s)) {
				auto p = m.get_province();

				auto i = province::to_map_id(p.id);

				prov_color[i] = color;
				prov_color[i + texture_size] = ~color;
			}
		}
	}
	return prov_color;
}

namespace map_mode {

	void set_map_mode(sys::state& state, mode mode) {
		std::vector<uint32_t> prov_color;
		switch(mode) {
		case map_mode::mode::migration:
		case map_mode::mode::population:
		case map_mode::mode::relation:
		case map_mode::mode::revolt:
		case map_mode::mode::supply:
		case map_mode::mode::admin:
		case map_mode::mode::crisis:
			//New mapmodes
		case map_mode::mode::literacy:
		case map_mode::mode::conciousness:
		case map_mode::mode::growth:
		case map_mode::mode::income:
		case map_mode::mode::employment:
		case map_mode::mode::militancy:
		case map_mode::mode::life_needs:
		case map_mode::mode::everyday_needs:
		case map_mode::mode::luxury_needs:
		case map_mode::mode::mobilization:
		case map_mode::mode::officers:
		case map_mode::mode::life_rating:
		case map_mode::mode::clerk_to_craftsmen_ratio:
			if(!state.ui_state.map_gradient_legend) {
				auto legend_win = ui::make_element_by_type<ui::map_legend_gradient>(state, "alice_map_legend_gradient_window");
				state.ui_state.map_gradient_legend = legend_win.get();
				state.ui_state.root->add_child_to_front(std::move(legend_win));
			}
			state.ui_state.map_gradient_legend->set_visible(state, true);
			break;
		default:
			if(state.ui_state.map_gradient_legend)
				state.ui_state.map_gradient_legend->set_visible(state, false);
			break;
		}
		if(mode == mode::civilization_level) {
			if(!state.ui_state.map_civ_level_legend) {
				auto legend_win = ui::make_element_by_type<ui::map_legend_civ_level>(state, "alice_map_legend_civ_level");
				state.ui_state.map_civ_level_legend = legend_win.get();
				state.ui_state.root->add_child_to_front(std::move(legend_win));
			}
			state.ui_state.map_civ_level_legend->set_visible(state, true);
		} else {
			if(state.ui_state.map_civ_level_legend)
			state.ui_state.map_civ_level_legend->set_visible(state, false);
		}
		if(mode == mode::colonial) {
			if(!state.ui_state.map_col_legend) {
				auto legend_win = ui::make_element_by_type<ui::map_legend_col>(state, "alice_map_legend_colonial");
				state.ui_state.map_col_legend = legend_win.get();
				state.ui_state.root->add_child_to_front(std::move(legend_win));
			}
			state.ui_state.map_col_legend->set_visible(state, true);
		} else {
			if(state.ui_state.map_col_legend)
			state.ui_state.map_col_legend->set_visible(state, false);
		}
		if(mode == mode::diplomatic) {
			if(!state.ui_state.map_dip_legend) {
				auto legend_win = ui::make_element_by_type<ui::map_legend_dip>(state, "alice_map_legend_diplomatic");
				state.ui_state.map_dip_legend = legend_win.get();
				state.ui_state.root->add_child_to_front(std::move(legend_win));
			}
			state.ui_state.map_dip_legend->set_visible(state, true);
		} else {
			if(state.ui_state.map_dip_legend)
			state.ui_state.map_dip_legend->set_visible(state, false);
		}
		if(mode == mode::infrastructure || mode == mode::fort) {
			if(!state.ui_state.map_rr_legend) {
				auto legend_win = ui::make_element_by_type<ui::map_legend_rr>(state, "alice_map_legend_infrastructure");
				state.ui_state.map_rr_legend = legend_win.get();
				state.ui_state.root->add_child_to_front(std::move(legend_win));
			}
			state.ui_state.map_rr_legend->set_visible(state, true);
		} else {
			if(state.ui_state.map_rr_legend)
			state.ui_state.map_rr_legend->set_visible(state, false);
		}
		if(mode == mode::naval) {
			if(!state.ui_state.map_nav_legend) {
				auto legend_win = ui::make_element_by_type<ui::map_legend_nav>(state, "alice_map_legend_naval");
				state.ui_state.map_nav_legend = legend_win.get();
				state.ui_state.root->add_child_to_front(std::move(legend_win));
			}
			state.ui_state.map_nav_legend->set_visible(state, true);
		} else {
			if(state.ui_state.map_nav_legend)
			state.ui_state.map_nav_legend->set_visible(state, false);
		}
		if(mode == mode::rank) {
			if(!state.ui_state.map_rank_legend) {
				auto legend_win = ui::make_element_by_type<ui::map_legend_rank>(state, "alice_map_legend_rank");
				state.ui_state.map_rank_legend = legend_win.get();
				state.ui_state.root->add_child_to_front(std::move(legend_win));
			}
			state.ui_state.map_rank_legend->set_visible(state, true);
		} else {
			if(state.ui_state.map_rank_legend)
			state.ui_state.map_rank_legend->set_visible(state, false);
		}
		if(mode == mode::recruitment) {
			if(!state.ui_state.map_rec_legend) {
				auto legend_win = ui::make_element_by_type<ui::map_legend_rec>(state, "alice_map_legend_rec");
				state.ui_state.map_rec_legend = legend_win.get();
				state.ui_state.root->add_child_to_front(std::move(legend_win));
			}
			state.ui_state.map_rec_legend->set_visible(state, true);
		} else {
			if(state.ui_state.map_rec_legend)
			state.ui_state.map_rec_legend->set_visible(state, false);
		}

		switch(mode) {
		case mode::state_select:
			prov_color = select_states_map_from(state);
			break;
		case mode::terrain:
			state.map_state.set_terrain_map_mode();
			return;
		case mode::political:
			prov_color = political_map_from(state);
			break;
		case mode::region:
			prov_color = region_map_from(state);
			break;
		case mode::population:
			prov_color = population_map_from(state);
			break;
		case mode::nationality:
			prov_color = nationality_map_from(state);
			break;
		case mode::sphere:
			prov_color = sphere_map_from(state);
			break;
		case mode::diplomatic:
			prov_color = diplomatic_map_from(state);
			break;
		case mode::rank:
			prov_color = rank_map_from(state);
			break;
		case mode::recruitment:
			prov_color = recruitment_map_from(state);
			break;
		case mode::supply:
			prov_color = supply_map_from(state);
			break;
		case mode::relation:
			prov_color = relation_map_from(state);
			break;
		case mode::civilization_level:
			prov_color = civilization_level_map_from(state);
			break;
		case mode::migration:
			prov_color = migration_map_from(state);
			break;
		case mode::infrastructure:
			prov_color = infrastructure_map_from(state);
			break;
		case mode::revolt:
			prov_color = revolt_map_from(state);
			break;
		case mode::party_loyalty:
			prov_color = party_loyalty_map_from(state);
			break;
		case mode::admin:
			prov_color = admin_map_from(state);
			break;
		case mode::naval:
			prov_color = naval_map_from(state);
			break;
		case mode::national_focus:
			prov_color = national_focus_map_from(state);
			break;
		case mode::crisis:
			prov_color = crisis_map_from(state);
			break;
		case mode::colonial:
			prov_color = colonial_map_from(state);
			break;
		case mode::rgo_output:
			prov_color = rgo_output_map_from(state);
			break;
		case mode::religion:
			prov_color = religion_map_from(state);
			break;
		case mode::issues:
			prov_color = issue_map_from(state);
			break;
		case mode::ideology:
			prov_color = ideology_map_from(state);
			break;
		case mode::fort:
			prov_color = fort_map_from(state);
			break;
		case mode::income:
			prov_color = income_map_from(state);
			break;
		case mode::conciousness:
			prov_color = con_map_from(state);
			break;
		case mode::militancy:
			prov_color = militancy_map_from(state);
			break;
		case mode::literacy:
			prov_color = literacy_map_from(state);
			break;
		case mode::employment:
			prov_color = employment_map_from(state);
			break;
		case mode::factories:
			prov_color = factory_map_from(state);
			break;
		case mode::growth:
			prov_color = growth_map_from(state);
			break;
			//even newer mapmodes
		case mode::players:
			prov_color = players_map_from(state);
			break;
		case mode::life_needs:
			prov_color = life_needs_map_from(state);
			break;
		case mode::everyday_needs:
			prov_color = everyday_needs_map_from(state);
			break;
		case mode::luxury_needs:
			prov_color = luxury_needs_map_from(state);
			break;
		case mode::life_rating:
			prov_color = life_rating_map_from(state);
			break;
		case mode::clerk_to_craftsmen_ratio:
			prov_color = ctc_map_from(state);
			break;
		case mode::crime:
			prov_color = crime_map_from(state);
			break;
		case mode::rally:
			prov_color = rally_map_from(state);
			break;
		case mode::officers:
			prov_color = officers_map_from(state);
			break;
		case mode::mobilization:
			prov_color = mobilization_map_from(state);
			break;
		case mode::workforce:
			prov_color = workforce_map_from(state);
			break;
		default:
			return;
		}
		state.map_state.set_province_color(prov_color, mode);
	}

	void update_map_mode(sys::state& state) {
		if(state.map_state.active_map_mode == mode::terrain || state.map_state.active_map_mode == mode::region) {
			return;
		}
		set_map_mode(state, state.map_state.active_map_mode);
	}
} // namespace map_mode
