#include "parsers_declarations.hpp"
#include "text.hpp"
#include <cmath>

namespace parsers {

	void make_good(std::string_view name, token_generator& gen, error_handler& err, good_group_context& context) {
		dcon::commodity_id new_id = context.outer_context.state.world.create_commodity();
		auto name_id = text::find_or_add_key(context.outer_context.state, name, false);
		context.outer_context.state.world.commodity_set_name(new_id, name_id);
		context.outer_context.state.world.commodity_set_commodity_group(new_id, context.group);
		context.outer_context.state.world.commodity_set_is_available_from_start(new_id, true);

		context.outer_context.map_of_commodity_names.insert_or_assign(std::string(name), new_id);
		good_context new_context{new_id, context.outer_context};
		parse_good(gen, err, new_context);
	}
	void make_goods_group(std::string_view name, token_generator& gen, error_handler& err, scenario_building_context& context) {
		if(auto it = context.map_of_commodity_group_names.find(std::string(name)); it != context.map_of_commodity_group_names.end()) {
			good_group_context new_context{ it->second, context };
			parse_goods_group(gen, err, new_context);
		} else {
			auto const id = dcon::commodity_group_id(uint8_t(context.state.commodity_group_names.size()));
			auto const name_k = context.state.add_key_win1252(name);
			context.state.commodity_group_names.push_back(name_k);
			context.map_of_commodity_group_names.insert_or_assign(std::string(name), id);
			// parse
			good_group_context new_context{ id, context };
			parse_goods_group(gen, err, new_context);
		}
	}

	void building_file::result(std::string_view name, building_definition&& res, error_handler& err, int32_t line, scenario_building_context& context) {
		// no, this messes things up
		// res.goods_cost.data.safe_get(dcon::commodity_id(0)) = float(res.cost);
		if(res.type == "factory") {
			auto factory_id = context.state.world.create_factory_type();
			context.map_of_factory_names.insert_or_assign(std::string(name), factory_id);

			auto desc_id = text::find_or_add_key(context.state, std::string(name) + "_desc", false);

			context.state.world.factory_type_set_name(factory_id, text::find_or_add_key(context.state, name, false));
			context.state.world.factory_type_set_description(factory_id, desc_id);
			context.state.world.factory_type_set_construction_time(factory_id, int16_t(res.time));
			context.state.world.factory_type_set_is_available_from_start(factory_id, res.default_enabled);
			context.state.world.factory_type_set_max_level(factory_id, res.max_level);
			/*for(uint32_t i = context.state.world.commodity_size(); i-- > 0; ) {
				dcon::commodity_id cid = dcon::commodity_id(dcon::commodity_id::value_base_t(i));
				context.state.world.factory_type_set_construction_costs(factory_id, cid, res.goods_cost.data[cid]);
			}*/
			uint32_t added = 0;
			auto& cc = context.state.world.factory_type_get_construction_costs(factory_id);
			context.state.world.for_each_commodity([&](dcon::commodity_id id) {
				auto amount = res.goods_cost.data.safe_get(id);
				if(amount > 0) {
					if(added >= economy::commodity_set::set_size) {
						err.accumulated_errors += "Too many factory cost goods in " + std::string(name) + " (" + err.file_name + ")\n";
					} else {
						cc.commodity_type[added] = id;
						cc.commodity_amounts[added] = amount;
						++added;
					}
				}
			});
			if(res.production_type.length() > 0) {
				context.map_of_production_types.insert_or_assign(std::string(res.production_type), factory_id);
			}
		} else {
			std::array<uint8_t, 8> colonial_points;
			for(uint32_t i = 0; i < 8 && i < res.colonial_points.data.size(); ++i) {
				colonial_points[i] = res.colonial_points.data[i];
			}
			economy::commodity_set cost;
			uint32_t added = 0;
			context.state.world.for_each_commodity([&](dcon::commodity_id id) {
				auto amount = res.goods_cost.data.safe_get(id);
				if(amount > 0) {
					if(added >= economy::commodity_set::set_size) {
						err.accumulated_warnings += "Too many special building cost goods in " + std::string(name) + " (" + err.file_name + ")\n";
					} else {
						cost.commodity_type[added] = id;
						cost.commodity_amounts[added] = amount;
						++added;
					}
				}
			});

			auto id = context.state.world.create_province_building_type();
			auto const name_k = text::find_or_add_key(context.state, name, false);
			context.state.world.province_building_type_set_colonial_points(id, colonial_points);
			context.state.world.province_building_type_set_colonial_range(id, res.colonial_range);
			context.state.world.province_building_type_set_cost(id, cost);
			context.state.world.province_building_type_set_infrastructure(id, res.infrastructure);
			context.state.world.province_building_type_set_max_level(id, res.max_level);
			context.state.world.province_building_type_set_time(id, res.time);
			context.state.world.province_building_type_set_name(id, name_k);
			if(res.next_to_add_p != 0) {
				auto const pmod = context.state.world.create_modifier();
				context.state.world.modifier_set_province_values(pmod, res.peek_province_mod());
				context.state.world.modifier_set_national_values(pmod, res.peek_national_mod());
				context.state.world.modifier_set_icon(pmod, uint8_t(res.icon_index));
				context.state.world.modifier_set_name(pmod, name_k);
				context.state.world.province_building_type_set_province_modifier(id, pmod);
			}
			if(res.type == "fort") {
				context.state.economy_definitions.fort_building = id;
			} else if(res.type == "naval_base") {
				context.state.economy_definitions.naval_base_building = id;
			} else if(res.type == "infrastructure") {
				context.state.economy_definitions.railroad_building = id;
			} else if(res.type == "bank") {
				context.state.economy_definitions.bank_building = id;
			} else if(res.type == "university") {
				context.state.economy_definitions.university_building = id;
			} else {
				err.accumulated_warnings += "Unsupported building type" + res.type + " (" + err.file_name + ")\n";
			}
			context.map_of_province_building_types.insert_or_assign(res.type, id);
		}
	}

	dcon::trigger_key make_production_bonus_trigger(token_generator& gen, error_handler& err, production_context& context) {
		trigger_building_context t_context{context.outer_context, trigger::slot_contents::state, trigger::slot_contents::nation,
			trigger::slot_contents::empty};
		return make_trigger(gen, err, t_context);
	}

	void make_production_type(std::string_view name, token_generator& gen, error_handler& err, production_context& context) {
		auto pt = parse_production_type(gen, err, context);

		if(!bool(pt.output_goods_)) { // must be a template
			if(pt.type_ == production_type_enum::factory && !pt.employees.employees.empty()) {
				if(!context.outer_context.state.culture_definitions.primary_factory_worker) {
					context.outer_context.state.culture_definitions.primary_factory_worker = pt.employees.employees[0].type;
				}
				if(!context.outer_context.state.culture_definitions.secondary_factory_worker) {
					if(pt.employees.employees.size() >= 2) {
						context.outer_context.state.culture_definitions.secondary_factory_worker = pt.employees.employees[1].type;
						//override, likely 2 worker types at once, amount should've been >0.1?
						context.outer_context.state.economy_definitions.craftsmen_fraction = std::clamp(pt.employees.employees[0].amount, 0.f, 1.f);
					} else if(pt.employees.employees[0].type != context.outer_context.state.culture_definitions.primary_factory_worker) {
						context.outer_context.state.culture_definitions.secondary_factory_worker = pt.employees.employees[0].type;
					}
				}
				if(!context.found_worker_types) {
					context.outer_context.state.economy_definitions.craftsmen_fraction = std::clamp(pt.employees.employees[0].amount, 0.f, 1.f);
				}
				context.found_worker_types = true;
			}
			if(pt.type_ == production_type_enum::rgo && bool(pt.owner.type)) {
				context.outer_context.state.culture_definitions.aristocrat = pt.owner.type;
				for(auto& wt : pt.employees.employees) {
					if(wt.type != context.outer_context.state.culture_definitions.slaves) {
						context.outer_context.state.world.pop_type_set_is_paid_rgo_worker(wt.type, true);
					}
				}
			}
			context.templates.insert_or_assign(std::string(name), std::move(pt));
		} else if(pt.type_ == production_type_enum::rgo) {
			context.outer_context.state.world.commodity_set_is_mine(pt.output_goods_, pt.mine);
			context.outer_context.state.world.commodity_set_rgo_amount(pt.output_goods_, pt.value);
			context.outer_context.state.world.commodity_set_rgo_workforce(pt.output_goods_, pt.workforce);
		} else if(pt.type_ == production_type_enum::artisan) {
			economy::commodity_set cset;
			uint32_t added = 0;
			context.outer_context.state.world.for_each_commodity([&](dcon::commodity_id id) {
				auto amount = pt.input_goods.data.safe_get(id);
				if(amount > 0) {
					if(added >= economy::commodity_set::set_size) {
						err.accumulated_errors += "Too many artisan input goods in" + std::string(name) + " (" + err.file_name + ")\n";
					} else {
						cset.commodity_type[added] = id;
						cset.commodity_amounts[added] = amount;
						++added;
					}
				}
			});
			context.outer_context.state.world.commodity_set_artisan_inputs(pt.output_goods_, cset);
			context.outer_context.state.world.commodity_set_artisan_output_amount(pt.output_goods_, pt.value);
		} else if(pt.type_ == production_type_enum::factory) {
			if(auto it = context.outer_context.map_of_production_types.find(std::string(name));
				it != context.outer_context.map_of_production_types.end()) {
				auto factory_handle = fatten(context.outer_context.state.world, it->second);

				economy::commodity_set cset;
				uint32_t added = 0;
				context.outer_context.state.world.for_each_commodity([&](dcon::commodity_id id) {
					auto amount = pt.input_goods.data.safe_get(id);
					if(amount > 0) {
						if(added >= economy::commodity_set::set_size) {
							err.accumulated_errors += "Too many factory input goods in " + std::string(name) + " (" + err.file_name + ")\n";
						} else {
							cset.commodity_type[added] = id;
							cset.commodity_amounts[added] = amount;
							++added;
						}
					}
				});

				economy::small_commodity_set sm_cset;
				uint32_t sm_added = 0;
				context.outer_context.state.world.for_each_commodity([&](dcon::commodity_id id) {
					auto amount = pt.efficiency.data.safe_get(id);
					if(amount > 0) {
						if(sm_added >= economy::small_commodity_set::set_size) {
							err.accumulated_errors += "Too many factory efficiency goods in " + std::string(name) + " (" + err.file_name + ")\n";
						} else {
							sm_cset.commodity_type[sm_added] = id;
							sm_cset.commodity_amounts[sm_added] = amount;
							++sm_added;
						}
					}
				});

				factory_handle.set_inputs(cset);
				factory_handle.set_efficiency_inputs(sm_cset);
				factory_handle.set_output(pt.output_goods_);
				factory_handle.set_output_amount(pt.value);
				factory_handle.set_is_coastal(pt.is_coastal);
				factory_handle.set_base_workforce(pt.workforce);

				if(pt.bonuses.size() > 0) {
					auto old_count = context.outer_context.state.value_modifier_segments.size();
					value_modifier_definition vmd;
					for(uint32_t i = 0; i < uint32_t(pt.bonuses.size()); i++) {
						context.outer_context.state.value_modifier_segments.push_back(sys::value_modifier_segment{pt.bonuses[i].value, pt.bonuses[i].trigger});
					}
					auto multiplier = vmd.factor ? *vmd.factor : 1.0f;
					auto overall_factor = vmd.base;
					auto new_count = context.outer_context.state.value_modifier_segments.size();
					auto vm_key = context.outer_context.state.value_modifiers.push_back(sys::value_modifier_description{ multiplier, overall_factor, uint16_t(old_count), uint16_t(new_count - old_count) });
					factory_handle.set_throughput_bonus(vm_key);
				}
				if(pt.input_bonuses.size() > 0) {
					auto old_count = context.outer_context.state.value_modifier_segments.size();
					value_modifier_definition vmd;
					for(uint32_t i = 0; i < uint32_t(pt.input_bonuses.size()); i++) {
						context.outer_context.state.value_modifier_segments.push_back(sys::value_modifier_segment{pt.input_bonuses[i].value, pt.input_bonuses[i].trigger});
					}
					auto multiplier = vmd.factor ? *vmd.factor : 1.0f;
					auto overall_factor = vmd.base;
					auto new_count = context.outer_context.state.value_modifier_segments.size();
					auto vm_key = context.outer_context.state.value_modifiers.push_back(sys::value_modifier_description{ multiplier, overall_factor, uint16_t(old_count), uint16_t(new_count - old_count) });
					factory_handle.set_input_bonus(vm_key);
				}
			} else {
				err.accumulated_warnings += "Unused factory production type: " + std::string(name) + "\n";
			}
		}
	}

	commodity_array make_prod_commodity_array(token_generator& gen, error_handler& err, production_context& context) {
		return parse_commodity_array(gen, err, context.outer_context);
	}

} // namespace parsers
