#include "trigger_parsing.hpp"
#include "parsers_declarations.hpp"
#include "script_constants.hpp"
#include <algorithm>

namespace parsers {
	inline std::string slot_contents_to_string(trigger::slot_contents v) {
		switch(v) {
		case trigger::slot_contents::empty:
			return "empty";
		case trigger::slot_contents::province:
			return "province";
		case trigger::slot_contents::state:
			return "state";
		case trigger::slot_contents::pop:
			return "pop";
		case trigger::slot_contents::nation:
			return "nation";
		case trigger::slot_contents::rebel:
			return "rebel";
		}
		return "unknown";
	}

	inline uint16_t association_to_trigger_code(association_type a) {
		switch(a) {
		case association_type::eq:
			return trigger::association_eq;
		case association_type::eq_default:
			return trigger::association_ge;
		case association_type::ge:
			return trigger::association_ge;
		case association_type::gt:
			return trigger::association_gt;
		case association_type::lt:
			return trigger::association_lt;
		case association_type::le:
			return trigger::association_le;
		case association_type::ne:
			return trigger::association_ne;
		case association_type::none:
			return trigger::association_ge;
		case association_type::list:
			return trigger::association_ge;
		default:
			return trigger::association_ge;
		}
	}

	inline uint16_t association_to_le_trigger_code(association_type a) {
		switch(a) {
		case association_type::eq:
			return trigger::association_eq;
		case association_type::eq_default:
			return trigger::association_le;
		case association_type::ge:
			return trigger::association_ge;
		case association_type::gt:
			return trigger::association_gt;
		case association_type::lt:
			return trigger::association_lt;
		case association_type::le:
			return trigger::association_le;
		case association_type::ne:
			return trigger::association_ne;
		case association_type::none:
			return trigger::association_le;
		case association_type::list:
			return trigger::association_le;
		default:
			return trigger::association_le;
		}
	}

	inline uint16_t association_to_bool_code(association_type a) {
		switch(a) {
		case association_type::eq:
			return trigger::association_eq;
		case association_type::eq_default:
			return trigger::association_eq;
		case association_type::ge:
			return trigger::association_eq;
		case association_type::gt:
			return trigger::association_ne;
		case association_type::lt:
			return trigger::association_ne;
		case association_type::le:
			return trigger::association_eq;
		case association_type::ne:
			return trigger::association_ne;
		case association_type::none:
			return trigger::association_ge;
		case association_type::list:
			return trigger::association_ge;
		default:
			return trigger::association_ge;
		}
	}

	inline uint16_t invert_association(uint16_t a) {
		return static_cast<uint16_t>(0x7000) - a;
	}

	inline uint16_t association_to_bool_code(association_type a, bool v) {
		if(v) {
			if((a == association_type::eq) || (a == association_type::eq_default)) {
				return trigger::association_eq;
			}
			return trigger::association_ne;
		} else {
			if((a == association_type::eq) || (a == association_type::eq_default)) {
				return trigger::association_ne;
			}
			return trigger::association_eq;
		}
	}

	constexpr bool scope_has_any_all(uint16_t code) {
		return (code >= trigger::x_neighbor_province_scope) && (code <= trigger::x_provinces_in_variable_region);
	}

	void tr_scope_and(token_generator& gen, error_handler& err, trigger_building_context& context) {
		context.compiled_trigger.push_back(uint16_t(trigger::generic_scope));
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;

		parse_trigger_body(gen, err, context);

		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}

	void tr_scope_or(token_generator& gen, error_handler& err, trigger_building_context& context) {
		context.compiled_trigger.push_back(uint16_t(trigger::generic_scope | trigger::is_disjunctive_scope));
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;

		parse_trigger_body(gen, err, context);

		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_not(token_generator& gen, error_handler& err, trigger_building_context& context) {
		context.compiled_trigger.push_back(uint16_t(trigger::placeholder_not_scope | trigger::is_disjunctive_scope));
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;

		parse_trigger_body(gen, err, context);

		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_any_neighbor_province(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_neighbor_province_scope | trigger::is_existence_scope));
			context.compiled_trigger.push_back(uint16_t(1));
			auto payload_size_offset = context.compiled_trigger.size() - 1;

			parse_trigger_body(gen, err, context);

			context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_neighbor_province_scope_state | trigger::is_existence_scope));
			context.compiled_trigger.push_back(uint16_t(1));
			auto payload_size_offset = context.compiled_trigger.size() - 1;

			parse_trigger_body(gen, err, context);

			context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
		} else {
			gen.discard_group();
			err.accumulated_errors += "any_neighbor_province trigger scope used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
	}
	void tr_scope_any_neighbor_country(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_neighbor_country_scope_nation | trigger::is_existence_scope));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_neighbor_country_scope_pop | trigger::is_existence_scope));
		} else {
			gen.discard_group();
			err.accumulated_errors += "any_neighbor_country trigger scope used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;

		auto old_main = context.main_slot;
		context.main_slot = trigger::slot_contents::nation;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;

		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_war_countries(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_war_countries_scope_nation | trigger::is_existence_scope));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_war_countries_scope_pop | trigger::is_existence_scope));
		} else {
			gen.discard_group();
			err.accumulated_errors += "war_countries trigger scope used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;
		auto old_main = context.main_slot;
		context.main_slot = trigger::slot_contents::nation;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;
		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_all_war_countries(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of trigger extension all_war_countries but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_war_countries_scope_nation));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_war_countries_scope_pop));
		} else {
			gen.discard_group();
			err.accumulated_errors += "all_war_countries trigger scope used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;
		auto old_main = context.main_slot;
		context.main_slot = trigger::slot_contents::nation;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;
		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_any_greater_power(token_generator& gen, error_handler& err, trigger_building_context& context) {
		context.compiled_trigger.push_back(uint16_t(trigger::x_greater_power_scope | trigger::is_existence_scope));
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;
		auto old_main = context.main_slot;
		context.main_slot = trigger::slot_contents::nation;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;
		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_any_country(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of trigger extension any_country but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		context.compiled_trigger.push_back(uint16_t(trigger::x_country_scope | trigger::is_existence_scope));
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;

		auto old_main = context.main_slot;
		context.main_slot = trigger::slot_contents::nation;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;

		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_every_country(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of trigger extension every_country but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		context.compiled_trigger.push_back(uint16_t(trigger::x_country_scope));
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;
		auto old_main = context.main_slot;
		context.main_slot = trigger::slot_contents::nation;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;
		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_any_owned_province(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_owned_province_scope_nation | trigger::is_existence_scope));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_owned_province_scope_state | trigger::is_existence_scope));
		} else if(context.main_slot == trigger::slot_contents::province) { // any province in a province -> generic scope
			context.compiled_trigger.push_back(uint16_t(trigger::generic_scope));
			context.compiled_trigger.push_back(uint16_t(1));
			auto payload_size_offset = context.compiled_trigger.size() - 1;
			parse_trigger_body(gen, err, context);
			context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
			return;
		} else {
			gen.discard_group();
			err.accumulated_errors += "any_owned_province trigger scope used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;

		auto old_main = context.main_slot;
		context.main_slot = trigger::slot_contents::province;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;

		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_any_core(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_core_scope_nation | trigger::is_existence_scope));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_core_scope_province | trigger::is_existence_scope));
		} else {
			gen.discard_group();
			err.accumulated_errors += "any_core trigger scope used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;

		auto old_main = context.main_slot;
		context.main_slot =
			(context.main_slot == trigger::slot_contents::nation) ? trigger::slot_contents::province : trigger::slot_contents::nation;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;

		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_all_core(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_core_scope_nation));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_core_scope_province));
		} else {
			gen.discard_group();
			err.accumulated_errors += "all_core trigger scope used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;

		auto old_main = context.main_slot;
		context.main_slot =
			(context.main_slot == trigger::slot_contents::nation) ? trigger::slot_contents::province : trigger::slot_contents::nation;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;

		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_any_state(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_state_scope | trigger::is_existence_scope));
			context.compiled_trigger.push_back(uint16_t(1));
			auto payload_size_offset = context.compiled_trigger.size() - 1;

			auto old_main = context.main_slot;
			context.main_slot = trigger::slot_contents::state;
			parse_trigger_body(gen, err, context);
			context.main_slot = old_main;

			context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
		} else {
			gen.discard_group();
			err.accumulated_errors += "any_state trigger scope used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
	}
	void tr_scope_any_substate(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_substate_scope | trigger::is_existence_scope));
			context.compiled_trigger.push_back(uint16_t(1));
			auto payload_size_offset = context.compiled_trigger.size() - 1;

			parse_trigger_body(gen, err, context);

			context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
		} else {
			gen.discard_group();
			err.accumulated_errors += "any_substate trigger scope used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
	}
	void tr_scope_any_sphere_member(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_sphere_member_scope | trigger::is_existence_scope));
			context.compiled_trigger.push_back(uint16_t(1));
			auto payload_size_offset = context.compiled_trigger.size() - 1;

			parse_trigger_body(gen, err, context);

			context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
		} else {
			gen.discard_group();
			err.accumulated_errors += "any_sphere_member trigger scope used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
	}
	void tr_scope_any_pop(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_pop_scope_nation | trigger::is_existence_scope));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_pop_scope_province | trigger::is_existence_scope));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_pop_scope_state | trigger::is_existence_scope));
		} else {
			gen.discard_group();
			err.accumulated_errors += "any_pop trigger scope used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;

		auto old_main = context.main_slot;
		context.main_slot = trigger::slot_contents::pop;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;

		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_all_state(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of trigger extension all_state but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_state_scope));
			context.compiled_trigger.push_back(uint16_t(1));
			auto payload_size_offset = context.compiled_trigger.size() - 1;
			auto old_main = context.main_slot;
			context.main_slot = trigger::slot_contents::state;
			parse_trigger_body(gen, err, context);
			context.main_slot = old_main;
			context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
		} else {
			gen.discard_group();
			err.accumulated_errors += "all_state trigger scope used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
	}
	void tr_scope_all_substate(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of trigger extension all_substate but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_substate_scope));
			context.compiled_trigger.push_back(uint16_t(1));
			auto payload_size_offset = context.compiled_trigger.size() - 1;
			parse_trigger_body(gen, err, context);
			context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
		} else {
			gen.discard_group();
			err.accumulated_errors += "all_substate trigger scope used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
	}
	void tr_scope_all_sphere_member(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of trigger extension all_pshere_member but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_sphere_member_scope));
			context.compiled_trigger.push_back(uint16_t(1));
			auto payload_size_offset = context.compiled_trigger.size() - 1;
			parse_trigger_body(gen, err, context);
			context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
		} else {
			gen.discard_group();
			err.accumulated_errors += "all_sphere_member trigger scope used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
	}
	void tr_scope_all_pop(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of trigger extension all_pop but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_pop_scope_nation));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_pop_scope_province));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_pop_scope_state));
		} else {
			gen.discard_group();
			err.accumulated_errors += "all_pop trigger scope used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;
		auto old_main = context.main_slot;
		context.main_slot = trigger::slot_contents::pop;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;
		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_all_greater_power(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of trigger extension all_greater_power but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		context.compiled_trigger.push_back(uint16_t(trigger::x_greater_power_scope));
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;
		auto old_main = context.main_slot;
		context.main_slot = trigger::slot_contents::nation;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;
		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_owner(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::owner_scope_province));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::owner_scope_state));
		} else if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::generic_scope));
			context.compiled_trigger.push_back(uint16_t(1));
			auto payload_size_offset = context.compiled_trigger.size() - 1;
			parse_trigger_body(gen, err, context);
			context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
			return;
		} else {
			gen.discard_group();
			err.accumulated_errors += "owner trigger scope used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;

		auto old_main = context.main_slot;
		context.main_slot = trigger::slot_contents::nation;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;

		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_controller(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::controller_scope));
		} else {
			gen.discard_group();
			err.accumulated_errors += "controller trigger scope used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;

		auto old_main = context.main_slot;
		context.main_slot = trigger::slot_contents::nation;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;

		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_location(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::location_scope));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::generic_scope));
			context.compiled_trigger.push_back(uint16_t(1));
			auto payload_size_offset = context.compiled_trigger.size() - 1;

			parse_trigger_body(gen, err, context);

			context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
			return;
		} else {
			gen.discard_group();
			err.accumulated_errors += "location trigger scope used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;

		auto old_main = context.main_slot;
		context.main_slot = trigger::slot_contents::province;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;

		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_country(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::country_scope_province));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::country_scope_state));
		} else if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::country_scope_nation));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::country_scope_pop));
		} else {
			gen.discard_group();
			err.accumulated_errors += "country trigger scope used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;

		auto old_main = context.main_slot;
		context.main_slot = trigger::slot_contents::nation;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;

		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_capital_scope(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::capital_scope));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::capital_scope_province));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::capital_scope_pop));
		} else {
			gen.discard_group();
			err.accumulated_errors += "capital_scope trigger scope used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;

		auto old_main = context.main_slot;
		context.main_slot = trigger::slot_contents::province;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;

		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_this(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.this_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::this_scope_province));
		} else if(context.this_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::this_scope_state));
		} else if(context.this_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::this_scope_nation));
		} else if(context.this_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::this_scope_pop));
		} else {
			gen.discard_group();
			err.accumulated_errors += "'this' trigger scope used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;

		auto old_main = context.main_slot;
		context.main_slot = context.this_slot;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;

		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_from(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.from_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::from_scope_province));
		} else if(context.from_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::from_scope_state));
		} else if(context.from_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::from_scope_nation));
		} else if(context.from_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::from_scope_pop));
		} else {
			gen.discard_group();
			err.accumulated_errors += "'from' trigger scope used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;

		auto old_main = context.main_slot;
		context.main_slot = context.from_slot;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;

		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_sea_zone(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::sea_zone_scope));
			context.compiled_trigger.push_back(uint16_t(1));
			auto payload_size_offset = context.compiled_trigger.size() - 1;

			parse_trigger_body(gen, err, context);

			context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
		} else {
			gen.discard_group();
			err.accumulated_errors += "sea_zone trigger scope used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
	}
	void tr_scope_cultural_union(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::cultural_union_scope));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::cultural_union_scope_pop));
		} else {
			gen.discard_group();
			err.accumulated_errors += "cultural_union trigger scope used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;

		auto old_main = context.main_slot;
		context.main_slot = trigger::slot_contents::nation;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;

		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_overlord(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::overlord_scope));
			context.compiled_trigger.push_back(uint16_t(1));
			auto payload_size_offset = context.compiled_trigger.size() - 1;

			parse_trigger_body(gen, err, context);

			context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
		} else {
			gen.discard_group();
			err.accumulated_errors += "overlord trigger scope used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
	}
	void tr_scope_sphere_owner(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::sphere_owner_scope));
			context.compiled_trigger.push_back(uint16_t(1));
			auto payload_size_offset = context.compiled_trigger.size() - 1;

			parse_trigger_body(gen, err, context);

			context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
		} else {
			gen.discard_group();
			err.accumulated_errors += "sphere_owner trigger scope used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
	}
	void tr_scope_independence(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.from_slot == trigger::slot_contents::rebel) {
			context.compiled_trigger.push_back(uint16_t(trigger::independence_scope));
			context.compiled_trigger.push_back(uint16_t(1));
			auto payload_size_offset = context.compiled_trigger.size() - 1;

			auto old_main = context.main_slot;
			context.main_slot = trigger::slot_contents::nation;
			parse_trigger_body(gen, err, context);
			context.main_slot = old_main;

			context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
		} else {
			gen.discard_group();
			err.accumulated_errors += "independence trigger scope used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
	}
	void tr_flashpoint_tag_scope(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::flashpoint_tag_scope));
			context.compiled_trigger.push_back(uint16_t(1));
			auto payload_size_offset = context.compiled_trigger.size() - 1;

			auto old_main = context.main_slot;
			context.main_slot = trigger::slot_contents::nation;
			parse_trigger_body(gen, err, context);
			context.main_slot = old_main;

			context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
		} else {
			gen.discard_group();
			err.accumulated_errors += "flashpoint_tag_scope trigger scope used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
	}
	void tr_crisis_state_scope(token_generator& gen, error_handler& err, trigger_building_context& context) {
		context.compiled_trigger.push_back(uint16_t(trigger::crisis_state_scope));
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;

		auto old_main = context.main_slot;
		context.main_slot = trigger::slot_contents::state;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;

		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_crisis_attacker_scope(token_generator& gen, error_handler& err, trigger_building_context& context) {
		context.compiled_trigger.push_back(uint16_t(trigger::crisis_attacker_scope));
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;

		auto old_main = context.main_slot;
		context.main_slot = trigger::slot_contents::state;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;
		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_crisis_defender_scope(token_generator& gen, error_handler& err, trigger_building_context& context) {
		context.compiled_trigger.push_back(uint16_t(trigger::crisis_defender_scope));
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;

		auto old_main = context.main_slot;
		context.main_slot = trigger::slot_contents::state;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;
		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_state_scope(token_generator& gen, error_handler& err, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::generic_scope));
			context.compiled_trigger.push_back(uint16_t(1));
			auto payload_size_offset = context.compiled_trigger.size() - 1;

			parse_trigger_body(gen, err, context);

			context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
			return;
		}

		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::state_scope_province));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::state_scope_pop));
		} else {
			gen.discard_group();
			err.accumulated_errors += "state_scope trigger scope used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = context.compiled_trigger.size() - 1;

		auto old_main = context.main_slot;
		context.main_slot = trigger::slot_contents::state;
		parse_trigger_body(gen, err, context);
		context.main_slot = old_main;

		context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
	}
	void tr_scope_variable(std::string_view name, token_generator& gen, error_handler& err, trigger_building_context& context) {
	std::string label_str{name};
		if(auto it = context.outer_context.map_of_state_names.find(label_str); it != context.outer_context.map_of_state_names.end()) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_provinces_in_variable_region));

			context.compiled_trigger.push_back(uint16_t(1));
			auto payload_size_offset = context.compiled_trigger.size() - 1;

			context.compiled_trigger.push_back(trigger::payload(it->second).value);

			auto old_main = context.main_slot;
			context.main_slot = trigger::slot_contents::province;
			parse_trigger_body(gen, err, context);
			context.main_slot = old_main;

			context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
		} else if(auto itr = context.outer_context.map_of_region_names.find(label_str); itr != context.outer_context.map_of_region_names.end()) {
			context.compiled_trigger.push_back(uint16_t(trigger::x_provinces_in_variable_region_proper));

			context.compiled_trigger.push_back(uint16_t(1));
			auto payload_size_offset = context.compiled_trigger.size() - 1;

			context.compiled_trigger.push_back(trigger::payload(itr->second).value);

			auto old_main = context.main_slot;
			context.main_slot = trigger::slot_contents::province;
			parse_trigger_body(gen, err, context);
			context.main_slot = old_main;

			context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
		} else if(is_integer(name.data(), name.data() + name.length())) {
			auto int_value = parse_int(name, 0, err);
			if(0 <= int_value && size_t(int_value) < context.outer_context.original_id_to_prov_id_map.size()) {
				context.compiled_trigger.push_back(uint16_t(trigger::integer_scope));
				context.compiled_trigger.push_back(uint16_t(1));
				auto payload_size_offset = context.compiled_trigger.size() - 1;
				context.compiled_trigger.push_back(trigger::payload(context.outer_context.original_id_to_prov_id_map[int_value]).value);

				auto old_main = context.main_slot;
				context.main_slot = trigger::slot_contents::province;
				parse_trigger_body(gen, err, context);
				context.main_slot = old_main;

				context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
			} else {
				err.accumulated_errors += "integer trigger scope given an invalid province id (" + err.file_name + ")\n";
				context.compiled_trigger.push_back(trigger::payload(dcon::province_id()).value);
			}

		} else if(name.length() == 3) {
			if(auto itb = context.outer_context.map_of_ident_names.find(nations::tag_to_int(name[0], name[1], name[2]));
				itb != context.outer_context.map_of_ident_names.end()) {
				context.compiled_trigger.push_back(uint16_t(trigger::tag_scope));
				context.compiled_trigger.push_back(uint16_t(1));
				auto payload_size_offset = context.compiled_trigger.size() - 1;
				context.compiled_trigger.push_back(trigger::payload(itb->second).value);

				auto old_main = context.main_slot;
				context.main_slot = trigger::slot_contents::nation;
				parse_trigger_body(gen, err, context);
				context.main_slot = old_main;

				context.compiled_trigger[payload_size_offset] = uint16_t(context.compiled_trigger.size() - payload_size_offset);
			} else {
				err.accumulated_errors += "unknown scope " + label_str + " introduced in a trigger (" + err.file_name + ")\n";
				gen.discard_group();
				return;
			}
		} else {
			err.accumulated_errors += "unknown scope " + label_str + " introduced in a trigger (" + err.file_name + ")\n";
			gen.discard_group();
			return;
		}
	}

	inline void invert_trigger_internal(uint16_t* source) {
		if((source[0] & trigger::code_mask) >= trigger::first_scope_code) {
			auto const neg_disjunctive_bit = trigger::is_disjunctive_scope & ~source[0];
			auto const neg_existence_bit = scope_has_any_all(source[0] & trigger::code_mask) ? (trigger::is_existence_scope & ~source[0]) : 0;
			auto const masked_source = source[0] & ~(trigger::is_disjunctive_scope | trigger::is_existence_scope);

			source[0] = uint16_t(masked_source | neg_disjunctive_bit | neg_existence_bit);
		} else {
			auto const inverse_association = invert_association(uint16_t(source[0] & trigger::association_mask));
			source[0] = uint16_t((source[0] & ~trigger::association_mask) | inverse_association);
		}
	}

	void invert_trigger(uint16_t* source) {
		trigger::recurse_over_triggers(source, invert_trigger_internal);
	}

	bool scope_is_empty(uint16_t const* source) {
		return trigger::get_trigger_scope_payload_size(source) <= 1 + trigger::trigger_scope_data_payload(source[0]);
	}
	// precondition: scope known to not be empty
	bool scope_has_single_member(uint16_t const* source) {
		auto const data_offset = 2 + trigger::trigger_scope_data_payload(source[0]);
		return trigger::get_trigger_scope_payload_size(source) == data_offset + trigger::get_trigger_payload_size(source + data_offset);
	}

	// yields new source size
	int32_t simplify_trigger(uint16_t* source) {
		assert((0 <= (*source & trigger::code_mask) && (*source & trigger::code_mask) < trigger::first_invalid_code) ||
				 (*source & trigger::code_mask) == trigger::placeholder_not_scope);
		if((source[0] & trigger::code_mask) >= trigger::first_scope_code) {
			if(scope_is_empty(source)) {
				return 0; // simplify an empty scope to nothing
			}

			// simplify each member
			auto source_size = 1 + trigger::get_trigger_scope_payload_size(source);
			auto const first_member = source + 2 + trigger::trigger_scope_data_payload(source[0]);

			{
				auto sub_units_start = first_member;
				while(sub_units_start < source + source_size) {
					auto const old_size = 1 + trigger::get_trigger_payload_size(sub_units_start);
					auto const new_size = simplify_trigger(sub_units_start);

					if(new_size != old_size) { // has been simplified, assumes that new size always <= old size
						std::copy(sub_units_start + old_size, source + source_size, sub_units_start + new_size);
						source_size -= (old_size - new_size);
					}
					sub_units_start += new_size;
				}
			}

			source[1] = uint16_t(source_size - 1);

			if((source[0] & trigger::code_mask) == trigger::placeholder_not_scope) { // remove not scopes
				invert_trigger(source);
				source[0] &= ~trigger::code_mask;
				/*
				if((source[0] & trigger::is_disjunctive_scope) != 0) {
					source[0] &= ~trigger::is_disjunctive_scope;
				} else {
					source[0] |= trigger::is_disjunctive_scope;
				}
				*/
				source[0] |= trigger::generic_scope;
			}

			if(source[0] == trigger::generic_scope || source[0] == (trigger::generic_scope | trigger::is_disjunctive_scope)) {
				auto sub_units_start = first_member;
				while(sub_units_start < source + source_size) {
					auto const size = 1 + trigger::get_trigger_payload_size(sub_units_start);
					if(sub_units_start[0] == source[0]) {
						std::copy(sub_units_start + 2, source + source_size, sub_units_start);
						source_size -= 2;
					} else {
						sub_units_start += size;
					}
				}
				source[1] = uint16_t(source_size - 1);
			}

			if((source[0] & trigger::code_mask) >= trigger::first_scope_code && scope_has_single_member(source)) {
				if((source[0] & trigger::code_mask) == trigger::generic_scope) { // remove single-member generic scopes
					std::copy(source + 2, source + source_size, source);
					source_size -= 2;
				} else if((first_member[0] & trigger::code_mask) == trigger::generic_scope) {
					// scope contains single generic scope

					source[1] = uint16_t(first_member[1] + trigger::trigger_scope_data_payload(source[0]));
					source[0] = uint16_t((source[0] & ~trigger::is_disjunctive_scope) | (first_member[0] & trigger::is_disjunctive_scope));

					std::copy(first_member + 2, source + source_size, first_member);

					source_size -= 2;
				}
			}

			if((source[0] & trigger::code_mask) >= trigger::first_scope_code && scope_is_empty(source)) {
				return 0; // simplify an empty scope to nothing
			}
			return source_size;
		} else {
			return 1 + trigger::get_trigger_non_scope_payload_size(source); // non scopes cannot be simplified
		}
	}

	dcon::trigger_key make_trigger(token_generator& gen, error_handler& err, trigger_building_context& context) {
		tr_scope_and(gen, err, context);

		auto const new_size = simplify_trigger(context.compiled_trigger.data());
		context.compiled_trigger.resize(static_cast<size_t>(new_size));

		return context.outer_context.state.commit_trigger_data(context.compiled_trigger);
	}

	void make_stored_trigger(std::string_view name, token_generator& gen, error_handler& err, scenario_building_context& context) {
	trigger_building_context tcontext{ context , trigger::slot_contents::empty, trigger::slot_contents::empty , trigger::slot_contents::empty };

		tcontext.compiled_trigger.push_back(uint16_t(trigger::generic_scope));
		tcontext.compiled_trigger.push_back(uint16_t(1));
		auto payload_size_offset = tcontext.compiled_trigger.size() - 1;

		parse_stored_condition(gen, err, tcontext);

		tcontext.compiled_trigger[payload_size_offset] = uint16_t(tcontext.compiled_trigger.size() - payload_size_offset);

		auto const new_size = simplify_trigger(tcontext.compiled_trigger.data());
		tcontext.compiled_trigger.resize(static_cast<size_t>(new_size));

		auto by_name = context.map_of_stored_triggers.find(std::string(name));

		bool can_store = (by_name == context.map_of_stored_triggers.end());
		if(!can_store) {
			can_store = true;
			auto& prev_stored = by_name->second;
			for(auto e : prev_stored) {
				if(e.main_slot == tcontext.main_slot && e.from_slot == tcontext.from_slot && e.this_slot == tcontext.this_slot) {
					err.accumulated_errors += "two stored triggers with the name " + std::string(name) + " defined for the same types of parameters (" + err.file_name + ")\n";
					can_store = false;
				}
			}
		}

		if(can_store) {
			auto saved_trigger = context.state.commit_trigger_data(tcontext.compiled_trigger);
			auto name_id = text::find_or_add_key(context.state, name, false);

			auto stored_t_id = context.state.world.create_stored_trigger();
			context.state.world.stored_trigger_set_function(stored_t_id, saved_trigger);
			context.state.world.stored_trigger_set_name(stored_t_id, name_id);

			if(by_name == context.map_of_stored_triggers.end()) {
			context.map_of_stored_triggers.insert_or_assign(std::string(name), std::vector<saved_stored_condition>{ saved_stored_condition{ stored_t_id, tcontext.main_slot, tcontext.this_slot, tcontext.from_slot } });
			} else {
			by_name->second.push_back(saved_stored_condition{ stored_t_id, tcontext.main_slot, tcontext.this_slot, tcontext.from_slot });
			}
		}
	}

	void make_value_modifier_segment(token_generator& gen, error_handler& err, trigger_building_context& context) {
		auto old_factor = context.factor;
		context.factor = 0.0f;

		tr_scope_and(gen, err, context);

		auto new_factor = context.factor;
		context.factor = old_factor;

		auto const new_size = simplify_trigger(context.compiled_trigger.data());
		context.compiled_trigger.resize(static_cast<size_t>(new_size));

		auto tkey = context.outer_context.state.commit_trigger_data(context.compiled_trigger);
		context.compiled_trigger.clear();

		context.outer_context.state.value_modifier_segments.push_back(sys::value_modifier_segment{new_factor, tkey});
	}

	dcon::value_modifier_key make_value_modifier(token_generator& gen, error_handler& err, trigger_building_context& context) {
		auto old_count = context.outer_context.state.value_modifier_segments.size();
		value_modifier_definition result = parse_value_modifier_definition(gen, err, context);

		auto multiplier = result.factor ? *result.factor : 1.0f;
		auto overall_factor = result.base;
		auto new_count = context.outer_context.state.value_modifier_segments.size();

		return context.outer_context.state.value_modifiers.push_back(
			sys::value_modifier_description{ multiplier, overall_factor, uint16_t(old_count), uint16_t(new_count - old_count)});
	}

	void tr_party_name::ideology(association_type t, std::string_view v, error_handler & err, int32_t line, trigger_building_context & context) {
		if(is_fixed_token_ci(v.data(), v.data() + v.length(), "ruling_party")) {
			// leave invalid
		} else if(auto it = context.outer_context.map_of_ideologies.find(std::string(v)); it != context.outer_context.map_of_ideologies.end()) {
			ideology_ = it->second.id;
		} else {
			err.accumulated_errors += "Invalid ideology " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	}
	void tr_party_name::name(association_type t, std::string_view v, error_handler& err, int32_t line, trigger_building_context& context) {
		name_ = text::find_or_add_key(context.outer_context.state, v, false);
	}

	void tr_party_position::ideology(association_type t, std::string_view v, error_handler& err, int32_t line, trigger_building_context& context) {
		if(is_fixed_token_ci(v.data(), v.data() + v.length(), "ruling_party")) {
			// leave invalid
		} else if(auto it = context.outer_context.map_of_ideologies.find(std::string(v)); it != context.outer_context.map_of_ideologies.end()) {
			ideology_ = it->second.id;
		} else {
			err.accumulated_errors += "Invalid ideology " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	}

	void trigger_body::is_canal_enabled(association_type a, int32_t value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(1 <= value && value <= int32_t(context.outer_context.state.province_definitions.canals.size())) {
			context.compiled_trigger.push_back(uint16_t(trigger::is_canal_enabled | association_to_bool_code(a)));
			context.compiled_trigger.push_back(trigger::payload(uint16_t(value)).value);
		} else {
			err.accumulated_errors += "canal index " + std::to_string(value) + " out of range (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}

	void trigger_body::badboy(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::badboy | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "badboy trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + "(" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value * context.outer_context.state.defines.badboy_limit);
	}

	void trigger_body::ruling_party(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {

			context.compiled_trigger.push_back(uint16_t(trigger::ruling_party | association_to_bool_code(a)));
		} else {
			err.accumulated_errors += "ruling_party trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		auto name_id = text::find_or_add_key(context.outer_context.state, value, false);
		context.add_int32_t_to_payload(name_id.index());
	}

	void trigger_body::has_leader(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::has_leader | association_to_bool_code(a)));
		} else {
			err.accumulated_errors += "has_leader trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
															"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		auto name_id = context.outer_context.state.add_unit_name(value);
		context.add_int32_t_to_payload(name_id.index());
	}

	// news xxx_eq
	void trigger_body::tags_eq(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(value.list.size() < 3) {
			err.accumulated_errors += "tags_eq trigger supplied with insufficient parameters (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(trigger::tags_eq));
		context.add_int32_t_to_payload(parse_int(value.list[0], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[1], line, err));
		if(auto it = context.outer_context.map_of_ident_names.find(parse_tag(value.list[2], line, err));
			it != context.outer_context.map_of_ident_names.end()) {
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			err.accumulated_errors += "tags_eq trigger supplied with an invalid tag \"" + std::string(value.list[2]) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::strings_eq(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(value.list.size() < 3) {
			err.accumulated_errors += "strings_eq trigger supplied with insufficient parameters (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(trigger::strings_eq));
		context.add_int32_t_to_payload(parse_int(value.list[0], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[1], line, err));
		auto tkey = text::find_or_add_key(context.outer_context.state, value.list[2], false);
		context.add_int32_t_to_payload(tkey.index());
	}
	void trigger_body::values_eq(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(value.list.size() < 3) {
			err.accumulated_errors += "values_eq trigger supplied with insufficient parameters (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(trigger::values_eq));
		context.add_int32_t_to_payload(parse_int(value.list[0], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[1], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[2], line, err));
	}
	void trigger_body::dates_eq(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(value.list.size() < 3) {
			err.accumulated_errors += "dates_eq trigger supplied with insufficient parameters (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(trigger::dates_eq));
		context.add_int32_t_to_payload(parse_int(value.list[0], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[1], line, err));
		auto d = sys::date(parse_date(value.list[2], line, err), context.outer_context.state.start_date);
		context.add_int32_t_to_payload(d.value);
	}

	//news xxx_greater
	void trigger_body::tags_greater(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(value.list.size() < 3) {
			err.accumulated_errors += "tags_greater trigger supplied with insufficient parameters (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(trigger::tags_eq | trigger::association_gt));
		context.add_int32_t_to_payload(parse_int(value.list[0], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[1], line, err));
		if(auto it = context.outer_context.map_of_ident_names.find(parse_tag(value.list[2], line, err));
			it != context.outer_context.map_of_ident_names.end()) {
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			err.accumulated_errors += "tags_greater trigger supplied with an invalid tag \"" + std::string(value.list[2]) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::strings_greater(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(value.list.size() < 3) {
			err.accumulated_errors += "strings_greater trigger supplied with insufficient parameters (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(trigger::strings_eq | trigger::association_gt));
		context.add_int32_t_to_payload(parse_int(value.list[0], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[1], line, err));
		auto tkey = text::find_or_add_key(context.outer_context.state, value.list[2], false);
		context.add_int32_t_to_payload(tkey.index());
	}
	void trigger_body::values_greater(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(value.list.size() < 3) {
			err.accumulated_errors += "values_greater trigger supplied with insufficient parameters (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(trigger::values_eq | trigger::association_gt));
		context.add_int32_t_to_payload(parse_int(value.list[0], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[1], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[2], line, err));
	}
	void trigger_body::dates_greater(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(value.list.size() < 3) {
			err.accumulated_errors += "dates_greater trigger supplied with insufficient parameters (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(trigger::dates_eq | trigger::association_gt));
		context.add_int32_t_to_payload(parse_int(value.list[0], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[1], line, err));
		auto d = sys::date(parse_date(value.list[2], line, err), context.outer_context.state.start_date);
		context.add_int32_t_to_payload(d.value);
	}

	//news xxx_match
	void trigger_body::tags_match(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(value.list.size() < 3) {
			err.accumulated_errors += "tags_match trigger supplied with insufficient parameters (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(trigger::tags_match));
		context.add_int32_t_to_payload(parse_int(value.list[0], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[1], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[2], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[3], line, err));
	}
	void trigger_body::strings_match(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(value.list.size() < 3) {
			err.accumulated_errors += "strings_match trigger supplied with insufficient parameters (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(trigger::strings_match));
		context.add_int32_t_to_payload(parse_int(value.list[0], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[1], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[2], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[3], line, err));
	}
	void trigger_body::values_match(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(value.list.size() < 3) {
			err.accumulated_errors += "values_match trigger supplied with insufficient parameters (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(trigger::values_match));
		context.add_int32_t_to_payload(parse_int(value.list[0], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[1], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[2], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[3], line, err));
	}
	void trigger_body::dates_match(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(value.list.size() < 3) {
			err.accumulated_errors += "dates_match trigger supplied with insufficient parameters (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(trigger::dates_match));
		context.add_int32_t_to_payload(parse_int(value.list[0], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[1], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[2], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[3], line, err));
	}

	//news xxx_greater
	void trigger_body::tags_contains(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(value.list.size() < 2) {
			err.accumulated_errors += "tags_contains trigger supplied with insufficient parameters (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(trigger::tags_contains));
		context.add_int32_t_to_payload(parse_int(value.list[0], line, err));
		if(auto it = context.outer_context.map_of_ident_names.find(parse_tag(value.list[1], line, err));
			it != context.outer_context.map_of_ident_names.end()) {
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			err.accumulated_errors += "tags_contains trigger supplied with an invalid tag \"" + std::string(value.list[2]) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::strings_contains(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(value.list.size() < 2) {
			err.accumulated_errors += "strings_contains trigger supplied with insufficient parameters (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(trigger::strings_contains));
		context.add_int32_t_to_payload(parse_int(value.list[0], line, err));
		auto tkey = text::find_or_add_key(context.outer_context.state, value.list[1], false);
		context.add_int32_t_to_payload(tkey.index());
	}
	void trigger_body::values_contains(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(value.list.size() < 2) {
			err.accumulated_errors += "values_contains trigger supplied with insufficient parameters (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(trigger::values_contains));
		context.add_int32_t_to_payload(parse_int(value.list[0], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[1], line, err));
	}
	void trigger_body::dates_contains(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(value.list.size() < 2) {
			err.accumulated_errors += "dates_contains trigger supplied with insufficient parameters (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(trigger::dates_contains));
		context.add_int32_t_to_payload(parse_int(value.list[0], line, err));
		auto d = sys::date(parse_date(value.list[2], line, err), context.outer_context.state.start_date);
		context.add_int32_t_to_payload(d.value);
	}

	void trigger_body::length_greater(tr_news_data_parameters const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(value.list.size() < 3) {
			err.accumulated_errors += "length_greater trigger supplied with insufficient parameters (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		if(is_fixed_token_ci(value.list[0].data(), value.list[0].data() + value.list[0].size(), "tags")
		|| is_fixed_token_ci(value.list[0].data(), value.list[0].data() + value.list[0].size(), "tag")) {
			context.compiled_trigger.push_back(uint16_t(trigger::length_greater_tags));
		} else if(is_fixed_token_ci(value.list[0].data(), value.list[0].data() + value.list[0].size(), "strings")
		|| is_fixed_token_ci(value.list[0].data(), value.list[0].data() + value.list[0].size(), "string")) {
			context.compiled_trigger.push_back(uint16_t(trigger::length_greater_strings));
		} else if(is_fixed_token_ci(value.list[0].data(), value.list[0].data() + value.list[0].size(), "values")
		|| is_fixed_token_ci(value.list[0].data(), value.list[0].data() + value.list[0].size(), "value")) {
			context.compiled_trigger.push_back(uint16_t(trigger::length_greater_values));
		} else if(is_fixed_token_ci(value.list[0].data(), value.list[0].data() + value.list[0].size(), "dates")
		|| is_fixed_token_ci(value.list[0].data(), value.list[0].data() + value.list[0].size(), "date")) {
			context.compiled_trigger.push_back(uint16_t(trigger::length_greater_dates));
		} else {
			err.accumulated_errors += "length_greater trigger supplied with invalid type " + value.list[0] + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.add_int32_t_to_payload(parse_int(value.list[1], line, err));
		context.add_int32_t_to_payload(parse_int(value.list[2], line, err));
	}

	void trigger_body::news_printing_count(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context) {
		context.compiled_trigger.push_back(uint16_t(trigger::news_printing_count | association_to_bool_code(a)));
		//TODO warn when value >= 16-bits
		context.compiled_trigger.push_back(uint16_t(value));
	}

	void trigger_body::party_name(tr_party_name const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "party_name effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(trigger::party_name);
		context.compiled_trigger.push_back(trigger::payload(value.ideology_).value);
		context.add_int32_t_to_payload(value.name_.index());
	}
	void trigger_body::party_position(tr_party_position const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "party_position trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		} else if(!value.opt_) {
			err.accumulated_errors += "party_position trigger used without a valid position " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(trigger::party_position);
		context.compiled_trigger.push_back(trigger::payload(value.ideology_).value);
		context.compiled_trigger.push_back(trigger::payload(value.opt_).value);
	}
	void tr_party_position::position(association_type t, std::string_view v, error_handler& err, int32_t line, trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_ioptions.find(std::string(v)); it != context.outer_context.map_of_ioptions.end()) {
			opt_ = it->second.id;
		} else {
			err.accumulated_errors += "Invalid issue position " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	}

	void trigger_body::ai(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::ai | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "ai trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" +
															err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}

	void trigger_body::year(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context) {
		context.compiled_trigger.push_back(uint16_t(trigger::year | association_to_trigger_code(a)));
		context.compiled_trigger.push_back(trigger::payload(uint16_t(value)).value);
	}
	void trigger_body::is_canal_enabled(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context);
	void trigger_body::month(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context) {
		context.compiled_trigger.push_back(uint16_t(trigger::month | association_to_trigger_code(a)));
		context.compiled_trigger.push_back(trigger::payload(uint16_t(value)).value);
	}
	void trigger_body::great_wars_enabled(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		context.compiled_trigger.push_back(
			uint16_t(trigger::great_wars_enabled | trigger::no_payload | association_to_bool_code(a, value)));
	}
	void trigger_body::world_wars_enabled(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		context.compiled_trigger.push_back(
			uint16_t(trigger::world_wars_enabled | trigger::no_payload | association_to_bool_code(a, value)));
	}
	void trigger_body::crisis_exist(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		context.compiled_trigger.push_back(
			uint16_t(trigger::crisis_exist | trigger::no_payload | association_to_bool_code(a, value)));
	}
	void trigger_body::is_liberation_crisis(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		context.compiled_trigger.push_back(
			uint16_t(trigger::is_liberation_crisis | trigger::no_payload | association_to_bool_code(a, value)));
	}
	void trigger_body::is_claim_crisis(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		context.compiled_trigger.push_back(
			uint16_t(trigger::is_claim_crisis | trigger::no_payload | association_to_bool_code(a, value)));
	}

	void trigger_body::port(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::port | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "port trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
															"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}

	void trigger_body::involved_in_crisis(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::involved_in_crisis_pop | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::involved_in_crisis_nation | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "involved_in_crisis trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::has_cultural_sphere(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation && context.this_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::has_cultural_sphere | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "has_cultural_sphere trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}

	void trigger_body::social_movement(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::social_movement | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.from_slot == trigger::slot_contents::rebel) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::social_movement_from_reb | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "social_movement trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::political_movement(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::political_movement | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.from_slot == trigger::slot_contents::rebel) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::political_movement_from_reb | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "political_movement trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::rich_tax_above_poor(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::rich_tax_above_poor | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "rich_tax_above_poor trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}

	void trigger_body::is_substate(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_substate | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "is_substate trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::has_flashpoint(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::has_flashpoint | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "has_flashpoint trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::is_disarmed(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_disarmed | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_disarmed_pop | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "is_disarmed trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::can_nationalize(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::can_nationalize | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "can_nationalize trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::part_of_sphere(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::part_of_sphere | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "part_of_sphere trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::constructing_cb_discovered(association_type a, bool value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::constructing_cb_discovered | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "constructing_cb_discovered trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::colonial_nation(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::colonial_nation | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "colonial_nation trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::is_capital(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_capital | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "is_capital trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::election(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::election | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "election trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
															"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::always(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | association_to_bool_code(a, value)));
	}
	void trigger_body::is_releasable_vassal(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(is_from(value)) {
			if(context.from_slot == trigger::slot_contents::nation)
			context.compiled_trigger.push_back(
					uint16_t(trigger::is_releasable_vassal_from | trigger::no_payload | association_to_bool_code(a)));
			else {
				err.accumulated_errors += "is_releasable_vassal trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else {
			if(context.from_slot == trigger::slot_contents::nation)
			context.compiled_trigger.push_back(uint16_t(trigger::is_releasable_vassal_other | trigger::no_payload |
																									association_to_bool_code(a, parse_bool(value, line, err))));
			else {
				err.accumulated_errors += "is_releasable_vassal trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		}
	}

	void trigger_body::someone_can_form_union_tag(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(is_from(value)) {
			if(context.from_slot == trigger::slot_contents::nation)
			context.compiled_trigger.push_back(
					uint16_t(trigger::someone_can_form_union_tag_from | trigger::no_payload | association_to_bool_code(a)));
			else {
				err.accumulated_errors += "someone_can_form_union_tag trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else {
			if(context.from_slot == trigger::slot_contents::nation)
			context.compiled_trigger.push_back(uint16_t(trigger::someone_can_form_union_tag_other | trigger::no_payload |
																									association_to_bool_code(a, parse_bool(value, line, err))));
			else {
				err.accumulated_errors += "someone_can_form_union_tag trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		}
	}
	void trigger_body::is_state_capital(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::is_state_capital | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::is_state_capital_pop | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "is_state_capital trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}

	void trigger_body::has_factories(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::has_factories_nation | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::has_factories_state | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "has_factories trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::has_empty_adjacent_province(association_type a, bool value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::has_empty_adjacent_province | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "has_empty_adjacent_province trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::minorities(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::minorities_nation | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::minorities_state | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::minorities_province | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "minorities trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::culture_has_union_tag(association_type a, bool value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::culture_has_union_tag_nation | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::culture_has_union_tag_pop | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "culture_has_union_tag trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::is_colonial(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::is_colonial_province | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::is_colonial_state | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::is_colonial_pop | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::colonial_nation | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "is_colonial trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::is_greater_power(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_greater_power_pop | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_greater_power_nation | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_greater_power_province | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "is_greater_power trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::can_create_vassals(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::can_create_vassals | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "can_create_vassals trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::has_recently_lost_war(association_type a, bool value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::has_recently_lost_war | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::has_recently_lost_war_pop | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "has_recently_lost_war trigger used in an incorrect scope type " +
			slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::is_mobilised(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_mobilised | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "is_mobilised trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::crime_higher_than_education(association_type a, bool value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::crime_higher_than_education_nation | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::crime_higher_than_education_state | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::crime_higher_than_education_province | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::crime_higher_than_education_pop | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "crime_higher_than_education trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}

	void trigger_body::civilized(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::civilized_nation | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::civilized_province | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::civilized_pop | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "civilized trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}

	void trigger_body::rank(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::rank | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "rank trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
															"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(trigger::payload(uint16_t(value)).value);
	}

	void trigger_body::crisis_temperature(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		context.compiled_trigger.push_back(uint16_t(trigger::crisis_temperature | association_to_trigger_code(a)));
		context.add_float_to_payload(value);
	}

	void trigger_body::has_recent_imigration(association_type a, int32_t value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::has_recent_imigration | association_to_le_trigger_code(a)));
		} else {
			err.accumulated_errors += "has_recent_imigration trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(trigger::payload(uint16_t(value)).value);
	}

	void trigger_body::province_control_days(association_type a, int32_t value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::province_control_days | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "province_control_days trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(trigger::payload(uint16_t(value)).value);
	}
	void trigger_body::num_of_substates(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::num_of_substates | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "num_of_substates trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(trigger::payload(uint16_t(value)).value);
	}
	void trigger_body::num_of_vassals_no_substates(association_type a, int32_t value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::num_of_vassals_no_substates | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "num_of_vassals_no_substates trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(trigger::payload(uint16_t(value)).value);
	}
	void trigger_body::number_of_states(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::number_of_states | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "number_of_states trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(trigger::payload(uint16_t(value)).value);
	}

	void trigger_body::war_score(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::war_score | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "war_score trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}

	void trigger_body::flashpoint_tension(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::flashpoint_tension | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::flashpoint_tension_province | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "flashpoint_tension trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}

	void trigger_body::life_needs(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::life_needs | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "life_needs trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}

	void trigger_body::everyday_needs(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::everyday_needs | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "everyday_needs trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}

	void trigger_body::luxury_needs(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::luxury_needs | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "luxury_needs trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::social_movement_strength(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::social_movement_strength | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "social_movement_strength trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::political_movement_strength(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::political_movement_strength | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "political_movement_strength trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}

	void trigger_body::total_num_of_ports(association_type a, int32_t value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::total_num_of_ports | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "total_num_of_ports trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(trigger::payload(uint16_t(value)).value);
	}
	void trigger_body::agree_with_ruling_party(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::agree_with_ruling_party | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "agree_with_ruling_party trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::constructing_cb_progress(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::constructing_cb_progress | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "constructing_cb_progress trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::civilization_progress(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::civilization_progress | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "civilization_progress trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}

	void trigger_body::rich_strata_life_needs(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::rich_strata_life_needs_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::rich_strata_life_needs_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::rich_strata_life_needs_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::rich_strata_life_needs_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "rich_strata_life_needs trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::rich_strata_everyday_needs(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::rich_strata_everyday_needs_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::rich_strata_everyday_needs_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::rich_strata_everyday_needs_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::rich_strata_everyday_needs_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "rich_strata_everyday_needs trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::rich_strata_luxury_needs(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::rich_strata_luxury_needs_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::rich_strata_luxury_needs_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::rich_strata_luxury_needs_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::rich_strata_luxury_needs_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "rich_strata_luxury_needs trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}

	void trigger_body::middle_strata_life_needs(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::middle_strata_life_needs_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::middle_strata_life_needs_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::middle_strata_life_needs_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::middle_strata_life_needs_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "middle_strata_life_needs trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::middle_strata_everyday_needs(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::middle_strata_everyday_needs_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::middle_strata_everyday_needs_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::middle_strata_everyday_needs_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::middle_strata_everyday_needs_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "middle_strata_everyday_needs trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::middle_strata_luxury_needs(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::middle_strata_luxury_needs_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::middle_strata_luxury_needs_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::middle_strata_luxury_needs_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::middle_strata_luxury_needs_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "middle_strata_luxury_needs trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}

	void trigger_body::poor_strata_life_needs(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::poor_strata_life_needs_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::poor_strata_life_needs_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::poor_strata_life_needs_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::poor_strata_life_needs_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "poor_strata_life_needs trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::poor_strata_everyday_needs(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::poor_strata_everyday_needs_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::poor_strata_everyday_needs_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::poor_strata_everyday_needs_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::poor_strata_everyday_needs_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "poor_strata_everyday_needs trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::poor_strata_luxury_needs(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::poor_strata_luxury_needs_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::poor_strata_luxury_needs_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::poor_strata_luxury_needs_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::poor_strata_luxury_needs_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "poor_strata_luxury_needs trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}

	void trigger_body::revanchism(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::revanchism_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::revanchism_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "revanchism trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}

	void trigger_body::poor_strata_militancy(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::poor_strata_militancy_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::poor_strata_militancy_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::poor_strata_militancy_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::poor_strata_militancy_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "poor_strata_militancy trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::middle_strata_militancy(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::middle_strata_militancy_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::middle_strata_militancy_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::middle_strata_militancy_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::middle_strata_militancy_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "middle_strata_militancy trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::rich_strata_militancy(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::rich_strata_militancy_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::rich_strata_militancy_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::rich_strata_militancy_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::rich_strata_militancy_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "rich_strata_militancy trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::consciousness(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::consciousness_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::consciousness_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::consciousness_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::consciousness_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "consciousness trigger used in an incorrect scope type " +
				slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
				std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::literacy(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::literacy_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::literacy_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::literacy_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::literacy_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "literacy trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
				"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::militancy(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::militancy_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::militancy_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::militancy_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::militancy_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "militancy trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::military_spending(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::military_spending_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::military_spending_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::military_spending_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::military_spending_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "military_spending trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(value * 100.0f));
	}
	void trigger_body::administration_spending(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::administration_spending_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::administration_spending_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::administration_spending_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::administration_spending_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "administration_spending trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(value * 100.0f));
	}
	void trigger_body::education_spending(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::education_spending_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::education_spending_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::education_spending_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::education_spending_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "education_spending trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(value * 100.0f));
	}
	void trigger_body::national_provinces_occupied(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::national_provinces_occupied | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "national_provinces_occupied trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::social_spending(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::social_spending_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::social_spending_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::social_spending_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "social_spending trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(value * 100.0f));
	}
	void trigger_body::brigades_compare(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(context.this_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::brigades_compare_this | association_to_trigger_code(a)));
			} else if(context.from_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::brigades_compare_from | association_to_trigger_code(a)));
			} else {
				err.accumulated_errors += "brigades_compare trigger used in an incorrect scope type " +
				slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
				std::to_string(line) + ")\n";
				return;
			}
		} else if(context.main_slot == trigger::slot_contents::province) {
			if(context.this_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::brigades_compare_province_this | association_to_trigger_code(a)));
			} else if(context.from_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::brigades_compare_province_from | association_to_trigger_code(a)));
			} else {
				err.accumulated_errors += "brigades_compare trigger used in an incorrect scope type " +
				slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
				std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "brigades_compare trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::rich_tax(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::rich_tax | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::rich_tax_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "rich_tax trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
															"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(value * 100.0f));
	}
	void trigger_body::middle_tax(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::middle_tax | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::middle_tax_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "middle_tax trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(value * 100.0f));
	}
	void trigger_body::poor_tax(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::poor_tax | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::poor_tax_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "poor_tax trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
															"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(uint16_t(value * 100.0f));
	}
	void trigger_body::mobilisation_size(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::mobilisation_size | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "mobilisation_size trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::province_id(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::province_id | association_to_bool_code(a)));
		} else {
			err.accumulated_errors += "province_id trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		if(0 <= value && size_t(value) < context.outer_context.original_id_to_prov_id_map.size()) {
			context.compiled_trigger.push_back(trigger::payload(context.outer_context.original_id_to_prov_id_map[value]).value);
		} else {
			err.accumulated_errors +=
				"province_id trigger given an invalid province id (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			context.compiled_trigger.push_back(trigger::payload(dcon::province_id()).value);
		}
	}
	void trigger_body::invention(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {

		if(auto it = context.outer_context.map_of_technologies.find(std::string(value));
			it != context.outer_context.map_of_technologies.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::technology | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::technology_province | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::technology_pop | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "invention trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}

			context.compiled_trigger.push_back(uint16_t(1 + 1)); // data size; if no payload add code | trigger_codes::no_payload
			context.compiled_trigger.push_back(trigger::payload(it->second.id).value);
		} else if(auto itb = context.outer_context.map_of_inventions.find(std::string(value));
						itb != context.outer_context.map_of_inventions.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::invention | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::invention_province | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::invention_pop | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "invention trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}

			context.compiled_trigger.push_back(trigger::payload(itb->second.id).value);
		} else {
			err.accumulated_errors += "invention trigger supplied with neither a valid technology nor a valid invention "
			+ std::string(value) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::big_producer(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_commodity_names.find(std::string(value));
			it != context.outer_context.map_of_commodity_names.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::big_producer | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "big_producer trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}

			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			err.accumulated_errors +=
				"big_producer trigger supplied with an invalid commodity \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::strata(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::pop) {
			if(is_fixed_token_ci(value.data(), value.data() + value.length(), "poor"))
			context.compiled_trigger.push_back(uint16_t(trigger::strata_poor | trigger::no_payload | association_to_bool_code(a)));
			else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "rich"))
			context.compiled_trigger.push_back(uint16_t(trigger::strata_rich | trigger::no_payload | association_to_bool_code(a)));
			else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "middle"))
			context.compiled_trigger.push_back(uint16_t(trigger::strata_middle | trigger::no_payload | association_to_bool_code(a)));
			else {
				err.accumulated_errors +=
					"strata trigger given unknown strata (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "strata trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
															"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}

	void trigger_body::life_rating(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::life_rating_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::life_rating_state | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "life_rating trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(trigger::payload(int16_t(value)).value);
	}

	void trigger_body::has_empty_adjacent_state(association_type a, bool value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::has_empty_adjacent_state_province | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::has_empty_adjacent_state_state | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "has_empty_adjacent_state trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}

	void trigger_body::state_id(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::state_id_province | association_to_bool_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::state_id_state | association_to_bool_code(a)));
		} else {
			err.accumulated_errors += "state_id trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
															"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		if(0 <= value && size_t(value) < context.outer_context.original_id_to_prov_id_map.size()) {
			context.compiled_trigger.push_back(trigger::payload(context.outer_context.original_id_to_prov_id_map[value]).value);
		} else {
			err.accumulated_errors +=
				"state_id trigger given an invalid province id (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			context.compiled_trigger.push_back(trigger::payload(dcon::province_id()).value);
		}
	}
	void trigger_body::cash_reserves(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::cash_reserves | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "cash_reserves trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::unemployment(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::unemployment_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::unemployment_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::unemployment_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::unemployment_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "unemployment trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}

	void trigger_body::is_slave(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_slave_nation | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_slave_state | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_slave_province | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_slave_pop | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "is_slave trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
															"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}

	void trigger_body::is_independant(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_independant | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "is_independant trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}

	void trigger_body::has_national_minority(association_type a, bool value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::has_national_minority_nation | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::has_national_minority_state | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::has_national_minority_province | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "has_national_minority trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}

	void trigger_body::government(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_governments.find(std::string(value));
			it != context.outer_context.map_of_governments.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::government_nation | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::government_pop | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "government trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}

			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			err.accumulated_errors += "government trigger supplied with an invalid government type \"" + std::string(value) + "\" (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
		}
	}

	void trigger_body::constructing_cb_type(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_cb_types.find(std::string(value));
			it != context.outer_context.map_of_cb_types.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::constructing_cb_type | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "constructing_cb_type trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}

			context.compiled_trigger.push_back(trigger::payload(it->second.id).value);
		} else {
			err.accumulated_errors += "constructing_cb_type trigger supplied with an invalid cb type \"" + std::string(value) + "\" (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
		}
	}

	void trigger_body::can_build_factory_in_capital_state(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_factory_names.find(std::string(value));
			it != context.outer_context.map_of_factory_names.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::can_build_factory_in_capital_state | association_to_bool_code(a)));
				context.compiled_trigger.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors += "can_build_factory_in_capital_state trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "can_build_factory_in_capital_state trigger supplied with an government type (" + err.file_name +
				", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::capital(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::capital | association_to_bool_code(a)));
		} else {
			err.accumulated_errors += "capital trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
															"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		if(0 <= value && size_t(value) < context.outer_context.original_id_to_prov_id_map.size()) {
			context.compiled_trigger.push_back(trigger::payload(context.outer_context.original_id_to_prov_id_map[value]).value);
		} else {
			err.accumulated_errors +=
				"capital trigger given an invalid province id (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			context.compiled_trigger.push_back(trigger::payload(dcon::province_id()).value);
		}
	}
	void trigger_body::tech_school(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_modifiers.find(std::string(value));
			it != context.outer_context.map_of_modifiers.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::tech_school | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "tech_school trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}

			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			err.accumulated_errors +=
				"tech_school trigger supplied with an invalid school \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::primary_culture(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(is_this(value)) {
			if(context.main_slot == trigger::slot_contents::nation) {
				if(context.this_slot == trigger::slot_contents::nation) {
					context.compiled_trigger.push_back(
						uint16_t(trigger::is_primary_culture_nation_this_nation | trigger::no_payload | association_to_bool_code(a)));
				} else if(context.this_slot == trigger::slot_contents::state) {
					context.compiled_trigger.push_back(
						uint16_t(trigger::is_primary_culture_nation_this_state | trigger::no_payload | association_to_bool_code(a)));
				} else if(context.this_slot == trigger::slot_contents::province) {
					context.compiled_trigger.push_back(
						uint16_t(trigger::is_primary_culture_nation_this_province | trigger::no_payload | association_to_bool_code(a)));
				} else if(context.this_slot == trigger::slot_contents::pop) {
					context.compiled_trigger.push_back(
						uint16_t(trigger::is_primary_culture_nation_this_pop | trigger::no_payload | association_to_bool_code(a)));
				} else {
					err.accumulated_errors += "primary_culture trigger used in an incorrect scope type " +
					slot_contents_to_string(context.this_slot) + "(" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors += "primary_culture = this trigger used in an incorrect scope type " +
				slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(is_from(value)) {
			if(context.main_slot == trigger::slot_contents::nation && context.from_slot == trigger::slot_contents::nation)
			context.compiled_trigger.push_back(
					uint16_t(trigger::primary_culture_from_nation | trigger::no_payload | association_to_bool_code(a)));
			else if(context.main_slot == trigger::slot_contents::nation && context.from_slot == trigger::slot_contents::province)
			context.compiled_trigger.push_back(
					uint16_t(trigger::primary_culture_from_province | trigger::no_payload | association_to_bool_code(a)));
			else {
				err.accumulated_errors += "primary_culture = from trigger used in an incorrect scope type " +
				slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
				std::to_string(line) + ")\n";
				return;
			}
		} else if(auto it = context.outer_context.map_of_culture_names.find(std::string(value)); it != context.outer_context.map_of_culture_names.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::primary_culture | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::primary_culture_pop | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "primary_culture trigger used in an incorrect scope type " +
				slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}

			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
			err.accumulated_errors += "primary_culture trigger supplied with an invalid culture \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::total_sunk_by_us(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context) {
		// this is a trigger we don't support currently, not even in trigger codes
		context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | association_to_trigger_code(a)));
	}
	void trigger_body::has_crime(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_crimes.find(std::string(value)); it != context.outer_context.map_of_crimes.end()) {
			if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_crime | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "has_crime trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}

			context.compiled_trigger.push_back(trigger::payload(it->second.id).value);
		} else {
			err.accumulated_errors +=
				"has_crime trigger supplied with an invalid crime \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::accepted_culture(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_culture_names.find(std::string(value));
			it != context.outer_context.map_of_culture_names.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::accepted_culture | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "accepted_culture trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}

			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
			err.accumulated_errors += "accepted_culture trigger supplied with an invalid culture \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::pop_majority_religion(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(is_this(value)) {
			if(context.main_slot == trigger::slot_contents::nation && context.this_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::pop_majority_religion_nation_this_nation | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "pop_majority_religion = THIS trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(auto it = context.outer_context.map_of_religion_names.find(std::string(value)); it != context.outer_context.map_of_religion_names.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::pop_majority_religion_nation | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::state) {
				context.compiled_trigger.push_back(uint16_t(trigger::pop_majority_religion_state | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::pop_majority_religion_province | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "pop_majority_religion trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			err.accumulated_errors += "pop_majority_religion trigger supplied with an invalid religion \"" + std::string(value) + "\" (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
		}
	}
	void trigger_body::pop_majority_culture(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_culture_names.find(std::string(value));
			it != context.outer_context.map_of_culture_names.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::pop_majority_culture_nation | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::state) {
				context.compiled_trigger.push_back(uint16_t(trigger::pop_majority_culture_state | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::pop_majority_culture_province | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "pop_majority_culture trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
			err.accumulated_errors += "pop_majority_culture trigger supplied with an invalid culture \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::pop_majority_issue(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_ioptions.find(std::string(value));
			it != context.outer_context.map_of_ioptions.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::pop_majority_issue_nation | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::state) {
				context.compiled_trigger.push_back(uint16_t(trigger::pop_majority_issue_state | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::pop_majority_issue_province | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::pop_majority_issue_pop | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "pop_majority_issue trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second.id).value);
		} else {
			err.accumulated_errors += "pop_majority_issue trigger supplied with an invalid option name \"" + std::string(value) + "\" (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
		}
	}
	void trigger_body::pop_majority_ideology(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_ideologies.find(std::string(value));
			it != context.outer_context.map_of_ideologies.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::pop_majority_ideology_nation | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::state) {
				context.compiled_trigger.push_back(uint16_t(trigger::pop_majority_ideology_state | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::pop_majority_ideology_province | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::pop_majority_ideology_pop | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "pop_majority_ideology trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second.id).value);
		} else {
			err.accumulated_errors += "pop_majority_ideology trigger supplied with an invalid ideology \"" + std::string(value) + "\" (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
		}
	}
	void trigger_body::trade_goods_in_state(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_commodity_names.find(std::string(value));
			it != context.outer_context.map_of_commodity_names.end()) {
			if(context.main_slot == trigger::slot_contents::state) {
				context.compiled_trigger.push_back(uint16_t(trigger::trade_goods_in_state_state | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::trade_goods_in_state_province | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "trade_goods_in_state trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			err.accumulated_errors += "trade_goods_in_state trigger supplied with an invalid commodity \"" + std::string(value) + "\" (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
		}
	}

	void trigger_body::culture(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(is_this(value)) {
			if(context.main_slot == trigger::slot_contents::pop) {
				if(context.this_slot == trigger::slot_contents::nation) {
					context.compiled_trigger.push_back((trigger::culture_this_nation | trigger::no_payload | association_to_bool_code(a)));
				} else if(context.this_slot == trigger::slot_contents::state) {
					context.compiled_trigger.push_back(uint16_t(trigger::culture_this_state | trigger::no_payload | association_to_bool_code(a)));
				} else if(context.this_slot == trigger::slot_contents::province) {
					context.compiled_trigger.push_back(uint16_t(trigger::culture_this_province | trigger::no_payload | association_to_bool_code(a)));
				} else if(context.this_slot == trigger::slot_contents::pop) {
					context.compiled_trigger.push_back(uint16_t(trigger::culture_this_pop | trigger::no_payload | association_to_bool_code(a)));
				} else {
					err.accumulated_errors += "culture trigger used in an incorrect scope type " +
					slot_contents_to_string(context.this_slot) + "(" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(context.main_slot == trigger::slot_contents::province) {
				if(context.this_slot == trigger::slot_contents::nation) {
					context.compiled_trigger.push_back((trigger::is_primary_culture_nation_this_nation | trigger::no_payload | association_to_bool_code(a)));
				} else if(context.this_slot == trigger::slot_contents::state) {
					context.compiled_trigger.push_back(uint16_t(trigger::is_primary_culture_nation_this_state | trigger::no_payload | association_to_bool_code(a)));
				} else if(context.this_slot == trigger::slot_contents::province) {
					context.compiled_trigger.push_back(uint16_t(trigger::is_primary_culture_nation_this_province | trigger::no_payload | association_to_bool_code(a)));
				} else if(context.this_slot == trigger::slot_contents::pop) {
					context.compiled_trigger.push_back(uint16_t(trigger::is_primary_culture_nation_this_pop | trigger::no_payload | association_to_bool_code(a)));
				} else {
					err.accumulated_errors += "culture trigger used in an incorrect scope type " +
					slot_contents_to_string(context.this_slot) + "(" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors += "culture = this trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else if(is_from(value)) {
			if(context.main_slot == trigger::slot_contents::pop && context.from_slot == trigger::slot_contents::nation)
			context.compiled_trigger.push_back(
					uint16_t(trigger::culture_from_nation | trigger::no_payload | association_to_bool_code(a)));
			else {
				err.accumulated_errors += "culture = from trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else if(is_reb(value)) {
			if(context.from_slot != trigger::slot_contents::rebel) {
				err.accumulated_errors += "culture = reb trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			} else if(context.main_slot == trigger::slot_contents::pop)
			context.compiled_trigger.push_back(
					uint16_t(trigger::culture_pop_reb | trigger::no_payload | association_to_bool_code(a)));
			else if(context.main_slot == trigger::slot_contents::state)
			context.compiled_trigger.push_back(
					uint16_t(trigger::culture_state_reb | trigger::no_payload | association_to_bool_code(a)));
			else if(context.main_slot == trigger::slot_contents::province)
			context.compiled_trigger.push_back(
					uint16_t(trigger::culture_province_reb | trigger::no_payload | association_to_bool_code(a)));
			else if(context.main_slot == trigger::slot_contents::nation)
			context.compiled_trigger.push_back(
					uint16_t(trigger::culture_nation_reb | trigger::no_payload | association_to_bool_code(a)));
			else {
				err.accumulated_errors += "culture = reb trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else if(auto it = context.outer_context.map_of_culture_names.find(std::string(value)); it != context.outer_context.map_of_culture_names.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::culture_nation | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::state) {
				context.compiled_trigger.push_back(uint16_t(trigger::culture_state | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::culture_province | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::culture_pop | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "culture trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
			err.accumulated_errors += "culture trigger supplied with an invalid culture \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::has_pop_culture(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(is_this(value)) {
			if(context.this_slot == trigger::slot_contents::pop) {
				if(context.main_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::has_pop_culture_nation_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(uint16_t(trigger::has_pop_culture_pop_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(uint16_t(trigger::has_pop_culture_state_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(uint16_t(trigger::has_pop_culture_province_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "has_pop_culture = this trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors += "has_pop_culture = this trigger used in an incorrect scope type " +
				slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
				std::to_string(line) + ")\n";
				return;
			}
		} else if(auto it = context.outer_context.map_of_culture_names.find(std::string(value));
						it != context.outer_context.map_of_culture_names.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_pop_culture_nation | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::state) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_pop_culture_state | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_pop_culture_province | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_pop_culture_pop | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "has_pop_culture trigger used in an incorrect scope type " +
				slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
				std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
			err.accumulated_errors += "has_pop_culture trigger supplied with an invalid culture \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::has_pop_religion(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(is_this(value)) {
			if(context.this_slot == trigger::slot_contents::pop) {
				if(context.main_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::has_pop_religion_nation_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(uint16_t(trigger::has_pop_religion_pop_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(uint16_t(trigger::has_pop_religion_state_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(uint16_t(trigger::has_pop_religion_province_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "has_pop_religion = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors += "has_pop_religion = this trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else if(auto it = context.outer_context.map_of_religion_names.find(std::string(value)); it != context.outer_context.map_of_religion_names.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_pop_religion_nation | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::state) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_pop_religion_state | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_pop_religion_province | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_pop_religion_pop | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "has_pop_religion trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			err.accumulated_errors += "has_pop_religion trigger supplied with an invalid religion \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}

	void trigger_body::culture_group(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(is_this(value)) {
			if(context.this_slot == trigger::slot_contents::nation) {
				if(context.main_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::culture_group_nation_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::culture_group_pop_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "culture_group = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(context.this_slot == trigger::slot_contents::state) {
				if(context.main_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::culture_group_nation_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::culture_group_pop_this_state | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "culture_group = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(context.this_slot == trigger::slot_contents::province) {
				if(context.main_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::culture_group_nation_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::culture_group_pop_this_province | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "culture_group = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(context.this_slot == trigger::slot_contents::pop) {
				if(context.main_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::culture_group_nation_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::culture_group_pop_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "culture_group = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors += "culture_group = this trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else if(is_from(value)) {
			if(context.from_slot == trigger::slot_contents::nation) {
				if(context.main_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::culture_group_nation_from_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::culture_group_pop_from_nation | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "culture_group = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors += "culture_group = from trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else if(is_reb(value)) {
			if(context.from_slot == trigger::slot_contents::rebel) {
				if(context.main_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::culture_group_reb_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::culture_group_reb_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "culture_group = reb trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors += "culture_group = reb trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else if(auto it = context.outer_context.map_of_culture_group_names.find(std::string(value));
						it != context.outer_context.map_of_culture_group_names.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::culture_group_nation | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::culture_group_pop | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::culture_group_province | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::state) {
				context.compiled_trigger.push_back(uint16_t(trigger::culture_group_state | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "culture_group trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
			err.accumulated_errors += "culture_group trigger supplied with an invalid culture group \"" + std::string(value) + "\" (" + err.file_name + ", line " +
			std::to_string(line) + ")\n";
		}
	}

	void trigger_body::religion(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(is_this(value)) {
			if(context.this_slot == trigger::slot_contents::nation) {
				if(context.main_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::religion_nation_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::religion_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "religion = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(context.this_slot == trigger::slot_contents::state) {
				if(context.main_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::religion_nation_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::religion_this_state | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "religion = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(context.this_slot == trigger::slot_contents::province) {
				if(context.main_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::religion_nation_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::religion_this_province | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "religion = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(context.this_slot == trigger::slot_contents::pop) {
				if(context.main_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::religion_nation_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::religion_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "religion = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors += "religion = this trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else if(is_from(value)) {
			if(context.from_slot == trigger::slot_contents::nation) {
				if(context.main_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::religion_nation_from_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::religion_from_nation | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "religion = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors += "religion = from trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else if(is_reb(value)) {
			if(context.from_slot == trigger::slot_contents::rebel) {
				if(context.main_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::religion_nation_reb | trigger::no_payload | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(uint16_t(trigger::religion_reb | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "religion = reb trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors += "religion = reb trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else if(auto it = context.outer_context.map_of_religion_names.find(std::string(value));
						it != context.outer_context.map_of_religion_names.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::religion_nation | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::religion | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "religion trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			err.accumulated_errors +=
				"religion trigger supplied with an invalid religion \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::terrain(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_terrain_types.find(std::string(value));
			it != context.outer_context.map_of_terrain_types.end()) {
			if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::terrain_pop | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::terrain_province | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "terrain trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second.id).value);
		} else {
			err.accumulated_errors +=
				"terrain trigger supplied with an invalid terrain \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::trade_goods(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_commodity_names.find(std::string(value));
			it != context.outer_context.map_of_commodity_names.end()) {
			if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::trade_goods | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "trade_goods trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			err.accumulated_errors +=
				"trade_goods trigger supplied with an invalid commodity \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::is_secondary_power(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_secondary_power_nation | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_secondary_power_pop | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "is_secondary_power trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::has_faction(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_rebeltypes.find(std::string(value));
			it != context.outer_context.map_of_rebeltypes.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_faction_nation | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_faction_pop | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "has_faction trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second.id).value);
		} else {
			err.accumulated_errors +=
				"has_faction trigger supplied with an invalid rebel type \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::has_unclaimed_cores(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::has_unclaimed_cores | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "has_unclaimed_cores trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::have_core_in(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of trigger extension have_core_in but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		if(is_this(value)) {
			if(context.main_slot == trigger::slot_contents::nation && context.this_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::have_core_in_nation_this | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "have_core_in = this trigger used in an incorrect scope type " +
				slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
				std::to_string(line) + ")\n";
				return;
			}
		} else if(is_from(value)) {
			if(context.main_slot == trigger::slot_contents::nation && context.from_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::have_core_in_nation_from | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "have_core_in = from trigger used in an incorrect scope type " +
				slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
				std::to_string(line) + ")\n";
				return;
			}
		} else if(value.length() == 3) {
			if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
				it != context.outer_context.map_of_ident_names.end()) {

				if(context.main_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::have_core_in_nation_tag | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "have_core_in = tag trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
				context.compiled_trigger.push_back(trigger::payload(it->second).value);
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
				"have_core_in trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			}
		} else {
			context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
			err.accumulated_errors +=
			"have_core_in trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::is_cultural_union(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(is_fixed_token_ci(value.data(), value.data() + value.length(), "yes") || is_fixed_token_ci(value.data(), value.data() + value.length(), "no")) {
			if(context.main_slot == trigger::slot_contents::nation)
			context.compiled_trigger.push_back(uint16_t(
					trigger::is_cultural_union_bool | trigger::no_payload | association_to_bool_code(a, parse_bool(value, line, err))));
			else {
				err.accumulated_errors += "is_cultural_union = bool trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else if(is_this(value)) {
			if(context.main_slot == trigger::slot_contents::pop && context.this_slot == trigger::slot_contents::nation)
			context.compiled_trigger.push_back(
					uint16_t(trigger::is_cultural_union_this_self_pop | trigger::no_payload | association_to_bool_code(a)));
			else if(context.main_slot == trigger::slot_contents::pop && context.this_slot == trigger::slot_contents::pop)
			context.compiled_trigger.push_back(
					uint16_t(trigger::is_cultural_union_pop_this_pop | trigger::no_payload | association_to_bool_code(a)));
			else if(context.main_slot == trigger::slot_contents::nation) {
				if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_cultural_union_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_cultural_union_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_cultural_union_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_cultural_union_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.from_slot == trigger::slot_contents::rebel)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_cultural_union_this_rebel | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "is_cultural_union = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors += "is_cultural_union = this trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else if(value.length() == 3) {
			if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
				it != context.outer_context.map_of_ident_names.end()) {

				if(context.main_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::is_cultural_union_tag_nation | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(uint16_t(trigger::is_cultural_union_tag_this_pop | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(uint16_t(trigger::is_cultural_union_tag_this_state | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_cultural_union_tag_this_province | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::is_cultural_union_tag_this_nation | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "is_cultural_union = tag trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
				context.compiled_trigger.push_back(trigger::payload(it->second).value);
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
					"is_cultural_union trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			}
		} else {
			context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
			err.accumulated_errors +=
				"is_cultural_union trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::can_build_factory(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::can_build_factory_nation | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::can_build_factory_province | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::can_build_factory_pop | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "can_build_factory trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::war(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::war_nation | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::war_pop | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "war trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
															"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::war_exhaustion(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::war_exhaustion_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::war_exhaustion_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::war_exhaustion_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "war_exhaustion trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value / 100.0f);
	}
	void trigger_body::blockade(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::blockade | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "blockade trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
															"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::owns(association_type a, std::string_view v, error_handler& err, int32_t line, trigger_building_context& context) {
		if(is_integer(v.data(), v.data() + v.length())) {
			auto value = parse_int(v, line, err);
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::owns | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::owns_province | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "owns trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
				"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			if(0 <= value && size_t(value) < context.outer_context.original_id_to_prov_id_map.size()) {
				context.compiled_trigger.push_back(trigger::payload(context.outer_context.original_id_to_prov_id_map[value]).value);
			} else {
				err.accumulated_errors +=
				"owns trigger given an invalid province id (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				context.compiled_trigger.push_back(trigger::payload(dcon::province_id()).value);
			}
		} else if(auto it = context.outer_context.map_of_state_names.find(std::string(v)); it != context.outer_context.map_of_state_names.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::owns_region | association_to_bool_code(a)));
				context.compiled_trigger.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors += "owns trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
				"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(auto it = context.outer_context.map_of_region_names.find(std::string(v)); it != context.outer_context.map_of_region_names.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::owns_region_proper | association_to_bool_code(a)));
				context.compiled_trigger.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors += "owns trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
				"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors +=
			"owns trigger given an invalid value (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			context.compiled_trigger.push_back(trigger::payload(dcon::province_id()).value);
		}
	}
	void trigger_body::controls(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::controls | association_to_bool_code(a)));
		} else {
			err.accumulated_errors += "controls trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
															"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		if(0 <= value && size_t(value) < context.outer_context.original_id_to_prov_id_map.size()) {
			context.compiled_trigger.push_back(trigger::payload(context.outer_context.original_id_to_prov_id_map[value]).value);
		} else {
			err.accumulated_errors +=
				"controls trigger given an invalid province id (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			context.compiled_trigger.push_back(trigger::payload(dcon::province_id()).value);
		}
	}

	void trigger_body::is_core(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(is_this(value) && context.main_slot == trigger::slot_contents::province) {
			if(context.this_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(
					uint16_t(trigger::is_core_this_nation | trigger::no_payload | association_to_bool_code(a)));
			} else if(context.this_slot == trigger::slot_contents::state) {
				context.compiled_trigger.push_back(
					uint16_t(trigger::is_core_this_state | trigger::no_payload | association_to_bool_code(a)));
			} else if(context.this_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(
					uint16_t(trigger::is_core_this_province | trigger::no_payload | association_to_bool_code(a)));
			} else if(context.this_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(
					uint16_t(trigger::is_core_this_pop | trigger::no_payload | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "is_core = this trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else if(is_from(value) && context.main_slot == trigger::slot_contents::province) {
			if(context.from_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(
					uint16_t(trigger::is_core_from_nation | trigger::no_payload | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "is_core = from trigger used in an incorrect scope type " +
				slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
				std::to_string(line) + ")\n";
				return;
			}
		} else if(is_this(value) && context.main_slot == trigger::slot_contents::state) {
			if(context.this_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::is_core_state_this_nation | trigger::no_payload | association_to_bool_code(a)));
			} else if(context.this_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::is_core_state_this_province | trigger::no_payload | association_to_bool_code(a)));
			} else if(context.this_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::is_core_state_this_pop | trigger::no_payload | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "is_core = this trigger used in an incorrect scope type " +
				slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
				std::to_string(line) + ")\n";
				return;
			}
		} else if(is_from(value) && context.main_slot == trigger::slot_contents::state) {
			if(context.from_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::is_core_state_from_nation | trigger::no_payload | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "is_core = from trigger used in an incorrect scope type " +
				slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
				std::to_string(line) + ")\n";
				return;
			}
		} else if(is_reb(value) && context.main_slot == trigger::slot_contents::province) {
			if(context.from_slot == trigger::slot_contents::rebel) {
				context.compiled_trigger.push_back(uint16_t(trigger::is_core_reb | trigger::no_payload | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "is_core = reb trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else if(is_integer(value.data(), value.data() + value.length())) {
			if(context.main_slot == trigger::slot_contents::nation) {
				auto int_value = parse_int(value, line, err);
				if(0 <= int_value && size_t(int_value) < context.outer_context.original_id_to_prov_id_map.size()) {
					context.compiled_trigger.push_back(uint16_t(trigger::is_core_integer | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(context.outer_context.original_id_to_prov_id_map[int_value]).value);
				} else {
					err.accumulated_errors +=
					"owns trigger given an invalid province id (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					context.compiled_trigger.push_back(trigger::payload(dcon::province_id()).value);
				}
			} else {
				err.accumulated_errors += "is_core trigger used in an incorrect scope type " +
				slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
				std::to_string(line) + ")\n";
				return;
			}
		} else if(context.main_slot == trigger::slot_contents::province && (is_fixed_token_ci(value.data(), value.data() + value.length(), "yes") || is_fixed_token_ci(value.data(), value.data() + value.length(), "no"))) {
			context.compiled_trigger.push_back(uint16_t(trigger::is_core_boolean | trigger::no_payload | association_to_bool_code(a, parse_bool(value, line, err))));
		} else if(value.length() == 3 && context.main_slot == trigger::slot_contents::province) {
			if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2])); it != context.outer_context.map_of_ident_names.end()) {
				context.compiled_trigger.push_back(uint16_t(trigger::is_core_tag | association_to_bool_code(a)));
				context.compiled_trigger.push_back(trigger::payload(it->second).value);
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
					"is_core trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			}
		} else if(value.length() == 3 && context.main_slot == trigger::slot_contents::state) {
			if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2])); it != context.outer_context.map_of_ident_names.end()) {
				context.compiled_trigger.push_back(uint16_t(trigger::is_core_state_tag | association_to_bool_code(a)));
				context.compiled_trigger.push_back(trigger::payload(it->second).value);
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
				"is_core trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			}
		} else if(value.length() == 3 && context.main_slot == trigger::slot_contents::pop) {
			if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2])); it != context.outer_context.map_of_ident_names.end()) {
				context.compiled_trigger.push_back(uint16_t(trigger::is_core_pop_tag | association_to_bool_code(a)));
				context.compiled_trigger.push_back(trigger::payload(it->second).value);
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
				"is_core trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			}
		} else {
			context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
			err.accumulated_errors +=
				"is_core trigger supplied with an invalid argument \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::num_of_revolts(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::num_of_revolts | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "num_of_revolts trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(trigger::payload(uint16_t(value)).value);
	}
	void trigger_body::revolt_percentage(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::revolt_percentage | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "revolt_percentage trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::num_of_cities(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(is_this(value) && context.main_slot == trigger::slot_contents::nation) {
			if(context.this_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(
					uint16_t(trigger::num_of_cities_this_nation | trigger::no_payload | association_to_trigger_code(a)));
			} else if(context.this_slot == trigger::slot_contents::state) {
				context.compiled_trigger.push_back(
					uint16_t(trigger::num_of_cities_this_province | trigger::no_payload | association_to_trigger_code(a)));
			} else if(context.this_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(
					uint16_t(trigger::num_of_cities_this_state | trigger::no_payload | association_to_trigger_code(a)));
			} else if(context.this_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(
					uint16_t(trigger::num_of_cities_this_pop | trigger::no_payload | association_to_trigger_code(a)));
			} else {
				err.accumulated_errors += "num_of_cities = this trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else if(is_from(value) && context.main_slot == trigger::slot_contents::nation) {
			if(context.from_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(
					uint16_t(trigger::num_of_cities_from_nation | trigger::no_payload | association_to_trigger_code(a)));
			} else {
				err.accumulated_errors += "num_of_cities = from trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::num_of_cities_int | association_to_trigger_code(a)));
			context.compiled_trigger.push_back(trigger::payload(uint16_t(parse_uint(value, line, err))).value);
		} else {
			err.accumulated_errors += "num_of_cities trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::num_of_ports(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::num_of_ports | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "num_of_ports trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(trigger::payload(uint16_t(value)).value);
	}
	void trigger_body::num_of_allies(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::num_of_allies | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "num_of_allies trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(trigger::payload(uint16_t(value)).value);
	}
	void trigger_body::num_of_vassals(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::num_of_vassals | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "num_of_vassals trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(trigger::payload(uint16_t(value)).value);
	}
	void trigger_body::owned_by(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::owned_by_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::owned_by_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::owned_by_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::owned_by_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "owned_by = this trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::owned_by_from_nation | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "owned_by = from trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::owned_by_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors +=
					"owned_by trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
				"owned_by trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(context.main_slot == trigger::slot_contents::state) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::owned_by_state_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::owned_by_state_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::owned_by_state_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::owned_by_state_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "owned_by = this trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::owned_by_state_from_nation | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "owned_by = from trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::owned_by_state_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors +=
					"owned_by trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
				"owned_by trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "owned_by trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
															"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}

	void trigger_body::exists(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(is_fixed_token_ci(value.data(), value.data() + value.length(), "yes") ||
			is_fixed_token_ci(value.data(), value.data() + value.length(), "no")) {
			if(context.main_slot == trigger::slot_contents::nation)
			context.compiled_trigger.push_back(
					uint16_t(trigger::exists_bool | trigger::no_payload | association_to_bool_code(a, parse_bool(value, line, err))));
			else {
				err.accumulated_errors += "exists = bool trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else if(value.length() == 3) {
			if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
				it != context.outer_context.map_of_ident_names.end()) {
				context.compiled_trigger.push_back(uint16_t(trigger::exists_tag | association_to_bool_code(a)));
				context.compiled_trigger.push_back(trigger::payload(it->second).value);
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
					"exists trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			}

		} else {
			context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
			err.accumulated_errors +=
				"exists trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::has_country_flag(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::has_country_flag | association_to_bool_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::has_country_flag_pop | association_to_bool_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::has_country_flag_province | association_to_bool_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::has_country_flag_state | association_to_bool_code(a)));
		} else {
			err.accumulated_errors += "has_country_flag trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(trigger::payload(context.outer_context.get_national_flag(std::string(value))).value);
	}
	void trigger_body::has_province_flag(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of trigger extension has_province_flag but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::has_province_flag | association_to_bool_code(a)));
		} else {
			err.accumulated_errors += "has_province_flag trigger used in an incorrect scope type " +
			slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
			std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(trigger::payload(context.outer_context.get_provincial_flag(std::string(value))).value);
	}
	void trigger_body::has_global_flag(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.outer_context.use_extensions && is_fixed_token_ci(value.data(), value.data() + value.length(), "katerina_engine")) {
			context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | association_to_bool_code(a, true)));
		} else {
			context.compiled_trigger.push_back(uint16_t(trigger::has_global_flag | association_to_bool_code(a)));
			context.compiled_trigger.push_back(trigger::payload(context.outer_context.get_global_flag(std::string(value))).value);
		}
	}

	void trigger_body::continent(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation) {
					context.compiled_trigger.push_back(
						uint16_t(trigger::continent_nation_this | trigger::no_payload | association_to_bool_code(a)));
				} else {
					err.accumulated_errors += "continent = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation) {
					context.compiled_trigger.push_back(
						uint16_t(trigger::continent_nation_from | trigger::no_payload | association_to_bool_code(a)));
				} else {
					err.accumulated_errors += "continent = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(auto it = context.outer_context.map_of_modifiers.find(std::string(value));
							it != context.outer_context.map_of_modifiers.end()) {
				context.compiled_trigger.push_back(uint16_t(trigger::continent_nation | association_to_bool_code(a)));
				context.compiled_trigger.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors +=
					"continent trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(context.main_slot == trigger::slot_contents::state) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation) {
					context.compiled_trigger.push_back(
						uint16_t(trigger::continent_state_this | trigger::no_payload | association_to_bool_code(a)));
				} else {
					err.accumulated_errors += "continent = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation) {
					context.compiled_trigger.push_back(
						uint16_t(trigger::continent_state_from | trigger::no_payload | association_to_bool_code(a)));
				} else {
					err.accumulated_errors += "continent = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(auto it = context.outer_context.map_of_modifiers.find(std::string(value));
							it != context.outer_context.map_of_modifiers.end()) {
				context.compiled_trigger.push_back(uint16_t(trigger::continent_state | association_to_bool_code(a)));
				context.compiled_trigger.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors +=
					"continent trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(context.main_slot == trigger::slot_contents::province) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation) {
					context.compiled_trigger.push_back(
						uint16_t(trigger::continent_province_this | trigger::no_payload | association_to_bool_code(a)));
				} else {
					err.accumulated_errors += "continent = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation) {
					context.compiled_trigger.push_back(
						uint16_t(trigger::continent_province_from | trigger::no_payload | association_to_bool_code(a)));
				} else {
					err.accumulated_errors += "continent = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(auto it = context.outer_context.map_of_modifiers.find(std::string(value));
							it != context.outer_context.map_of_modifiers.end()) {
				context.compiled_trigger.push_back(uint16_t(trigger::continent_province | association_to_bool_code(a)));
				context.compiled_trigger.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors +=
					"continent trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(context.main_slot == trigger::slot_contents::pop) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation) {
					context.compiled_trigger.push_back(
						uint16_t(trigger::continent_pop_this | trigger::no_payload | association_to_bool_code(a)));
				} else {
					err.accumulated_errors += "continent = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation) {
					context.compiled_trigger.push_back(
						uint16_t(trigger::continent_pop_from | trigger::no_payload | association_to_bool_code(a)));
				} else {
					err.accumulated_errors += "continent = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(auto it = context.outer_context.map_of_modifiers.find(std::string(value));
							it != context.outer_context.map_of_modifiers.end()) {
				context.compiled_trigger.push_back(uint16_t(trigger::continent_pop | association_to_bool_code(a)));
				context.compiled_trigger.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors +=
					"continent trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "continent trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::casus_belli(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::casus_belli_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::casus_belli_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::casus_belli_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::casus_belli_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "casus_belli = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::casus_belli_from | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "casus_belli = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::casus_belli_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors +=
						"casus_belli trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
					"casus_belli trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "casus_belli trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}

	void trigger_body::military_access(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::military_access_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::military_access_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::military_access_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::military_access_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "military_access = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::military_access_from | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "military_access = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::military_access_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors +=
						"military_access trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
					"military_access trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "military_access trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}

	void trigger_body::prestige(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::prestige_this_pop | trigger::no_payload | association_to_trigger_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::prestige_this_state | trigger::no_payload | association_to_trigger_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::prestige_this_province | trigger::no_payload | association_to_trigger_code(a)));
				else if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::prestige_this_nation | trigger::no_payload | association_to_trigger_code(a)));
				else {
					err.accumulated_errors += "prestige = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::prestige_from | trigger::no_payload | association_to_trigger_code(a)));
				else {
					err.accumulated_errors += "prestige = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else {
				float fvalue = parse_float(value, line, err);
				context.compiled_trigger.push_back(uint16_t(trigger::prestige_value | association_to_trigger_code(a)));
				context.add_float_to_payload(fvalue);
			}
		} else {
			err.accumulated_errors += "prestige trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
															"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::badboy(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context);
	void trigger_body::has_building(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::state) {
			if(is_fixed_token_ci(value.data(), value.data() + value.length(), "factory")) {
				context.compiled_trigger.push_back(
					uint16_t(trigger::has_building_factory | trigger::no_payload | association_to_bool_code(a)));
			}  else if(auto it = context.outer_context.map_of_factory_names.find(std::string(value));
						it != context.outer_context.map_of_factory_names.end()) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_building_state | association_to_bool_code(a)));
				context.compiled_trigger.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors +=
					"has_building trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(context.main_slot == trigger::slot_contents::province) {
			if(is_fixed_token_ci(value.data(), value.data() + value.length(), "fort")) {
				context.compiled_trigger.push_back(
					uint16_t(trigger::has_building_fort | trigger::no_payload | association_to_bool_code(a)));
			} else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "railroad")) {
				context.compiled_trigger.push_back(
					uint16_t(trigger::has_building_railroad | trigger::no_payload | association_to_bool_code(a)));
			} else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "naval_base")) {
				context.compiled_trigger.push_back(
					uint16_t(trigger::has_building_naval_base | trigger::no_payload | association_to_bool_code(a)));
			} else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "bank")) {
				context.compiled_trigger.push_back(
					uint16_t(trigger::has_building_bank | trigger::no_payload | association_to_bool_code(a)));
			} else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "university")) {
				context.compiled_trigger.push_back(
					uint16_t(trigger::has_building_university | trigger::no_payload | association_to_bool_code(a)));
			} else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "factory")) {
				context.compiled_trigger.push_back(
					uint16_t(trigger::has_building_factory_from_province | trigger::no_payload | association_to_bool_code(a)));
			} else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "province_selector")) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_province_modifier | association_to_bool_code(a)));
				context.compiled_trigger.push_back(trigger::payload(economy::get_province_selector_modifier(context.outer_context.state)).value);
			} else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "immigrator")
			|| is_fixed_token_ci(value.data(), value.data() + value.length(), "immigrator_selector")
			|| is_fixed_token_ci(value.data(), value.data() + value.length(), "province_immigrator")) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_province_modifier | association_to_bool_code(a)));
				context.compiled_trigger.push_back(trigger::payload(economy::get_province_immigrator_modifier(context.outer_context.state)).value);
			} else if(auto it = context.outer_context.map_of_factory_names.find(std::string(value));
							it != context.outer_context.map_of_factory_names.end()) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_building_state_from_province | association_to_bool_code(a)));
				context.compiled_trigger.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors +=
					"has_building trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "has_building trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::empty(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::empty | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::empty_state | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "empty trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
															"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::is_blockaded(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::is_blockaded | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "is_blockaded trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}

	void trigger_body::has_country_modifier(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_modifiers.find(std::string(value));
			it != context.outer_context.map_of_modifiers.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_country_modifier | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_country_modifier_province | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "has_country_modifier trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			err.accumulated_errors += "has_country_modifier trigger supplied with an invalid modifier \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
		}
	}
	void trigger_body::has_province_modifier(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_modifiers.find(std::string(value));
			it != context.outer_context.map_of_modifiers.end()) {
			if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_province_modifier | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "has_province_modifier trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			err.accumulated_errors += "has_province_modifier trigger supplied with an invalid modifier \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
		}
	}
	void trigger_body::nationalvalue(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_modifiers.find(std::string(value));
			it != context.outer_context.map_of_modifiers.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::nationalvalue_nation | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::nationalvalue_province | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::nationalvalue_pop | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "nationalvalue trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			err.accumulated_errors +=
				"nationalvalue trigger supplied with an invalid modifier \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::region(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_state_names.find(std::string(value)); it != context.outer_context.map_of_state_names.end()) {
			if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::region | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::state) {
				context.compiled_trigger.push_back(uint16_t(trigger::region_state | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::region_pop | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "region trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
				"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else if(auto it = context.outer_context.map_of_region_names.find(std::string(value)); it != context.outer_context.map_of_region_names.end()) {
			if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::region_proper | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::state) {
				context.compiled_trigger.push_back(uint16_t(trigger::region_proper_state | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::region_proper_pop | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "region trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
				"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			err.accumulated_errors +=
				"region trigger not supplied with a state name (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}

	void trigger_body::tag(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(uint16_t(trigger::tag_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::tag_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors += "tag = this trigger used in an incorrect scope type " + slot_contents_to_string(context.this_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::tag_from_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(uint16_t(trigger::tag_from_province | trigger::no_payload | association_to_bool_code(a)));
				else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors += "tag = from trigger used in an incorrect scope type " + slot_contents_to_string(context.from_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::tag_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors += "tag trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors += "tag trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(context.main_slot == trigger::slot_contents::province) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(uint16_t(trigger::owned_by_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(uint16_t(trigger::owned_by_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(uint16_t(trigger::owned_by_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::owned_by_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors += "tag = this trigger used in an incorrect scope type " + slot_contents_to_string(context.this_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::owned_by_from_nation | trigger::no_payload | association_to_bool_code(a)));
				else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors += "tag = from trigger used in an incorrect scope type " + slot_contents_to_string(context.from_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::owned_by_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors += "tag trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors += "tag trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(context.main_slot == trigger::slot_contents::state) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(uint16_t(trigger::owned_by_state_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(uint16_t(trigger::owned_by_state_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(uint16_t(trigger::owned_by_state_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::owned_by_state_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors += "tag = this trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::owned_by_state_from_nation | trigger::no_payload | association_to_bool_code(a)));
				else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors += "tag = from trigger used in an incorrect scope type " + slot_contents_to_string(context.from_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::owned_by_state_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors += "tag trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors += "tag trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(context.main_slot == trigger::slot_contents::pop) {
			if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::tag_pop | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors += "tag trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors += "tag trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
			err.accumulated_errors += "tag trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::stronger_army_than(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(uint16_t(trigger::stronger_army_than_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::stronger_army_than_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(uint16_t(trigger::stronger_army_than_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(uint16_t(trigger::stronger_army_than_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "stronger_army_than = this trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::stronger_army_than_from_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(uint16_t(trigger::stronger_army_than_from_province | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "stronger_army_than = from trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::stronger_army_than_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors +=
					"stronger_army_than trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
				"stronger_army_than trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "stronger_army_than trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
			" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::neighbour(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::neighbour_this | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::neighbour_this_province | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "neighbour = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::neighbour_from | trigger::no_payload | association_to_bool_code(a)));
				else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::neighbour_from_province | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "neighbour = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::neighbour_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors +=
						"neighbour trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
					"neighbour trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "neighbour trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::country_units_in_state(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::state) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::country_units_in_state_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::country_units_in_state_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::country_units_in_state_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::country_units_in_state_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "country_units_in_state = this trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::country_units_in_state_from | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "country_units_in_state = from trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::country_units_in_state_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors +=
					"country_units_in_state trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
				"country_units_in_state trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			}
		} else {
			err.accumulated_errors += "country_units_in_state trigger used in an incorrect scope type " +
			slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
			std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::units_in_province(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::units_in_province_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::units_in_province_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::units_in_province_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::units_in_province_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "units_in_province = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::units_in_province_from | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "units_in_province = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_integer(value.data(), value.data() + value.length())) {
				context.compiled_trigger.push_back(uint16_t(trigger::units_in_province_value | association_to_trigger_code(a)));
				context.compiled_trigger.push_back(trigger::payload(uint16_t(parse_uint(value, line, err))).value);
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::units_in_province_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors +=
					"units_in_province trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
				"units_in_province trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			}
		} else {
			err.accumulated_errors += "units_in_province trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::war_with(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::war_with_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::war_with_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::war_with_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::war_with_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "war_with = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::war_with_from | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "war_with = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::war_with_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors +=
						"war_with trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
					"war_with trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "war_with trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
															"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::unit_in_battle(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::unit_in_battle | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "unit_in_battle trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::unit_has_leader(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::unit_has_leader | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "unit_has_leader trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::has_national_focus(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of trigger extension has_national_focus but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		if(auto it = context.outer_context.map_of_national_focuses.find(std::string(value));
		it != context.outer_context.map_of_national_focuses.end()) {
			if(context.main_slot == trigger::slot_contents::state) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_national_focus_state | trigger::no_payload | association_to_trigger_code(a)));
				context.compiled_trigger.push_back(trigger::payload(it->second).value);
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_national_focus_province | trigger::no_payload | association_to_trigger_code(a)));
				context.compiled_trigger.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors += "has_national_focus trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "has_national_focus given an invalid focus '" + std::string(value) + "' (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::total_amount_of_divisions(association_type a, int32_t value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::total_amount_of_divisions | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "total_amount_of_divisions trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(trigger::payload(uint16_t(value)).value);
	}
	void trigger_body::money(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::money | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::money_province | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "money trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) +
															"(" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::lost_national(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::lost_national | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "lost_national trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::is_vassal(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::is_vassal | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "is_vassal trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::ruling_party_ideology(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_ideologies.find(std::string(value));
			it != context.outer_context.map_of_ideologies.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::ruling_party_ideology_nation | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::ruling_party_ideology_pop | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::ruling_party_ideology_province | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "ruling_party_ideology trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second.id).value);
		} else {
			err.accumulated_errors += "ruling_party_ideology trigger supplied with an invalid ideology \"" + std::string(value) + "\" (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
		}
	}
	void trigger_body::ruling_party(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context);
	void trigger_body::has_leader(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context);
	void trigger_body::is_ideology_enabled(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_ideologies.find(std::string(value));
			it != context.outer_context.map_of_ideologies.end()) {
			context.compiled_trigger.push_back(uint16_t(trigger::is_ideology_enabled | association_to_bool_code(a)));
			context.compiled_trigger.push_back(trigger::payload(it->second.id).value);
		} else {
			err.accumulated_errors += "is_ideology_enabled trigger supplied with an invalid ideology \"" + std::string(value) + "\" (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
		}
	}
	void trigger_body::political_reform_want(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::political_reform_want_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::political_reform_want_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "political_reform_want trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::social_reform_want(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::social_reform_want_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::social_reform_want_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "social_reform_want trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::total_amount_of_ships(association_type a, int32_t value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::total_amount_of_ships | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "total_amount_of_ships trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(trigger::payload(uint16_t(value)).value);
	}
	void trigger_body::plurality(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::plurality | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::plurality_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "plurality trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value * 100.0f);
	}
	void trigger_body::corruption(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::corruption | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "corruption trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::is_state_religion(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_state_religion_state | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_state_religion_province | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_state_religion_pop | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "is_state_religion trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::is_primary_culture(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(is_this(value)) {
			if(context.main_slot == trigger::slot_contents::nation) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_primary_culture_nation_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_primary_culture_nation_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_primary_culture_nation_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_primary_culture_nation_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "is_primary_culture = this trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(context.main_slot == trigger::slot_contents::state) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_primary_culture_state_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_primary_culture_state_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_primary_culture_state_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_primary_culture_state_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "is_primary_culture = this trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(context.main_slot == trigger::slot_contents::province) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_primary_culture_province_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_primary_culture_province_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_primary_culture_province_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_primary_culture_province_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "is_primary_culture = this trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(context.main_slot == trigger::slot_contents::pop) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_primary_culture_pop_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_primary_culture_pop_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_primary_culture_pop_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_primary_culture_pop_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "is_primary_culture = this trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors += "is_primary_culture = this trigger used in an incorrect scope type " +
				slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
				std::to_string(line) + ")\n";
				return;
			}
		} else if(auto it = context.outer_context.map_of_culture_names.find(std::string(value)); it != context.outer_context.map_of_culture_names.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::primary_culture | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::culture_pop | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "is_primary_culture (treated as primary_culture or culture) trigger used in an incorrect scope type " +
				slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			bool v = parse_bool(value, line, err);
			if(context.main_slot == trigger::slot_contents::state)
			context.compiled_trigger.push_back(
					uint16_t(trigger::is_primary_culture_state | trigger::no_payload | association_to_bool_code(a, v)));
			else if(context.main_slot == trigger::slot_contents::province)
			context.compiled_trigger.push_back(
					uint16_t(trigger::is_primary_culture_province | trigger::no_payload | association_to_bool_code(a, v)));
			else if(context.main_slot == trigger::slot_contents::pop)
			context.compiled_trigger.push_back(
					uint16_t(trigger::is_primary_culture_pop | trigger::no_payload | association_to_bool_code(a, v)));
			else {
				err.accumulated_errors += "is_primary_culture trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		}
	}
	void trigger_body::is_accepted_culture(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(is_this(value)) {
			if(context.main_slot == trigger::slot_contents::nation) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_accepted_culture_nation_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_accepted_culture_nation_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_accepted_culture_nation_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_accepted_culture_nation_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "is_accepted_culture = this trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(context.main_slot == trigger::slot_contents::state) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_accepted_culture_state_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_accepted_culture_state_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_accepted_culture_state_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_accepted_culture_state_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "is_accepted_culture = this trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(context.main_slot == trigger::slot_contents::province) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_accepted_culture_province_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_accepted_culture_province_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_accepted_culture_province_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_accepted_culture_province_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "is_accepted_culture = this trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(context.main_slot == trigger::slot_contents::pop) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_accepted_culture_pop_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_accepted_culture_pop_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_accepted_culture_pop_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_accepted_culture_pop_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "is_accepted_culture = this trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors += "is_accepted_culture = this trigger used in an incorrect scope type " +
				slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
				std::to_string(line) + ")\n";
				return;
			}
		} else if(auto it = context.outer_context.map_of_culture_names.find(std::string(value)); it != context.outer_context.map_of_culture_names.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::accepted_culture | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "is_accepted_culture (treated as accepted_culture) trigger used in an incorrect scope type " +
				slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			bool v = parse_bool(value, line, err);
			if(context.main_slot == trigger::slot_contents::state) {
				context.compiled_trigger.push_back(
					uint16_t(trigger::is_accepted_culture_state | trigger::no_payload | association_to_bool_code(a, v)));
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(
					uint16_t(trigger::is_accepted_culture_province | trigger::no_payload | association_to_bool_code(a, v)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(
					uint16_t(trigger::is_accepted_culture_pop | trigger::no_payload | association_to_bool_code(a, v)));
			} else {
				err.accumulated_errors += "is_accepted_culture trigger used in an incorrect scope type " +
				slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
				std::to_string(line) + ")\n";
				return;
			}
		}
	}
	void trigger_body::is_coastal(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_coastal_state | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_coastal_province | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "is_coastal trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::in_sphere(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::in_sphere_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::in_sphere_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::in_sphere_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::in_sphere_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "in_sphere = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::in_sphere_from | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "in_sphere = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::in_sphere_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors +=
						"in_sphere trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
					"in_sphere trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "in_sphere trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::produces(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_commodity_names.find(std::string(value));
			it != context.outer_context.map_of_commodity_names.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::produces_nation | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::state) {
				context.compiled_trigger.push_back(uint16_t(trigger::produces_state | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::produces_province | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::produces_pop | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "produces trigger used in an incorrect scope type " +
				slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			err.accumulated_errors += "produces trigger supplied with an invalid commodity \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::has_pop_type(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_poptypes.find(std::string(value)); it != context.outer_context.map_of_poptypes.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_pop_type_nation | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::state) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_pop_type_state | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_pop_type_province | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::has_pop_type_pop | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "has_pop_type trigger used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			err.accumulated_errors += "has_pop_type trigger supplied with an invalid type name \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void trigger_body::total_pops(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::total_pops_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::total_pops_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::total_pops_province | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::total_pops_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "total_pops trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::average_militancy(association_type a, float value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::average_militancy_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::average_militancy_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::average_militancy_province | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "average_militancy trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::average_consciousness(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::average_consciousness_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(uint16_t(trigger::average_consciousness_state | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::average_consciousness_province | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "average_consciousness trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::is_next_reform(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_ioptions.find(std::string(value));
			it != context.outer_context.map_of_ioptions.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::is_next_reform_nation | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::is_next_reform_pop | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "is_next_reform trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(it->second.id).value);
		} else if(auto itb = context.outer_context.map_of_roptions.find(std::string(value));
						itb != context.outer_context.map_of_roptions.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_trigger.push_back(uint16_t(trigger::is_next_rreform_nation | association_to_bool_code(a)));
			} else if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_trigger.push_back(uint16_t(trigger::is_next_rreform_pop | association_to_bool_code(a)));
			} else {
				err.accumulated_errors += "is_next_reform trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(itb->second.id).value);
		} else {
			err.accumulated_errors += "is_next_reform trigger supplied with an invalid issue/reform \"" + std::string(value) + "\" (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
		}
	}
	void trigger_body::rebel_power_fraction(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::rebel_power_fraction | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "rebel_power_fraction trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::recruited_percentage(association_type a, float value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_trigger.push_back(uint16_t(trigger::recruited_percentage_nation | association_to_trigger_code(a)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(uint16_t(trigger::recruited_percentage_pop | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "recruited_percentage trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.add_float_to_payload(value);
	}
	void trigger_body::has_culture_core(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::has_culture_core | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::province && context.this_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::has_culture_core_province_this_pop | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "has_culture_core trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::nationalism(association_type a, int32_t value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(uint16_t(trigger::nationalism | association_to_trigger_code(a)));
		} else {
			err.accumulated_errors += "nationalism trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(trigger::payload(uint16_t(value)).value);
	}
	void trigger_body::is_overseas(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_overseas | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_overseas_pop | trigger::no_payload | association_to_bool_code(a, value)));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::is_overseas_state | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "is_overseas trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::controlled_by_rebels(association_type a, bool value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_trigger.push_back(
				uint16_t(trigger::controlled_by_rebels | trigger::no_payload | association_to_bool_code(a, value)));
		} else {
			err.accumulated_errors += "controlled_by_rebels trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::controlled_by(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::controlled_by_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::controlled_by_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::controlled_by_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::controlled_by_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "controlled_by = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::controlled_by_from | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "controlled_by = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_reb(value)) {
				if(context.from_slot == trigger::slot_contents::rebel)
				context.compiled_trigger.push_back(
						uint16_t(trigger::controlled_by_reb | trigger::no_payload | association_to_bool_code(a)));
				else {
					context.compiled_trigger.push_back(
						uint16_t(trigger::controlled_by_rebels | trigger::no_payload | association_to_bool_code(a)));
				}
			} else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "owner")) {
				context.compiled_trigger.push_back(
					uint16_t(trigger::controlled_by_owner | trigger::no_payload | association_to_bool_code(a)));
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::controlled_by_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors +=
						"controlled_by trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
					"controlled_by trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "controlled_by trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::truce_with(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::truce_with_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::truce_with_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::truce_with_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::truce_with_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "truce_with = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::truce_with_from | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "truce_with = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::truce_with_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors +=
						"truce_with trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
					"truce_with trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "truce_with trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::is_sphere_leader_of(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_sphere_leader_of_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_sphere_leader_of_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_sphere_leader_of_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_sphere_leader_of_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "is_sphere_leader_of = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_sphere_leader_of_from | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "is_sphere_leader_of = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::is_sphere_leader_of_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors += "is_sphere_leader_of trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors += "is_sphere_leader_of trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "is_sphere_leader_of trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::constructing_cb(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::constructing_cb_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::constructing_cb_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::constructing_cb_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::constructing_cb_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "constructing_cb = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::constructing_cb_from | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "constructing_cb = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::constructing_cb_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors +=
						"constructing_cb trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
					"constructing_cb trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "constructing_cb trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::vassal_of(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::vassal_of_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(uint16_t(trigger::vassal_of_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(uint16_t(trigger::vassal_of_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(uint16_t(trigger::vassal_of_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "vassal_of = this trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::vassal_of_from | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "vassal_of = from trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::vassal_of_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors += "vassal_of trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors += "vassal_of trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(context.main_slot == trigger::slot_contents::province) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::vassal_of_province_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(uint16_t(trigger::vassal_of_province_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(uint16_t(trigger::vassal_of_province_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(uint16_t(trigger::vassal_of_province_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "vassal_of = this trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::vassal_of_province_from | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "vassal_of = from trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::vassal_of_province_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors += "vassal_of trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors += "vassal_of trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "vassal_of trigger used in an incorrect scope type " +
			slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::substate_of(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::substate_of_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::substate_of_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::substate_of_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::substate_of_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "substate_of = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::substate_of_from | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "substate_of = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::substate_of_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors +=
						"substate_of trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
					"substate_of trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "substate_of trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::is_our_vassal(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_our_vassal_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_our_vassal_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_our_vassal_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_our_vassal_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "is_our_vassal = this trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_our_vassal_from | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "is_our_vassal = from trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::is_our_vassal_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors +=
					"is_our_vassal trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
				"is_our_vassal trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(context.main_slot == trigger::slot_contents::province) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_our_vassal_province_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_our_vassal_province_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_our_vassal_province_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_our_vassal_province_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "is_our_vassal = this trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::is_our_vassal_province_from | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "is_our_vassal = from trigger used in an incorrect scope type " +
					slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::is_our_vassal_province_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors +=
					"is_our_vassal trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
				"is_our_vassal trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "is_our_vassal trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::this_culture_union(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::this_culture_union_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::this_culture_union_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::this_culture_union_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::this_culture_union_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "this_culture_union = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "this_union")) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::this_culture_union_this_union_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::this_culture_union_this_union_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::this_culture_union_this_union_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::this_culture_union_this_union_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "this_culture_union = this_union trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::this_culture_union_from | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "this_culture_union = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::this_culture_union_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors += "this_culture_union trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors += "this_culture_union trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "this_culture_union trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::alliance_with(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::alliance_with_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::alliance_with_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::alliance_with_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::alliance_with_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "alliance_with = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::alliance_with_from | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "alliance_with = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::alliance_with_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors +=
						"alliance_with trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
					"alliance_with trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "alliance_with trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::in_default(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_fixed_token_ci(value.data(), value.data() + value.length(), "yes") ||
			is_fixed_token_ci(value.data(), value.data() + value.length(), "no")) {

				context.compiled_trigger.push_back(uint16_t(trigger::in_default_bool | trigger::no_payload | association_to_bool_code(a, parse_bool(value, line, err))));

			} else if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::in_default_this_nation | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(uint16_t(trigger::in_default_this_state | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(uint16_t(trigger::in_default_this_province | trigger::no_payload | association_to_bool_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(uint16_t(trigger::in_default_this_pop | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "in_default = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::in_default_from | trigger::no_payload | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "in_default = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::in_default_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors += "in_default trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors += "in_default trigger supplied with an invalid value \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "in_default trigger used in an incorrect scope type " +
			slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::industrial_score(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::industrial_score_this_nation | trigger::no_payload | association_to_trigger_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::industrial_score_this_state | trigger::no_payload | association_to_trigger_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::industrial_score_this_province | trigger::no_payload | association_to_trigger_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::industrial_score_this_pop | trigger::no_payload | association_to_trigger_code(a)));
				else {
					err.accumulated_errors += "industrial_score = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::industrial_score_from_nation | trigger::no_payload | association_to_trigger_code(a)));
				else {
					err.accumulated_errors += "industrial_score = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(!parsers::is_integer(value.data(), value.data() + value.length()) && value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::industrial_score_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors += "industrial_score trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::industrial_score_value | association_to_trigger_code(a)));
				context.compiled_trigger.push_back(trigger::payload(uint16_t(parse_uint(value, line, err))).value);
			}
		} else {
			err.accumulated_errors += "industrial_score trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::military_score(association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::military_score_this_nation | trigger::no_payload | association_to_trigger_code(a)));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(
						uint16_t(trigger::military_score_this_state | trigger::no_payload | association_to_trigger_code(a)));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(
						uint16_t(trigger::military_score_this_province | trigger::no_payload | association_to_trigger_code(a)));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(
						uint16_t(trigger::military_score_this_pop | trigger::no_payload | association_to_trigger_code(a)));
				else {
					err.accumulated_errors += "military_score = this trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(
						uint16_t(trigger::military_score_from_nation | trigger::no_payload | association_to_trigger_code(a)));
				else {
					err.accumulated_errors += "military_score = from trigger used in an incorrect scope type " +
																	slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(!parsers::is_integer(value.data(), value.data() + value.length()) && value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_trigger.push_back(uint16_t(trigger::military_score_tag | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
					err.accumulated_errors += "military_score trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::military_score_value | association_to_trigger_code(a)));
				context.compiled_trigger.push_back(trigger::payload(uint16_t(parse_uint(value, line, err))).value);
			}
		} else {
			err.accumulated_errors += "military_score trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::test(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of trigger extension test but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		if(auto it = context.outer_context.map_of_stored_triggers.find(std::string(value)); it != context.outer_context.map_of_stored_triggers.end()) {
			dcon::stored_trigger_id st;
			for(auto r : it->second) {
				if((r.main_slot == context.main_slot || r.main_slot == trigger::slot_contents::empty)
				&& (r.from_slot == context.from_slot || r.from_slot == trigger::slot_contents::empty)
				&& (r.this_slot == context.this_slot || r.this_slot == trigger::slot_contents::empty)) {

					if(!st) {
						st = r.id;
					} else {
						err.accumulated_errors += "test trigger found multiple matching possibilities for the name " +
						std::string(value) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
						return;
					}
				}
			}
			if(st) {
				context.compiled_trigger.push_back(uint16_t(trigger::test | association_to_bool_code(a)));
				context.compiled_trigger.push_back(trigger::payload(st).value);
			} else {
				err.accumulated_errors += "test trigger was unable to find a version of " + std::string(value) + " that matched the local parameters (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			}
		} else {
			err.accumulated_errors += "test trigger used with an unknown scripted trigger " + std::string(value) + " (" + err.file_name + ", line " +
			std::to_string(line) + ")\n";
		}
	}
	void trigger_body::is_possible_vassal(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(value.length() == 3) {
			if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
				it != context.outer_context.map_of_ident_names.end()) {
				if(context.main_slot == trigger::slot_contents::nation) {
					context.compiled_trigger.push_back(uint16_t(trigger::is_possible_vassal | association_to_bool_code(a)));
					context.compiled_trigger.push_back(trigger::payload(it->second).value);
				} else {
					err.accumulated_errors += "is_possible_vassal trigger used in an incorrect scope type " +
						slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
						std::to_string(line) + ")\n";
					return;
				}
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors += "is_possible_vassal trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
			}
		} else {
			context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
			err.accumulated_errors +=
				"is_possible_vassal trigger supplied with an invalid tag \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}

	void trigger_body::diplomatic_influence(tr_diplomatic_influence const& value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "diplomatic_influence trigger used in an incorrect scope type " +
															slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		} else if(is_from(value.who)) {
			if(context.from_slot == trigger::slot_contents::nation)
			context.compiled_trigger.push_back(
					uint16_t(trigger::diplomatic_influence_from_nation | association_to_trigger_code(value.a)));
			else if(context.from_slot == trigger::slot_contents::province)
			context.compiled_trigger.push_back(
					uint16_t(trigger::diplomatic_influence_from_province | association_to_trigger_code(value.a)));
			else {
				err.accumulated_errors += "diplomatic_influence trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(uint16_t(value.value_)).value);
		} else if(is_this(value.who)) {
			if(context.this_slot == trigger::slot_contents::nation)
			context.compiled_trigger.push_back(
					uint16_t(trigger::diplomatic_influence_this_nation | association_to_trigger_code(value.a)));
			else if(context.this_slot == trigger::slot_contents::province)
			context.compiled_trigger.push_back(
					uint16_t(trigger::diplomatic_influence_this_province | association_to_trigger_code(value.a)));
			else {
				err.accumulated_errors += "diplomatic_influence trigger used in an incorrect scope type " +
																slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(uint16_t(value.value_)).value);
		} else if(value.who.length() == 3) {
			if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value.who[0], value.who[1], value.who[2]));
				it != context.outer_context.map_of_ident_names.end()) {
				context.compiled_trigger.push_back(uint16_t(trigger::diplomatic_influence_tag | association_to_trigger_code(value.a)));
				context.compiled_trigger.push_back(trigger::payload(uint16_t(value.value_)).value);
				context.compiled_trigger.push_back(trigger::payload(it->second).value);
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors += "diplomatic_influence trigger supplied with an invalid tag \"" + std::string(value.who) + "\" (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
			}
		} else {
			context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
			err.accumulated_errors += "diplomatic_influence trigger supplied with an invalid tag \"" + std::string(value.who) + "\" (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::pop_unemployment(tr_pop_unemployment const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(is_this(value.type)) {
			if(context.this_slot != trigger::slot_contents::pop) {
				err.accumulated_errors += "pop_unemployment = this trigger used in an invalid context (" + err.file_name + ", line " +
				std::to_string(line) + ")\n";
				return;
			} else if(context.main_slot == trigger::slot_contents::nation)
			context.compiled_trigger.push_back(
					uint16_t(trigger::pop_unemployment_nation_this_pop | association_to_trigger_code(value.a)));
			else if(context.main_slot == trigger::slot_contents::state)
			context.compiled_trigger.push_back(
					uint16_t(trigger::pop_unemployment_state_this_pop | association_to_trigger_code(value.a)));
			else if(context.main_slot == trigger::slot_contents::province)
			context.compiled_trigger.push_back(
					uint16_t(trigger::pop_unemployment_province_this_pop | association_to_trigger_code(value.a)));
			else {
				err.accumulated_errors += "pop_unemployment = this trigger used in an invalid context (" + err.file_name + ", line " +
				std::to_string(line) + ")\n";
				return;
			}
			context.add_float_to_payload(value.value_);
		} else if(is_from(value.type)) {
			if(context.this_slot != trigger::slot_contents::pop) {
				err.accumulated_errors += "pop_unemployment = this trigger used in an invalid context (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			} else if(context.main_slot == trigger::slot_contents::nation)
			context.compiled_trigger.push_back(
					uint16_t(trigger::pop_unemployment_nation_this_pop | association_to_trigger_code(value.a)));
			else if(context.main_slot == trigger::slot_contents::state)
			context.compiled_trigger.push_back(
					uint16_t(trigger::pop_unemployment_state_this_pop | association_to_trigger_code(value.a)));
			else if(context.main_slot == trigger::slot_contents::province)
			context.compiled_trigger.push_back(
					uint16_t(trigger::pop_unemployment_province_this_pop | association_to_trigger_code(value.a)));
			else {
				err.accumulated_errors += "pop_unemployment = this trigger used in an invalid context (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.add_float_to_payload(value.value_);
		} else if(auto it = context.outer_context.map_of_poptypes.find(std::string(value.type));
						it != context.outer_context.map_of_poptypes.end()) {
			if(context.main_slot == trigger::slot_contents::nation)
			context.compiled_trigger.push_back(uint16_t(trigger::pop_unemployment_nation | association_to_trigger_code(value.a)));
			else if(context.main_slot == trigger::slot_contents::state)
			context.compiled_trigger.push_back(uint16_t(trigger::pop_unemployment_state | association_to_trigger_code(value.a)));
			else if(context.main_slot == trigger::slot_contents::province)
			context.compiled_trigger.push_back(uint16_t(trigger::pop_unemployment_province | association_to_trigger_code(value.a)));
			else if(context.main_slot == trigger::slot_contents::pop)
			context.compiled_trigger.push_back(uint16_t(trigger::pop_unemployment_pop | association_to_trigger_code(value.a)));
			else {
				err.accumulated_errors +=
					"pop_unemployment trigger used in an invalid context (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}

			context.add_float_to_payload(value.value_);
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			err.accumulated_errors += "pop_unemployment trigger supplied with an invalid pop type (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::relation(tr_relation const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors +=
				"relation trigger used in an invalid context (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		} else if(is_from(value.who)) {
			if(context.from_slot == trigger::slot_contents::nation)
			context.compiled_trigger.push_back(uint16_t(trigger::relation_from_nation | association_to_trigger_code(value.a)));
			else if(context.from_slot == trigger::slot_contents::province)
			context.compiled_trigger.push_back(uint16_t(trigger::relation_from_province | association_to_trigger_code(value.a)));
			else {
				err.accumulated_errors +=
					"relation = from trigger used in an invalid context (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(int16_t(value.value_)).value);
		} else if(is_this(value.who)) {
			if(context.this_slot == trigger::slot_contents::nation)
			context.compiled_trigger.push_back(uint16_t(trigger::relation_this_nation | association_to_trigger_code(value.a)));
			else if(context.this_slot == trigger::slot_contents::province)
			context.compiled_trigger.push_back(uint16_t(trigger::relation_this_province | association_to_trigger_code(value.a)));
			else if(context.this_slot == trigger::slot_contents::pop)
			context.compiled_trigger.push_back(uint16_t(trigger::relation_this_pop | association_to_trigger_code(value.a)));
			else {
				err.accumulated_errors +=
					"relation = this trigger used in an invalid context (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(int16_t(value.value_)).value);
		} else if(value.who.length() == 3) {
			if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value.who[0], value.who[1], value.who[2]));
				it != context.outer_context.map_of_ident_names.end()) {
				context.compiled_trigger.push_back(uint16_t(trigger::relation_tag | association_to_trigger_code(value.a)));
				context.compiled_trigger.push_back(trigger::payload(int16_t(value.value_)).value);
				context.compiled_trigger.push_back(trigger::payload(it->second).value);
			} else {
				context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
				err.accumulated_errors +=
					"relation trigger supplied with an invalid tag \"" + std::string(value.who) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			}
		} else {
			context.compiled_trigger.push_back(uint16_t(trigger::always | trigger::no_payload | trigger::association_ne));
			err.accumulated_errors +=
				"relation trigger supplied with an invalid tag \"" + std::string(value.who) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::check_variable(tr_check_variable const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		context.compiled_trigger.push_back(uint16_t(trigger::check_variable | association_to_trigger_code(value.a)));
		context.add_float_to_payload(value.value_);
		context.compiled_trigger.push_back(
			trigger::payload(context.outer_context.get_national_variable(std::string(value.which))).value);
	}
	void trigger_body::upper_house(tr_upper_house const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_ideologies.find(std::string(value.ideology));
			it != context.outer_context.map_of_ideologies.end()) {
			if(context.main_slot != trigger::slot_contents::nation) {
				err.accumulated_errors +=
					"upper_house trigger used in an invalid context (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(uint16_t(trigger::upper_house | association_to_trigger_code(value.a)));
			context.add_float_to_payload(value.value_);
			context.compiled_trigger.push_back(trigger::payload(it->second.id).value);
		} else {
			err.accumulated_errors +=
				"upper_house trigger supplied with an invalid ideology \"" + std::string(value.ideology) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void trigger_body::unemployment_by_type(tr_unemployment_by_type const& value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_poptypes.find(std::string(value.type));
			it != context.outer_context.map_of_poptypes.end()) {
			if(context.main_slot == trigger::slot_contents::nation)
			context.compiled_trigger.push_back(uint16_t(trigger::unemployment_by_type_nation | association_to_trigger_code(value.a)));
			else if(context.main_slot == trigger::slot_contents::state)
			context.compiled_trigger.push_back(uint16_t(trigger::unemployment_by_type_state | association_to_trigger_code(value.a)));
			else if(context.main_slot == trigger::slot_contents::province)
			context.compiled_trigger.push_back(
					uint16_t(trigger::unemployment_by_type_province | association_to_trigger_code(value.a)));
			else if(context.main_slot == trigger::slot_contents::pop)
			context.compiled_trigger.push_back(uint16_t(trigger::unemployment_by_type_pop | association_to_trigger_code(value.a)));
			else {
				err.accumulated_errors += "unemployment_by_type trigger used in an invalid context (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.add_float_to_payload(value.value_);
			context.compiled_trigger.push_back(trigger::payload(it->second).value);
		} else {
			err.accumulated_errors += "unemployment_by_type trigger supplied with an invalid pop type \"" + std::string(value.type) + "\" (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}

	void trigger_body::party_loyalty(tr_party_loyalty const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(auto it = context.outer_context.map_of_ideologies.find(std::string(value.ideology));
			it != context.outer_context.map_of_ideologies.end()) {
			if(value.province_id != 0) {
				if(0 <= value.province_id && size_t(value.province_id) < context.outer_context.original_id_to_prov_id_map.size()) {
					if(context.main_slot == trigger::slot_contents::nation)
					context.compiled_trigger.push_back(
							uint16_t(trigger::party_loyalty_nation_province_id | association_to_trigger_code(value.a)));
					else if(context.from_slot == trigger::slot_contents::nation)
					context.compiled_trigger.push_back(
							uint16_t(trigger::party_loyalty_from_nation_province_id | association_to_trigger_code(value.a)));
					else if(context.main_slot == trigger::slot_contents::province)
					context.compiled_trigger.push_back(
							uint16_t(trigger::party_loyalty_province_province_id | association_to_trigger_code(value.a)));
					else if(context.from_slot == trigger::slot_contents::province)
					context.compiled_trigger.push_back(
							uint16_t(trigger::party_loyalty_from_province_province_id | association_to_trigger_code(value.a)));
					else {
						err.accumulated_errors +=
							"party_loyalty trigger used in an invalid context (" + err.file_name + ", line " + std::to_string(line) + ")\n";
						return;
					}
					context.compiled_trigger.push_back(
						trigger::payload(context.outer_context.original_id_to_prov_id_map[value.province_id]).value);
					context.compiled_trigger.push_back(trigger::payload(int16_t(value.value_)).value);
					context.compiled_trigger.push_back(trigger::payload(it->second.id).value);
				} else {
					err.accumulated_errors += "party_loyalty trigger supplied with an invalid province id " + std::to_string(value.province_id) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else {
				if(context.main_slot == trigger::slot_contents::province) {
					if(context.from_slot == trigger::slot_contents::nation)
					context.compiled_trigger.push_back(
							uint16_t(trigger::party_loyalty_from_nation_scope_province | association_to_trigger_code(value.a)));
					else if(context.from_slot == trigger::slot_contents::province)
					context.compiled_trigger.push_back(
							uint16_t(trigger::party_loyalty_from_province_scope_province | association_to_trigger_code(value.a)));
					else
					context.compiled_trigger.push_back(uint16_t(trigger::party_loyalty_generic | association_to_trigger_code(value.a)));
				} else {
					err.accumulated_errors +=
						"party_loyalty trigger used in an invalid context (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
				context.compiled_trigger.push_back(trigger::payload(int16_t(value.value_)).value);
				context.compiled_trigger.push_back(trigger::payload(it->second.id).value);
			}
		} else {
			err.accumulated_errors +=
				"party_loyalty trigger supplied with an invalid ideology \"" + std::string(value.ideology) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}

	void trigger_body::can_build_in_province(tr_can_build_in_province const& value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot != trigger::slot_contents::province) {
			err.accumulated_errors +=
				"can_build_in_province trigger used in an invalid context (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		} else if(value.limit_to_world_greatest_level) {
			if(context.this_slot == trigger::slot_contents::nation) {
				if(is_fixed_token_ci(value.building.data(), value.building.data() + value.building.length(), "railroad"))
				context.compiled_trigger.push_back(uint16_t(
						trigger::can_build_in_province_railroad_yes_limit_this_nation | trigger::association_eq | trigger::no_payload));
				else if(is_fixed_token_ci(value.building.data(), value.building.data() + value.building.length(), "naval_base"))
				context.compiled_trigger.push_back(uint16_t(
						trigger::can_build_in_province_naval_base_yes_limit_this_nation | trigger::association_eq | trigger::no_payload));
				else if(is_fixed_token_ci(value.building.data(), value.building.data() + value.building.length(), "fort"))
				context.compiled_trigger.push_back(uint16_t(
						trigger::can_build_in_province_fort_yes_limit_this_nation | trigger::association_eq | trigger::no_payload));
			} else if(context.from_slot == trigger::slot_contents::nation) {
				if(is_fixed_token_ci(value.building.data(), value.building.data() + value.building.length(), "railroad"))
				context.compiled_trigger.push_back(uint16_t(
						trigger::can_build_in_province_railroad_yes_limit_from_nation | trigger::association_eq | trigger::no_payload));
				else if(is_fixed_token_ci(value.building.data(), value.building.data() + value.building.length(), "naval_base"))
				context.compiled_trigger.push_back(uint16_t(
						trigger::can_build_in_province_naval_base_yes_limit_from_nation | trigger::association_eq | trigger::no_payload));
				else if(is_fixed_token_ci(value.building.data(), value.building.data() + value.building.length(), "fort"))
				context.compiled_trigger.push_back(uint16_t(
						trigger::can_build_in_province_fort_yes_limit_from_nation | trigger::association_eq | trigger::no_payload));
			} else {
				err.accumulated_errors += "can_build_in_province trigger used in an invalid context (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else {
			if(context.this_slot == trigger::slot_contents::nation) {
				if(is_fixed_token_ci(value.building.data(), value.building.data() + value.building.length(), "railroad"))
				context.compiled_trigger.push_back(uint16_t(
						trigger::can_build_in_province_railroad_no_limit_this_nation | trigger::association_eq | trigger::no_payload));
				else if(is_fixed_token_ci(value.building.data(), value.building.data() + value.building.length(), "naval_base"))
				context.compiled_trigger.push_back(uint16_t(
						trigger::can_build_in_province_naval_base_no_limit_this_nation | trigger::association_eq | trigger::no_payload));
				else if(is_fixed_token_ci(value.building.data(), value.building.data() + value.building.length(), "fort"))
				context.compiled_trigger.push_back(
						uint16_t(trigger::can_build_in_province_fort_no_limit_this_nation | trigger::association_eq | trigger::no_payload));
			} else if(context.from_slot == trigger::slot_contents::nation) {
				if(is_fixed_token_ci(value.building.data(), value.building.data() + value.building.length(), "railroad"))
				context.compiled_trigger.push_back(uint16_t(
						trigger::can_build_in_province_railroad_no_limit_from_nation | trigger::association_eq | trigger::no_payload));
				else if(is_fixed_token_ci(value.building.data(), value.building.data() + value.building.length(), "naval_base"))
				context.compiled_trigger.push_back(uint16_t(
						trigger::can_build_in_province_naval_base_no_limit_from_nation | trigger::association_eq | trigger::no_payload));
				else if(is_fixed_token_ci(value.building.data(), value.building.data() + value.building.length(), "fort"))
				context.compiled_trigger.push_back(
						uint16_t(trigger::can_build_in_province_fort_no_limit_from_nation | trigger::association_eq | trigger::no_payload));
			} else {
				err.accumulated_errors += "can_build_in_province trigger used in an invalid context (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		}
	}
	void trigger_body::can_build_railway_in_capital(tr_can_build_railway_in_capital const& value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "can_build_railway_in_capital trigger used in an invalid context (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		} else if(value.in_whole_capital_state) {
			if(value.limit_to_world_greatest_level)
			context.compiled_trigger.push_back(uint16_t(
					trigger::can_build_railway_in_capital_yes_whole_state_yes_limit | trigger::association_eq | trigger::no_payload));
			else
			context.compiled_trigger.push_back(uint16_t(
					trigger::can_build_railway_in_capital_yes_whole_state_no_limit | trigger::association_eq | trigger::no_payload));
		} else {
			if(value.limit_to_world_greatest_level)
			context.compiled_trigger.push_back(uint16_t(
					trigger::can_build_railway_in_capital_no_whole_state_yes_limit | trigger::association_eq | trigger::no_payload));
			else
			context.compiled_trigger.push_back(uint16_t(
					trigger::can_build_railway_in_capital_no_whole_state_no_limit | trigger::association_eq | trigger::no_payload));
		}
	}
	void trigger_body::can_build_fort_in_capital(tr_can_build_fort_in_capital const& value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "can_build_fort_in_capital trigger used in an invalid context (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		} else if(value.in_whole_capital_state) {
			if(value.limit_to_world_greatest_level)
			context.compiled_trigger.push_back(uint16_t(
					trigger::can_build_fort_in_capital_yes_whole_state_yes_limit | trigger::association_eq | trigger::no_payload));
			else
			context.compiled_trigger.push_back(uint16_t(
					trigger::can_build_fort_in_capital_yes_whole_state_no_limit | trigger::association_eq | trigger::no_payload));
		} else {
			if(value.limit_to_world_greatest_level)
			context.compiled_trigger.push_back(uint16_t(
					trigger::can_build_fort_in_capital_no_whole_state_yes_limit | trigger::association_eq | trigger::no_payload));
			else
			context.compiled_trigger.push_back(
					uint16_t(trigger::can_build_fort_in_capital_no_whole_state_no_limit | trigger::association_eq | trigger::no_payload));
		}
	}

	void trigger_body::work_available(tr_work_available const& value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(value.pop_type_list.size() > 1 || value.pop_type_list.size() == 0) {
			err.accumulated_errors += "work_available trigger used with an unsupported number of worker types (" + err.file_name +
															", line " + std::to_string(line) + ")\n";
			return;
		}

		if(context.main_slot == trigger::slot_contents::nation)
		context.compiled_trigger.push_back(uint16_t(trigger::work_available_nation | trigger::association_eq));
		else if(context.main_slot == trigger::slot_contents::state)
		context.compiled_trigger.push_back(uint16_t(trigger::work_available_state | trigger::association_eq));
		else if(context.main_slot == trigger::slot_contents::province)
		context.compiled_trigger.push_back(uint16_t(trigger::work_available_province | trigger::association_eq));
		else {
			err.accumulated_errors +=
				"work_available trigger used in an invalid context (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_trigger.push_back(trigger::payload(value.pop_type_list[0]).value);
	}

	void trigger_body::any_value(std::string_view label, association_type a, std::string_view value, error_handler& err, int32_t line,
		trigger_building_context& context) {
		std::string str_label{ label };
		if(auto itg = context.outer_context.map_of_ioptions.find(str_label); itg != context.outer_context.map_of_ioptions.end()) {
			if(context.main_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::variable_issue_name_nation | association_to_trigger_code(a)));
			else if(context.main_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(uint16_t(trigger::variable_issue_name_pop | association_to_trigger_code(a)));
			else if(context.main_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(uint16_t(trigger::variable_issue_name_province | association_to_trigger_code(a)));
			else if(context.main_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(uint16_t(trigger::variable_issue_name_state | association_to_trigger_code(a)));
			else {
				err.accumulated_errors += "named issue option trigger used in an invalid context (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(itg->second.id).value);
			context.add_float_to_payload(parse_float(value, line, err) / 100.0f);
		} else if(auto itf = context.outer_context.map_of_iissues.find(str_label); itf != context.outer_context.map_of_iissues.end()) {
			if(auto itopt = context.outer_context.map_of_ioptions.find(std::string(value));
				itopt != context.outer_context.map_of_ioptions.end()) {
				if(context.main_slot == trigger::slot_contents::nation)
					context.compiled_trigger.push_back(uint16_t(trigger::variable_issue_group_name_nation | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::pop)
					context.compiled_trigger.push_back(uint16_t(trigger::variable_issue_group_name_pop | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::province)
					context.compiled_trigger.push_back(uint16_t(trigger::variable_issue_group_name_province | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::state)
					context.compiled_trigger.push_back(uint16_t(trigger::variable_issue_group_name_state | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "named issue trigger used in an invalid context (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
				context.compiled_trigger.push_back(trigger::payload(itf->second).value);
				context.compiled_trigger.push_back(trigger::payload(itopt->second.id).value);
			} else {
				err.accumulated_errors += "named issue trigger used with an invalid option name (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(auto ith = context.outer_context.map_of_reforms.find(str_label); ith != context.outer_context.map_of_reforms.end()) {
			if(auto itopt = context.outer_context.map_of_roptions.find(std::string(value));
				itopt != context.outer_context.map_of_roptions.end()) {
				if(context.main_slot == trigger::slot_contents::nation)
					context.compiled_trigger.push_back(uint16_t(trigger::variable_reform_group_name_nation | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::pop)
					context.compiled_trigger.push_back(uint16_t(trigger::variable_reform_group_name_pop | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::province)
					context.compiled_trigger.push_back(uint16_t(trigger::variable_reform_group_name_province | association_to_bool_code(a)));
				else if(context.main_slot == trigger::slot_contents::state)
					context.compiled_trigger.push_back(uint16_t(trigger::variable_reform_group_name_state | association_to_bool_code(a)));
				else {
					err.accumulated_errors += "named reform trigger used in an invalid context (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
				context.compiled_trigger.push_back(trigger::payload(ith->second).value);
				context.compiled_trigger.push_back(trigger::payload(itopt->second.id).value);
			} else {
				err.accumulated_errors += "named reform trigger used with an invalid option name (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(auto it = context.outer_context.map_of_technologies.find(str_label);
			it != context.outer_context.map_of_technologies.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				bool bvalue = parse_bool(value, line, err);
				context.compiled_trigger.push_back(uint16_t(trigger::technology | association_to_bool_code(a, bvalue)));
				context.compiled_trigger.push_back(trigger::payload(it->second.id).value);
			} else if(context.main_slot == trigger::slot_contents::province) {
				bool bvalue = parse_bool(value, line, err);
				context.compiled_trigger.push_back(uint16_t(trigger::technology_province | association_to_bool_code(a, bvalue)));
				context.compiled_trigger.push_back(trigger::payload(it->second.id).value);
			} else if(context.main_slot == trigger::slot_contents::pop) {
				bool bvalue = parse_bool(value, line, err);
				context.compiled_trigger.push_back(uint16_t(trigger::technology_pop | association_to_bool_code(a, bvalue)));
				context.compiled_trigger.push_back(trigger::payload(it->second.id).value);
			} else {
				err.accumulated_errors += "named technology trigger used in an invalid context (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(auto itb = context.outer_context.map_of_inventions.find(str_label); itb != context.outer_context.map_of_inventions.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				bool bvalue = parse_bool(value, line, err);
				context.compiled_trigger.push_back(uint16_t(trigger::invention | association_to_bool_code(a, bvalue)));
				context.compiled_trigger.push_back(trigger::payload(itb->second.id).value);
			} else if(context.main_slot == trigger::slot_contents::province) {
				bool bvalue = parse_bool(value, line, err);
				context.compiled_trigger.push_back(uint16_t(trigger::invention_province | association_to_bool_code(a, bvalue)));
				context.compiled_trigger.push_back(trigger::payload(itb->second.id).value);
			} else if(context.main_slot == trigger::slot_contents::pop) {
				bool bvalue = parse_bool(value, line, err);
				context.compiled_trigger.push_back(uint16_t(trigger::invention_pop | association_to_bool_code(a, bvalue)));
				context.compiled_trigger.push_back(trigger::payload(itb->second.id).value);
			} else {
				err.accumulated_errors += "named invention trigger used in an invalid context (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(auto itc = context.outer_context.map_of_ideologies.find(str_label); itc != context.outer_context.map_of_ideologies.end()) {
			if(context.main_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::variable_ideology_name_nation | association_to_trigger_code(a)));
			else if(context.main_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(uint16_t(trigger::variable_ideology_name_pop | association_to_trigger_code(a)));
			else if(context.main_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(uint16_t(trigger::variable_ideology_name_province | association_to_trigger_code(a)));
			else if(context.main_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(uint16_t(trigger::variable_ideology_name_state | association_to_trigger_code(a)));
			else {
				err.accumulated_errors += "named ideology trigger used in an invalid context (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(itc->second.id).value);
			context.add_float_to_payload(parse_float(value, line, err));
		} else if(auto itd = context.outer_context.map_of_poptypes.find(str_label); itd != context.outer_context.map_of_poptypes.end()) {
			if(context.main_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::variable_pop_type_name_nation | association_to_trigger_code(a)));
			else if(context.main_slot == trigger::slot_contents::pop)
				context.compiled_trigger.push_back(uint16_t(trigger::variable_pop_type_name_pop | association_to_trigger_code(a)));
			else if(context.main_slot == trigger::slot_contents::province)
				context.compiled_trigger.push_back(uint16_t(trigger::variable_pop_type_name_province | association_to_trigger_code(a)));
			else if(context.main_slot == trigger::slot_contents::state)
				context.compiled_trigger.push_back(uint16_t(trigger::variable_pop_type_name_state | association_to_trigger_code(a)));
			else {
				err.accumulated_errors += "named pop type trigger used in an invalid context (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(itd->second).value);
			context.add_float_to_payload(parse_float(value, line, err));
		} else if(auto ite = context.outer_context.map_of_commodity_names.find(str_label); ite != context.outer_context.map_of_commodity_names.end()) {
			if(context.main_slot == trigger::slot_contents::nation)
				context.compiled_trigger.push_back(uint16_t(trigger::variable_good_name | association_to_trigger_code(a)));
			else {
				err.accumulated_errors += "named commodity trigger used in an invalid context (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_trigger.push_back(trigger::payload(ite->second).value);
			context.add_float_to_payload(parse_float(value, line, err));
		} else {
			err.accumulated_errors += "unknown key: " + str_label + " found in trigger(" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}

	void stored_condition::main_parameter(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.compiled_trigger.size() <= 2) {
			if(is_fixed_token_ci(value.data(), value.data() + value.size(), "nation") || is_fixed_token_ci(value.data(), value.data() + value.size(), "country")) {
				context.main_slot = trigger::slot_contents::nation;
				} else if(is_fixed_token_ci(value.data(), value.data() + value.size(), "state")) {
					context.main_slot = trigger::slot_contents::state;
				} else if(is_fixed_token_ci(value.data(), value.data() + value.size(), "province")) {
					context.main_slot = trigger::slot_contents::province;
				} else if(is_fixed_token_ci(value.data(), value.data() + value.size(), "pop")) {
					context.main_slot = trigger::slot_contents::pop;
				} else if(is_fixed_token_ci(value.data(), value.data() + value.size(), "rebel")) {
					context.main_slot = trigger::slot_contents::rebel;
				} else if(is_fixed_token_ci(value.data(), value.data() + value.size(), "none")) {
					context.main_slot = trigger::slot_contents::empty;
				} else {
					err.accumulated_errors +=
					"unknown parameter type " + std::string(value)  + " defined for a scripted trigger (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				}
		} else {
			err.accumulated_errors +=
			"parameters for a scripted trigger must be given first (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void stored_condition::this_parameter(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.compiled_trigger.size() <= 2) {
			if(is_fixed_token_ci(value.data(), value.data() + value.size(), "nation")
			|| is_fixed_token_ci(value.data(), value.data() + value.size(), "country")) {
				context.this_slot = trigger::slot_contents::nation;
			} else if(is_fixed_token_ci(value.data(), value.data() + value.size(), "state")) {
				context.this_slot = trigger::slot_contents::state;
			} else if(is_fixed_token_ci(value.data(), value.data() + value.size(), "province")) {
				context.this_slot = trigger::slot_contents::province;
			} else if(is_fixed_token_ci(value.data(), value.data() + value.size(), "pop")) {
				context.this_slot = trigger::slot_contents::pop;
			} else if(is_fixed_token_ci(value.data(), value.data() + value.size(), "rebel")) {
				context.this_slot = trigger::slot_contents::rebel;
			} else if(is_fixed_token_ci(value.data(), value.data() + value.size(), "none")) {
				context.this_slot = trigger::slot_contents::empty;
			} else {
				err.accumulated_errors += "unknown parameter type " + std::string(value) + " defined for a scripted trigger (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			}
		} else {
			err.accumulated_errors += "parameters for a scripted trigger must be given first (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void stored_condition::from_parameter(association_type a, std::string_view value, error_handler& err, int32_t line, trigger_building_context& context) {
		if(context.compiled_trigger.size() <= 2) {
			if(is_fixed_token_ci(value.data(), value.data() + value.size(), "nation")
			|| is_fixed_token_ci(value.data(), value.data() + value.size(), "country")) {
				context.from_slot = trigger::slot_contents::nation;
			} else if(is_fixed_token_ci(value.data(), value.data() + value.size(), "state")) {
				context.from_slot = trigger::slot_contents::state;
			} else if(is_fixed_token_ci(value.data(), value.data() + value.size(), "province")) {
				context.from_slot = trigger::slot_contents::province;
			} else if(is_fixed_token_ci(value.data(), value.data() + value.size(), "pop")) {
				context.from_slot = trigger::slot_contents::pop;
			} else if(is_fixed_token_ci(value.data(), value.data() + value.size(), "rebel")) {
				context.from_slot = trigger::slot_contents::rebel;
			} else if(is_fixed_token_ci(value.data(), value.data() + value.size(), "none")) {
				context.from_slot = trigger::slot_contents::empty;
			} else {
				err.accumulated_errors += "unknown parameter type " + std::string(value) + " defined for a scripted trigger (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			}
		} else {
			err.accumulated_errors += "parameters for a scripted trigger must be given first (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
} // namespace parsers
