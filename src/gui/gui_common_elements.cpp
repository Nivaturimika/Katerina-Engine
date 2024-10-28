#include "gui_common_elements.hpp"
#include "ai.hpp"
#include "pdqsort.h"

namespace ui {
	void simple_multiline_body_text::render(sys::state& state, int32_t x, int32_t y) noexcept {
		auto old_handle = base_data.data.text_common.font_handle;
		if(!state.user_settings.use_classic_fonts) {
			base_data.data.text_common.font_handle &= ~(0x01 << 7);
			auto old_value = base_data.data.text_common.font_handle & 0x3F;
			base_data.data.text_common.font_handle &= ~(0x003F);
			base_data.data.text_common.font_handle |= (old_value - 2);
		}
		multiline_text_element_base::render(state, x, y);
		base_data.data.text_common.font_handle = old_handle;
	}
	void simple_multiline_body_text::on_update(sys::state& state) noexcept {
		text::alignment align = text::alignment::left;
		switch(base_data.data.text.get_alignment()) {
		case ui::alignment::right:
			align = text::alignment::right;
			break;
		case ui::alignment::centered:
			align = text::alignment::center;
			break;
		default:
			break;
		}
		auto border = base_data.data.text.border_size;

		auto color = black_text ? text::text_color::black : text::text_color::white;

		auto old_handle = base_data.data.text_common.font_handle;
		if(!state.user_settings.use_classic_fonts) {
			base_data.data.text_common.font_handle &= ~(0x01 << 7);
			auto old_value = base_data.data.text_common.font_handle & 0x3F;
			base_data.data.text_common.font_handle &= ~(0x003F);
			base_data.data.text_common.font_handle |= (old_value - 2);
		}
		auto container = text::create_endless_layout(state,
		internal_layout,
		text::layout_parameters{
			border.x,
			border.y,
			int16_t(base_data.size.x - border.x * 2),
			int16_t(base_data.size.y - border.y * 2),
			base_data.data.text.font_handle,
			0,
			align,
			color,
			false });
		populate_layout(state, container);
		base_data.data.text_common.font_handle = old_handle;
	}

	void expanded_hitbox_text::on_reset_text(sys::state& state) noexcept {
		auto actual_size = base_data.size.x;
		base_data.size.x -= int16_t(top_left_extension.x + bottom_right_extension.x);
		simple_text_element_base::on_reset_text(state);
		base_data.size.x = actual_size;
	}
	void expanded_hitbox_text::on_create(sys::state & state) noexcept {
		if(base_data.get_element_type() == element_type::button) {
			black_text = text::is_black_from_font_id(base_data.data.button.font_handle);
		} else if(base_data.get_element_type() == element_type::text) {
			black_text = text::is_black_from_font_id(base_data.data.text.font_handle);
		}
		base_data.size.x += int16_t(top_left_extension.x + bottom_right_extension.x);
		base_data.size.y += int16_t(top_left_extension.y + bottom_right_extension.y);
		base_data.position.x -= int16_t(top_left_extension.x);
		base_data.position.y -= int16_t(top_left_extension.y);
	}
	void expanded_hitbox_text::render(sys::state& state, int32_t x, int32_t y) noexcept {
		simple_text_element_base::render(state, x + top_left_extension.x, y + top_left_extension.y);
	}

	void nation_industry_score_text::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto n = retrieve<dcon::nation_id>(state, parent);
		if(state.world.nation_get_owned_province_count(n) == 0) {
			return;
		}
		auto iweight = state.defines.investment_score_factor;
		for(auto si : state.world.nation_get_state_ownership(n)) {
			float total_level = 0;
			float worker_total =
				si.get_state().get_demographics(demographics::to_employment_key(state, state.culture_definitions.primary_factory_worker)) +
				si.get_state().get_demographics(demographics::to_employment_key(state, state.culture_definitions.secondary_factory_worker));
			float total_factory_capacity = 0;
			province::for_each_province_in_state_instance(state, si.get_state(), [&](dcon::province_id p) {
				for(auto f : state.world.province_get_factory_location(p)) {
					total_factory_capacity += float(f.get_factory().get_level() * f.get_factory().get_building_type().get_base_workforce());
					total_level += float(f.get_factory().get_level());
				}
			});
			float per_state = 4.0f * total_level * std::max(std::min(1.0f, worker_total / total_factory_capacity), 0.001f);
			if(per_state > 0.f) {
				auto box = text::open_layout_box(contents);
				text::layout_box name_entry = box;
				text::layout_box level_entry = box;
				text::layout_box workers_entry = box;
				text::layout_box max_workers_entry = box;
				text::layout_box score_box = box;
				name_entry.x_size /= 10;
				text::add_to_layout_box(state, contents, name_entry, text::get_short_state_name(state, si.get_state()).substr(0, 20), text::text_color::yellow);
				level_entry.x_position += 150;
				text::add_to_layout_box(state, contents, level_entry, text::int_wholenum{ int32_t(total_level) });
				workers_entry.x_position += 180;
				text::add_to_layout_box(state, contents, workers_entry, text::int_wholenum{ int32_t(worker_total) });
				max_workers_entry.x_position += 250;
				text::add_to_layout_box(state, contents, max_workers_entry, text::int_wholenum{ int32_t(total_factory_capacity) });
				score_box.x_position += 350;
				text::add_to_layout_box(state, contents, score_box, text::fp_two_places{ per_state });
				//text::localised_format_box(state, contents, box, std::string_view("alice_indscore_1"), sub);
				text::add_to_layout_box(state, contents, box, std::string(" "));
				text::close_layout_box(contents, box);
			}
		}
		float total_invest = nations::get_foreign_investment_as_gp(state, n);
		if(total_invest > 0.f) {
			text::add_line(state, contents, "industry_score_explain_2", text::variable_type::x, text::fp_four_places{ iweight });
			for(auto ur : state.world.nation_get_unilateral_relationship_as_source(n)) {
				if(ur.get_foreign_investment() > 0.f) {
					text::substitution_map sub{};
					text::add_to_substitution_map(sub, text::variable_type::x, ur.get_target());
					text::add_to_substitution_map(sub, text::variable_type::y, text::fp_currency{ ur.get_foreign_investment() });
					auto box = text::open_layout_box(contents);
					text::localised_format_box(state, contents, box, std::string_view("industry_score_explain_3"), sub);
					text::close_layout_box(contents, box);
				}
			}
		}
	}

	void nation_military_score_text::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto n = retrieve<dcon::nation_id>(state, parent);
		auto recruitable = state.world.nation_get_recruitable_regiments(n);
		auto active_regs = state.world.nation_get_active_regiments(n);
		auto is_disarmed = state.world.nation_get_disarmed_until(n) > state.current_date;
		auto disarm_factor = is_disarmed ? state.defines.disarmament_army_hit : 1.f;
		auto supply_mod = std::max(state.world.nation_get_modifier_values(n, sys::national_mod_offsets::supply_consumption) + 1.0f, 0.1f);
		auto avg_land_score = state.world.nation_get_averge_land_unit_score(n);
		auto num_capital_ships = state.world.nation_get_capital_ship_score(n);
		auto gen_range = state.world.nation_get_leader_loyalty(n);
		auto num_leaders = int32_t(gen_range.end() - gen_range.begin());
		text::add_line(state, contents, "military_score_explain_1", text::variable_type::x, text::fp_two_places{ num_capital_ships });
		text::add_line(state, contents, "military_score_explain_2", text::variable_type::x, text::int_wholenum{ recruitable });
		text::add_line(state, contents, "military_score_explain_3", text::variable_type::x, text::int_wholenum{ active_regs });
		text::add_line_with_condition(state, contents, "military_score_explain_4", is_disarmed, text::variable_type::x, text::fp_two_places{ state.defines.disarmament_army_hit });
		text::add_line(state, contents, "military_score_explain_5", text::variable_type::x, text::fp_two_places{ supply_mod });
		active_modifiers_description(state, contents, n, 0, sys::national_mod_offsets::supply_consumption, true);
		text::add_line(state, contents, "military_score_explain_6", text::variable_type::x, text::fp_two_places{ avg_land_score });
		text::add_line(state, contents, "military_score_explain_7", text::variable_type::x, text::int_wholenum{ num_leaders });
	}

	void nation_ruling_party_text::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto nation_id = retrieve<dcon::nation_id>(state, parent);
		auto fat_id = dcon::fatten(state.world, nation_id);
		if(fat_id) {
			std::string ruling_party = text::get_name_as_string(state, fat_id.get_ruling_party());
			ruling_party = ruling_party + " (" + text::get_name_as_string(state, state.world.political_party_get_ideology(state.world.nation_get_ruling_party(nation_id))) + ")";
			auto box = text::open_layout_box(contents, 0);
			text::localised_single_sub_box(state, contents, box, std::string_view("topbar_ruling_party"), text::variable_type::curr, std::string_view(ruling_party));
			text::add_divider_to_layout_box(state, contents, box);
			text::close_layout_box(contents, box);
		}
		uint32_t rules = 0;
		for(auto pi : state.culture_definitions.party_issues) {
			rules |= fat_id.get_ruling_party().get_party_issues(pi).get_rules();
		}
		reform_rules_description(state, contents, rules);
	}

	void nation_colonial_power_text::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto n = retrieve<dcon::nation_id>(state, parent);
		text::substitution_map sub;
		std::string value = text::prettify(nations::free_colonial_points(state, n));
		text::add_to_substitution_map(sub, text::variable_type::value, std::string_view(value));
		auto box = text::open_layout_box(contents, 0);
		text::localised_format_box(state, contents, box, std::string_view("colonial_points_from_technology"), sub);
		text::add_space_to_layout_box(state, contents, box);
		text::add_to_layout_box(state, contents, box, text::format_float(nations::colonial_points_from_technology(state, n), 1), text::text_color::green);
		text::add_line_break_to_layout_box(state, contents, box);
		text::localised_format_box(state, contents, box, std::string_view("colonial_points_from_naval_bases"), sub);
		text::add_space_to_layout_box(state, contents, box);
		text::add_to_layout_box(state, contents, box, text::format_float(nations::colonial_points_from_naval_bases(state, n), 1), text::text_color::green);
		text::add_line_break_to_layout_box(state, contents, box);
		text::localised_format_box(state, contents, box, std::string_view("colonial_points_from_ships"), sub);
		text::add_space_to_layout_box(state, contents, box);
		text::add_to_layout_box(state, contents, box, text::format_float(nations::colonial_points_from_ships(state, n), 1), text::text_color::green);
		text::add_line_break_to_layout_box(state, contents, box);
		text::localised_format_box(state, contents, box, std::string_view("used_colonial_maintenance"), sub);
		text::add_space_to_layout_box(state, contents, box);
		text::add_to_layout_box(state, contents, box, text::format_float(nations::used_colonial_points(state, n), 1), text::text_color::red);
		text::close_layout_box(contents, box);
	}

	void national_tech_school::on_update(sys::state& state) noexcept {
		auto n = retrieve<dcon::nation_id>(state, parent);
		auto mod_id = state.world.nation_get_tech_school(n);
		if(bool(mod_id)) {
			set_text(state, text::produce_simple_string(state, state.world.modifier_get_name(mod_id)));
		} else {
			set_text(state, text::produce_simple_string(state, "traditional_academic"));
		}
	}
	void national_tech_school::update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept {
		auto n = state.local_player_nation;
		auto mod_id = state.world.nation_get_tech_school(retrieve<dcon::nation_id>(state, parent));
		if(bool(mod_id)) {
			auto box = text::open_layout_box(contents, 0);
			text::add_to_layout_box(state, contents, box, state.world.modifier_get_name(mod_id), text::text_color::yellow);
			if(auto desc = state.world.modifier_get_desc(mod_id); state.key_is_localized(desc)) {
				text::substitution_map sub{};
				text::add_to_substitution_map(sub, text::variable_type::country, n);
				text::add_to_substitution_map(sub, text::variable_type::country_adj, text::get_adjective(state, n));
				text::add_to_substitution_map(sub, text::variable_type::capital, state.world.nation_get_capital(n));
				text::add_to_substitution_map(sub, text::variable_type::continentname, state.world.modifier_get_name(state.world.province_get_continent(state.world.nation_get_capital(n))));
				text::add_to_layout_box(state, contents, box, state.world.modifier_get_desc(mod_id), sub);
			}
			text::close_layout_box(contents, box);
			modifier_description(state, contents, mod_id);
		}
	}

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
		std::function<bool(dcon::nation_id a, dcon::nation_id b)> fn;
		switch(sort) {
		case country_list_sort::country:
		{
			fn = [&](dcon::nation_id a, dcon::nation_id b) {
				auto a_name = text::produce_simple_string(state, text::get_name(state, a));
				auto b_name = text::produce_simple_string(state, text::get_name(state, b));
				if(a_name != b_name)
					return a_name < b_name;
				return a.index() < b.index();
			};
			break;
		}
		case country_list_sort::economic_rank:
		{
			fn = [&](dcon::nation_id a, dcon::nation_id b) {
				auto av = state.world.nation_get_industrial_rank(a);
				auto bv = state.world.nation_get_industrial_rank(b);
				if(av != bv)
					return av < bv;
				return a.index() < b.index();
			};
			break;
		}
		case country_list_sort::military_rank:
		{
			fn = [&](dcon::nation_id a, dcon::nation_id b) {
				auto av = state.world.nation_get_military_rank(a);
				auto bv = state.world.nation_get_military_rank(b);
				if(av != bv)
					return av < bv;
				return a.index() < b.index();
			};
			break;
		}
		case country_list_sort::prestige_rank:
		{
			fn = [&](dcon::nation_id a, dcon::nation_id b) {
				auto av = state.world.nation_get_prestige_rank(a);
				auto bv = state.world.nation_get_prestige_rank(b);
				if(av != bv)
					return av < bv;
				return a.index() < b.index();
			};
			break;
		}
		case country_list_sort::total_rank:
		{
			fn = [&](dcon::nation_id a, dcon::nation_id b) {
				auto av = state.world.nation_get_rank(a);
				auto bv = state.world.nation_get_rank(b);
				if(av != bv)
					return av < bv;
				return a.index() < b.index();
			};
			break;
		}
		case country_list_sort::relation:
		{
			fn = [&](dcon::nation_id a, dcon::nation_id b) {
				auto rid_a = state.world.get_diplomatic_relation_by_diplomatic_pair(state.local_player_nation, a);
				auto va = bool(rid_a) ? state.world.diplomatic_relation_get_value(rid_a) : 0;
				auto rid_b = state.world.get_diplomatic_relation_by_diplomatic_pair(state.local_player_nation, b);
				auto vb = bool(rid_b) ? state.world.diplomatic_relation_get_value(rid_b) : 0;
				if(va != vb)
					return va > vb;
				return a.index() < b.index();
			};
			break;
		}
		case country_list_sort::opinion:
		{
			fn = [&](dcon::nation_id a, dcon::nation_id b) {
				auto grid_a = state.world.get_gp_relationship_by_gp_influence_pair(a, state.local_player_nation);
				auto va = bool(grid_a) ? state.world.gp_relationship_get_status(grid_a) & nations::influence::level_mask : 0;
				auto grid_b = state.world.get_gp_relationship_by_gp_influence_pair(b, state.local_player_nation);
				auto vb = bool(grid_b) ? state.world.gp_relationship_get_status(grid_b) & nations::influence::level_mask : 0;
				if(va != vb)
					return nations::influence::is_influence_level_greater(va, vb);
				return a.index() < b.index();
			};
			break;
		}
		case country_list_sort::priority:
		{
			fn = [&](dcon::nation_id a, dcon::nation_id b) {
				auto rel_a = state.world.get_gp_relationship_by_gp_influence_pair(a, state.local_player_nation);
				uint8_t rel_flags_a = bool(rel_a) ? state.world.gp_relationship_get_status(rel_a) & nations::influence::priority_mask : 0;
				auto rel_b = state.world.get_gp_relationship_by_gp_influence_pair(b, state.local_player_nation);
				uint8_t rel_flags_b = bool(rel_b) ? state.world.gp_relationship_get_status(rel_b) & nations::influence::priority_mask : 0;
				if(rel_flags_a != rel_flags_b)
					return rel_flags_a < rel_flags_b;
				return a.index() < b.index();
			};
			break;
		}
		case country_list_sort::boss:
		{
			fn = [&](dcon::nation_id a, dcon::nation_id b) {
				auto av = state.world.nation_get_in_sphere_of(a).id.index();
				auto bv = state.world.nation_get_in_sphere_of(b).id.index();
				if(av != bv)
					return av < bv;
				return a.index() < b.index();
			};
			break;
		}
		case country_list_sort::player_influence:
		{
			fn = [&](dcon::nation_id a, dcon::nation_id b) {
				auto rel_a = state.world.get_gp_relationship_by_gp_influence_pair(a, state.local_player_nation);
				auto rel_b = state.world.get_gp_relationship_by_gp_influence_pair(b, state.local_player_nation);
				auto av = state.world.gp_relationship_get_influence(rel_a);
				auto bv = state.world.gp_relationship_get_influence(rel_b);
				if(av != bv)
					return av < bv;
				return a.index() < b.index();
			};
			break;
		}
		case country_list_sort::player_investment:
		{
			fn = [&](dcon::nation_id a, dcon::nation_id b) {
				auto urel_a = state.world.get_unilateral_relationship_by_unilateral_pair(a, state.local_player_nation);
				auto urel_b = state.world.get_unilateral_relationship_by_unilateral_pair(b, state.local_player_nation);
				auto av = state.world.unilateral_relationship_get_foreign_investment(urel_a);
				auto bv = state.world.unilateral_relationship_get_foreign_investment(urel_b);
				if(av != bv)
					return av < bv;
				return a.index() < b.index();
			};
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
			fn = [&](dcon::nation_id a, dcon::nation_id b) {
				auto av = num_factories(a);
				auto bv = num_factories(b);
				if(av != bv)
					return av < bv;
				return a.index() < b.index();
			};
			break;
		}
		default:
		{
			uint8_t rank = uint8_t(sort) & 0x3F;
			dcon::nation_id gp = nations::get_nth_great_power(state, rank);
			if((uint8_t(sort) & uint8_t(country_list_sort::gp_influence)) != 0) {
				fn = [&](dcon::nation_id a, dcon::nation_id b) {
					auto rel_a = state.world.get_gp_relationship_by_gp_influence_pair(a, gp);
					auto rel_b = state.world.get_gp_relationship_by_gp_influence_pair(b, gp);
					if(rel_a != rel_b)
						return state.world.gp_relationship_get_influence(rel_a) > state.world.gp_relationship_get_influence(rel_b);
					return a.index() < b.index();
				};
			} else if((uint8_t(sort) & uint8_t(country_list_sort::gp_investment)) != 0) {
				fn = [&](dcon::nation_id a, dcon::nation_id b) {
					auto urel_a = state.world.get_unilateral_relationship_by_unilateral_pair(a, gp);
					auto urel_b = state.world.get_unilateral_relationship_by_unilateral_pair(b, gp);
					if(urel_a != urel_b)
						return state.world.unilateral_relationship_get_foreign_investment(urel_a) > state.world.unilateral_relationship_get_foreign_investment(urel_b);
					return a.index() < b.index();
				};
			} else {
				assert(0);
			}
			break;
		}
		}
		auto it = std::unique(list.begin(), list.end(), [&](auto a, auto b) {
			return a.index() == b.index();
		});
		list.erase(it, list.end());
		pdqsort(list.begin(), list.end(), fn);
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
