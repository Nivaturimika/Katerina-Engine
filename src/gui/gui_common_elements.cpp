#include "gui_common_elements.hpp"
#include "ai.hpp"
#include "pdqsort.h"

namespace ui {
	bool country_category_filter_check(sys::state& state, country_list_filter filt, dcon::nation_id a, dcon::nation_id b) {
		switch(filt) {
		case country_list_filter::all:
			return true;
		case country_list_filter::allies:
			return nations::are_allied(state, a, b);
		case country_list_filter::enemies:
			return military::are_at_war(state, a, b);
		case country_list_filter::sphere:
			return state.world.nation_get_in_sphere_of(b) == a;
		case country_list_filter::neighbors:
			return bool(state.world.get_nation_adjacency_by_nation_adjacency_pair(a, b));
		case country_list_filter::find_allies:
			return ai::ai_will_accept_alliance(state, b, a)
				&& command::can_ask_for_alliance(state, a, b, false);
		case country_list_filter::influenced:
			return (state.world.gp_relationship_get_status(state.world.get_gp_relationship_by_gp_influence_pair(b, a))
			& nations::influence::priority_mask) != nations::influence::priority_zero
				&& state.world.nation_get_in_sphere_of(b) != a;
		case country_list_filter::neighbors_no_vassals:
			for(const auto sub : state.world.nation_get_overlord_as_ruler(b)) {
				if(state.world.get_nation_adjacency_by_nation_adjacency_pair(a, sub.get_subject()))
					return true;
			}
			return !state.world.overlord_get_ruler(state.world.nation_get_overlord_as_subject(b))
				&& state.world.get_nation_adjacency_by_nation_adjacency_pair(a, b);
		default:
			return true;
		}
	}

	void sort_countries(sys::state& state, std::vector<dcon::nation_id>& list, country_list_sort sort, bool sort_ascend) {
		switch(sort) {
			case country_list_sort::country:
			{
				pdqsort(list.begin(), list.end(), [&](dcon::nation_id a, dcon::nation_id b) {
					auto a_name = text::produce_simple_string(state, text::get_name(state, a));
					auto b_name = text::produce_simple_string(state, text::get_name(state, b));
					if(a_name != b_name)
						return a_name < b_name;
					return a.index() < b.index();
				});
				break;
			}
			case country_list_sort::economic_rank:
			{
				pdqsort(list.begin(), list.end(), [&](dcon::nation_id a, dcon::nation_id b) {
					auto av = state.world.nation_get_industrial_rank(a);
					auto bv = state.world.nation_get_industrial_rank(b);
					if(av != bv)
						return av < bv;
					return a.index() < b.index();
				});
				break;
			}
			case country_list_sort::military_rank:
			{
				pdqsort(list.begin(), list.end(), [&](dcon::nation_id a, dcon::nation_id b) {
					auto av = state.world.nation_get_military_rank(a);
					auto bv = state.world.nation_get_military_rank(b);
					if(av != bv)
						return av < bv;
					return a.index() < b.index();
				});
				break;
			}
			case country_list_sort::prestige_rank:
			{
				pdqsort(list.begin(), list.end(), [&](dcon::nation_id a, dcon::nation_id b) {
					auto av = state.world.nation_get_prestige_rank(a);
					auto bv = state.world.nation_get_prestige_rank(b);
					if(av != bv)
						return av < bv;
					return a.index() < b.index();
				});
				break;
			}
			case country_list_sort::total_rank:
			{
				pdqsort(list.begin(), list.end(), [&](dcon::nation_id a, dcon::nation_id b) {
					auto av = state.world.nation_get_rank(a);
					auto bv = state.world.nation_get_rank(b);
					if(av != bv)
						return av < bv;
					return a.index() < b.index();
				});
				break;
			}
			case country_list_sort::relation:
			{
				pdqsort(list.begin(), list.end(), [&](dcon::nation_id a, dcon::nation_id b) {
					auto rid_a = state.world.get_diplomatic_relation_by_diplomatic_pair(state.local_player_nation, a);
					auto va = bool(rid_a) ? state.world.diplomatic_relation_get_value(rid_a) : 0;
					auto rid_b = state.world.get_diplomatic_relation_by_diplomatic_pair(state.local_player_nation, b);
					auto vb = bool(rid_b) ? state.world.diplomatic_relation_get_value(rid_b) : 0;
					if(va != vb)
						return va > vb;
					return a.index() < b.index();
				});
				break;
			}
			case country_list_sort::opinion:
			{
				pdqsort(list.begin(), list.end(), [&](dcon::nation_id a, dcon::nation_id b) {
					auto grid_a = state.world.get_gp_relationship_by_gp_influence_pair(a, state.local_player_nation);
					auto va = bool(grid_a) ? state.world.gp_relationship_get_status(grid_a) & nations::influence::level_mask : 0;
					auto grid_b = state.world.get_gp_relationship_by_gp_influence_pair(b, state.local_player_nation);
					auto vb = bool(grid_b) ? state.world.gp_relationship_get_status(grid_b) & nations::influence::level_mask : 0;
					if(va != vb)
						return nations::influence::is_influence_level_greater(va, vb);
					return a.index() < b.index();
				});
				break;
			}
			case country_list_sort::priority:
			{
				pdqsort(list.begin(), list.end(), [&](dcon::nation_id a, dcon::nation_id b) {
					auto rel_a = state.world.get_gp_relationship_by_gp_influence_pair(a, state.local_player_nation);
					uint8_t rel_flags_a = bool(rel_a) ? state.world.gp_relationship_get_status(rel_a) & nations::influence::priority_mask : 0;
					auto rel_b = state.world.get_gp_relationship_by_gp_influence_pair(b, state.local_player_nation);
					uint8_t rel_flags_b = bool(rel_b) ? state.world.gp_relationship_get_status(rel_b) & nations::influence::priority_mask : 0;
					if(rel_flags_a != rel_flags_b)
						return rel_flags_a < rel_flags_b;
					return a.index() < b.index();
				});
				break;
			}
			case country_list_sort::boss:
			{
				pdqsort(list.begin(), list.end(), [&](dcon::nation_id a, dcon::nation_id b) {
					auto av = state.world.nation_get_in_sphere_of(a).id.index();
					auto bv = state.world.nation_get_in_sphere_of(b).id.index();
					if(av != bv)
						return av < bv;
					return a.index() < b.index();
				});
				break;
			}
			case country_list_sort::player_influence:
			{
				pdqsort(list.begin(), list.end(), [&](dcon::nation_id a, dcon::nation_id b) {
					auto rel_a = state.world.get_gp_relationship_by_gp_influence_pair(a, state.local_player_nation);
					auto rel_b = state.world.get_gp_relationship_by_gp_influence_pair(b, state.local_player_nation);
					auto av = state.world.gp_relationship_get_influence(rel_a);
					auto bv = state.world.gp_relationship_get_influence(rel_b);
					if(av != bv)
						return av < bv;
					return a.index() < b.index();
				});
				break;
			}
			case country_list_sort::player_investment:
			{
				pdqsort(list.begin(), list.end(), [&](dcon::nation_id a, dcon::nation_id b) {
					auto urel_a = state.world.get_unilateral_relationship_by_unilateral_pair(a, state.local_player_nation);
					auto urel_b = state.world.get_unilateral_relationship_by_unilateral_pair(b, state.local_player_nation);
					auto av = state.world.unilateral_relationship_get_foreign_investment(urel_a);
					auto bv = state.world.unilateral_relationship_get_foreign_investment(urel_b);
					if(av != bv)
						return av < bv;
					return a.index() < b.index();
				});
				break;
			}
			case country_list_sort::factories:
			{
				auto num_factories = [&](dcon::nation_id n) {
					int32_t total = 0;
					for(auto p : state.world.nation_get_province_ownership(n)) {
						auto fac = p.get_province().get_factory_location();
						total += int32_t(fac.end() - fac.begin());
					}
					return total;
				};
				pdqsort(list.begin(), list.end(), [&](dcon::nation_id a, dcon::nation_id b) {
					auto av = num_factories(a);
					auto bv = num_factories(b);
					if(av != bv)
						return av < bv;
					return a.index() < b.index();
				});
				break;
			}
			default:
			{
				uint8_t rank = uint8_t(sort) & 0x3F;
				dcon::nation_id gp = nations::get_nth_great_power(state, rank);
				if((uint8_t(sort) & uint8_t(country_list_sort::gp_influence)) != 0) {
					pdqsort(list.begin(), list.end(), [&](dcon::nation_id a, dcon::nation_id b) {
						auto rel_a = state.world.get_gp_relationship_by_gp_influence_pair(a, gp);
						auto rel_b = state.world.get_gp_relationship_by_gp_influence_pair(b, gp);
						if(rel_a != rel_b)
							return state.world.gp_relationship_get_influence(rel_a) > state.world.gp_relationship_get_influence(rel_b);
						return a.index() < b.index();
					});
				} else if((uint8_t(sort) & uint8_t(country_list_sort::gp_investment)) != 0) {
					pdqsort(list.begin(), list.end(), [&](dcon::nation_id a, dcon::nation_id b) {
						auto urel_a = state.world.get_unilateral_relationship_by_unilateral_pair(a, gp);
						auto urel_b = state.world.get_unilateral_relationship_by_unilateral_pair(b, gp);
						if(urel_a != urel_b)
							return state.world.unilateral_relationship_get_foreign_investment(urel_a) > state.world.unilateral_relationship_get_foreign_investment(urel_b);
						return a.index() < b.index();
					});
				} else {
					assert(0);
				}
				break;
			}
		}

		if(!sort_ascend) {
			std::reverse(list.begin(), list.end());
		}

	}
	std::string get_status_text(sys::state& state, dcon::nation_id nation_id) {
		switch(nations::get_status(state, nation_id)) {
			case nations::status::great_power:
			return text::produce_simple_string(state, "diplomacy_greatnation_status");
			case nations::status::secondary_power:
			return text::produce_simple_string(state, "diplomacy_colonialnation_status");
			case nations::status::civilized:
			return text::produce_simple_string(state, "diplomacy_civilizednation_status");
			case nations::status::westernizing:
			return text::produce_simple_string(state, "diplomacy_almost_western_nation_status");
			case nations::status::uncivilized:
			return text::produce_simple_string(state, "diplomacy_uncivilizednation_status");
			case nations::status::primitive:
			return text::produce_simple_string(state, "diplomacy_primitivenation_status");
			default:
			return text::produce_simple_string(state, "diplomacy_greatnation_status");
		}
	}
} // namespace ui
