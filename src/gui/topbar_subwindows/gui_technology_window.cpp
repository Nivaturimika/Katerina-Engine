#include "gui_element_templates.hpp"
#include "gui_technology_window.hpp"

namespace ui {
	void unit_modifier_description(sys::state& state, text::layout_base& contents, int32_t indent, sys::unit_modifier& mod) {
		bool is_land = true;
		if(mod.type == state.military_definitions.base_army_unit) {
			// ???
		} else if(mod.type == state.military_definitions.base_naval_unit) {
			is_land = false;
		} else {
			is_land = state.military_definitions.unit_base_definitions[mod.type].is_land;
		}

		auto unit_modifier_name_description = [&](text::layout_box& box) {
			if(mod.type == state.military_definitions.base_army_unit) {
				text::localised_format_box(state, contents, box, "armies");
			} else if(mod.type == state.military_definitions.base_naval_unit) {
				text::localised_format_box(state, contents, box, "navies");
			} else {
				text::add_to_layout_box(state, contents, box, state.military_definitions.unit_base_definitions[mod.type].name);
			}
			text::add_space_to_layout_box(state, contents, box);
		};

		if(mod.build_time != 0) {
			auto box = text::open_layout_box(contents, indent + 15);
			unit_modifier_name_description(box);
			text::localised_format_box(state, contents, box, "build_time");
			text::add_to_layout_box(state, contents, box, std::string_view{": "});
			if(mod.build_time < 0) {
				text::add_to_layout_box(state, contents, box, int64_t{mod.build_time}, text::text_color::green);
			} else {
				text::add_to_layout_box(state, contents, box, std::string_view{"+"}, text::text_color::red);
				text::add_to_layout_box(state, contents, box, int64_t{mod.build_time}, text::text_color::red);
			}
			text::close_layout_box(contents, box);
		}
		if(mod.default_organisation != 0) {
			auto box = text::open_layout_box(contents, indent + 15);
			unit_modifier_name_description(box);
			text::localised_format_box(state, contents, box, "default_org");
			text::add_to_layout_box(state, contents, box, std::string_view{": "});
			if(mod.default_organisation < 0) {
				text::add_to_layout_box(state, contents, box, int64_t{mod.default_organisation}, text::text_color::red);
			} else {
				text::add_to_layout_box(state, contents, box, std::string_view{"+"}, text::text_color::green);
				text::add_to_layout_box(state, contents, box, int64_t{mod.default_organisation}, text::text_color::green);
			}
			text::close_layout_box(contents, box);
		}
		if(mod.maximum_speed != 0) {
			auto box = text::open_layout_box(contents, indent + 15);
			unit_modifier_name_description(box);
			text::localised_format_box(state, contents, box, "maximum_speed");
			text::add_to_layout_box(state, contents, box, std::string_view{": "});
			if(mod.maximum_speed < 0) {
				text::add_to_layout_box(state, contents, box, text::fp_two_places{mod.maximum_speed}, text::text_color::red);
			} else {
				text::add_to_layout_box(state, contents, box, std::string_view{"+"}, text::text_color::green);
				text::add_to_layout_box(state, contents, box, text::fp_two_places{mod.maximum_speed}, text::text_color::green);
			}
			text::close_layout_box(contents, box);
		}
		if(mod.supply_consumption != 0) {
			auto box = text::open_layout_box(contents, indent + 15);
			unit_modifier_name_description(box);
			text::localised_format_box(state, contents, box, "supply_consumption");
			text::add_to_layout_box(state, contents, box, std::string_view{": "});
			if(mod.supply_consumption < 0) {
				text::add_to_layout_box(state, contents, box, text::fp_percentage{mod.supply_consumption}, text::text_color::green);
			} else {
				text::add_to_layout_box(state, contents, box, std::string_view{"+"}, text::text_color::red);
				text::add_to_layout_box(state, contents, box, text::fp_percentage{mod.supply_consumption}, text::text_color::red);
			}
			text::close_layout_box(contents, box);
		}

		if(is_land) {
			if(mod.attack_or_gun_power != 0) {
				auto box = text::open_layout_box(contents, indent + 15);
				unit_modifier_name_description(box);
				text::localised_format_box(state, contents, box, "attack");
				text::add_to_layout_box(state, contents, box, std::string_view{": "});
				if(mod.attack_or_gun_power < 0) {
					text::add_to_layout_box(state, contents, box, text::fp_two_places{mod.attack_or_gun_power}, text::text_color::red);
				} else {
					text::add_to_layout_box(state, contents, box, std::string_view{"+"}, text::text_color::green);
					text::add_to_layout_box(state, contents, box, text::fp_two_places{mod.attack_or_gun_power}, text::text_color::green);
				}
				text::close_layout_box(contents, box);
			}
			if(mod.defence_or_hull != 0) {
				auto box = text::open_layout_box(contents, indent + 15);
				unit_modifier_name_description(box);
				text::localised_format_box(state, contents, box, "defence");
				text::add_to_layout_box(state, contents, box, std::string_view{": "});
				if(mod.defence_or_hull < 0) {
					text::add_to_layout_box(state, contents, box, text::fp_two_places{mod.defence_or_hull}, text::text_color::red);
				} else {
					text::add_to_layout_box(state, contents, box, std::string_view{"+"}, text::text_color::green);
					text::add_to_layout_box(state, contents, box, text::fp_two_places{mod.defence_or_hull}, text::text_color::green);
				}
				text::close_layout_box(contents, box);
			}
			if(mod.reconnaissance_or_fire_range != 0) {
				auto box = text::open_layout_box(contents, indent + 15);
				unit_modifier_name_description(box);
				text::localised_format_box(state, contents, box, "reconaissance");
				text::add_to_layout_box(state, contents, box, std::string_view{": "});
				if(mod.reconnaissance_or_fire_range < 0) {
					text::add_to_layout_box(state, contents, box, text::fp_two_places{mod.reconnaissance_or_fire_range}, text::text_color::red);
				} else {
					text::add_to_layout_box(state, contents, box, std::string_view{"+"}, text::text_color::green);
					text::add_to_layout_box(state, contents, box, text::fp_two_places{mod.reconnaissance_or_fire_range}, text::text_color::green);
				}
				text::close_layout_box(contents, box);
			}
			if(mod.siege_or_torpedo_attack != 0) {
				auto box = text::open_layout_box(contents, indent + 15);
				unit_modifier_name_description(box);
				text::localised_format_box(state, contents, box, "siege");
				text::add_to_layout_box(state, contents, box, std::string_view{": "});
				if(mod.siege_or_torpedo_attack < 0) {
					text::add_to_layout_box(state, contents, box, text::fp_two_places{mod.siege_or_torpedo_attack}, text::text_color::red);
				} else {
					text::add_to_layout_box(state, contents, box, std::string_view{"+"}, text::text_color::green);
					text::add_to_layout_box(state, contents, box, text::fp_two_places{mod.siege_or_torpedo_attack}, text::text_color::green);
				}
				text::close_layout_box(contents, box);
			}
			if(mod.support != 0) {
				auto box = text::open_layout_box(contents, indent + 15);
				unit_modifier_name_description(box);
				text::localised_format_box(state, contents, box, "support");
				text::add_to_layout_box(state, contents, box, std::string_view{": "});
				if(mod.support < 0) {
					text::add_to_layout_box(state, contents, box, text::fp_percentage{mod.support}, text::text_color::red);
				} else {
					text::add_to_layout_box(state, contents, box, std::string_view{"+"}, text::text_color::green);
					text::add_to_layout_box(state, contents, box, text::fp_percentage{mod.support}, text::text_color::green);
				}
				text::close_layout_box(contents, box);
			}
			if(mod.discipline_or_evasion != 0) {
				auto box = text::open_layout_box(contents, indent + 15);
				unit_modifier_name_description(box);
				text::localised_format_box(state, contents, box, "discipline");
				text::add_to_layout_box(state, contents, box, std::string_view{ ": " });
				if(mod.discipline_or_evasion < 0) {
					text::add_to_layout_box(state, contents, box, text::fp_percentage{ mod.discipline_or_evasion }, text::text_color::red);
				} else {
					text::add_to_layout_box(state, contents, box, std::string_view{ "+" }, text::text_color::green);
					text::add_to_layout_box(state, contents, box, text::fp_percentage{ mod.discipline_or_evasion }, text::text_color::green);
				}
				text::close_layout_box(contents, box);
			}
		} else {
			if(mod.attack_or_gun_power != 0) {
				auto box = text::open_layout_box(contents, indent + 15);
				unit_modifier_name_description(box);
				text::localised_format_box(state, contents, box, "gun_power");
				text::add_to_layout_box(state, contents, box, std::string_view{": "});
				if(mod.attack_or_gun_power < 0) {
					text::add_to_layout_box(state, contents, box, text::fp_two_places{mod.attack_or_gun_power}, text::text_color::red);
				} else {
					text::add_to_layout_box(state, contents, box, std::string_view{"+"}, text::text_color::green);
					text::add_to_layout_box(state, contents, box, text::fp_two_places{mod.attack_or_gun_power}, text::text_color::green);
				}
				text::close_layout_box(contents, box);
			}
			if(mod.defence_or_hull != 0) {
				auto box = text::open_layout_box(contents, indent + 15);
				unit_modifier_name_description(box);
				text::localised_format_box(state, contents, box, "hull");
				text::add_to_layout_box(state, contents, box, std::string_view{": "});
				if(mod.defence_or_hull < 0) {
					text::add_to_layout_box(state, contents, box, text::fp_two_places{mod.defence_or_hull}, text::text_color::red);
				} else {
					text::add_to_layout_box(state, contents, box, std::string_view{"+"}, text::text_color::green);
					text::add_to_layout_box(state, contents, box, text::fp_two_places{mod.defence_or_hull}, text::text_color::green);
				}
				text::close_layout_box(contents, box);
			}
			if(mod.reconnaissance_or_fire_range != 0) {
				auto box = text::open_layout_box(contents, indent + 15);
				unit_modifier_name_description(box);
				text::localised_format_box(state, contents, box, "gun_range");
				text::add_to_layout_box(state, contents, box, std::string_view{": "});
				if(mod.reconnaissance_or_fire_range < 0) {
					text::add_to_layout_box(state, contents, box, text::fp_two_places{mod.reconnaissance_or_fire_range}, text::text_color::red);
				} else {
					text::add_to_layout_box(state, contents, box, std::string_view{"+"}, text::text_color::green);
					text::add_to_layout_box(state, contents, box, text::fp_two_places{mod.reconnaissance_or_fire_range}, text::text_color::green);
				}
				text::close_layout_box(contents, box);
			}
			if(mod.siege_or_torpedo_attack != 0) {
				auto box = text::open_layout_box(contents, indent + 15);
				unit_modifier_name_description(box);
				text::localised_format_box(state, contents, box, "torpedo_attack");
				text::add_to_layout_box(state, contents, box, std::string_view{": "});
				if(mod.siege_or_torpedo_attack < 0) {
					text::add_to_layout_box(state, contents, box, text::fp_two_places{mod.siege_or_torpedo_attack}, text::text_color::red);
				} else {
					text::add_to_layout_box(state, contents, box, std::string_view{"+"}, text::text_color::green);
					text::add_to_layout_box(state, contents, box, text::fp_two_places{mod.siege_or_torpedo_attack}, text::text_color::green);
				}
				text::close_layout_box(contents, box);
			}
			if(mod.discipline_or_evasion != 0) {
				auto box = text::open_layout_box(contents, indent + 15);
				unit_modifier_name_description(box);
				text::localised_format_box(state, contents, box, "evasion");
				text::add_to_layout_box(state, contents, box, std::string_view{ ": " });
				if(mod.discipline_or_evasion < 0) {
					text::add_to_layout_box(state, contents, box, text::fp_percentage{ mod.discipline_or_evasion }, text::text_color::red);
				} else {
					text::add_to_layout_box(state, contents, box, std::string_view{ "+" }, text::text_color::green);
					text::add_to_layout_box(state, contents, box, text::fp_percentage{ mod.discipline_or_evasion }, text::text_color::green);
				}
				text::close_layout_box(contents, box);
			}
		}
	}

	template<typename T>
	void commodity_mod_description(sys::state& state, text::layout_base& contents, int32_t indent, const T& list, std::string_view locale_base_name, std::string_view locale_farm_base_name) {
		for(const auto mod : list) {
			auto box = text::open_layout_box(contents, indent);
			auto name = state.world.commodity_get_name(mod.type);
			text::add_to_layout_box(state, contents, box, name);
			text::add_space_to_layout_box(state, contents, box);
			text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, state.world.commodity_get_is_mine(mod.type) ? locale_base_name : locale_farm_base_name));
			text::add_to_layout_box(state, contents, box, std::string{":"});
			text::add_space_to_layout_box(state, contents, box);
			auto color = mod.amount > 0.f ? text::text_color::green : text::text_color::red;
			text::add_to_layout_box(state, contents, box, (mod.amount > 0.f ? "+" : "") + text::format_percentage(mod.amount, 1), color);
			text::close_layout_box(contents, box);
		}
	}

	template<typename T>
	void activate_unit_description(sys::state& state, text::layout_base& contents, int32_t indent, T fat_id, dcon::unit_type_id id) {
		if(dcon::fatten(state.world, fat_id).get_activate_unit(id)) {
			auto unit_type_name = state.military_definitions.unit_base_definitions[id].name;
			auto box = text::open_layout_box(contents, indent);
			text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, "enable_unit_tech"), text::text_color::white);
			text::add_space_to_layout_box(state, contents, box);
			text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, unit_type_name), text::text_color::yellow);
			text::close_layout_box(contents, box);
		}
	}

	template<typename T>
	auto activate_factory_description(sys::state& state, text::layout_base& contents, int32_t indent, T fat_id, dcon::factory_type_id id) {
		if(dcon::fatten(state.world, fat_id).get_activate_building(id)) {
			auto factory_type_fat_id = dcon::fatten(state.world, id);
			auto box = text::open_layout_box(contents, indent);
			text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, "enable_building_tech"), text::text_color::white);
			text::add_space_to_layout_box(state, contents, box);
			text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, factory_type_fat_id.get_name()), text::text_color::yellow);
			text::close_layout_box(contents, box);
			box = text::open_layout_box(contents, indent);
			text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, "activate_goods"), text::text_color::white);
			text::add_space_to_layout_box(state, contents, box);
			text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, factory_type_fat_id.get_output().get_name()), text::text_color::yellow);
			text::close_layout_box(contents, box);
		}
	}

	void invention_description(sys::state& state, text::layout_base& contents, dcon::invention_id inv_id, int32_t indent) noexcept {
		auto iid = fatten(state.world, inv_id);

		if(iid.get_modifier()) {
			modifier_description(state, contents, iid.get_modifier(), indent);
		}

		if(iid.get_enable_gas_attack()) {
			text::add_line(state, contents, "may_gas_attack", indent);
		}
		if(iid.get_enable_gas_defense()) {
			text::add_line(state, contents, "may_gas_defend", indent);
		}

		for(const auto t : state.world.in_province_building_type) {
			auto increase_building = iid.get_increase_building(t);
			if(increase_building) {
				auto box = text::open_layout_box(contents, 0);
				text::add_to_layout_box(state, contents, box, t.get_name(), text::text_color::white);
				text::add_space_to_layout_box(state, contents, box);
				text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, "tech_max_level"), text::text_color::white);
				text::add_to_layout_box(state, contents, box, std::string_view{ ":" }, text::text_color::white);
				text::add_space_to_layout_box(state, contents, box);
				text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, "+1"), text::text_color::green);
				text::close_layout_box(contents, box);
			}
		}

		if(auto p = iid.get_shared_prestige(); p > 0) {
			int32_t total = 1;
			for(auto n : state.world.in_nation) {
				if(n.get_active_inventions(iid)) {
					++total;
				}
			}
			auto box = text::open_layout_box(contents, indent);
			text::localised_format_box(state, contents, box, "shared_prestige_tech");
			text::add_to_layout_box(state, contents, box, std::string_view{": "});
			text::add_to_layout_box(state, contents, box, std::string_view{"+"}, text::text_color::green);
			text::add_to_layout_box( state, contents, box, text::fp_one_place { p / float(total) }, text::text_color::green);
			text::close_layout_box(contents, box);

		}
		if(iid.get_plurality() != 0) {
			auto box = text::open_layout_box(contents, 15);
			text::localised_format_box(state, contents, box, "tech_plurality");
			text::add_to_layout_box(state, contents, box, std::string_view{": "});
			if(iid.get_plurality() < 0) {
				text::add_to_layout_box(state, contents, box, text::fp_percentage{iid.get_plurality()}, text::text_color::red);
			} else {
				text::add_to_layout_box(state, contents, box, std::string_view{"+"}, text::text_color::green);
				text::add_to_layout_box(state, contents, box, text::fp_percentage{iid.get_plurality()}, text::text_color::green);
			}
			text::close_layout_box(contents, box);
		}

		for(uint8_t unit_type_i = 0; unit_type_i < iid.get_activate_unit_size(); ++unit_type_i) {
			dcon::unit_type_id id(unit_type_i);
			activate_unit_description(state, contents, indent, iid.id, id);
		}

		for(uint8_t building_type_i = 0; building_type_i < iid.get_activate_building_size(); ++building_type_i) {
			dcon::factory_type_id id(building_type_i);
			activate_factory_description(state, contents, indent, iid.id, id);
		}

		for(auto i = state.culture_definitions.crimes.size(); i-- > 0;) {
			dcon::crime_id c{dcon::crime_id::value_base_t(i)};
			if(iid.get_activate_crime(c)) {
				auto box = text::open_layout_box(contents, indent);
				text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, "enable_crime_tech"), text::text_color::white);
				text::add_to_layout_box(state, contents, box, state.culture_definitions.crimes[c].name, text::text_color::yellow);
				text::close_layout_box(contents, box);
			}
		}
		commodity_mod_description(state, contents, indent, iid.get_factory_goods_output(), "tech_output", "tech_output");
		commodity_mod_description(state, contents, indent, iid.get_rgo_goods_output(), "tech_mine_output", "tech_farm_output");
		commodity_mod_description(state, contents, indent, iid.get_factory_goods_throughput(), "tech_throughput", "tech_throughput");
		commodity_mod_description(state, contents, indent, iid.get_rgo_size(), "tech_mine_size", "tech_farm_size");

		auto colonial_points = iid.get_colonial_points();
		if(colonial_points != 0) {
			auto box = text::open_layout_box(contents, 0);
			text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, "colonial_points_tech"), text::text_color::white);
			text::add_to_layout_box(state, contents, box, std::string_view{ ":" }, text::text_color::white);
			text::add_space_to_layout_box(state, contents, box);
			auto color = colonial_points > 0.f ? text::text_color::green : text::text_color::red;
			text::add_to_layout_box(state, contents, box, (colonial_points > 0.f ? "+" : "") + text::prettify(int64_t(colonial_points)), color);
			text::close_layout_box(contents, box);
		}

		for(auto& mod : iid.get_modified_units()) {
			unit_modifier_description(state, contents, indent, mod);
		}

		for(auto& mod : iid.get_rebel_org()) {
			if(!mod.type) {
				auto box = text::open_layout_box(contents, indent);
				text::localised_format_box(state, contents, box, "tech_rebel_org_gain");
				text::add_to_layout_box(state, contents, box, std::string_view{": "});
				if(mod.amount < 0) {
					text::add_to_layout_box(state, contents, box, text::fp_percentage{mod.amount}, text::text_color::green);
				} else {
					text::add_to_layout_box(state, contents, box, std::string_view{"+"}, text::text_color::red);
					text::add_to_layout_box(state, contents, box, text::fp_percentage{mod.amount}, text::text_color::red);
				}
				text::close_layout_box(contents, box);
			} else {
				auto box = text::open_layout_box(contents, indent);
				text::add_to_layout_box(state, contents, box, state.world.rebel_type_get_title(mod.type));
				text::add_space_to_layout_box(state, contents, box);
				text::localised_format_box(state, contents, box, "blank_org_gain");
				text::add_to_layout_box(state, contents, box, std::string_view{": "});
				if(mod.amount < 0) {
					text::add_to_layout_box(state, contents, box, text::fp_percentage{mod.amount}, text::text_color::green);
				} else {
					text::add_to_layout_box(state, contents, box, std::string_view{"+"}, text::text_color::red);
					text::add_to_layout_box(state, contents, box, text::fp_percentage{mod.amount}, text::text_color::red);
				}
				text::close_layout_box(contents, box);
			}
		}
	}

	void technology_description(sys::state& state, text::layout_base& contents, dcon::technology_id tech_id) noexcept {
		auto tech_fat_id = dcon::fatten(state.world, tech_id);
		auto mod_id = tech_fat_id.get_modifier().id;
		if(mod_id) {
			modifier_description(state, contents, mod_id);
		}

		for(const auto t : state.world.in_province_building_type) {
			auto increase_building = tech_fat_id.get_increase_building(t);
			if(increase_building) {
				auto box = text::open_layout_box(contents, 0);
				text::add_to_layout_box(state, contents, box, t.get_name(), text::text_color::white);
				text::add_space_to_layout_box(state, contents, box);
				text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, "tech_max_level"), text::text_color::white);
				text::add_to_layout_box(state, contents, box, std::string_view{":"}, text::text_color::white);
				text::add_space_to_layout_box(state, contents, box);
				text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, "+1"), text::text_color::green);
				text::close_layout_box(contents, box);
			}
		}
		if(tech_fat_id.get_plurality() != 0) {
			auto box = text::open_layout_box(contents, 15);
			text::localised_format_box(state, contents, box, "tech_plurality");
			text::add_to_layout_box(state, contents, box, std::string_view{ ": " });
			if(tech_fat_id.get_plurality() < 0) {
				text::add_to_layout_box(state, contents, box, text::fp_percentage{ tech_fat_id.get_plurality() }, text::text_color::red);
			} else {
				text::add_to_layout_box(state, contents, box, std::string_view{ "+" }, text::text_color::green);
				text::add_to_layout_box(state, contents, box, text::fp_percentage{ tech_fat_id.get_plurality() }, text::text_color::green);
			}
			text::close_layout_box(contents, box);
		}

		for(uint8_t unit_type_i = 0; unit_type_i < tech_fat_id.get_activate_unit_size(); ++unit_type_i) {
			dcon::unit_type_id id(unit_type_i);
			activate_unit_description(state, contents, 0, tech_fat_id.id, id);
		}

		for(uint8_t building_type_i = 0; building_type_i < tech_fat_id.get_activate_building_size(); ++building_type_i) {
			dcon::factory_type_id id(building_type_i);
			activate_factory_description(state, contents, 0, tech_fat_id.id, id);
		}
		
		commodity_mod_description(state, contents, 0, tech_fat_id.get_factory_goods_output(), "tech_output", "tech_output");
		commodity_mod_description(state, contents, 0, tech_fat_id.get_rgo_goods_output(), "tech_mine_output", "tech_farm_output");
		commodity_mod_description(state, contents, 0, tech_fat_id.get_rgo_size(), "tech_mine_size", "tech_farm_size");

		auto colonial_points = tech_fat_id.get_colonial_points();
		if(colonial_points != 0) {
			auto box = text::open_layout_box(contents, 0);
			text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, "colonial_points_tech"), text::text_color::white);
			text::add_to_layout_box(state, contents, box, std::string_view{":"}, text::text_color::white);
			text::add_space_to_layout_box(state, contents, box);
			auto color = colonial_points > 0.f ? text::text_color::green : text::text_color::red;
			text::add_to_layout_box(state, contents, box, (colonial_points > 0.f ? "+" : "") + text::prettify(int64_t(colonial_points)), color);
			text::close_layout_box(contents, box);
		}

		for(auto& mod : tech_fat_id.get_modified_units()) {
			unit_modifier_description(state, contents, 0, mod);
		}

		bool an_invention = false;

		state.world.for_each_invention([&](dcon::invention_id id) {
			auto lim_trigger_k = state.world.invention_get_limit(id);
			bool activable_by_this_tech = false;
			trigger::recurse_over_triggers(state.trigger_data.data() + state.trigger_data_indices[lim_trigger_k.index() + 1], [&](uint16_t* tval) {
				if((tval[0] & trigger::code_mask) == trigger::technology && trigger::payload(tval[1]).tech_id == tech_id) {
					activable_by_this_tech = true;
				}
			});
			if(activable_by_this_tech) {
				if(!an_invention) {
					auto box = text::open_layout_box(contents);
					text::add_line_break_to_layout_box(state, contents, box);
					text::localised_format_box(state, contents, box, "be_invent");
					text::close_layout_box(contents, box);
					an_invention = true;
				}
				auto box = text::open_layout_box(contents);
				text::add_to_layout_box(state, contents, box, state.world.invention_get_name(id), text::text_color::yellow);
				text::close_layout_box(contents, box);
				invention_description(state, contents, id, 15);
			}
		});
	}

	bool technology_folder_tab_sub_button::is_active(sys::state& state) noexcept {
		return parent && parent->parent && static_cast<technology_window*>(parent->parent)->active_tab == category;
	}

	void technology_folder_tab_sub_button::button_action(sys::state& state) noexcept {
		if(parent) {
			Cyto::Any payload = category;
			parent->parent->impl_set(state, payload);
		}
	}

	message_result technology_item_window::set(sys::state& state, Cyto::Any& payload) noexcept {
		if(payload.holds_type<dcon::technology_id>()) {
			tech_id = any_cast<dcon::technology_id>(payload);
			auto tech = dcon::fatten(state.world, tech_id);
			category = state.culture_definitions.tech_folders[tech.get_folder_index()].category;
			return message_result::consumed;
		} else if(payload.holds_type<culture::tech_category>()) {
			auto enum_val = any_cast<culture::tech_category>(payload);
			set_visible(state, category == enum_val);
			return message_result::consumed;
		}
		return message_result::unseen;
	}

	void technology_possible_invention_listbox::on_update(sys::state& state) noexcept {
		row_contents.clear();
		state.world.for_each_invention([&](dcon::invention_id id) {
			auto lim_trigger_k = state.world.invention_get_limit(id);
			if(!state.world.nation_get_active_inventions(state.local_player_nation, id) && trigger::evaluate(state, lim_trigger_k, trigger::to_generic(state.local_player_nation),
						trigger::to_generic(state.local_player_nation), -1))
			row_contents.push_back(id);
		});

		auto sort_order = retrieve< invention_sort_type>(state, parent);
		switch(sort_order) {
		case invention_sort_type::name:
			sys::merge_sort(row_contents.begin(), row_contents.end(), [&](dcon::invention_id a, dcon::invention_id b) {
				auto a_name = text::produce_simple_string(state, dcon::fatten(state.world, a).get_name());
				auto b_name = text::produce_simple_string(state, dcon::fatten(state.world, b).get_name());
				if(a_name != b_name)
					return a_name < b_name;
				return a.index() < b.index();
			});
			break;
		case invention_sort_type::type:
			sys::merge_sort(row_contents.begin(), row_contents.end(), [&](dcon::invention_id a, dcon::invention_id b) {
				auto av = state.world.invention_get_technology_type(a);
				auto bv = state.world.invention_get_technology_type(b);
				if(av != bv)
					return av < bv;
				return a.index() < b.index();
			});
			break;
		case invention_sort_type::chance:
			sys::merge_sort(row_contents.begin(), row_contents.end(), [&](dcon::invention_id a, dcon::invention_id b) {
				auto mod_a = state.world.invention_get_chance(a);
				auto mod_b = state.world.invention_get_chance(b);
				if(mod_a != mod_b) {
					auto chances_a = trigger::evaluate_additive_modifier(state, mod_a, trigger::to_generic(state.local_player_nation), trigger::to_generic(state.local_player_nation), 0);
					auto chances_b = trigger::evaluate_additive_modifier(state, mod_b, trigger::to_generic(state.local_player_nation), trigger::to_generic(state.local_player_nation), 0);
					if(chances_a != chances_b) {
						return chances_a > chances_b;
					}
				}
				return a.index() < b.index();
			});
			break;
		}
		update(state);
	}
} // namespace ui
