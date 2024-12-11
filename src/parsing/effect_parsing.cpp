#include "effect_parsing.hpp"
#include "parsers.hpp"
#include "parsers_declarations.hpp"
#include "system_state.hpp"

namespace parsers {

	dcon::trigger_key ef_limit(token_generator& gen, error_handler& err, effect_building_context& context) {
	trigger_building_context tr_context{context.outer_context, context.main_slot, context.this_slot, context.from_slot};
		return make_trigger(gen, err, tr_context);
	}

	void ef_scope_if(token_generator& gen, error_handler& err, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension if but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		auto old_limit_offset = context.limit_position;

		context.compiled_effect.push_back(uint16_t(effect::if_scope | effect::scope_has_limit));
		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;
		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
	}

	void ef_scope_else_if(token_generator& gen, error_handler& err, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension else_if but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		auto old_limit_offset = context.limit_position;

		context.compiled_effect.push_back(uint16_t(effect::else_if_scope | effect::scope_has_limit));
		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;
		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
	}

	void ef_scope_hidden_tooltip(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;

		context.compiled_effect.push_back(uint16_t(effect::generic_scope | effect::scope_has_limit));
		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;
		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
	}

	void ef_scope_any_neighbor_province(token_generator& gen, error_handler& err, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			auto old_limit_offset = context.limit_position;

			context.compiled_effect.push_back(uint16_t(effect::x_neighbor_province_scope | effect::scope_has_limit));
			context.compiled_effect.push_back(uint16_t(0));
			auto payload_size_offset = context.compiled_effect.size() - 1;
			context.limit_position = context.compiled_effect.size();
			context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

			parse_effect_body(gen, err, context);

			context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
			context.limit_position = old_limit_offset;
		} else {
			gen.discard_group();
			err.accumulated_errors += "any_neighbor_province effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}
	}

	void ef_scope_any_neighbor_country(token_generator& gen, error_handler& err, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			auto old_limit_offset = context.limit_position;

			context.compiled_effect.push_back(uint16_t(effect::x_neighbor_country_scope | effect::scope_has_limit));
			context.compiled_effect.push_back(uint16_t(0));
			auto payload_size_offset = context.compiled_effect.size() - 1;
			context.limit_position = context.compiled_effect.size();
			context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

			parse_effect_body(gen, err, context);

			context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
			context.limit_position = old_limit_offset;
		} else {
			gen.discard_group();
			err.accumulated_errors += "any_neighbor_province effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}
	}
	void ef_scope_random_neighbor_country(token_generator& gen, error_handler& err, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			auto old_limit_offset = context.limit_position;

			context.compiled_effect.push_back(uint16_t(effect::x_neighbor_country_scope | effect::is_random_scope | effect::scope_has_limit));
			context.compiled_effect.push_back(uint16_t(0));
			auto payload_size_offset = context.compiled_effect.size() - 1;
			context.limit_position = context.compiled_effect.size();
			context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

			parse_effect_body(gen, err, context);

			context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
			context.limit_position = old_limit_offset;
		} else {
			gen.discard_group();
			err.accumulated_errors += "any_neighbor_province effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}
	}

	void ef_scope_any_country(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;
		if(context.main_slot == trigger::slot_contents::nation) {
			if(bool(context.outer_context.state.defines.alice_disable_divergent_any_country_effect)) {
				context.compiled_effect.push_back(uint16_t(effect::x_country_scope_nation | effect::scope_has_limit));
			} else {
				if(context.effect_is_for_event) {
					context.compiled_effect.push_back(uint16_t(effect::x_event_country_scope_nation | effect::scope_has_limit));
				} else {
					context.compiled_effect.push_back(uint16_t(effect::x_decision_country_scope_nation | effect::scope_has_limit));
				}
			}
		} else {
			if(bool(context.outer_context.state.defines.alice_disable_divergent_any_country_effect)) {
				context.compiled_effect.push_back(uint16_t(effect::x_country_scope | effect::scope_has_limit));
			} else {
				if(context.effect_is_for_event) {
					context.compiled_effect.push_back(uint16_t(effect::x_event_country_scope | effect::scope_has_limit));
				} else {
					context.compiled_effect.push_back(uint16_t(effect::x_decision_country_scope | effect::scope_has_limit));
				}
			}
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::nation;
		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}

	void ef_scope_from_bounce(token_generator& gen, error_handler& err, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension from_bounce but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		auto old_limit_offset = context.limit_position;
		context.compiled_effect.push_back(uint16_t(effect::from_bounce_scope | effect::scope_has_limit));
		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;
		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);
		//
		auto old_from = context.from_slot;
		context.from_slot = context.main_slot;
		parse_effect_body(gen, err, context);
		context.from_slot = old_from;
		//
		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
	}
	void ef_scope_this_bounce(token_generator& gen, error_handler& err, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension this_bounce but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		auto old_limit_offset = context.limit_position;
		context.compiled_effect.push_back(uint16_t(effect::this_bounce_scope | effect::scope_has_limit));
		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;
		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);
		//
		auto old_this = context.this_slot;
		context.this_slot = context.main_slot;
		parse_effect_body(gen, err, context);
		context.this_slot = old_this;
		//
		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
	}

	void ef_scope_loop_bounded(token_generator& gen, error_handler& err, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension loop_bounded but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		auto old_limit_offset = context.limit_position;

		context.compiled_effect.push_back(uint16_t(effect::loop_bounded_scope | effect::scope_has_limit));

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);
		context.compiled_effect.push_back(uint16_t(0));

		auto read_body = parse_ef_scope_random_by_modifier(gen, err, context);
		
		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.compiled_effect[payload_size_offset + 2] = uint16_t(read_body.loop_iterations);
		static_assert(sizeof(dcon::value_modifier_key::value_base_t) == sizeof(uint16_t));
		context.limit_position = old_limit_offset;
	}

	void ef_scope_any_existing_country_except_scoped(token_generator& gen, error_handler& err, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension any_existing_country_except_scoped but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::x_decision_country_scope_nation | effect::scope_has_limit));
		} else {
			context.compiled_effect.push_back(uint16_t(effect::x_decision_country_scope | effect::scope_has_limit));
		}
		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::nation;
		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}
	void ef_scope_any_defined_country(token_generator& gen, error_handler& err, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension any_defined_country but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::x_event_country_scope_nation | effect::scope_has_limit));
		} else {
			context.compiled_effect.push_back(uint16_t(effect::x_event_country_scope | effect::scope_has_limit));
		}
		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::nation;
		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}

	void ef_scope_random_country(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.main_slot == trigger::slot_contents::nation) {
			if(bool(context.outer_context.state.defines.alice_disable_divergent_any_country_effect)) {
				context.compiled_effect.push_back(uint16_t(effect::x_country_scope_nation | effect::is_random_scope | effect::scope_has_limit));
			} else {
				if(context.effect_is_for_event) {
					context.compiled_effect.push_back(uint16_t(effect::x_event_country_scope_nation | effect::is_random_scope | effect::scope_has_limit));
				} else {
					context.compiled_effect.push_back(uint16_t(effect::x_decision_country_scope_nation | effect::is_random_scope | effect::scope_has_limit));
				}
			}
		} else {
			if(bool(context.outer_context.state.defines.alice_disable_divergent_any_country_effect)) {
				context.compiled_effect.push_back(uint16_t(effect::x_country_scope | effect::is_random_scope | effect::scope_has_limit));
			} else {
				if(context.effect_is_for_event) {
					context.compiled_effect.push_back(uint16_t(effect::x_event_country_scope | effect::is_random_scope | effect::scope_has_limit));
				} else {
					context.compiled_effect.push_back(uint16_t(effect::x_decision_country_scope | effect::is_random_scope | effect::scope_has_limit));
				}
			}
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::nation;
		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}

	void ef_scope_random_neighbor_province(token_generator& gen, error_handler& err, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			auto old_limit_offset = context.limit_position;

			context.compiled_effect.push_back(uint16_t(effect::x_neighbor_province_scope | effect::is_random_scope | effect::scope_has_limit));
			context.compiled_effect.push_back(uint16_t(0));
			auto payload_size_offset = context.compiled_effect.size() - 1;
			context.limit_position = context.compiled_effect.size();
			context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

			parse_effect_body(gen, err, context);

			context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
			context.limit_position = old_limit_offset;
		} else {
			gen.discard_group();
			err.accumulated_errors += "any_neighbor_province effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}
	}

	void ef_scope_any_empty_neighbor_province(token_generator& gen, error_handler& err, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			auto old_limit_offset = context.limit_position;

			context.compiled_effect.push_back(uint16_t(effect::x_empty_neighbor_province_scope | effect::scope_has_limit));
			context.compiled_effect.push_back(uint16_t(0));
			auto payload_size_offset = context.compiled_effect.size() - 1;
			context.limit_position = context.compiled_effect.size();
			context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

			parse_effect_body(gen, err, context);

			context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
			context.limit_position = old_limit_offset;
		} else {
			gen.discard_group();
			err.accumulated_errors += "any_empty_neighbor_province effect scope used in an incorrect scope type(" + err.file_name + ")\n";
			return;
		}
	}

	void ef_scope_random_empty_neighbor_province(token_generator& gen, error_handler& err, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			auto old_limit_offset = context.limit_position;

			context.compiled_effect.push_back(uint16_t(effect::x_empty_neighbor_province_scope | effect::is_random_scope | effect::scope_has_limit));
			context.compiled_effect.push_back(uint16_t(0));
			auto payload_size_offset = context.compiled_effect.size() - 1;
			context.limit_position = context.compiled_effect.size();
			context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

			parse_effect_body(gen, err, context);

			context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
			context.limit_position = old_limit_offset;
		} else {
			gen.discard_group();
			err.accumulated_errors += "random_empty_neighbor_province effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}
	}

	void ef_scope_any_greater_power(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		context.compiled_effect.push_back(uint16_t(effect::x_greater_power_scope | effect::scope_has_limit));
		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::nation;
		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}

	void ef_scope_random_greater_power(token_generator& gen, error_handler& err, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension random_greater_power but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		context.compiled_effect.push_back(uint16_t(effect::x_greater_power_scope | effect::is_random_scope | effect::scope_has_limit));
		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::nation;
		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}

	void ef_scope_poor_strata(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::poor_strata_scope_province | effect::scope_has_limit));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::poor_strata_scope_state | effect::scope_has_limit));
		} else if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::poor_strata_scope_nation | effect::scope_has_limit));
		} else {
			gen.discard_group();
			err.accumulated_errors += "poor_strata effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::pop;
		parse_effect_body(gen, err, context);
		if(context.compiled_effect[context.limit_position] != trigger::payload(dcon::trigger_key()).value) {
			err.accumulated_warnings += "Usage of limit in poor_strata scope is an extension (" + err.file_name + ")\n";
		}

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}

	void ef_scope_middle_strata(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::middle_strata_scope_province | effect::scope_has_limit));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::middle_strata_scope_state | effect::scope_has_limit));
		} else if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::middle_strata_scope_nation | effect::scope_has_limit));
		} else {
			gen.discard_group();
			err.accumulated_errors += "middle_strata effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::pop;
		parse_effect_body(gen, err, context);
		if(context.compiled_effect[context.limit_position] != trigger::payload(dcon::trigger_key()).value) {
			err.accumulated_warnings += "Usage of limit in middle_strata scope is an extension (" + err.file_name + ")\n";
		}

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}

	void ef_scope_rich_strata(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::rich_strata_scope_province | effect::scope_has_limit));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::rich_strata_scope_state | effect::scope_has_limit));
		} else if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::rich_strata_scope_nation | effect::scope_has_limit));
		} else {
			gen.discard_group();
			err.accumulated_errors += "rich_strata effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::pop;
		parse_effect_body(gen, err, context);
		if(context.compiled_effect[context.limit_position] != trigger::payload(dcon::trigger_key()).value) {
			err.accumulated_warnings += "Usage of limit in rich_strata scope is an extension (" + err.file_name + ")\n";
		}

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}

	void ef_scope_random_pop(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::x_pop_scope_province | effect::is_random_scope | effect::scope_has_limit));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::x_pop_scope_state | effect::is_random_scope | effect::scope_has_limit));
		} else if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::x_pop_scope_nation | effect::is_random_scope | effect::scope_has_limit));
		} else {
			gen.discard_group();
			err.accumulated_errors += "random_pop effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::pop;
		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}
	void ef_scope_random_owned(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		auto const scope_offset = context.compiled_effect.size();
		if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::x_owned_scope_state | effect::is_random_scope | effect::scope_has_limit));
		} else if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::x_owned_scope_nation | effect::is_random_scope | effect::scope_has_limit));
		} else if(context.main_slot == trigger::slot_contents::province) { // we parse this as basically a do-nothing scope
			context.compiled_effect.push_back(uint16_t(effect::generic_scope | effect::scope_has_limit));
			context.compiled_effect.push_back(uint16_t(0));
			auto payload_size_offset = context.compiled_effect.size() - 1;
			context.limit_position = context.compiled_effect.size();
			context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

			parse_effect_body(gen, err, context);

			context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
			context.limit_position = old_limit_offset;

			auto const new_scope_size = simplify_effect(context.compiled_effect.data() + scope_offset);
			context.compiled_effect.resize(scope_offset + size_t(new_scope_size));
			return;
		} else {
			gen.discard_group();
			err.accumulated_errors += "random_owned effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::province;
		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;

		auto const new_scope_size = simplify_effect(context.compiled_effect.data() + scope_offset);
		context.compiled_effect.resize(scope_offset + size_t(new_scope_size));
	}

	void ef_scope_any_owned(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::x_owned_scope_state | effect::scope_has_limit));
		} else if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::x_owned_scope_nation | effect::scope_has_limit));
		} else if(context.main_slot == trigger::slot_contents::province) { // we parse this as basically a do-nothing scope
			context.compiled_effect.push_back(uint16_t(effect::generic_scope | effect::scope_has_limit));
			context.compiled_effect.push_back(uint16_t(0));
			auto payload_size_offset = context.compiled_effect.size() - 1;
			context.limit_position = context.compiled_effect.size();
			context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

			parse_effect_body(gen, err, context);

			context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
			context.limit_position = old_limit_offset;
			return;
		} else {
			gen.discard_group();
			err.accumulated_errors += "any_owned effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::province;
		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}
	void ef_scope_all_core(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::x_core_scope | effect::scope_has_limit));
			context.main_slot = trigger::slot_contents::province;
		} else if(context.outer_context.use_extensions && context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::x_core_scope_province | effect::scope_has_limit));
			context.main_slot = trigger::slot_contents::nation;
		} else {
			gen.discard_group();
			err.accumulated_errors += "all_core effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);


		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}
	void ef_scope_any_state(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::x_state_scope | effect::scope_has_limit));
		} else {
			gen.discard_group();
			err.accumulated_errors += "any_state effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::state;
		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}

	void ef_scope_any_substate(token_generator& gen, error_handler& err, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension any_substate but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		auto old_limit_offset = context.limit_position;
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::x_substate_scope | effect::scope_has_limit));
		} else {
			gen.discard_group();
			err.accumulated_errors += "any_substate effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
	}

	void ef_scope_random_state(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::x_state_scope | effect::is_random_scope | effect::scope_has_limit));
		} else {
			gen.discard_group();
			err.accumulated_errors += "random_state effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::state;
		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}

	void ef_scope_any_pop(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::x_pop_scope_province | effect::scope_has_limit));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::x_pop_scope_state | effect::scope_has_limit));
		} else if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::x_pop_scope_nation | effect::scope_has_limit));
		} else {
			gen.discard_group();
			err.accumulated_errors += "any_pop effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::pop;
		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}
	void ef_scope_owner(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::owner_scope_state | effect::scope_has_limit));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::owner_scope_province | effect::scope_has_limit));
		} else if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::generic_scope | effect::scope_has_limit));
			context.compiled_effect.push_back(uint16_t(0));
			auto payload_size_offset = context.compiled_effect.size() - 1;
			context.limit_position = context.compiled_effect.size();
			context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

			parse_effect_body(gen, err, context);

			context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
			context.limit_position = old_limit_offset;
			return;
		} else {
			gen.discard_group();
			err.accumulated_errors += "owner effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::nation;
		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}
	void ef_scope_controller(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::controller_scope | effect::scope_has_limit));
		} else {
			gen.discard_group();
			err.accumulated_errors += "controller effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::nation;
		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}
	void ef_scope_location(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_effect.push_back(uint16_t(effect::location_scope | effect::scope_has_limit));
		} else {
			gen.discard_group();
			err.accumulated_errors += "location effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::province;
		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}
	void ef_scope_country(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::country_scope_state | effect::scope_has_limit));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_effect.push_back(uint16_t(effect::country_scope_pop | effect::scope_has_limit));
		} else {
			gen.discard_group();
			err.accumulated_errors += "country effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::nation;
		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}
	void ef_capital_scope(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::capital_scope | effect::scope_has_limit));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::capital_scope_province | effect::scope_has_limit));
		} else {
			gen.discard_group();
			err.accumulated_errors += "capital_scope effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::province;
		parse_effect_body(gen, err, context);
		if(context.compiled_effect[context.limit_position] != trigger::payload(dcon::trigger_key()).value) {
			err.accumulated_warnings += "Usage of limit in capital_scope scope is an extension (" + err.file_name + ")\n";
		}

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}
	void ef_scope_this(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.this_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::this_scope_state | effect::scope_has_limit));
		} else if(context.this_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::this_scope_province | effect::scope_has_limit));
		} else if(context.this_slot == trigger::slot_contents::pop) {
			context.compiled_effect.push_back(uint16_t(effect::this_scope_pop | effect::scope_has_limit));
		} else if(context.this_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::this_scope_nation | effect::scope_has_limit));
		} else {
			gen.discard_group();
			err.accumulated_errors += "this effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = context.this_slot;
		parse_effect_body(gen, err, context);
		if(context.compiled_effect[context.limit_position] != trigger::payload(dcon::trigger_key()).value) {
			err.accumulated_warnings += "Usage of limit in this scope is an extension (" + err.file_name + ")\n";
		}

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}

	void ef_scope_from(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.from_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::from_scope_state | effect::scope_has_limit));
		} else if(context.from_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::from_scope_province | effect::scope_has_limit));
		} else if(context.from_slot == trigger::slot_contents::pop) {
			context.compiled_effect.push_back(uint16_t(effect::from_scope_pop | effect::scope_has_limit));
		} else if(context.from_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::from_scope_nation | effect::scope_has_limit));
		} else {
			gen.discard_group();
			err.accumulated_errors += "from effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = context.from_slot;
		parse_effect_body(gen, err, context);
		if(context.compiled_effect[context.limit_position] != trigger::payload(dcon::trigger_key()).value) {
			err.accumulated_warnings += "Usage of limit in from_scope scope is an extension (" + err.file_name + ")\n";
		}

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}

	void ef_scope_sea_zone(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;

		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::sea_zone_scope | effect::scope_has_limit));
		} else {
			gen.discard_group();
			err.accumulated_errors += "sea_zone effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		parse_effect_body(gen, err, context);
		if(context.compiled_effect[context.limit_position] != trigger::payload(dcon::trigger_key()).value) {
			err.accumulated_warnings += "Usage of limit in sea_zone scope is an extension (" + err.file_name + ")\n";
		}

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
	}

	void ef_scope_cultural_union(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;

		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::cultural_union_scope | effect::scope_has_limit));
		} else {
			gen.discard_group();
			err.accumulated_errors += "cultural_union effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		parse_effect_body(gen, err, context);
		if(context.compiled_effect[context.limit_position] != trigger::payload(dcon::trigger_key()).value) {
			err.accumulated_warnings += "Usage of limit in cultural_union scope is an extension (" + err.file_name + ")\n";
		}

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
	}

	void ef_scope_overlord(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;

		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::overlord_scope | effect::scope_has_limit));
		} else {
			gen.discard_group();
			err.accumulated_errors += "overlord effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		parse_effect_body(gen, err, context);
		if(context.compiled_effect[context.limit_position] != trigger::payload(dcon::trigger_key()).value) {
			err.accumulated_warnings += "Usage of limit in overlord scope is an extension (" + err.file_name + ")\n";
		}

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
	}

	void ef_scope_sphere_owner(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;

		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::sphere_owner_scope | effect::scope_has_limit));
		} else {
			gen.discard_group();
			err.accumulated_errors += "sphere_owner effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		parse_effect_body(gen, err, context);
		if(context.compiled_effect[context.limit_position] != trigger::payload(dcon::trigger_key()).value) {
			err.accumulated_warnings += "Usage of limit in sphere_owner scope is an extension (" + err.file_name + ")\n";
		}

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
	}

	void ef_scope_independence(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.main_slot == trigger::slot_contents::rebel) {
			context.compiled_effect.push_back(uint16_t(effect::independence_scope | effect::scope_has_limit));
		} else {
			gen.discard_group();
			err.accumulated_errors += "independence effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::nation;
		parse_effect_body(gen, err, context);
		if(context.compiled_effect[context.limit_position] != trigger::payload(dcon::trigger_key()).value) {
			err.accumulated_warnings += "Usage of limit in independence scope is an extension (" + err.file_name + ")\n";
		}

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}
	void ef_flashpoint_tag_scope(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::flashpoint_tag_scope | effect::scope_has_limit));
		} else {
			gen.discard_group();
			err.accumulated_errors += "flashpoint_tag_scope effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::nation;
		parse_effect_body(gen, err, context);
		if(context.compiled_effect[context.limit_position] != trigger::payload(dcon::trigger_key()).value) {
			err.accumulated_warnings += "Usage of limit in flashpoint_tag scope is an extension (" + err.file_name + ")\n";
		}

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}
	void ef_crisis_state_scope(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		context.compiled_effect.push_back(uint16_t(effect::crisis_state_scope | effect::scope_has_limit));

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::state;
		parse_effect_body(gen, err, context);
		if(context.compiled_effect[context.limit_position] != trigger::payload(dcon::trigger_key()).value) {
			err.accumulated_warnings += "Usage of limit in crisis_state scope is an extension (" + err.file_name + ")\n";
		}

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}

	void ef_state_scope(token_generator& gen, error_handler& err, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::state) { // for the case where state context is added within state context
			auto old_limit_offset = context.limit_position;

			context.compiled_effect.push_back(uint16_t(effect::generic_scope | effect::scope_has_limit));
			context.compiled_effect.push_back(uint16_t(0));
			auto payload_size_offset = context.compiled_effect.size() - 1;
			context.limit_position = context.compiled_effect.size();
			context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

			parse_effect_body(gen, err, context);

			context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
			context.limit_position = old_limit_offset;

			return;
		}

		auto old_limit_offset = context.limit_position;
		auto old_main = context.main_slot;

		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::state_scope_province | effect::scope_has_limit));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_effect.push_back(uint16_t(effect::state_scope_pop | effect::scope_has_limit));
		} else {
			gen.discard_group();
			err.accumulated_errors += "state_scope effect scope used in an incorrect scope type (" + err.file_name + ")\n";
			return;
		}

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		context.main_slot = trigger::slot_contents::state;
		parse_effect_body(gen, err, context);
		if(context.compiled_effect[context.limit_position] != trigger::payload(dcon::trigger_key()).value) {
			err.accumulated_warnings += "Usage of limit in state scope is an extension (" + err.file_name + ")\n";
		}

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;
		context.main_slot = old_main;
	}

	void ef_scope_random(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;

		context.compiled_effect.push_back(uint16_t(effect::random_scope | effect::scope_has_limit));

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);
		context.compiled_effect.push_back(uint16_t(0));

		auto read_body = parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.compiled_effect[payload_size_offset + 2] = uint16_t(read_body.chance);
		context.limit_position = old_limit_offset;
	}

	void ef_random_by_modifier(token_generator& gen, error_handler& err, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension random_by_modifier but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		auto old_limit_offset = context.limit_position;

		context.compiled_effect.push_back(uint16_t(effect::random_by_modifier_scope | effect::scope_has_limit));

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);
		context.compiled_effect.push_back(uint16_t(0));

		auto read_body = parse_ef_scope_random_by_modifier(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.compiled_effect[payload_size_offset + 2] = uint16_t(read_body.chance_modifier.index());
		static_assert(sizeof(dcon::value_modifier_key::value_base_t) == sizeof(uint16_t));
		context.limit_position = old_limit_offset;
	}

	dcon::value_modifier_key read_chance_modifier(token_generator& gen, error_handler& err, effect_building_context& context) {
		trigger_building_context t_context{ context.outer_context, context.main_slot, context.this_slot, context.from_slot };
		return make_value_modifier(gen, err, t_context);
	}

	int32_t add_to_random_list(std::string_view label, token_generator& gen, error_handler& err, effect_building_context& context) {
		auto ivalue = parse_int(label, 0, err);
		context.compiled_effect.push_back(uint16_t(ivalue));

		auto old_limit_offset = context.limit_position;
		context.compiled_effect.push_back(uint16_t(effect::generic_scope | effect::scope_has_limit));
		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;
		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		parse_effect_body(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.limit_position = old_limit_offset;

		return ivalue;
	}

	void ef_scope_random_list(token_generator& gen, error_handler& err, effect_building_context& context) {
		auto old_limit_offset = context.limit_position;

		context.compiled_effect.push_back(uint16_t(effect::random_list_scope));

		context.compiled_effect.push_back(uint16_t(0));
		auto payload_size_offset = context.compiled_effect.size() - 1;

		context.limit_position = context.compiled_effect.size();
		context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

		auto random_set = parse_ef_random_list(gen, err, context);

		context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
		context.compiled_effect[payload_size_offset + 1] = uint16_t(std::max(random_set.chances_sum, 1));

		context.limit_position = old_limit_offset;
	}

	void ef_scope_variable(std::string_view label, token_generator& gen, error_handler& err, effect_building_context& context) {
		std::string str_label{label};
		if(auto it = context.outer_context.map_of_state_names.find(str_label); it != context.outer_context.map_of_state_names.end()) {
			auto old_limit_offset = context.limit_position;
			auto old_main = context.main_slot;

			context.compiled_effect.push_back(uint16_t(effect::region_scope | effect::scope_has_limit));

			context.compiled_effect.push_back(uint16_t(0));
			auto payload_size_offset = context.compiled_effect.size() - 1;

			context.limit_position = context.compiled_effect.size();
			context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

			context.compiled_effect.push_back(trigger::payload(it->second).value);

			context.main_slot = trigger::slot_contents::province;
			parse_effect_body(gen, err, context);
			if(context.compiled_effect[context.limit_position] != trigger::payload(dcon::trigger_key()).value) {
				err.accumulated_warnings += "Usage of limit in variable state scope is an extension (" + err.file_name + ")\n";
			}

			context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
			context.limit_position = old_limit_offset;
			context.main_slot = old_main;
		} else if(auto itr = context.outer_context.map_of_region_names.find(str_label); itr != context.outer_context.map_of_region_names.end()) {
			auto old_limit_offset = context.limit_position;
			auto old_main = context.main_slot;

			context.compiled_effect.push_back(uint16_t(effect::region_proper_scope | effect::scope_has_limit));

			context.compiled_effect.push_back(uint16_t(0));
			auto payload_size_offset = context.compiled_effect.size() - 1;

			context.limit_position = context.compiled_effect.size();
			context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

			context.compiled_effect.push_back(trigger::payload(itr->second).value);

			context.main_slot = trigger::slot_contents::province;
			parse_effect_body(gen, err, context);
			if(context.compiled_effect[context.limit_position] != trigger::payload(dcon::trigger_key()).value) {
				err.accumulated_warnings += "Usage of limit in variable region scope is an extension (" + err.file_name + ")\n";
			}

			context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
			context.limit_position = old_limit_offset;
			context.main_slot = old_main;
		} else if(auto itb = context.outer_context.map_of_poptypes.find(str_label);
						itb != context.outer_context.map_of_poptypes.end()) {

			if(context.main_slot == trigger::slot_contents::nation)
			context.compiled_effect.push_back(uint16_t(effect::pop_type_scope_nation | effect::scope_has_limit));
			else if(context.main_slot == trigger::slot_contents::state)
			context.compiled_effect.push_back(uint16_t(effect::pop_type_scope_state | effect::scope_has_limit));
			else if(context.main_slot == trigger::slot_contents::province)
			context.compiled_effect.push_back(uint16_t(effect::pop_type_scope_province | effect::scope_has_limit));
			else {
				gen.discard_group();
				err.accumulated_errors += "pop type effect scope used in an incorrect scope type (" + err.file_name + ")\n";
				return;
			}

			auto old_limit_offset = context.limit_position;
			auto old_main = context.main_slot;

			context.compiled_effect.push_back(uint16_t(0));
			auto payload_size_offset = context.compiled_effect.size() - 1;

			context.limit_position = context.compiled_effect.size();
			context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);

			context.compiled_effect.push_back(trigger::payload(itb->second).value);

			context.main_slot = trigger::slot_contents::pop;
			parse_effect_body(gen, err, context);
			if(context.compiled_effect[context.limit_position] != trigger::payload(dcon::trigger_key()).value) {
				err.accumulated_warnings += "Usage of limit in variable pop type scope is an extension (" + err.file_name + ")\n";
			}

			context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
			context.limit_position = old_limit_offset;
			context.main_slot = old_main;
		} else if(is_integer(label.data(), label.data() + label.length())) {
			auto value = parse_int(label, 0, err);
			if(0 <= value && size_t(value) < context.outer_context.original_id_to_prov_id_map.size()) {

				auto old_limit_offset = context.limit_position;
				auto old_main = context.main_slot;

				context.compiled_effect.push_back(uint16_t(effect::integer_scope | effect::scope_has_limit));

				context.compiled_effect.push_back(uint16_t(0));
				auto payload_size_offset = context.compiled_effect.size() - 1;

				context.limit_position = context.compiled_effect.size();
				context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);
				context.compiled_effect.push_back(trigger::payload(context.outer_context.original_id_to_prov_id_map[value]).value);

				context.main_slot = trigger::slot_contents::province;
				parse_effect_body(gen, err, context);
				if(context.compiled_effect[context.limit_position] != trigger::payload(dcon::trigger_key()).value) {
					err.accumulated_warnings += "Usage of limit in variable province scope is an extension (" + err.file_name + ")\n";
				}

				context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
				context.limit_position = old_limit_offset;

				context.main_slot = old_main;
			} else {
				gen.discard_group();
				err.accumulated_errors += "Invalid province id used as an effect scope (" + err.file_name + ")\n";
				return;
			}
		} else if(label.length() == 3) {
			if(auto itc = context.outer_context.map_of_ident_names.find(nations::tag_to_int(label[0], label[1], label[2]));
				itc != context.outer_context.map_of_ident_names.end()) {

				auto old_limit_offset = context.limit_position;
				auto old_main = context.main_slot;

				context.compiled_effect.push_back(uint16_t(effect::tag_scope | effect::scope_has_limit));

				context.compiled_effect.push_back(uint16_t(0));
				auto payload_size_offset = context.compiled_effect.size() - 1;

				context.limit_position = context.compiled_effect.size();
				context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);
				context.compiled_effect.push_back(trigger::payload(itc->second).value);

				context.main_slot = trigger::slot_contents::nation;
				parse_effect_body(gen, err, context);
				if(context.compiled_effect[context.limit_position] != trigger::payload(dcon::trigger_key()).value) {
					err.accumulated_warnings += "Usage of limit in variable tag scope is an extension (" + err.file_name + ")\n";
				}

				context.compiled_effect[payload_size_offset] = uint16_t(context.compiled_effect.size() - payload_size_offset);
				context.limit_position = old_limit_offset;

				context.main_slot = old_main;
			} else {
				gen.discard_group();
				err.accumulated_errors += "Invalid tag " + str_label + " used as an effect scope (" + err.file_name + ")\n";
				return;
			}
		} else {
			gen.discard_group();
			err.accumulated_errors += "unknown effect scope " + str_label + " (" + err.file_name + ")\n";
			return;
		}
	}

	int32_t simplify_effect(uint16_t* source) {
		assert(0 <= (*source & effect::code_mask) && (*source & effect::code_mask) < effect::first_invalid_code);
		if((source[0] & effect::code_mask) >= effect::first_scope_code) {
			auto source_size = 1 + effect::get_effect_scope_payload_size(source);

			if((source[0] & effect::code_mask) == effect::random_list_scope) {
				auto sub_units_start = source + 4; // [code] + [payload size] + [chances total] + [first sub effect chance]
				while(sub_units_start < source + source_size) {
					auto const old_size = 1 + effect::get_generic_effect_payload_size(sub_units_start);
					auto const new_size = simplify_effect(sub_units_start);
					if(new_size > 0) {
						if(new_size != old_size) { // has been simplified
							assert(new_size < old_size);
							std::copy(sub_units_start + old_size, source + source_size, sub_units_start + new_size);
							source_size -= (old_size - new_size);
						}
						sub_units_start += new_size + 1;
					} else {
						std::copy(sub_units_start + old_size, source + source_size, sub_units_start - 1);
						source_size -= (1 + old_size);
					}
				}
			} else {
				auto sub_units_start = source + 2 + effect::effect_scope_data_payload(source[0]);
				while(sub_units_start < source + source_size) {
					// [0      0     ] | clr_global_flag	[1      2     ] | clr_global_flag
					// [0      1     ] | <flag>				[1      3     ] | <flag>
					// --------------- | n = 1				...............
					// [n      2n    ]
					// [n      2n + 1]
					int32_t old_size = 1 + effect::get_generic_effect_payload_size(sub_units_start);
					int32_t new_size = simplify_effect(sub_units_start);
					if((sub_units_start[0] & effect::code_mask) == effect::clr_global_flag) {
						auto repeats = 0;
						while(sub_units_start + old_size < source + source_size
						&& (sub_units_start[old_size] & effect::code_mask) == effect::clr_global_flag
						&& repeats < 8) {
							old_size += 1 + effect::data_sizes[effect::clr_global_flag];
							++repeats;
						}
						static const uint16_t fop_table[] = {
							effect::clr_global_flag,
							effect::fop_clr_global_flag_2,
							effect::fop_clr_global_flag_3,
							effect::fop_clr_global_flag_4,
							effect::fop_clr_global_flag_5,
							effect::fop_clr_global_flag_6,
							effect::fop_clr_global_flag_7,
							effect::fop_clr_global_flag_8,
							effect::fop_clr_global_flag_9,
							effect::fop_clr_global_flag_10,
							effect::fop_clr_global_flag_11,
							effect::fop_clr_global_flag_12
						};
						sub_units_start[0] = fop_table[repeats];
						new_size = 1 + effect::data_sizes[fop_table[repeats]];
						for(auto i = 1; i <= repeats + 1; i++) {
							// todo: copy n size
							sub_units_start[effect::data_sizes[effect::clr_global_flag] * i]
							= sub_units_start[(1 + effect::data_sizes[effect::clr_global_flag]) * i - 1];
						}
					}
					if(new_size != old_size) { // has been simplified
						assert(new_size < old_size);
						std::copy(sub_units_start + old_size, source + source_size, sub_units_start + new_size);
						source_size -= (old_size - new_size);
					}
					sub_units_start += new_size;
				}
			}

			source[1] = uint16_t(source_size - 1);

			if((source[0] & effect::scope_has_limit) != 0 && !bool(trigger::payload(source[2]).tr_id)) {
				std::copy(source + 3, source + source_size, source + 2);
				--source_size;
				source[0] = uint16_t(source[0] & ~effect::scope_has_limit);
				source[1] = uint16_t(source_size - 1);
			}

			if((source[0] & effect::code_mask) == effect::generic_scope) {
				if(source_size == 2) {
					return 0; // simplify empty scope to nothing
				} else if(((source[0] & effect::scope_has_limit) == 0) && effect::effect_scope_has_single_member(source)) {
					std::copy(source + 2, source + source_size, source);
					source_size -= 2;
				}
			} else if((source[0] & effect::code_mask) == effect::owner_scope_province
			&& (source[0] & effect::scope_has_limit) == 0
			&& (source[0] & effect::is_random_scope) == 0) {
				bool can_elim = true;
				auto sub_units_start = source + 2 + effect::effect_scope_data_payload(source[0]);
				while(sub_units_start < source + source_size && can_elim) {
					switch(sub_units_start[0] & effect::code_mask) {
						case effect::clr_global_flag:
						case effect::set_global_flag:
						case effect::fop_clr_global_flag_2:
						case effect::fop_clr_global_flag_3:
						case effect::fop_clr_global_flag_4:
						case effect::fop_clr_global_flag_5:
						case effect::fop_clr_global_flag_6:
						case effect::fop_clr_global_flag_7:
						case effect::fop_clr_global_flag_8:
						case effect::fop_clr_global_flag_9:
						case effect::fop_clr_global_flag_10:
						case effect::fop_clr_global_flag_11:
						case effect::fop_clr_global_flag_12:
						case effect::fop_change_province_name:
						case effect::integer_scope:
						break;
						default:
						can_elim = false;
						break;
					}
					sub_units_start += 1 + effect::get_generic_effect_payload_size(sub_units_start);
				}
				if(can_elim) { //eliminate
					std::copy(source + 2, source + source_size, source);
					source_size -= 2;
				}
			} else if((source[0] & effect::code_mask) == effect::integer_scope
			&& (source[0] & effect::scope_has_limit) == 0
			&& (source[0] & effect::is_random_scope) == 0
			&& source[1] == 4
			&& effect::effect_scope_has_single_member(source)) {
				auto sub_units_start = source + 2 + effect::effect_scope_data_payload(source[0]);
				auto const old_size = 1 + effect::get_generic_effect_payload_size(source);
				if(sub_units_start[0] == effect::change_province_name) {
					auto const prov = source[2]; //[code] [size] [province]
					auto const name_1 = sub_units_start[1];
					auto const name_2 = sub_units_start[2];
					source[0] = effect::fop_change_province_name;
					source[1] = name_1; //name
					source[2] = name_2; //name
					source[3] = prov; //province
					auto const new_size = 1 + effect::data_sizes[effect::fop_change_province_name];
					assert(new_size < old_size);
					std::copy(source + old_size, source + source_size, source + new_size);
					source_size -= (old_size - new_size);
				}
			}
			return source_size;
		} else {
			return 1 + effect::get_effect_non_scope_payload_size(source); // non scopes cannot be simplified
		}
	}

	template<typename T>
	void recurse_over_effects(uint16_t* source, T const& f) {
		f(source);

		if((source[0] & effect::code_mask) >= effect::first_scope_code) {
			if((source[0] & effect::code_mask) == effect::random_list_scope) {
				auto const source_size = 1 + effect::get_generic_effect_payload_size(source);

				auto sub_units_start = source + 4; // [code] + [payload size] + [chances total] + [first sub effect chance]
				while(sub_units_start < source + source_size) {
					recurse_over_effects(sub_units_start, f);
					sub_units_start += 2 + effect::get_generic_effect_payload_size(sub_units_start); // each member preceded by uint16_t
				}
			} else {
				auto const source_size = 1 + effect::get_generic_effect_payload_size(source);

				auto sub_units_start = source + 2 + effect::effect_scope_data_payload(source[0]);
				while(sub_units_start < source + source_size) {
					recurse_over_effects(sub_units_start, f);
					sub_units_start += 1 + effect::get_generic_effect_payload_size(sub_units_start);
				}
			}
		}
	}

	dcon::effect_key make_effect(token_generator& gen, error_handler& err, effect_building_context& context) {
		ef_scope_hidden_tooltip(gen, err, context);

		if(context.compiled_effect.size() >= std::numeric_limits<uint16_t>::max()) {
			err.accumulated_errors += "effect is " + std::to_string(context.compiled_effect.size()) + " cells big, which exceeds 64 KB bytecode limit (" + err.file_name + ")\n";
		return dcon::effect_key{0};
		} else if(context.compiled_effect.empty()) {
			err.accumulated_errors += "effect is empty limit (" + err.file_name + ")\n";
		return dcon::effect_key{ 0 };
		}

		auto const new_size = simplify_effect(context.compiled_effect.data());
		context.compiled_effect.resize(static_cast<size_t>(new_size));

		return context.outer_context.state.commit_effect_data(context.compiled_effect);
	}

	void ef_province_event::id(association_type t, int32_t value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(auto it = context.outer_context.map_of_provincial_events.find(value);
			it != context.outer_context.map_of_provincial_events.end()) {
			if(it->second.id) {
				id_ = it->second.id;
			} else {
				id_ = context.outer_context.state.world.create_provincial_event();
				it->second.id = id_;
				it->second.main_slot = trigger::slot_contents::province;
				it->second.this_slot = trigger::slot_contents::nation;
				it->second.from_slot = context.this_slot;
				it->second.just_in_case_placeholder = false;
			}
		} else {
			id_ = context.outer_context.state.world.create_provincial_event();
			context.outer_context.map_of_provincial_events.insert_or_assign(value,
				pending_prov_event{id_, trigger::slot_contents::province, trigger::slot_contents::province, context.this_slot});
		}
	}

	void ef_country_event::id(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context) {
		if(auto it = context.outer_context.map_of_national_events.find(value);
			it != context.outer_context.map_of_national_events.end()) {
			if(it->second.id) {
				id_ = it->second.id;
			} else {
				id_ = context.outer_context.state.world.create_national_event();
				it->second.id = id_;
				it->second.main_slot = trigger::slot_contents::nation;
				it->second.this_slot = trigger::slot_contents::nation;
				it->second.from_slot = context.this_slot;
				it->second.just_in_case_placeholder = false;
			}
		} else {
			id_ = context.outer_context.state.world.create_national_event();
			context.outer_context.map_of_national_events.insert_or_assign(value,
				pending_nat_event{id_, trigger::slot_contents::nation, trigger::slot_contents::nation, context.this_slot});
		}
	}

	void effect_body::country_event(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::country_event_immediate_this_nation));
			else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::country_event_immediate_this_province));
			else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(uint16_t(effect::country_event_immediate_this_state));
			else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_effect.push_back(uint16_t(effect::country_event_immediate_this_pop));
			else {
				err.accumulated_errors +=
					"country_event effect used in an incorrect scope type (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			if(auto it = context.outer_context.map_of_national_events.find(value);
				it != context.outer_context.map_of_national_events.end()) {
				if(it->second.id) {
					context.compiled_effect.push_back(trigger::payload(it->second.id).value);
				} else {
					auto ev_id = context.outer_context.state.world.create_national_event();
					it->second.id = ev_id;
					it->second.main_slot = trigger::slot_contents::nation;
					it->second.this_slot = trigger::slot_contents::nation;
					it->second.from_slot = context.this_slot;
					it->second.just_in_case_placeholder = false;
					context.compiled_effect.push_back(trigger::payload(ev_id).value);
				}
			} else {
				auto ev_id = context.outer_context.state.world.create_national_event();
				context.outer_context.map_of_national_events.insert_or_assign(value,
					pending_nat_event{ev_id, trigger::slot_contents::nation, trigger::slot_contents::nation, context.this_slot});
				context.compiled_effect.push_back(trigger::payload(ev_id).value);
			}
		} else if(context.main_slot == trigger::slot_contents::province) {
			if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::country_event_immediate_province_this_nation));
			else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::country_event_immediate_province_this_province));
			else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(uint16_t(effect::country_event_immediate_province_this_state));
			else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_effect.push_back(uint16_t(effect::country_event_immediate_province_this_pop));
			else {
				err.accumulated_errors += "country_event effect used in an incorrect scope type (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			if(auto it = context.outer_context.map_of_national_events.find(value);
				it != context.outer_context.map_of_national_events.end()) {
				if(it->second.id) {
					context.compiled_effect.push_back(trigger::payload(it->second.id).value);
				} else {
					auto ev_id = context.outer_context.state.world.create_national_event();
					it->second.id = ev_id;
					it->second.main_slot = trigger::slot_contents::nation;
					it->second.this_slot = trigger::slot_contents::nation;
					it->second.from_slot = context.this_slot;
					it->second.just_in_case_placeholder = false;
					context.compiled_effect.push_back(trigger::payload(ev_id).value);
				}
			} else {
				auto ev_id = context.outer_context.state.world.create_national_event();
				context.outer_context.map_of_national_events.insert_or_assign(value, pending_nat_event{ev_id, trigger::slot_contents::nation, trigger::slot_contents::nation, context.this_slot});
				context.compiled_effect.push_back(trigger::payload(ev_id).value);
			}
		} else {
			err.accumulated_errors +=
				"country_event effect used in an incorrect scope type (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}

	void effect_body::province_event(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::province_event_immediate_this_nation));
			else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::province_event_immediate_this_province));
			else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(uint16_t(effect::province_event_immediate_this_state));
			else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_effect.push_back(uint16_t(effect::province_event_immediate_this_pop));
			else {
				err.accumulated_errors += "province_event effect used in an incorrect scope type (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			if(auto it = context.outer_context.map_of_provincial_events.find(value);
				it != context.outer_context.map_of_provincial_events.end()) {
				if(it->second.id) {
					context.compiled_effect.push_back(trigger::payload(it->second.id).value);
				} else {
					auto ev_id = context.outer_context.state.world.create_provincial_event();
					it->second.id = ev_id;
					it->second.main_slot = trigger::slot_contents::province;
					it->second.this_slot = trigger::slot_contents::province;
					it->second.from_slot = context.this_slot;
					it->second.just_in_case_placeholder = false;
					context.compiled_effect.push_back(trigger::payload(ev_id).value);
				}
			} else {
				auto ev_id = context.outer_context.state.world.create_provincial_event();
				context.outer_context.map_of_provincial_events.insert_or_assign(value,
					pending_prov_event{ev_id, trigger::slot_contents::province, trigger::slot_contents::province, context.this_slot});
				context.compiled_effect.push_back(trigger::payload(ev_id).value);
			}
		} else {
			err.accumulated_errors +=
				"province_event effect used in an incorrect scope type (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}

	void effect_body::country_event(ef_country_event const& value, error_handler& err, int32_t line, effect_building_context& context) {
		auto days = value.days;
		if(days < 0) {
			err.accumulated_warnings += "country_event given negative days, parsed as 0 (" + err.file_name + ")\n";
			days = 0;
		}
		if(days > 32767) {
			err.accumulated_warnings += "country_event given " + std::to_string(days) + " days (" + err.file_name + ")\n";
			days = 32767;
		}
		if(context.main_slot == trigger::slot_contents::nation) {
			if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::country_event_this_nation));
			else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::country_event_this_province));
			else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(uint16_t(effect::country_event_this_state));
			else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_effect.push_back(uint16_t(effect::country_event_this_pop));
			else {
				err.accumulated_errors +=
					"country_event effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_effect.push_back(trigger::payload(value.id_).value);
			context.compiled_effect.push_back(uint16_t(days));
		} else if(context.main_slot == trigger::slot_contents::province) {
			if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::country_event_province_this_nation));
			else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::country_event_province_this_province));
			else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(uint16_t(effect::country_event_province_this_state));
			else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_effect.push_back(uint16_t(effect::country_event_province_this_pop));
			else {
				err.accumulated_errors += "country_event effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_effect.push_back(trigger::payload(value.id_).value);
			context.compiled_effect.push_back(uint16_t(days));
		} else {
			err.accumulated_errors += "country_event effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::province_event(ef_province_event const& value, error_handler& err, int32_t line, effect_building_context& context) {
		auto days = value.days;
		if(!context.outer_context.use_extensions && days > 1) {
			err.accumulated_warnings += "province_event with more than 1 day could cause crashes (" + err.file_name + ")\n";
		}
		if(days < 0) {
			err.accumulated_warnings += "province_event given negative days, parsed as 0 (" + err.file_name + ")\n";
			days = 0;
		}
		if(days > 32767) {
			err.accumulated_warnings += "province_event given " + std::to_string(days) + " days (" + err.file_name + ")\n";
			days = 32767;
		}
		if(context.main_slot == trigger::slot_contents::province) {
			if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::province_event_this_nation));
			else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::province_event_this_province));
			else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(uint16_t(effect::province_event_this_state));
			else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_effect.push_back(uint16_t(effect::province_event_this_pop));
			else {
				err.accumulated_errors += "province_event effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_effect.push_back(trigger::payload(value.id_).value);
			context.compiled_effect.push_back(uint16_t(days));
		} else {
			err.accumulated_errors += "province_event effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}

	void effect_body::define_general(ef_define_general const& value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors +=
				"define_general effect used in an incorrect scope type (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_effect.push_back(effect::define_general);
		context.add_int32_t_to_payload(context.outer_context.state.add_unit_name(value.name).index());
		context.compiled_effect.push_back(trigger::payload(value.personality_).value);
		context.compiled_effect.push_back(trigger::payload(value.background_).value);
	}
	void effect_body::kill_leader(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(effect::kill_leader);
			context.add_int32_t_to_payload(context.outer_context.state.add_unit_name(value).index());
		} else {
			err.accumulated_errors += "kill_leader effect used in an incorrect scope type (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::define_admiral(ef_define_admiral const& value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors +=
				"define_admiral effect used in an incorrect scope type (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_effect.push_back(effect::define_admiral);
		context.add_int32_t_to_payload(context.outer_context.state.add_unit_name(value.name).index());
		context.compiled_effect.push_back(trigger::payload(value.personality_).value);
		context.compiled_effect.push_back(trigger::payload(value.background_).value);
	}
	void effect_body::change_province_name(association_type t, std::string_view value, error_handler & err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::change_province_name));
		} else {
			err.accumulated_errors += "change_province_name effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	
		auto name = text::find_or_add_key(context.outer_context.state, value, false);
		context.add_int32_t_to_payload(name.index());
	}
	void effect_body::change_region_name(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::change_region_name_state));
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::change_region_name_province));
		} else {
			err.accumulated_errors += "change_region_name effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	
		auto name = text::find_or_add_key(context.outer_context.state, value, false);
		context.add_int32_t_to_payload(name.index());
	}
	void effect_body::enable_canal(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context) {
		if(1 <= value && value <= int32_t(context.outer_context.state.province_definitions.canals.size())) {
			context.compiled_effect.push_back(uint16_t(effect::enable_canal));
			context.compiled_effect.push_back(trigger::payload(uint16_t(value)).value);
		} else {
			err.accumulated_errors +=
				"canal index " + std::to_string(value) + " out of range (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}

	void effect_body::add_province_modifier(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension add_province_modifier = <modifier> but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		if(context.main_slot == trigger::slot_contents::province) {
			if(auto it = context.outer_context.map_of_modifiers.find(std::string(value));
				it != context.outer_context.map_of_modifiers.end()) {
				context.compiled_effect.push_back(uint16_t(effect::add_province_modifier_no_duration));
				context.compiled_effect.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors += "add_province_modifier effect supplied with invalid modifier name " + std::string(value) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(context.main_slot == trigger::slot_contents::state) {
			if(auto it = context.outer_context.map_of_modifiers.find(std::string(value));
				it != context.outer_context.map_of_modifiers.end()) {
				context.compiled_effect.push_back(uint16_t(effect::add_province_modifier_state_no_duration));
				context.compiled_effect.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors += "add_province_modifier effect supplied with invalid modifier name " + std::string(value) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "add_province_modifier effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::add_country_modifier(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension add_country_modifier = <modifier> but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}

		if(context.main_slot == trigger::slot_contents::nation) {
			if(auto it = context.outer_context.map_of_modifiers.find(std::string(value));
				it != context.outer_context.map_of_modifiers.end()) {
				context.compiled_effect.push_back(uint16_t(effect::add_country_modifier_no_duration));
				context.compiled_effect.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors += "add_country_modifier effect supplied with invalid modifier name " + std::string(value) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "add_country_modifier effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::trigger_revolt(ef_trigger_revolt const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(!value.type_) {
			err.accumulated_errors +=
				"trigger_revolt must have a valid rebel type (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		if(context.main_slot == trigger::slot_contents::nation)
		context.compiled_effect.push_back(effect::trigger_revolt_nation);
		else if(context.main_slot == trigger::slot_contents::state)
		context.compiled_effect.push_back(effect::trigger_revolt_state);
		else if(context.main_slot == trigger::slot_contents::province)
		context.compiled_effect.push_back(effect::trigger_revolt_province);
		else {
			err.accumulated_errors +=
				"trigger_revolt effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_effect.push_back(trigger::payload(value.type_).value);
		context.compiled_effect.push_back(trigger::payload(value.culture_).value);
		context.compiled_effect.push_back(trigger::payload(value.religion_).value);
		context.compiled_effect.push_back(trigger::payload(value.ideology_).value);
	}
	void effect_body::diplomatic_influence(ef_diplomatic_influence const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation && context.main_slot != trigger::slot_contents::province) {
			err.accumulated_errors += "diplomatic_influence effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		if(is_from(value.who)) {
			if(context.from_slot == trigger::slot_contents::nation) {
				if(context.main_slot == trigger::slot_contents::province) {
					context.compiled_effect.push_back(uint16_t(effect::owner_scope_province | effect::scope_has_limit));
					context.compiled_effect.push_back(uint16_t(2 + 2));
					auto payload_size_offset = context.compiled_effect.size() - 1;
					context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);
				}
				context.compiled_effect.push_back(effect::diplomatic_influence_from_nation);
			} else if(context.from_slot == trigger::slot_contents::province) {
				if(context.main_slot == trigger::slot_contents::province) {
					context.compiled_effect.push_back(uint16_t(effect::owner_scope_province | effect::scope_has_limit));
					context.compiled_effect.push_back(uint16_t(2 + 2));
					auto payload_size_offset = context.compiled_effect.size() - 1;
					context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);
				}
				context.compiled_effect.push_back(effect::diplomatic_influence_from_province);
			} else {
				err.accumulated_errors += "diplomatic_influence = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_effect.push_back(trigger::payload(int16_t(value.value)).value);
		} else if(is_this(value.who)) {
			if(context.this_slot == trigger::slot_contents::nation) {
				if(context.main_slot == trigger::slot_contents::province) {
					context.compiled_effect.push_back(uint16_t(effect::owner_scope_province | effect::scope_has_limit));
					context.compiled_effect.push_back(uint16_t(2 + 2));
					auto payload_size_offset = context.compiled_effect.size() - 1;
					context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);
				}
				context.compiled_effect.push_back(effect::diplomatic_influence_this_nation);
			} else if(context.this_slot == trigger::slot_contents::province) {
				if(context.main_slot == trigger::slot_contents::province) {
					context.compiled_effect.push_back(uint16_t(effect::owner_scope_province | effect::scope_has_limit));
					context.compiled_effect.push_back(uint16_t(2 + 2));
					auto payload_size_offset = context.compiled_effect.size() - 1;
					context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);
				}
				context.compiled_effect.push_back(effect::diplomatic_influence_this_province);
			}  else {
				err.accumulated_errors += "diplomatic_influence = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_effect.push_back(trigger::payload(int16_t(value.value)).value);
		} else if(value.who.length() == 3) {

			if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value.who[0], value.who[1], value.who[2]));
				it != context.outer_context.map_of_ident_names.end()) {

				if(context.main_slot == trigger::slot_contents::province) {
					context.compiled_effect.push_back(uint16_t(effect::owner_scope_province | effect::scope_has_limit));
					context.compiled_effect.push_back(uint16_t(2 + 3));
					auto payload_size_offset = context.compiled_effect.size() - 1;
					context.compiled_effect.push_back(trigger::payload(dcon::trigger_key()).value);
				}

				context.compiled_effect.push_back(uint16_t(effect::diplomatic_influence));
				context.compiled_effect.push_back(trigger::payload(it->second).value);
				context.compiled_effect.push_back(trigger::payload(int16_t(value.value)).value);
			} else {
				err.accumulated_errors += "diplomatic_influence effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "diplomatic_influence effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::relation(ef_relation const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_from(value.who)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(effect::relation_from_nation);
				else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(effect::relation_from_province);
				else {
					err.accumulated_errors += "relation = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
				context.compiled_effect.push_back(trigger::payload(int16_t(value.value)).value);
			} else if(is_this(value.who)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(effect::relation_this_nation);
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(effect::relation_this_province);
				else {
					err.accumulated_errors += "relation = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
				context.compiled_effect.push_back(trigger::payload(int16_t(value.value)).value);
			} else if(is_reb(value.who)) {
				if(context.from_slot == trigger::slot_contents::rebel)
				context.compiled_effect.push_back(effect::relation_reb);
				else {
					err.accumulated_errors += "relation = reb effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
				context.compiled_effect.push_back(trigger::payload(int16_t(value.value)).value);
			} else if(value.who.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value.who[0], value.who[1], value.who[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_effect.push_back(uint16_t(effect::relation));
					context.compiled_effect.push_back(trigger::payload(it->second).value);
					context.compiled_effect.push_back(trigger::payload(int16_t(value.value)).value);
				} else {
					err.accumulated_errors +=
						"relation effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors +=
					"relation effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(context.main_slot == trigger::slot_contents::province) {
			if(is_from(value.who)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(effect::relation_province_from_nation);
				else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(effect::relation_province_from_province);
				else {
					err.accumulated_errors += "relation = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
				context.compiled_effect.push_back(trigger::payload(int16_t(value.value)).value);
			} else if(is_this(value.who)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(effect::relation_province_this_nation);
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(effect::relation_province_this_province);
				else {
					err.accumulated_errors += "relation = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
				context.compiled_effect.push_back(trigger::payload(int16_t(value.value)).value);
			} else if(is_reb(value.who)) {
				if(context.from_slot == trigger::slot_contents::rebel)
				context.compiled_effect.push_back(effect::relation_province_reb);
				else {
					err.accumulated_errors += "relation = reb effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
				context.compiled_effect.push_back(trigger::payload(int16_t(value.value)).value);
			} else if(value.who.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value.who[0], value.who[1], value.who[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_effect.push_back(uint16_t(effect::relation_province));
					context.compiled_effect.push_back(trigger::payload(it->second).value);
					context.compiled_effect.push_back(trigger::payload(int16_t(value.value)).value);
				} else {
					err.accumulated_errors +=
						"relation effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors +=
					"relation effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors +=
				"relation effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::add_province_modifier(ef_add_province_modifier const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			if(value.duration <= 0) {
				context.compiled_effect.push_back(effect::add_province_modifier_no_duration);
				context.compiled_effect.push_back(trigger::payload(value.name_).value);
			} else {
				context.compiled_effect.push_back(effect::add_province_modifier);
				context.compiled_effect.push_back(trigger::payload(value.name_).value);
				context.compiled_effect.push_back(trigger::payload(int16_t(value.duration)).value);
			}
		} else if(context.main_slot == trigger::slot_contents::state) {
			if(value.duration <= 0) {
				context.compiled_effect.push_back(effect::add_province_modifier_state_no_duration);
				context.compiled_effect.push_back(trigger::payload(value.name_).value);
			} else {
				context.compiled_effect.push_back(effect::add_province_modifier_state);
				context.compiled_effect.push_back(trigger::payload(value.name_).value);
				context.compiled_effect.push_back(trigger::payload(int16_t(value.duration)).value);
			}
		} else {
			err.accumulated_errors += "add_province_modifier effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::add_country_modifier(ef_add_country_modifier const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			if(value.duration <= 0) {
				context.compiled_effect.push_back(effect::add_country_modifier_province_no_duration);
				context.compiled_effect.push_back(trigger::payload(value.name_).value);
			} else {
				context.compiled_effect.push_back(effect::add_country_modifier_province);
				context.compiled_effect.push_back(trigger::payload(value.name_).value);
				context.compiled_effect.push_back(trigger::payload(int16_t(value.duration)).value);
			}
		} else if(context.main_slot == trigger::slot_contents::nation) {
			if(value.duration <= 0) {
				context.compiled_effect.push_back(effect::add_country_modifier_no_duration);
				context.compiled_effect.push_back(trigger::payload(value.name_).value);
			} else {
				context.compiled_effect.push_back(effect::add_country_modifier);
				context.compiled_effect.push_back(trigger::payload(value.name_).value);
				context.compiled_effect.push_back(trigger::payload(int16_t(value.duration)).value);
			}
		} else {
			err.accumulated_errors += "add_country_modifier effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::casus_belli(ef_casus_belli const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "casus_belli effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		if(is_from(value.target)) {
			if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(effect::casus_belli_from_nation);
			else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(effect::casus_belli_from_province);
			else {
				err.accumulated_errors += "casus_belli = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_effect.push_back(trigger::payload(value.type_).value);
			context.compiled_effect.push_back(uint16_t(value.months));
		} else if(is_this(value.target)) {
			if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(effect::casus_belli_this_nation);
			else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(effect::casus_belli_this_province);
			else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(effect::casus_belli_this_state);
			else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_effect.push_back(effect::casus_belli_this_pop);
			else {
				err.accumulated_errors += "casus_belli = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_effect.push_back(trigger::payload(value.type_).value);
			context.compiled_effect.push_back(uint16_t(value.months));
		} else if(is_integer(value.target.data(), value.target.data() + value.target.length())) {
			auto ivalue = parse_int(value.target, line, err);
			if(0 <= ivalue && size_t(ivalue) < context.outer_context.original_id_to_prov_id_map.size()) {
				context.compiled_effect.push_back(effect::casus_belli_int);
				context.compiled_effect.push_back(trigger::payload(value.type_).value);
				context.compiled_effect.push_back(uint16_t(value.months));
				context.compiled_effect.push_back(trigger::payload(context.outer_context.original_id_to_prov_id_map[ivalue]).value);
			} else {
				err.accumulated_errors +=
					"casus_belli effect given an invalid province id (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(value.target.length() == 3) {
			if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value.target[0], value.target[1], value.target[2]));
				it != context.outer_context.map_of_ident_names.end()) {
				context.compiled_effect.push_back(uint16_t(effect::casus_belli_tag));
				context.compiled_effect.push_back(trigger::payload(value.type_).value);
				context.compiled_effect.push_back(uint16_t(value.months));
				context.compiled_effect.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors +=
					"casus_belli effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors +=
				"casus_belli effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::add_casus_belli(ef_add_casus_belli const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "add_casus_belli effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		if(is_from(value.target)) {
			if(context.from_slot == trigger::slot_contents::nation)
			context.compiled_effect.push_back(effect::add_casus_belli_from_nation);
			else if(context.from_slot == trigger::slot_contents::province)
			context.compiled_effect.push_back(effect::add_casus_belli_from_province);
			else {
				err.accumulated_errors += "add_casus_belli = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.compiled_effect.push_back(trigger::payload(value.type_).value);
			context.compiled_effect.push_back(uint16_t(value.months));
		} else if(is_this(value.target)) {
			if(context.this_slot == trigger::slot_contents::nation)
			context.compiled_effect.push_back(effect::add_casus_belli_this_nation);
			else if(context.this_slot == trigger::slot_contents::province)
			context.compiled_effect.push_back(effect::add_casus_belli_this_province);
			else if(context.this_slot == trigger::slot_contents::state)
			context.compiled_effect.push_back(effect::add_casus_belli_this_state);
			else if(context.this_slot == trigger::slot_contents::pop)
			context.compiled_effect.push_back(effect::add_casus_belli_this_pop);
			else {
				err.accumulated_errors += "add_casus_belli = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
			context.compiled_effect.push_back(trigger::payload(value.type_).value);
			context.compiled_effect.push_back(uint16_t(value.months));
		} else if(is_integer(value.target.data(), value.target.data() + value.target.length())) {
			auto ivalue = parse_int(value.target, line, err);
			if(0 <= ivalue && size_t(ivalue) < context.outer_context.original_id_to_prov_id_map.size()) {
				context.compiled_effect.push_back(effect::add_casus_belli_int);
				context.compiled_effect.push_back(trigger::payload(value.type_).value);
				context.compiled_effect.push_back(uint16_t(value.months));
				context.compiled_effect.push_back(trigger::payload(context.outer_context.original_id_to_prov_id_map[ivalue]).value);
			} else {
				err.accumulated_errors +=
					"add_casus_belli effect given an invalid province id (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(value.target.length() == 3) {
			if(auto it = context.outer_context.map_of_ident_names.find(
						nations::tag_to_int(value.target[0], value.target[1], value.target[2]));
				it != context.outer_context.map_of_ident_names.end()) {
				context.compiled_effect.push_back(uint16_t(effect::add_casus_belli_tag));
				context.compiled_effect.push_back(trigger::payload(value.type_).value);
				context.compiled_effect.push_back(uint16_t(value.months));
				context.compiled_effect.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors +=
					"add_casus_belli effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors +=
				"add_casus_belli effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::remove_casus_belli(ef_remove_casus_belli const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "remove_casus_belli effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		if(is_from(value.target)) {
			if(context.from_slot == trigger::slot_contents::nation)
			context.compiled_effect.push_back(effect::remove_casus_belli_from_nation);
			else if(context.from_slot == trigger::slot_contents::province)
			context.compiled_effect.push_back(effect::remove_casus_belli_from_province);
			else {
				err.accumulated_errors += "remove_casus_belli = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name +
																", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_effect.push_back(trigger::payload(value.type_).value);
		} else if(is_this(value.target)) {
			if(context.this_slot == trigger::slot_contents::nation)
			context.compiled_effect.push_back(effect::remove_casus_belli_this_nation);
			else if(context.this_slot == trigger::slot_contents::province)
			context.compiled_effect.push_back(effect::remove_casus_belli_this_province);
			else if(context.this_slot == trigger::slot_contents::state)
			context.compiled_effect.push_back(effect::remove_casus_belli_this_state);
			else if(context.this_slot == trigger::slot_contents::pop)
			context.compiled_effect.push_back(effect::remove_casus_belli_this_pop);
			else {
				err.accumulated_errors += "remove_casus_belli = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name +
																", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_effect.push_back(trigger::payload(value.type_).value);
		} else if(is_integer(value.target.data(), value.target.data() + value.target.length())) {
			auto ivalue = parse_int(value.target, line, err);
			if(0 <= ivalue && size_t(ivalue) < context.outer_context.original_id_to_prov_id_map.size()) {
				context.compiled_effect.push_back(effect::remove_casus_belli_int);
				context.compiled_effect.push_back(trigger::payload(value.type_).value);
				context.compiled_effect.push_back(trigger::payload(context.outer_context.original_id_to_prov_id_map[ivalue]).value);
			} else {
				err.accumulated_errors +=
					"remove_casus_belli effect given an invalid province id (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(value.target.length() == 3) {
			if(auto it = context.outer_context.map_of_ident_names.find(
						nations::tag_to_int(value.target[0], value.target[1], value.target[2]));
				it != context.outer_context.map_of_ident_names.end()) {
				context.compiled_effect.push_back(uint16_t(effect::remove_casus_belli_tag));
				context.compiled_effect.push_back(trigger::payload(value.type_).value);
				context.compiled_effect.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors +=
					"remove_casus_belli effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors +=
				"remove_casus_belli effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}

	void effect_body::this_remove_casus_belli(ef_this_remove_casus_belli const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "remove_casus_belli effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
		if(is_from(value.target)) {
			if(context.from_slot == trigger::slot_contents::nation)
			context.compiled_effect.push_back(effect::this_remove_casus_belli_from_nation);
			else if(context.from_slot == trigger::slot_contents::province)
			context.compiled_effect.push_back(effect::this_remove_casus_belli_from_province);
			else {
				err.accumulated_errors += "this_remove_casus_belli = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name +
																", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_effect.push_back(trigger::payload(value.type_).value);
		} else if(is_this(value.target)) {
			if(context.this_slot == trigger::slot_contents::nation)
			context.compiled_effect.push_back(effect::this_remove_casus_belli_this_nation);
			else if(context.this_slot == trigger::slot_contents::province)
			context.compiled_effect.push_back(effect::this_remove_casus_belli_this_province);
			else if(context.this_slot == trigger::slot_contents::state)
			context.compiled_effect.push_back(effect::this_remove_casus_belli_this_state);
			else if(context.this_slot == trigger::slot_contents::pop)
			context.compiled_effect.push_back(effect::this_remove_casus_belli_this_pop);
			else {
				err.accumulated_errors += "this_remove_casus_belli = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name +
																", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_effect.push_back(trigger::payload(value.type_).value);
		} else if(is_integer(value.target.data(), value.target.data() + value.target.length())) {
			auto ivalue = parse_int(value.target, line, err);
			if(0 <= ivalue && size_t(ivalue) < context.outer_context.original_id_to_prov_id_map.size()) {
				context.compiled_effect.push_back(effect::this_remove_casus_belli_int);
				context.compiled_effect.push_back(trigger::payload(value.type_).value);
				context.compiled_effect.push_back(trigger::payload(context.outer_context.original_id_to_prov_id_map[ivalue]).value);
			} else {
				err.accumulated_errors += "this_remove_casus_belli effect given an invalid province id (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else if(value.target.length() == 3) {
			if(auto it = context.outer_context.map_of_ident_names.find(
						nations::tag_to_int(value.target[0], value.target[1], value.target[2]));
				it != context.outer_context.map_of_ident_names.end()) {
				context.compiled_effect.push_back(uint16_t(effect::this_remove_casus_belli_tag));
				context.compiled_effect.push_back(trigger::payload(value.type_).value);
				context.compiled_effect.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors +=
					"this_remove_casus_belli effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors +=
				"this_remove_casus_belli effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}

	void effect_body::war(ef_war const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "war effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		if(value.attacker_goal.special_end_wargoal) {
			if(is_from(value.target)) {
				if(context.from_slot == trigger::slot_contents::nation)
					context.compiled_effect.push_back(effect::add_truce_from_nation);
				else if(context.from_slot == trigger::slot_contents::province)
					context.compiled_effect.push_back(effect::add_truce_from_province);
				else {
					err.accumulated_errors +=
					"war = from effect (used to create a truce) used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(is_this(value.target)) {
				if(context.this_slot == trigger::slot_contents::nation)
					context.compiled_effect.push_back(effect::add_truce_this_nation);
				else if(context.this_slot == trigger::slot_contents::province)
					context.compiled_effect.push_back(effect::add_truce_this_province);
				else if(context.this_slot == trigger::slot_contents::state)
					context.compiled_effect.push_back(effect::add_truce_this_state);
				else if(context.this_slot == trigger::slot_contents::province)
					context.compiled_effect.push_back(effect::add_truce_this_province);
				else {
					err.accumulated_errors += "war = this  (used to create a truce) used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(value.target.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value.target[0], value.target[1], value.target[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_effect.push_back(effect::add_truce_tag);
					context.compiled_effect.push_back(trigger::payload(it->second).value);
				} else {
					err.accumulated_errors += "war = effect (used to create a truce) given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors += "war = effect (used to create a truce) given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_effect.push_back(uint16_t(12)); // months
			return;
		} else if(value.attacker_goal.special_call_ally_wargoal) {
			context.compiled_effect.push_back(effect::call_allies);
			return;
		}
		if(is_from(value.target)) {
			if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(value.call_ally ? effect::war_from_nation : effect::war_no_ally_from_nation);
			else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(value.call_ally ? effect::war_from_province : effect::war_no_ally_from_province);
			else {
				err.accumulated_errors += "war = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(is_this(value.target)) {
			if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(value.call_ally ? effect::war_this_nation : effect::war_no_ally_this_nation);
			else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(value.call_ally ? effect::war_this_province : effect::war_no_ally_this_province);
			else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(value.call_ally ? effect::war_this_state : effect::war_no_ally_this_state);
			else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_effect.push_back(value.call_ally ? effect::war_this_pop : effect::war_no_ally_this_pop);
			else {
				err.accumulated_errors += "war = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(value.target.length() == 3) {
			if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value.target[0], value.target[1], value.target[2]));
				it != context.outer_context.map_of_ident_names.end()) {
				context.compiled_effect.push_back(uint16_t(value.call_ally ? effect::war_tag : effect::war_no_ally_tag));
				context.compiled_effect.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors += "war = effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(value.target.length() == 0) { // some mods use this as a call allies effect. So far, we just discard it
			return;
		} else {
			err.accumulated_errors += "war = effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_effect.push_back(trigger::payload(value.defender_goal.casus_belli_).value);
		context.compiled_effect.push_back(trigger::payload(value.defender_goal.state_province_id_).value);
		context.compiled_effect.push_back(trigger::payload(value.defender_goal.country_).value);
		context.compiled_effect.push_back(trigger::payload(value.attacker_goal.casus_belli_).value);
		context.compiled_effect.push_back(trigger::payload(value.attacker_goal.state_province_id_).value);
		context.compiled_effect.push_back(trigger::payload(value.attacker_goal.country_).value);
	}

	void effect_body::set_global_flag(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		context.compiled_effect.push_back(uint16_t(effect::set_global_flag));
		context.compiled_effect.push_back(trigger::payload(context.outer_context.get_global_flag(std::string(value))).value);
	}
	void effect_body::clr_global_flag(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		context.compiled_effect.push_back(uint16_t(effect::clr_global_flag));
		context.compiled_effect.push_back(trigger::payload(context.outer_context.get_global_flag(std::string(value))).value);
	}
	void effect_body::nationalvalue(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(auto it = context.outer_context.map_of_modifiers.find(std::string(value));
			it != context.outer_context.map_of_modifiers.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_effect.push_back(uint16_t(effect::nationalvalue_nation));
				context.compiled_effect.push_back(trigger::payload(it->second).value);
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_effect.push_back(uint16_t(effect::nationalvalue_province));
				context.compiled_effect.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors +=
					"nationalvalue effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "nationalvalue effect supplied with invalid modifier name " + std::string(value) + " (" +
															err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::civilized(association_type t, bool value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(value)
			context.compiled_effect.push_back(uint16_t(effect::civilized_yes | effect::no_payload));
			else
			context.compiled_effect.push_back(uint16_t(effect::civilized_no | effect::no_payload));
		} else {
			err.accumulated_errors +=
				"civilized effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::election(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::election | effect::no_payload));
		} else {
			err.accumulated_errors +=
				"election effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::social_reform(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(auto it = context.outer_context.map_of_ioptions.find(std::string(value));
			it != context.outer_context.map_of_ioptions.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_effect.push_back(uint16_t(effect::social_reform));
				context.compiled_effect.push_back(trigger::payload(it->second.id).value);
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_effect.push_back(uint16_t(effect::social_reform_province));
				context.compiled_effect.push_back(trigger::payload(it->second.id).value);
			} else {
				err.accumulated_errors +=
					"social_reform effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "social_reform effect supplied with invalid issue option name " + std::string(value) + " (" +
															err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::political_reform(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(auto it = context.outer_context.map_of_ioptions.find(std::string(value));
			it != context.outer_context.map_of_ioptions.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_effect.push_back(uint16_t(effect::political_reform));
				context.compiled_effect.push_back(trigger::payload(it->second.id).value);
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_effect.push_back(uint16_t(effect::political_reform_province));
				context.compiled_effect.push_back(trigger::payload(it->second.id).value);
			} else {
				err.accumulated_errors += "political_reform effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "political_reform effect supplied with invalid issue option name " + std::string(value) + " (" +
															err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::add_tax_relative_income(association_type t, float value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::add_tax_relative_income));
			context.add_float_to_payload(value);
		} else {
			err.accumulated_errors += "add_tax_relative_income effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::neutrality(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::neutrality | effect::no_payload));
		} else {
			err.accumulated_errors +=
				"neutrality effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::reduce_pop(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context) {
		if(value < 0.f) {
			err.accumulated_warnings += "reduce_pop effect with a negative value (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
		value = std::max(0.0f, value);
		if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_effect.push_back(uint16_t(effect::reduce_pop));
			context.add_float_to_payload(value);
		} else if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::reduce_pop_nation));
			context.add_float_to_payload(value);
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::reduce_pop_province));
			context.add_float_to_payload(value);
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::reduce_pop_state));
			context.add_float_to_payload(value);
		} else {
			err.accumulated_errors += "reduce_pop effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::move_pop(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::pop) {
			if(0 <= value && size_t(value) < context.outer_context.original_id_to_prov_id_map.size()) {
				context.compiled_effect.push_back(uint16_t(effect::move_pop));
				context.compiled_effect.push_back(trigger::payload(context.outer_context.original_id_to_prov_id_map[value]).value);
			} else {
				err.accumulated_errors +=
					"move_pop effect given an invalid province id (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors +=
				"move_pop effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::pop_type(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(auto it = context.outer_context.map_of_poptypes.find(std::string(value));
			it != context.outer_context.map_of_poptypes.end()) {
			if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_effect.push_back(uint16_t(effect::pop_type));
				context.compiled_effect.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors +=
					"pop_type effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "pop_type effect supplied with invalid pop type " + std::string(value) + " (" + err.file_name +
															", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::years_of_research(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::years_of_research));
			context.add_float_to_payload(value);
		} else {
			err.accumulated_errors +=
				"years_of_research effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::prestige_factor(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(value >= 0.0f)
			context.compiled_effect.push_back(uint16_t(effect::prestige_factor_positive));
			else
			context.compiled_effect.push_back(uint16_t(effect::prestige_factor_negative));
			context.add_float_to_payload(value);
		} else {
			err.accumulated_errors +=
				"prestige_factor effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::military_reform(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(auto it = context.outer_context.map_of_roptions.find(std::string(value));
			it != context.outer_context.map_of_roptions.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_effect.push_back(uint16_t(effect::military_reform));
				context.compiled_effect.push_back(trigger::payload(it->second.id).value);
			} else {
				err.accumulated_errors +=
					"military_reform effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "military_reform effect supplied with invalid issue option name " + std::string(value) + " (" +
															err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::economic_reform(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(auto it = context.outer_context.map_of_roptions.find(std::string(value));
			it != context.outer_context.map_of_roptions.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_effect.push_back(uint16_t(effect::economic_reform));
				context.compiled_effect.push_back(trigger::payload(it->second.id).value);
			} else {
				err.accumulated_errors +=
					"economic_reform effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "economic_reform effect supplied with invalid issue option name " + std::string(value) + " (" +
															err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::remove_random_military_reforms(association_type t, int32_t value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::remove_random_military_reforms));
			context.compiled_effect.push_back(trigger::payload(int16_t(value)).value);
		} else {
			err.accumulated_errors += "remove_random_military_reforms effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name +
															", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::remove_random_economic_reforms(association_type t, int32_t value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::remove_random_economic_reforms));
			context.compiled_effect.push_back(trigger::payload(int16_t(value)).value);
		} else {
			err.accumulated_errors += "remove_random_economic_reforms effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name +
															", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::add_crime(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(auto it = context.outer_context.map_of_crimes.find(std::string(value)); it != context.outer_context.map_of_crimes.end()) {
			if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_effect.push_back(uint16_t(effect::add_crime));
				context.compiled_effect.push_back(trigger::payload(it->second.id).value);
			} else {
				err.accumulated_errors +=
					"add_crime effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "none")) {
			context.compiled_effect.push_back(uint16_t(effect::add_crime_none) | effect::no_payload);
		} else {
			err.accumulated_errors += "add_crime effect supplied with invalid modifier name " + std::string(value) + " (" +
															err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::nationalize(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::nationalize | effect::no_payload));
		} else {
			err.accumulated_errors += "nationalize effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::build_factory_in_capital_state(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(auto it = context.outer_context.map_of_factory_names.find(std::string(value));
			it != context.outer_context.map_of_factory_names.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_effect.push_back(uint16_t(effect::build_factory_in_capital_state));
				context.compiled_effect.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors += "build_factory_in_capital_state effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "build_factory_in_capital_state effect supplied with invalid factory name " + std::string(value) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::activate_technology(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(auto it = context.outer_context.map_of_technologies.find(std::string(value)); it != context.outer_context.map_of_technologies.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_effect.push_back(uint16_t(effect::activate_technology));
				context.compiled_effect.push_back(trigger::payload(it->second.id).value);
			} else {
				err.accumulated_errors += "activate_technology effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(auto itb = context.outer_context.map_of_inventions.find(std::string(value)); itb != context.outer_context.map_of_inventions.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_effect.push_back(uint16_t(effect::activate_invention));
				context.compiled_effect.push_back(trigger::payload(itb->second.id).value);
			} else {
				err.accumulated_errors += "activate_technology effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "activate_technology effect supplied with invalid technology/invention name " + std::string(value) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::great_wars_enabled(association_type t, bool value, error_handler& err, int32_t line, effect_building_context& context) {
		if(value)
			context.compiled_effect.push_back(uint16_t(effect::great_wars_enabled_yes | effect::no_payload));
		else
			context.compiled_effect.push_back(uint16_t(effect::great_wars_enabled_no | effect::no_payload));
	}
	void effect_body::world_wars_enabled(association_type t, bool value, error_handler& err, int32_t line, effect_building_context& context) {
		if(value)
			context.compiled_effect.push_back(uint16_t(effect::world_wars_enabled_yes | effect::no_payload));
		else
			context.compiled_effect.push_back(uint16_t(effect::world_wars_enabled_no | effect::no_payload));
	}
	void effect_body::assimilate(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::assimilate_province | effect::no_payload));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_effect.push_back(uint16_t(effect::assimilate_pop | effect::no_payload));
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::assimilate_state | effect::no_payload));
		} else {
			err.accumulated_errors += "assimilate effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::literacy(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_effect.push_back(uint16_t(effect::literacy));
			context.add_float_to_payload(value);
		} else {
			err.accumulated_errors += "literacy effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::add_crisis_interest(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::add_crisis_interest | effect::no_payload));
		} else {
			err.accumulated_errors += "add_crisis_interest effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::flashpoint_tension(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::flashpoint_tension));
			context.add_float_to_payload(value);
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::flashpoint_tension_province));
			context.add_float_to_payload(value);
		} else {
			err.accumulated_errors += "flashpoint_tension effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::add_crisis_temperature(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context) {
		context.compiled_effect.push_back(uint16_t(effect::add_crisis_temperature));
		context.add_float_to_payload(value);
	}
	void effect_body::consciousness(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_effect.push_back(uint16_t(effect::consciousness));
			context.add_float_to_payload(value);
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::consciousness_province));
			context.add_float_to_payload(value);
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::consciousness_state));
			context.add_float_to_payload(value);
		} else if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::consciousness_nation));
			context.add_float_to_payload(value);
		} else {
			err.accumulated_errors +=
				"consciousness effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::militancy(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_effect.push_back(uint16_t(effect::militancy));
			context.add_float_to_payload(value);
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::militancy_province));
			context.add_float_to_payload(value);
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::militancy_state));
			context.add_float_to_payload(value);
		} else if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::militancy_nation));
			context.add_float_to_payload(value);
		} else {
			err.accumulated_errors +=
				"militancy effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::rgo_size(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::rgo_size));
			context.compiled_effect.push_back(trigger::payload(int16_t(value)).value);
		} else {
			err.accumulated_errors += "rgo_size effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}

	void effect_body::set_country_flag(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::set_country_flag));
			context.compiled_effect.push_back(trigger::payload(context.outer_context.get_national_flag(std::string(value))).value);
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::set_country_flag_province));
			context.compiled_effect.push_back(trigger::payload(context.outer_context.get_national_flag(std::string(value))).value);
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_effect.push_back(uint16_t(effect::set_country_flag_pop));
			context.compiled_effect.push_back(trigger::payload(context.outer_context.get_national_flag(std::string(value))).value);
		} else {
			err.accumulated_errors +=
				"set_country_flag effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::clr_country_flag(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::clr_country_flag));
			context.compiled_effect.push_back(trigger::payload(context.outer_context.get_national_flag(std::string(value))).value);
		} else {
			err.accumulated_errors += "clr_country_flag effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::set_province_flag(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::set_province_flag));
			context.compiled_effect.push_back(trigger::payload(context.outer_context.get_provincial_flag(std::string(value))).value);
		} else {
			err.accumulated_errors += "set_province_flag effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::clr_province_flag(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::clr_province_flag));
			context.compiled_effect.push_back(trigger::payload(context.outer_context.get_provincial_flag(std::string(value))).value);
		} else {
			err.accumulated_errors += "clr_province_flag effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::remove_crisis(association_type t, bool value, error_handler& err, int32_t line, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension remove_crisis but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}
		context.compiled_effect.push_back(uint16_t(effect::remove_crisis));
	}
	void effect_body::trigger_crisis(ef_trigger_crisis value, error_handler& err, int32_t line, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension trigger_crisis but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}
		if(value.type_ == sys::crisis_type::none) {
			err.accumulated_errors += "Invalid crisis type (" + err.file_name + ")\n";
			return;
		} else {
			if(value.overwrite) {
				context.compiled_effect.push_back(uint16_t(effect::remove_crisis));
			}
			context.compiled_effect.push_back(uint16_t(effect::set_crisis_type));
			context.compiled_effect.push_back(uint16_t(value.type_));
		}
		if(value.colony.empty()) {
			/* Nothing */
		} else if(is_this(value.colony)) {
			if(context.this_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(uint16_t(effect::set_crisis_colony_this));
			else {
				err.accumulated_errors += "Invalid THIS slot " +  slot_contents_to_string(context.this_slot) + " for crisis colony (" + err.file_name + ")\n";
			}
		} else if(is_from(value.colony)) {
			if(context.from_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(uint16_t(effect::set_crisis_colony_from));
			else {
				err.accumulated_errors += "Invalid FROM slot " +  slot_contents_to_string(context.from_slot) + " for crisis colony (" + err.file_name + ")\n";
			}
		} else if(auto it = context.outer_context.map_of_state_names.find(std::string(value.colony)); it != context.outer_context.map_of_state_names.end()) {
			context.compiled_effect.push_back(uint16_t(effect::set_crisis_colony));
			context.compiled_effect.push_back(trigger::payload(it->second).value);
		} else {
			err.accumulated_errors += "Invalid state " + std::string(value.colony) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
		if(value.liberation_tag.empty()) {
			/* Nothing */
		} else if(is_this(value.liberation_tag)) {
			if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::set_crisis_liberation_tag_this));
			else {
				err.accumulated_errors += "Invalid THIS slot " +  slot_contents_to_string(context.this_slot) + " for crisis colony (" + err.file_name + ")\n";
			}
		} else if(is_from(value.liberation_tag)) {
			if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::set_crisis_liberation_tag_from));
			else {
				err.accumulated_errors += "Invalid FROM slot " +  slot_contents_to_string(context.from_slot) + " for crisis colony (" + err.file_name + ")\n";
			}
		} else if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value.colony[0], value.colony[1], value.colony[2])); it != context.outer_context.map_of_ident_names.end()) {
			context.compiled_effect.push_back(uint16_t(effect::set_crisis_liberation_tag));
			context.compiled_effect.push_back(trigger::payload(it->second).value);
		} else {
			err.accumulated_errors += "Invalid liberation_tag " + std::string(value.liberation_tag) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
		context.compiled_effect.push_back(uint16_t(effect::trigger_crisis));
	}
	void effect_body::military_access(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::military_access_this_nation | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::military_access_this_province | effect::no_payload));
				else {
					err.accumulated_errors += "military_access = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name +
																	", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::military_access_from_nation | effect::no_payload));
				else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::military_access_from_province | effect::no_payload));
				else {
					err.accumulated_errors += "military_access = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name +
																	", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_effect.push_back(uint16_t(effect::military_access));
					context.compiled_effect.push_back(trigger::payload(it->second).value);
				} else {
					err.accumulated_errors +=
						"military_access effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors +=
					"military_access effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}

		} else {
			err.accumulated_errors +=
				"military_access effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::badboy(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::badboy));
			context.add_float_to_payload(value);
		} else {
			err.accumulated_errors +=
				"badboy effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::secede_province(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::secede_province_this_nation | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::secede_province_this_province | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(uint16_t(effect::secede_province_this_state | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_effect.push_back(uint16_t(effect::secede_province_this_pop | effect::no_payload));
				else {
					err.accumulated_errors += "secede_province = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::secede_province_from_nation | effect::no_payload));
				else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::secede_province_from_province | effect::no_payload));
				else {
					err.accumulated_errors += "secede_province = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(is_reb(value)) {
				if(context.from_slot == trigger::slot_contents::rebel)
				context.compiled_effect.push_back(uint16_t(effect::secede_province_reb | effect::no_payload));
				else {
					err.accumulated_errors += "secede_province = reb effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "---")) {
				context.compiled_effect.push_back(uint16_t(effect::annex_to_null_province | effect::no_payload));
				return;
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_effect.push_back(uint16_t(effect::secede_province));
					context.compiled_effect.push_back(trigger::payload(it->second).value);
				} else {
					context.compiled_effect.push_back(uint16_t(effect::annex_to_null_province | effect::no_payload));
					err.accumulated_warnings += "secede_province effect given an invalid tag '" + std::string(value) + "' will assume null (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "null")) {
				context.compiled_effect.push_back(uint16_t(effect::annex_to_null_province | effect::no_payload));
				return;
			} else {
				err.accumulated_errors += "secede_province effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(context.main_slot == trigger::slot_contents::state) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::secede_province_state_this_nation | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::secede_province_state_this_province | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(uint16_t(effect::secede_province_state_this_state | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_effect.push_back(uint16_t(effect::secede_province_state_this_pop | effect::no_payload));
				else {
					err.accumulated_errors += "secede_province = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::secede_province_state_from_nation | effect::no_payload));
				else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::secede_province_state_from_province | effect::no_payload));
				else {
					err.accumulated_errors += "secede_province = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(is_reb(value)) {
				if(context.from_slot == trigger::slot_contents::rebel)
				context.compiled_effect.push_back(uint16_t(effect::secede_province_state_reb | effect::no_payload));
				else {
					err.accumulated_errors += "secede_province = reb effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_effect.push_back(uint16_t(effect::secede_province_state));
					context.compiled_effect.push_back(trigger::payload(it->second).value);
				} else {
					err.accumulated_errors += "secede_province effect given an invalid tag '" + std::string(value) + "' (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors += "secede_province effect given an invalid value '"	+ std::string(value) + "' (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "secede_province effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::inherit(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::inherit_this_nation | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::inherit_this_province | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(uint16_t(effect::inherit_this_state | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_effect.push_back(uint16_t(effect::inherit_this_pop | effect::no_payload));
				else {
					err.accumulated_errors += "inherit = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::inherit_from_nation | effect::no_payload));
				else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::inherit_from_province | effect::no_payload));
				else {
					err.accumulated_errors += "inherit = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_effect.push_back(uint16_t(effect::inherit));
					context.compiled_effect.push_back(trigger::payload(it->second).value);
				} else {
					err.accumulated_errors +=
						"inherit effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors +=
					"inherit effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}

		} else {
			err.accumulated_errors +=
				"inherit effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::annex_to(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::annex_to_this_nation | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::annex_to_this_province | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(uint16_t(effect::annex_to_this_state | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_effect.push_back(uint16_t(effect::annex_to_this_pop | effect::no_payload));
				else {
					err.accumulated_errors += "annex_to = this effect used in an incorrect scope type " + slot_contents_to_string(context.this_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::annex_to_from_nation | effect::no_payload));
				else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::annex_to_from_province | effect::no_payload));
				else {
					context.compiled_effect.push_back(uint16_t(effect::annex_to_null_nation | effect::no_payload));
					err.accumulated_errors += "annex_to = from effect used in an incorrect scope type - will assume null " + slot_contents_to_string(context.from_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_effect.push_back(uint16_t(effect::annex_to));
					context.compiled_effect.push_back(trigger::payload(it->second).value);
				} else {
					err.accumulated_errors +=
					"annex_to effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "null")) {
				context.compiled_effect.push_back(uint16_t(effect::annex_to_null_nation | effect::no_payload));
				return;
			} else {
				err.accumulated_errors +=
				"annex_to effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(context.main_slot == trigger::slot_contents::province) {
			if(is_fixed_token_ci(value.data(), value.data() + value.length(), "null")) {
				context.compiled_effect.push_back(uint16_t(effect::annex_to_null_province | effect::no_payload));
				return;
			} else {
				err.accumulated_errors += "annex_to effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
				std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors +=
				"annex_to effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::release(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::release_this_nation | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::release_this_province | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(uint16_t(effect::release_this_state | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_effect.push_back(uint16_t(effect::release_this_pop | effect::no_payload));
				else {
					err.accumulated_errors += "release = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::release_from_nation | effect::no_payload));
				else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::release_from_province | effect::no_payload));
				else {
					err.accumulated_errors += "release = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_effect.push_back(uint16_t(effect::release));
					context.compiled_effect.push_back(trigger::payload(it->second).value);
				} else {
					err.accumulated_errors +=
						"release effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors +=
					"release effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}

		} else {
			err.accumulated_errors +=
				"release effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::change_controller(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::change_controller_this_nation | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::change_controller_this_province | effect::no_payload));
				else {
					err.accumulated_errors += "change_controller = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name +
					", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::change_controller_from_nation | effect::no_payload));
				else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::change_controller_from_province | effect::no_payload));
				else {
					err.accumulated_errors += "change_controller = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name +
					", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_effect.push_back(uint16_t(effect::change_controller));
					context.compiled_effect.push_back(trigger::payload(it->second).value);
				} else {
					err.accumulated_errors +=
					"change_controller effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors +=
				"change_controller effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(context.main_slot == trigger::slot_contents::state) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::change_controller_state_this_nation | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::change_controller_state_this_province | effect::no_payload));
				else {
					err.accumulated_errors += "change_controller = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name +
					", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::change_controller_state_from_nation | effect::no_payload));
				else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::change_controller_state_from_province | effect::no_payload));
				else {
					err.accumulated_errors += "change_controller = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name +
					", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_effect.push_back(uint16_t(effect::change_controller_state));
					context.compiled_effect.push_back(trigger::payload(it->second).value);
				} else {
					err.accumulated_errors +=
					"change_controller effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors +=
				"change_controller effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors +=
				"change_controller effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::infrastructure(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::infrastructure));
			context.compiled_effect.push_back(trigger::payload(int16_t(value)).value);
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::infrastructure_state));
			context.compiled_effect.push_back(trigger::payload(int16_t(value)).value);
		} else {
			err.accumulated_errors +=
				"infrastructure effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::fort(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::fort));
			context.compiled_effect.push_back(trigger::payload(int16_t(value)).value);
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::fort_state));
			context.compiled_effect.push_back(trigger::payload(int16_t(value)).value);
		} else {
			err.accumulated_errors +=
				"fort effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::naval_base(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::naval_base));
			context.compiled_effect.push_back(trigger::payload(int16_t(value)).value);
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::naval_base_state));
			context.compiled_effect.push_back(trigger::payload(int16_t(value)).value);
		} else {
			err.accumulated_errors +=
				"naval_base effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::bank(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::bank));
			context.compiled_effect.push_back(trigger::payload(int16_t(value)).value);
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::bank_state));
			context.compiled_effect.push_back(trigger::payload(int16_t(value)).value);
		} else {
			err.accumulated_errors +=
			"bank effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::university(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::university));
			context.compiled_effect.push_back(trigger::payload(int16_t(value)).value);
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::university_state));
			context.compiled_effect.push_back(trigger::payload(int16_t(value)).value);
		} else {
			err.accumulated_errors +=
			"university effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::province_selector(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			if(value == 1) {
				context.compiled_effect.push_back(uint16_t(effect::add_province_modifier_no_duration));
				context.compiled_effect.push_back(trigger::payload(economy::get_province_selector_modifier(context.outer_context.state)).value);
			} else if(value == -1) {
				context.compiled_effect.push_back(uint16_t(effect::remove_province_modifier));
				context.compiled_effect.push_back(trigger::payload(economy::get_province_selector_modifier(context.outer_context.state)).value);
			} else {
				err.accumulated_errors +=
				"province_selector effect with invalid value " + std::to_string(value) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			}
		} else {
			err.accumulated_errors +=
			"province_selector effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::province_immigrator(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			if(value == 1) {
				context.compiled_effect.push_back(uint16_t(effect::add_province_modifier_no_duration));
				context.compiled_effect.push_back(trigger::payload(economy::get_province_immigrator_modifier(context.outer_context.state)).value);
			} else if(value == -1) {
				context.compiled_effect.push_back(uint16_t(effect::remove_province_modifier));
				context.compiled_effect.push_back(trigger::payload(economy::get_province_immigrator_modifier(context.outer_context.state)).value);
			} else {
				err.accumulated_errors +=
				"province_immigrator effect with invalid value " + std::to_string(value) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			}
		} else {
			err.accumulated_errors +=
			"province_immigrator effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::money(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::treasury));
			context.add_float_to_payload(value);
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::treasury_province));
			context.add_float_to_payload(value);
		} else if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_effect.push_back(uint16_t(effect::money));
			context.add_float_to_payload(value);
		} else {
			err.accumulated_errors +=
				"money effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::leadership(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::leadership));
			context.compiled_effect.push_back(trigger::payload(int16_t(value)).value);
		} else {
			err.accumulated_errors +=
				"leadership effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::create_vassal(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::create_vassal_this_nation | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::create_vassal_this_province | effect::no_payload));
				else {
					err.accumulated_errors += "create_vassal = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::create_vassal_from_nation | effect::no_payload));
				else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::create_vassal_from_province | effect::no_payload));
				else {
					err.accumulated_errors += "create_vassal = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_effect.push_back(uint16_t(effect::create_vassal));
					context.compiled_effect.push_back(trigger::payload(it->second).value);
				} else {
					err.accumulated_errors +=
						"create_vassal effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors +=
					"create_vassal effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}

		} else {
			err.accumulated_errors +=
				"create_vassal effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::end_military_access(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::end_military_access_this_nation | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::end_military_access_this_province | effect::no_payload));
				else {
					err.accumulated_errors += "end_military_access = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name +
																	", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::end_military_access_from_nation | effect::no_payload));
				else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::end_military_access_from_province | effect::no_payload));
				else {
					err.accumulated_errors += "end_military_access = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name +
																	", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_effect.push_back(uint16_t(effect::end_military_access));
					context.compiled_effect.push_back(trigger::payload(it->second).value);
				} else {
					err.accumulated_errors +=
						"end_military_access effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors +=
					"end_military_access effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}

		} else {
			err.accumulated_errors += "end_military_access effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::leave_alliance(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::leave_alliance_this_nation | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::leave_alliance_this_province | effect::no_payload));
				else {
					err.accumulated_errors += "leave_alliance = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::leave_alliance_from_nation | effect::no_payload));
				else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::leave_alliance_from_province | effect::no_payload));
				else {
					err.accumulated_errors += "leave_alliance = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_effect.push_back(uint16_t(effect::leave_alliance));
					context.compiled_effect.push_back(trigger::payload(it->second).value);
				} else {
					err.accumulated_errors +=
						"leave_alliance effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors +=
					"leave_alliance effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}

		} else {
			err.accumulated_errors += "end_military_access effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::end_war(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::end_war_this_nation | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::end_war_this_province | effect::no_payload));
				else {
					err.accumulated_errors += "end_war = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::end_war_from_nation | effect::no_payload));
				else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::end_war_from_province | effect::no_payload));
				else {
					err.accumulated_errors += "end_war = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_effect.push_back(uint16_t(effect::end_war));
					context.compiled_effect.push_back(trigger::payload(it->second).value);
				} else {
					err.accumulated_errors +=
						"end_war effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors +=
					"end_war effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}

		} else {
			err.accumulated_errors +=
				"end_war effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::enable_ideology(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(auto it = context.outer_context.map_of_ideologies.find(std::string(value));
			it != context.outer_context.map_of_ideologies.end()) {
			context.compiled_effect.push_back(uint16_t(effect::enable_ideology));
			context.compiled_effect.push_back(trigger::payload(it->second.id).value);
		} else {
			err.accumulated_errors += "enable_ideology effect supplied with invalid ideology name " + std::string(value) + " (" +
															err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::ruling_party_ideology(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				context.compiled_effect.push_back(uint16_t(effect::ruling_party_this));
			} else if(is_from(value)) {
				context.compiled_effect.push_back(uint16_t(effect::ruling_party_from));
			} else if(auto it = context.outer_context.map_of_ideologies.find(std::string(value));
				it != context.outer_context.map_of_ideologies.end()) {
				context.compiled_effect.push_back(uint16_t(effect::ruling_party_ideology));
				context.compiled_effect.push_back(trigger::payload(it->second.id).value);
			} else {
				err.accumulated_errors += "ruling_party_ideology effect supplied with invalid ideology name " + std::string(value) +
				" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "ruling_party_ideology effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::plurality(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::plurality));
			context.add_float_to_payload(value);
		} else {
			err.accumulated_errors +=
				"plurality effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::remove_province_modifier(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			if(auto it = context.outer_context.map_of_modifiers.find(std::string(value));
				it != context.outer_context.map_of_modifiers.end()) {
				context.compiled_effect.push_back(uint16_t(effect::remove_province_modifier));
				context.compiled_effect.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors += "remove_province_modifier effect supplied with invalid modifier name " + std::string(value) +
				" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(context.main_slot == trigger::slot_contents::state) {
			if(auto it = context.outer_context.map_of_modifiers.find(std::string(value));
				it != context.outer_context.map_of_modifiers.end()) {
				context.compiled_effect.push_back(uint16_t(effect::remove_province_modifier_state));
				context.compiled_effect.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors += "remove_province_modifier effect supplied with invalid modifier name " + std::string(value) +
				" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "remove_province_modifier effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::remove_country_modifier(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(auto it = context.outer_context.map_of_modifiers.find(std::string(value));
				it != context.outer_context.map_of_modifiers.end()) {
				context.compiled_effect.push_back(uint16_t(effect::remove_country_modifier));
				context.compiled_effect.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors += "remove_country_modifier effect supplied with invalid modifier name " + std::string(value) +
																" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "remove_country_modifier effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::create_alliance(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::create_alliance_this_nation | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::create_alliance_this_province | effect::no_payload));
				else {
					err.accumulated_errors += "create_alliance = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name +
																	", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::create_alliance_from_nation | effect::no_payload));
				else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::create_alliance_from_province | effect::no_payload));
				else {
					err.accumulated_errors += "create_alliance = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name +
																	", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_effect.push_back(uint16_t(effect::create_alliance));
					context.compiled_effect.push_back(trigger::payload(it->second).value);
				} else {
					err.accumulated_errors +=
						"create_alliance effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors +=
					"create_alliance effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}

		} else {
			err.accumulated_errors +=
				"create_alliance effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::release_vassal(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::release_vassal_this_nation | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::release_vassal_this_province | effect::no_payload));
				else {
					err.accumulated_errors += "release_vassal = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::release_vassal_from_nation | effect::no_payload));
				else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::release_vassal_from_province | effect::no_payload));
				else {
					err.accumulated_errors += "release_vassal = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(is_reb(value)) {
				if(context.from_slot == trigger::slot_contents::rebel)
				context.compiled_effect.push_back(uint16_t(effect::release_vassal_reb | effect::no_payload));
				else {
					err.accumulated_errors += "release_vassal = reb effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "random")) {
				context.compiled_effect.push_back(uint16_t(effect::release_vassal_random | effect::no_payload));
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_effect.push_back(uint16_t(effect::release_vassal));
					context.compiled_effect.push_back(trigger::payload(it->second).value);
				} else {
					err.accumulated_errors += "release_vassal effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors += "release_vassal effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(context.main_slot == trigger::slot_contents::province) {
			if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::release_vassal_province_this_nation | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::release_vassal_province_this_province | effect::no_payload));
				else {
					err.accumulated_errors += "release_vassal = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::release_vassal_province_from_nation | effect::no_payload));
				else if(context.from_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::release_vassal_province_from_province | effect::no_payload));
				else {
					err.accumulated_errors += "release_vassal = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(is_reb(value)) {
				if(context.from_slot == trigger::slot_contents::rebel)
				context.compiled_effect.push_back(uint16_t(effect::release_vassal_province_reb | effect::no_payload));
				else {
					err.accumulated_errors += "release_vassal = reb effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
					std::to_string(line) + ")\n";
					return;
				}
			} else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "random")) {
				context.compiled_effect.push_back(uint16_t(effect::release_vassal_province_random | effect::no_payload));
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_effect.push_back(uint16_t(effect::release_vassal_province));
					context.compiled_effect.push_back(trigger::payload(it->second).value);
				} else {
					err.accumulated_errors += "release_vassal effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors += "release_vassal effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "release_vassal effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}

	void effect_body::trade_goods(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(auto it = context.outer_context.map_of_commodity_names.find(std::string(value));
			it != context.outer_context.map_of_commodity_names.end()) {
			if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_effect.push_back(uint16_t(effect::trade_goods));
				context.compiled_effect.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors +=
					"trade_goods effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "trade_goods effect supplied with invalid commodity name " + std::string(value) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::add_accepted_culture(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_fixed_token_ci(value.data(), value.data() + value.length(), "union")) {
				context.compiled_effect.push_back(uint16_t(effect::add_accepted_culture_union | effect::no_payload));
			} else if(is_fixed_token_ci(value.data(), value.data() + value.length(), "this_union")) {
				context.compiled_effect.push_back(uint16_t(effect::add_accepted_culture_union_this | effect::no_payload));
			} else if(context.outer_context.use_extensions && is_fixed_token_ci(value.data(), value.data() + value.length(), "from_union")) {
				context.compiled_effect.push_back(uint16_t(effect::add_accepted_culture_union_from | effect::no_payload));
			} else if(is_this(value)) {
				context.compiled_effect.push_back(uint16_t(effect::add_accepted_culture_this | effect::no_payload));
			} else if(is_from(value)) {
				context.compiled_effect.push_back(uint16_t(effect::add_accepted_culture_from | effect::no_payload));
			} else if(auto it = context.outer_context.map_of_culture_names.find(std::string(value));
							it != context.outer_context.map_of_culture_names.end()) {
				context.compiled_effect.push_back(uint16_t(effect::add_accepted_culture));
				context.compiled_effect.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors += "add_accepted_culture effect supplied with invalid culture name " + std::string(value) + " (" +
																err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "add_accepted_culture effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::primary_culture(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_from(value)) {
				if(context.from_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::primary_culture_from_nation | effect::no_payload));
				else {
					err.accumulated_errors += "primary_culture = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name +
																	", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(is_this(value)) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::primary_culture_this_nation | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(uint16_t(effect::primary_culture_this_state | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::primary_culture_this_province | effect::no_payload));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_effect.push_back(uint16_t(effect::primary_culture_this_pop | effect::no_payload));
				else {
					err.accumulated_errors += "primary_culture = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name +
																	", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				if(auto it = context.outer_context.map_of_culture_names.find(std::string(value));
					it != context.outer_context.map_of_culture_names.end()) {
					context.compiled_effect.push_back(uint16_t(effect::primary_culture));
					context.compiled_effect.push_back(trigger::payload(it->second).value);
				} else {
					err.accumulated_errors += "primary_culture effect supplied with invalid culture name " + std::string(value) + " (" +
																	err.file_name + ", line " + std::to_string(line) + ")\n";
				}
			}
		} else {
			err.accumulated_errors +=
				"primary_culture effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::remove_accepted_culture(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(auto it = context.outer_context.map_of_culture_names.find(std::string(value));
				it != context.outer_context.map_of_culture_names.end()) {
				context.compiled_effect.push_back(uint16_t(effect::remove_accepted_culture));
				context.compiled_effect.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors += "remove_accepted_culture effect supplied with invalid culture name " + std::string(value) +
																" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "remove_accepted_culture effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::life_rating(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::life_rating));
			context.compiled_effect.push_back(trigger::payload(int16_t(value)).value);
		} else if(context.main_slot == trigger::slot_contents::state) {
			context.compiled_effect.push_back(uint16_t(effect::life_rating_state));
			context.compiled_effect.push_back(trigger::payload(int16_t(value)).value);
		} else {
			err.accumulated_errors +=
				"life_rating effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::religion(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(auto it = context.outer_context.map_of_religion_names.find(std::string(value));
				it != context.outer_context.map_of_religion_names.end()) {
				context.compiled_effect.push_back(uint16_t(effect::religion));
				context.compiled_effect.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors += "religion effect supplied with invalid religion name " + std::string(value) + " (" +
																err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors +=
				"religion effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::is_slave(association_type t, bool value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::state) {
			if(value)
			context.compiled_effect.push_back(uint16_t(effect::is_slave_state_yes | effect::no_payload));
			else
			context.compiled_effect.push_back(uint16_t(effect::is_slave_state_no | effect::no_payload));
		} else if(context.main_slot == trigger::slot_contents::province) {
			if(value)
			context.compiled_effect.push_back(uint16_t(effect::is_slave_province_yes | effect::no_payload));
			else
			context.compiled_effect.push_back(uint16_t(effect::is_slave_province_no | effect::no_payload));
		} else if(context.main_slot == trigger::slot_contents::pop) {
			if(value)
			context.compiled_effect.push_back(uint16_t(effect::is_slave_pop_yes | effect::no_payload));
			else
			context.compiled_effect.push_back(uint16_t(effect::is_slave_pop_no | effect::no_payload));
		} else {
			err.accumulated_errors +=
				"is_slave effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::research_points(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::research_points));
			context.compiled_effect.push_back(trigger::payload(int16_t(value)).value);
		} else {
			err.accumulated_errors +=
				"research_points effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::tech_school(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(auto it = context.outer_context.map_of_modifiers.find(std::string(value));
				it != context.outer_context.map_of_modifiers.end()) {
				context.compiled_effect.push_back(uint16_t(effect::tech_school));
				context.compiled_effect.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors += "tech_school effect supplied with invalid modifier name " + std::string(value) + " (" +
																err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors +=
				"tech_school effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::government(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_reb(value)) {
				if(context.from_slot == trigger::slot_contents::rebel)
				context.compiled_effect.push_back(uint16_t(effect::government_reb | effect::no_payload));
				else {
					err.accumulated_errors += "government = reb effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else if(auto it = context.outer_context.map_of_governments.find(std::string(value));
							it != context.outer_context.map_of_governments.end()) {
				context.compiled_effect.push_back(uint16_t(effect::government));
				context.compiled_effect.push_back(trigger::payload(it->second).value);
			} else {
				err.accumulated_errors += "government effect supplied with invalid government name " + std::string(value) + " (" +
																err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors +=
				"government effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::treasury(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::treasury));
			context.add_float_to_payload(value);
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(uint16_t(effect::treasury_province));
			context.add_float_to_payload(value);
		} else {
			err.accumulated_errors +=
				"treasury effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::war_exhaustion(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::war_exhaustion));
			context.add_float_to_payload(value);
		} else {
			err.accumulated_errors +=
				"war_exhaustion effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::prestige(association_type t, float value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::prestige));
			context.add_float_to_payload(value);
		} else {
			err.accumulated_errors +=
				"prestige effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::change_tag(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_fixed_token_ci(value.data(), value.data() + value.length(), "culture")) {
				context.compiled_effect.push_back(uint16_t(effect::change_tag_culture | effect::no_payload));
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_effect.push_back(uint16_t(effect::change_tag));
					context.compiled_effect.push_back(trigger::payload(it->second).value);
				} else {
					err.accumulated_errors +=
						"change_tag effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors +=
					"change_tag effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}

		} else {
			err.accumulated_errors +=
				"change_tag effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::change_tag_no_core_switch(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_fixed_token_ci(value.data(), value.data() + value.length(), "culture")) {
				context.compiled_effect.push_back(uint16_t(effect::change_tag_no_core_switch_culture | effect::no_payload));
			} else if(value.length() == 3) {
				if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
					it != context.outer_context.map_of_ident_names.end()) {
					context.compiled_effect.push_back(uint16_t(effect::change_tag_no_core_switch));
					context.compiled_effect.push_back(trigger::payload(it->second).value);
				} else {
					err.accumulated_errors += "change_tag_no_core_switch effect given an invalid tag (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors +=
					"change_tag_no_core_switch effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}

		} else {
			err.accumulated_errors += "change_tag_no_core_switch effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
															std::to_string(line) + ")\n";
			return;
		}
	}

	void effect_body::capital(association_type t, int32_t value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(uint16_t(effect::capital));
		} else {
			err.accumulated_errors +=
				"capital effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		if(0 <= value && size_t(value) < context.outer_context.original_id_to_prov_id_map.size()) {
			context.compiled_effect.push_back(trigger::payload(context.outer_context.original_id_to_prov_id_map[value]).value);
		} else {
			err.accumulated_errors +=
				"capital effect given an invalid province id (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			context.compiled_effect.push_back(trigger::payload(dcon::province_id()).value);
		}
	}
	void effect_body::add_core(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(is_integer(value.data(), value.data() + value.length())) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_effect.push_back(uint16_t(effect::add_core_int));
				auto ivalue = parse_int(value, line, err);
				if(0 <= ivalue && size_t(ivalue) < context.outer_context.original_id_to_prov_id_map.size()) {
					context.compiled_effect.push_back(trigger::payload(context.outer_context.original_id_to_prov_id_map[ivalue]).value);
				} else {
					err.accumulated_errors +=
						"add_core = int effect given an invalid province id (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					context.compiled_effect.push_back(trigger::payload(dcon::province_id()).value);
				}
			} else {
				err.accumulated_errors +=
					"add_core = int effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			if(context.main_slot == trigger::slot_contents::province) {
				if(is_reb(value)) {
					if(context.from_slot == trigger::slot_contents::rebel)
					context.compiled_effect.push_back(uint16_t(effect::add_core_reb | effect::no_payload));
					else {
						err.accumulated_errors += "add_core = reb effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																		std::to_string(line) + ")\n";
						return;
					}
				} else if(is_from(value)) {
					if(context.from_slot == trigger::slot_contents::nation)
					context.compiled_effect.push_back(uint16_t(effect::add_core_from_nation | effect::no_payload));
					else if(context.from_slot == trigger::slot_contents::province)
					context.compiled_effect.push_back(uint16_t(effect::add_core_from_province | effect::no_payload));
					else {
						err.accumulated_errors += "add_core = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																		std::to_string(line) + ")\n";
						return;
					}
				} else if(is_this(value)) {
					if(context.this_slot == trigger::slot_contents::nation)
					context.compiled_effect.push_back(uint16_t(effect::add_core_this_nation | effect::no_payload));
					else if(context.this_slot == trigger::slot_contents::province)
					context.compiled_effect.push_back(uint16_t(effect::add_core_this_province | effect::no_payload));
					else if(context.this_slot == trigger::slot_contents::state)
					context.compiled_effect.push_back(uint16_t(effect::add_core_this_state | effect::no_payload));
					else if(context.this_slot == trigger::slot_contents::pop)
					context.compiled_effect.push_back(uint16_t(effect::add_core_this_pop | effect::no_payload));
					else {
						err.accumulated_errors += "add_core = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																		std::to_string(line) + ")\n";
						return;
					}
				} else if(value.length() == 3) {
					if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
						it != context.outer_context.map_of_ident_names.end()) {
						context.compiled_effect.push_back(uint16_t(effect::add_core_tag));
						context.compiled_effect.push_back(trigger::payload(it->second).value);
					} else {
						err.accumulated_errors +=
							"add_core effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
						return;
					}
				} else {
					err.accumulated_errors +=
						"add_core effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(context.main_slot == trigger::slot_contents::state) {
				if(is_reb(value)) {
					if(context.from_slot == trigger::slot_contents::rebel)
					context.compiled_effect.push_back(uint16_t(effect::add_core_state_reb | effect::no_payload));
					else {
						err.accumulated_errors += "add_core = reb effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
						std::to_string(line) + ")\n";
						return;
					}
				} else if(is_from(value)) {
					if(context.from_slot == trigger::slot_contents::nation)
					context.compiled_effect.push_back(uint16_t(effect::add_core_state_from_nation | effect::no_payload));
					else if(context.from_slot == trigger::slot_contents::province)
					context.compiled_effect.push_back(uint16_t(effect::add_core_state_from_province | effect::no_payload));
					else {
						err.accumulated_errors += "add_core = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
						std::to_string(line) + ")\n";
						return;
					}
				} else if(is_this(value)) {
					if(context.this_slot == trigger::slot_contents::nation)
					context.compiled_effect.push_back(uint16_t(effect::add_core_state_this_nation | effect::no_payload));
					else if(context.this_slot == trigger::slot_contents::province)
					context.compiled_effect.push_back(uint16_t(effect::add_core_state_this_province | effect::no_payload));
					else if(context.this_slot == trigger::slot_contents::state)
					context.compiled_effect.push_back(uint16_t(effect::add_core_state_this_state | effect::no_payload));
					else if(context.this_slot == trigger::slot_contents::pop)
					context.compiled_effect.push_back(uint16_t(effect::add_core_state_this_pop | effect::no_payload));
					else {
						err.accumulated_errors += "add_core = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
						std::to_string(line) + ")\n";
						return;
					}
				} else if(value.length() == 3) {
					if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
						it != context.outer_context.map_of_ident_names.end()) {
						context.compiled_effect.push_back(uint16_t(effect::add_core_tag_state));
						context.compiled_effect.push_back(trigger::payload(it->second).value);
					} else {
						err.accumulated_errors +=
							"add_core effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
						return;
					}
				} else {
					err.accumulated_errors +=
						"add_core effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors +=
					"add_core effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		}
	}
	void effect_body::remove_core(association_type t, std::string_view value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(is_integer(value.data(), value.data() + value.length())) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_effect.push_back(uint16_t(effect::remove_core_int));
				auto ivalue = parse_int(value, line, err);
				if(0 <= ivalue && size_t(ivalue) < context.outer_context.original_id_to_prov_id_map.size()) {
					context.compiled_effect.push_back(trigger::payload(context.outer_context.original_id_to_prov_id_map[ivalue]).value);
				} else {
					err.accumulated_errors += "remove_core = int effect given an invalid province id (" + err.file_name + ", line " +
																	std::to_string(line) + ")\n";
					context.compiled_effect.push_back(trigger::payload(dcon::province_id()).value);
				}
			} else {
				err.accumulated_errors += "remove_core = int effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																std::to_string(line) + ")\n";
				return;
			}
		} else {
			if(context.main_slot == trigger::slot_contents::province) {
				if(is_reb(value)) {
					if(context.from_slot == trigger::slot_contents::rebel)
					context.compiled_effect.push_back(uint16_t(effect::remove_core_reb | effect::no_payload));
					else {
						err.accumulated_errors += "remove_core = reb effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																		std::to_string(line) + ")\n";
						return;
					}
				} else if(is_from(value)) {
					if(context.from_slot == trigger::slot_contents::nation)
					context.compiled_effect.push_back(uint16_t(effect::remove_core_from_nation | effect::no_payload));
					else if(context.from_slot == trigger::slot_contents::province)
					context.compiled_effect.push_back(uint16_t(effect::remove_core_from_province | effect::no_payload));
					else {
						err.accumulated_errors += "remove_core = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																		std::to_string(line) + ")\n";
						return;
					}
				} else if(is_this(value)) {
					if(context.this_slot == trigger::slot_contents::nation)
					context.compiled_effect.push_back(uint16_t(effect::remove_core_this_nation | effect::no_payload));
					else if(context.this_slot == trigger::slot_contents::province)
					context.compiled_effect.push_back(uint16_t(effect::remove_core_this_province | effect::no_payload));
					else if(context.this_slot == trigger::slot_contents::state)
					context.compiled_effect.push_back(uint16_t(effect::remove_core_this_state | effect::no_payload));
					else if(context.this_slot == trigger::slot_contents::pop)
					context.compiled_effect.push_back(uint16_t(effect::remove_core_this_pop | effect::no_payload));
					else {
						err.accumulated_errors += "remove_core = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
																		std::to_string(line) + ")\n";
						return;
					}
				} else if(value.length() == 3) {
					if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
						it != context.outer_context.map_of_ident_names.end()) {
						context.compiled_effect.push_back(uint16_t(effect::remove_core_tag));
						context.compiled_effect.push_back(trigger::payload(it->second).value);
					} else {
						err.accumulated_errors +=
							"remove_core effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
						return;
					}
				} else {
					err.accumulated_errors +=
						"remove_core effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(context.main_slot == trigger::slot_contents::state) {
				if(is_reb(value)) {
					if(context.from_slot == trigger::slot_contents::rebel)
					context.compiled_effect.push_back(uint16_t(effect::remove_core_state_reb | effect::no_payload));
					else {
						err.accumulated_errors += "remove_core = reb effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
						std::to_string(line) + ")\n";
						return;
					}
				} else if(is_from(value)) {
					if(context.from_slot == trigger::slot_contents::nation)
					context.compiled_effect.push_back(uint16_t(effect::remove_core_state_from_nation | effect::no_payload));
					else if(context.from_slot == trigger::slot_contents::province)
					context.compiled_effect.push_back(uint16_t(effect::remove_core_state_from_province | effect::no_payload));
					else {
						err.accumulated_errors += "remove_core = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
						std::to_string(line) + ")\n";
						return;
					}
				} else if(is_this(value)) {
					if(context.this_slot == trigger::slot_contents::nation)
					context.compiled_effect.push_back(uint16_t(effect::remove_core_state_this_nation | effect::no_payload));
					else if(context.this_slot == trigger::slot_contents::province)
					context.compiled_effect.push_back(uint16_t(effect::remove_core_state_this_province | effect::no_payload));
					else if(context.this_slot == trigger::slot_contents::state)
					context.compiled_effect.push_back(uint16_t(effect::remove_core_state_this_state | effect::no_payload));
					else if(context.this_slot == trigger::slot_contents::pop)
					context.compiled_effect.push_back(uint16_t(effect::remove_core_state_this_pop | effect::no_payload));
					else {
						err.accumulated_errors += "remove_core = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
						std::to_string(line) + ")\n";
						return;
					}
				} else if(value.length() == 3) {
					if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
						it != context.outer_context.map_of_ident_names.end()) {
						context.compiled_effect.push_back(uint16_t(effect::remove_core_tag_state));
						context.compiled_effect.push_back(trigger::payload(it->second).value);
					} else {
						err.accumulated_errors +=
							"remove_core effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
						return;
					}
				} else {
					err.accumulated_errors +=
						"remove_core effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else if(context.main_slot == trigger::slot_contents::nation) {
				if(is_reb(value)) {
					if(context.from_slot == trigger::slot_contents::rebel)
					context.compiled_effect.push_back(uint16_t(effect::remove_core_nation_reb | effect::no_payload));
					else {
						err.accumulated_errors += "remove_core = reb effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
						std::to_string(line) + ")\n";
						return;
					}
				} else if(is_from(value)) {
					if(context.from_slot == trigger::slot_contents::nation)
					context.compiled_effect.push_back(uint16_t(effect::remove_core_nation_from_nation | effect::no_payload));
					else if(context.from_slot == trigger::slot_contents::province)
					context.compiled_effect.push_back(uint16_t(effect::remove_core_nation_from_province | effect::no_payload));
					else {
						err.accumulated_errors += "remove_core = from effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
						std::to_string(line) + ")\n";
						return;
					}
				} else if(is_this(value)) {
					if(context.this_slot == trigger::slot_contents::nation)
					context.compiled_effect.push_back(uint16_t(effect::remove_core_nation_this_nation | effect::no_payload));
					else if(context.this_slot == trigger::slot_contents::province)
					context.compiled_effect.push_back(uint16_t(effect::remove_core_nation_this_province | effect::no_payload));
					else if(context.this_slot == trigger::slot_contents::state)
					context.compiled_effect.push_back(uint16_t(effect::remove_core_nation_this_state | effect::no_payload));
					else if(context.this_slot == trigger::slot_contents::pop)
					context.compiled_effect.push_back(uint16_t(effect::remove_core_nation_this_pop | effect::no_payload));
					else {
						err.accumulated_errors += "remove_core = this effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " +
						std::to_string(line) + ")\n";
						return;
					}
				} else if(value.length() == 3) {
					if(auto it = context.outer_context.map_of_ident_names.find(nations::tag_to_int(value[0], value[1], value[2]));
						it != context.outer_context.map_of_ident_names.end()) {
						context.compiled_effect.push_back(uint16_t(effect::remove_core_tag_nation));
						context.compiled_effect.push_back(trigger::payload(it->second).value);
					} else {
						err.accumulated_errors +=
						"remove_core effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
						return;
					}
				} else {
					err.accumulated_errors +=
					"remove_core effect given an invalid tag (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors +=
					"remove_core effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		}
	}

	void effect_body::sub_unit(ef_sub_unit const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(is_this(value.value)) {
			if(context.main_slot != trigger::slot_contents::nation || context.this_slot != trigger::slot_contents::province) {
				err.accumulated_errors += "sub_unit effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_effect.push_back(effect::sub_unit_this);
			context.compiled_effect.push_back(trigger::payload(value.type_).value);
		} else if(is_from(value.value)) {
			if(context.main_slot != trigger::slot_contents::nation || context.from_slot != trigger::slot_contents::province) {
				err.accumulated_errors += "sub_unit effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_effect.push_back(effect::sub_unit_from);
			context.compiled_effect.push_back(trigger::payload(value.type_).value);
		} else if(is_fixed_token_ci(value.value.data(), value.value.data() + value.value.length(), "current")) {
			if(context.main_slot != trigger::slot_contents::province) {
				err.accumulated_errors += "sub_unit effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_effect.push_back(effect::sub_unit_current);
			context.compiled_effect.push_back(trigger::payload(value.type_).value);
		} else {
			auto ivalue = parse_int(value.value, line, err);
			if(0 <= ivalue && size_t(ivalue) < context.outer_context.original_id_to_prov_id_map.size()) {
				auto prov_id = context.outer_context.original_id_to_prov_id_map[ivalue];
				context.compiled_effect.push_back(effect::sub_unit_int);
				context.compiled_effect.push_back(trigger::payload(value.type_).value);
				context.compiled_effect.push_back(trigger::payload(prov_id).value);
			} else {
				err.accumulated_errors += "sub_unit effect given an invalid province id (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		}
	}
	void effect_body::set_variable(ef_set_variable const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "set_variable effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_effect.push_back(effect::set_variable);
		context.compiled_effect.push_back(trigger::payload(value.which_).value);
		context.add_float_to_payload(value.value);
	}
	void effect_body::change_variable(ef_change_variable const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "change_variable effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_effect.push_back(effect::change_variable);
		context.compiled_effect.push_back(trigger::payload(value.which_).value);
		context.add_float_to_payload(value.value);
	}
	void effect_body::increment_variable(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension increment_variable but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "change_variable effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		auto which_ = context.outer_context.get_national_variable(std::string(value));
		context.compiled_effect.push_back(effect::change_variable);
		context.compiled_effect.push_back(trigger::payload(which_).value);
		context.add_float_to_payload(1.f);
	}
	void effect_body::decrement_variable(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension decrement_variable but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "decrement_variable effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		auto which_ = context.outer_context.get_national_variable(std::string(value));
		context.compiled_effect.push_back(effect::change_variable);
		context.compiled_effect.push_back(trigger::payload(which_).value);
		context.add_float_to_payload(-1.f);
	}
	void effect_body::set_variable_to_zero(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension set_variable_to_zero but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "set_variable_to_zero effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		auto which_ = context.outer_context.get_national_variable(std::string(value));
		context.compiled_effect.push_back(effect::set_variable);
		context.compiled_effect.push_back(trigger::payload(which_).value);
		context.add_float_to_payload(0.f);
	}
	void effect_body::change_terrain(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension change_terrain but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}
		if(auto it = context.outer_context.map_of_terrain_types.find(std::string(value)); it != context.outer_context.map_of_terrain_types.end()) {
			if(context.main_slot == trigger::slot_contents::pop) {
				context.compiled_effect.push_back(uint16_t(effect::change_terrain_pop));
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_effect.push_back(uint16_t(effect::change_terrain_province));
			} else {
				err.accumulated_errors += "chenge_terrain effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
			context.compiled_effect.push_back(trigger::payload(it->second.id).value);
		} else {
			err.accumulated_errors += "chenge_terrain effect supplied with an invalid terrain \"" + std::string(value) + "\" (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void effect_body::masquerade_as_nation(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension masquerade_as_nation but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}
		if(context.main_slot == trigger::slot_contents::nation) {
			if(is_this(value)) {
				context.compiled_effect.push_back(uint16_t(effect::masquerade_as_nation_this));
			} else if(is_from(value)) {
				context.compiled_effect.push_back(uint16_t(effect::masquerade_as_nation_from));
			} else {
				err.accumulated_errors += "masquerade_as_nation effect given an invalid parameter " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors += "masquerade_as_nation effect not used in a nation scope (" + err.file_name + ", line " + std::to_string(line) + ")\n";
		}
	}
	void effect_body::change_party_name(ef_change_party_name const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension change_party_name but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "change_party_name effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_effect.push_back(effect::change_party_name);
		context.compiled_effect.push_back(trigger::payload(value.ideology_).value);
		context.add_int32_t_to_payload(value.name_.index());
	}
	void effect_body::change_party_position(ef_change_party_position const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension change_party_position but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "change_party_position effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		} else if(!value.opt_) {
			err.accumulated_errors += "change_party_position effect used without a valid position " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_effect.push_back(effect::change_party_position);
		context.compiled_effect.push_back(trigger::payload(value.ideology_).value);
		context.compiled_effect.push_back(trigger::payload(value.opt_).value);
	}
	void effect_body::ideology(ef_ideology const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot != trigger::slot_contents::pop) {
			err.accumulated_errors += "ideology effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_effect.push_back(effect::ideology);
		context.compiled_effect.push_back(trigger::payload(value.value_).value);
		context.add_float_to_payload(value.factor);
	}
	void effect_body::dominant_issue(ef_dominant_issue const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::pop) {
			context.compiled_effect.push_back(effect::dominant_issue);
			context.compiled_effect.push_back(trigger::payload(value.value_).value);
			context.add_float_to_payload(value.factor);
		} else if(context.main_slot == trigger::slot_contents::nation) {
			context.compiled_effect.push_back(effect::dominant_issue_nation);
			context.compiled_effect.push_back(trigger::payload(value.value_).value);
			context.add_float_to_payload(value.factor);
		} else {
			err.accumulated_errors += "dominant_issue effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::upper_house(ef_upper_house const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "upper_house effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_effect.push_back(effect::upper_house);
		context.compiled_effect.push_back(trigger::payload(value.ideology_).value);
		context.add_float_to_payload(value.value);
	}
	void effect_body::scaled_militancy(ef_scaled_militancy const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::pop) {
			if(bool(value.ideology_)) {
				context.compiled_effect.push_back(effect::scaled_militancy_ideology);
				context.compiled_effect.push_back(trigger::payload(value.ideology_).value);
				context.add_float_to_payload(value.factor);
			} else if(bool(value.issue_)) {
				context.compiled_effect.push_back(effect::scaled_militancy_issue);
				context.compiled_effect.push_back(trigger::payload(value.issue_).value);
				context.add_float_to_payload(value.factor);
			} else {
				context.compiled_effect.push_back(effect::scaled_militancy_unemployment);
				context.add_float_to_payload(value.factor * value.unemployment);
			}
		} else if(context.main_slot == trigger::slot_contents::nation) {
			if(bool(value.ideology_)) {
				context.compiled_effect.push_back(effect::scaled_militancy_nation_ideology);
				context.compiled_effect.push_back(trigger::payload(value.ideology_).value);
				context.add_float_to_payload(value.factor);
			} else if(bool(value.issue_)) {
				context.compiled_effect.push_back(effect::scaled_militancy_nation_issue);
				context.compiled_effect.push_back(trigger::payload(value.issue_).value);
				context.add_float_to_payload(value.factor);
			} else {
				context.compiled_effect.push_back(effect::scaled_militancy_nation_unemployment);
				context.add_float_to_payload(value.factor * value.unemployment);
			}
		} else if(context.main_slot == trigger::slot_contents::state) {
			if(bool(value.ideology_)) {
				context.compiled_effect.push_back(effect::scaled_militancy_state_ideology);
				context.compiled_effect.push_back(trigger::payload(value.ideology_).value);
				context.add_float_to_payload(value.factor);
			} else if(bool(value.issue_)) {
				context.compiled_effect.push_back(effect::scaled_militancy_state_issue);
				context.compiled_effect.push_back(trigger::payload(value.issue_).value);
				context.add_float_to_payload(value.factor);
			} else {
				context.compiled_effect.push_back(effect::scaled_militancy_state_unemployment);
				context.add_float_to_payload(value.factor * value.unemployment);
			}
		} else if(context.main_slot == trigger::slot_contents::province) {
			if(bool(value.ideology_)) {
				context.compiled_effect.push_back(effect::scaled_militancy_province_ideology);
				context.compiled_effect.push_back(trigger::payload(value.ideology_).value);
				context.add_float_to_payload(value.factor);
			} else if(bool(value.issue_)) {
				context.compiled_effect.push_back(effect::scaled_militancy_province_issue);
				context.compiled_effect.push_back(trigger::payload(value.issue_).value);
				context.add_float_to_payload(value.factor);
			} else {
				context.compiled_effect.push_back(effect::scaled_militancy_province_unemployment);
				context.add_float_to_payload(value.factor * value.unemployment);
			}
		} else {
			err.accumulated_errors +=
				"scaled_militancy effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}

	void effect_body::scaled_consciousness(ef_scaled_consciousness const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot == trigger::slot_contents::pop) {
			if(bool(value.ideology_)) {
				context.compiled_effect.push_back(effect::scaled_consciousness_ideology);
				context.compiled_effect.push_back(trigger::payload(value.ideology_).value);
				context.add_float_to_payload(value.factor);
			} else if(bool(value.issue_)) {
				context.compiled_effect.push_back(effect::scaled_consciousness_issue);
				context.compiled_effect.push_back(trigger::payload(value.issue_).value);
				context.add_float_to_payload(value.factor);
			} else {
				context.compiled_effect.push_back(effect::scaled_consciousness_unemployment);
				context.add_float_to_payload(value.factor * value.unemployment);
			}
		} else if(context.main_slot == trigger::slot_contents::nation) {
			if(bool(value.ideology_)) {
				context.compiled_effect.push_back(effect::scaled_consciousness_nation_ideology);
				context.compiled_effect.push_back(trigger::payload(value.ideology_).value);
				context.add_float_to_payload(value.factor);
			} else if(bool(value.issue_)) {
				context.compiled_effect.push_back(effect::scaled_consciousness_nation_issue);
				context.compiled_effect.push_back(trigger::payload(value.issue_).value);
				context.add_float_to_payload(value.factor);
			} else {
				context.compiled_effect.push_back(effect::scaled_consciousness_nation_unemployment);
				context.add_float_to_payload(value.factor * value.unemployment);
			}
		} else if(context.main_slot == trigger::slot_contents::state) {
			if(bool(value.ideology_)) {
				context.compiled_effect.push_back(effect::scaled_consciousness_state_ideology);
				context.compiled_effect.push_back(trigger::payload(value.ideology_).value);
				context.add_float_to_payload(value.factor);
			} else if(bool(value.issue_)) {
				context.compiled_effect.push_back(effect::scaled_consciousness_state_issue);
				context.compiled_effect.push_back(trigger::payload(value.issue_).value);
				context.add_float_to_payload(value.factor);
			} else {
				context.compiled_effect.push_back(effect::scaled_consciousness_state_unemployment);
				context.add_float_to_payload(value.factor * value.unemployment);
			}
		} else if(context.main_slot == trigger::slot_contents::province) {
			if(bool(value.ideology_)) {
				context.compiled_effect.push_back(effect::scaled_consciousness_province_ideology);
				context.compiled_effect.push_back(trigger::payload(value.ideology_).value);
				context.add_float_to_payload(value.factor);
			} else if(bool(value.issue_)) {
				context.compiled_effect.push_back(effect::scaled_consciousness_province_issue);
				context.compiled_effect.push_back(trigger::payload(value.issue_).value);
				context.add_float_to_payload(value.factor);
			} else {
				context.compiled_effect.push_back(effect::scaled_consciousness_province_unemployment);
				context.add_float_to_payload(value.factor * value.unemployment);
			}
		} else {
			err.accumulated_errors += "scaled_consciousness effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}

	void effect_body::add_war_goal(ef_add_war_goal const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation || context.from_slot != trigger::slot_contents::nation) {
			err.accumulated_errors +=
				"add_war_goal effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_effect.push_back(effect::add_war_goal);
		context.compiled_effect.push_back(trigger::payload(value.casus_belli_).value);
	}

	void effect_body::move_issue_percentage(ef_move_issue_percentage const& value, error_handler& err, int32_t line, effect_building_context& context) {
		float norm_value = value.value;
		if(norm_value < 0.f || norm_value > 1.f) {
			err.accumulated_errors += "move_issue_percentage with value out of bounds " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			norm_value = std::clamp(norm_value, 0.f, 1.f);
		}
		if(context.main_slot == trigger::slot_contents::nation)
			context.compiled_effect.push_back(effect::move_issue_percentage_nation);
		else if(context.main_slot == trigger::slot_contents::state)
			context.compiled_effect.push_back(effect::move_issue_percentage_state);
		else if(context.main_slot == trigger::slot_contents::province)
			context.compiled_effect.push_back(effect::move_issue_percentage_province);
		else if(context.main_slot == trigger::slot_contents::pop)
			context.compiled_effect.push_back(effect::move_issue_percentage_pop);
		else {
			err.accumulated_errors += "move_issue_percentage effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		context.compiled_effect.push_back(trigger::payload(value.from_).value);
		context.compiled_effect.push_back(trigger::payload(value.to_).value);
		context.add_float_to_payload(norm_value);
	}

	void effect_body::party_loyalty(ef_party_loyalty const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(bool(value.province_id_)) {
			context.compiled_effect.push_back(effect::party_loyalty);
			context.compiled_effect.push_back(trigger::payload(value.province_id_).value);
			context.compiled_effect.push_back(trigger::payload(value.ideology_).value);
			context.compiled_effect.push_back(trigger::payload(int16_t(value.loyalty_value)).value);
		} else if(context.main_slot == trigger::slot_contents::province) {
			context.compiled_effect.push_back(effect::party_loyalty_province);
			context.compiled_effect.push_back(trigger::payload(value.ideology_).value);
			context.compiled_effect.push_back(trigger::payload(int16_t(value.loyalty_value)).value);
		} else {
			err.accumulated_errors += "party_loyalty effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}
	void effect_body::build_railway_in_capital(ef_build_railway_in_capital const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "build_railway_in_capital effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		if(value.in_whole_capital_state && value.limit_to_world_greatest_level)
			context.compiled_effect.push_back(uint16_t(effect::build_railway_in_capital_yes_whole_state_yes_limit | effect::no_payload));
		else if(value.in_whole_capital_state && !value.limit_to_world_greatest_level)
			context.compiled_effect.push_back(uint16_t(effect::build_railway_in_capital_yes_whole_state_no_limit | effect::no_payload));
		else if(!value.in_whole_capital_state && value.limit_to_world_greatest_level)
			context.compiled_effect.push_back(uint16_t(effect::build_railway_in_capital_no_whole_state_yes_limit | effect::no_payload));
		else
			context.compiled_effect.push_back(uint16_t(effect::build_railway_in_capital_no_whole_state_no_limit | effect::no_payload));
	}
	void effect_body::build_bank_in_capital(ef_build_bank_in_capital const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "build_bank_in_capital effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		if(value.in_whole_capital_state && value.limit_to_world_greatest_level)
			context.compiled_effect.push_back(uint16_t(effect::build_bank_in_capital_yes_whole_state_yes_limit | effect::no_payload));
		else if(value.in_whole_capital_state && !value.limit_to_world_greatest_level)
			context.compiled_effect.push_back(uint16_t(effect::build_bank_in_capital_yes_whole_state_no_limit | effect::no_payload));
		else if(!value.in_whole_capital_state && value.limit_to_world_greatest_level)
			context.compiled_effect.push_back(uint16_t(effect::build_bank_in_capital_no_whole_state_yes_limit | effect::no_payload));
		else
			context.compiled_effect.push_back(uint16_t(effect::build_bank_in_capital_no_whole_state_no_limit | effect::no_payload));
	}

	void effect_body::build_university_in_capital(ef_build_university_in_capital const& value, error_handler& err, int32_t line, effect_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "build_university_in_capital effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		if(value.in_whole_capital_state && value.limit_to_world_greatest_level)
			context.compiled_effect.push_back(uint16_t(effect::build_university_in_capital_yes_whole_state_yes_limit | effect::no_payload));
		else if(value.in_whole_capital_state && !value.limit_to_world_greatest_level)
			context.compiled_effect.push_back(uint16_t(effect::build_university_in_capital_yes_whole_state_no_limit | effect::no_payload));
		else if(!value.in_whole_capital_state && value.limit_to_world_greatest_level)
			context.compiled_effect.push_back(uint16_t(effect::build_university_in_capital_no_whole_state_yes_limit | effect::no_payload));
		else
			context.compiled_effect.push_back(uint16_t(effect::build_university_in_capital_no_whole_state_no_limit | effect::no_payload));
	}

	void effect_body::build_fort_in_capital(ef_build_fort_in_capital const& value, error_handler& err, int32_t line,
		effect_building_context& context) {
		if(context.main_slot != trigger::slot_contents::nation) {
			err.accumulated_errors += "build_fort_in_capital effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
		if(value.in_whole_capital_state && value.limit_to_world_greatest_level)
			context.compiled_effect.push_back(uint16_t(effect::build_fort_in_capital_yes_whole_state_yes_limit | effect::no_payload));
		else if(value.in_whole_capital_state && !value.limit_to_world_greatest_level)
			context.compiled_effect.push_back(uint16_t(effect::build_fort_in_capital_yes_whole_state_no_limit | effect::no_payload));
		else if(!value.in_whole_capital_state && value.limit_to_world_greatest_level)
			context.compiled_effect.push_back(uint16_t(effect::build_fort_in_capital_no_whole_state_yes_limit | effect::no_payload));
		else
			context.compiled_effect.push_back(uint16_t(effect::build_fort_in_capital_no_whole_state_no_limit | effect::no_payload));
	}

	void effect_body::any_value(std::string_view label, association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		std::string str_label{ label };
		if(auto it = context.outer_context.map_of_commodity_names.find(str_label);
			it != context.outer_context.map_of_commodity_names.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				context.compiled_effect.push_back(effect::variable_good_name);
				context.compiled_effect.push_back(trigger::payload(it->second).value);
				context.add_float_to_payload(parse_float(value, line, err));
			} else if(context.main_slot == trigger::slot_contents::province) {
				context.compiled_effect.push_back(effect::variable_good_name_province);
				context.compiled_effect.push_back(trigger::payload(it->second).value);
				context.add_float_to_payload(parse_float(value, line, err));
			} else {
				err.accumulated_errors += "variable commodity name effect used in an incorrect scope type (" + err.file_name + ", line " +
				std::to_string(line) + ")\n";
				return;
			}
		} else if(auto itb = context.outer_context.map_of_technologies.find(str_label); itb != context.outer_context.map_of_technologies.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				if(parse_bool(value, line, err))
					context.compiled_effect.push_back(effect::variable_tech_name_yes);
				else
					context.compiled_effect.push_back(effect::variable_tech_name_no);
				context.compiled_effect.push_back(trigger::payload(itb->second.id).value);
			} else {
				err.accumulated_errors += "variable technology name effect used in an incorrect scope type (" + err.file_name +
				", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(auto itc = context.outer_context.map_of_inventions.find(str_label); itc != context.outer_context.map_of_inventions.end()) {
			if(context.main_slot == trigger::slot_contents::nation) {
				if(parse_bool(value, line, err))
					context.compiled_effect.push_back(effect::variable_invention_name_yes);
				else
					context.compiled_effect.push_back(effect::variable_invention_name_no);
				context.compiled_effect.push_back(trigger::payload(itc->second.id).value);
			} else {
				err.accumulated_errors += "variable invention name effect used in an incorrect scope type (" + err.file_name + ", line " +
				std::to_string(line) + ")\n";
				return;
			}
		} else if(auto itf = context.outer_context.map_of_iissues.find(str_label); itf != context.outer_context.map_of_iissues.end()) {
			if(auto itopt = context.outer_context.map_of_ioptions.find(std::string(value)); itopt != context.outer_context.map_of_ioptions.end()) {
				if(context.main_slot == trigger::slot_contents::nation) {
					auto cat = context.outer_context.state.world.issue_get_issue_type(itf->second);
					if(cat == uint8_t(::culture::issue_category::political)) {
						context.compiled_effect.push_back(uint16_t(effect::political_reform));
						context.compiled_effect.push_back(trigger::payload(itopt->second.id).value);
					} else if(cat == uint8_t(::culture::issue_category::social)) {
						context.compiled_effect.push_back(uint16_t(effect::social_reform));
						context.compiled_effect.push_back(trigger::payload(itopt->second.id).value);
					} else {
						err.accumulated_errors += "named issue effect used with a party issue (" + err.file_name + ", line " + std::to_string(line) + ")\n";
						return;
					}
				} else {
					err.accumulated_errors += "named issue effect used in an invalid context (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors += "named issue effect used with an invalid option name (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else if(auto ith = context.outer_context.map_of_reforms.find(str_label); ith != context.outer_context.map_of_reforms.end()) {
			if(auto itopt = context.outer_context.map_of_roptions.find(std::string(value));
				itopt != context.outer_context.map_of_roptions.end()) {
				if(context.main_slot == trigger::slot_contents::nation) {
					auto cat = context.outer_context.state.world.reform_get_reform_type(ith->second);
					if(cat == uint8_t(::culture::issue_category::military)) {
						context.compiled_effect.push_back(uint16_t(effect::military_reform));
						context.compiled_effect.push_back(trigger::payload(itopt->second.id).value);
					} else if(cat == uint8_t(::culture::issue_category::economic)) {
						context.compiled_effect.push_back(uint16_t(effect::economic_reform));
						context.compiled_effect.push_back(trigger::payload(itopt->second.id).value);
					} else {
						err.accumulated_errors += "named reform effect used with an invalid issue type (" + err.file_name + ", line " + std::to_string(line) + ")\n";
						return;
					}
				} else {
					err.accumulated_errors += "named reform effect used in an invalid context (" + err.file_name + ", line " + std::to_string(line) + ")\n";
					return;
				}
			} else {
				err.accumulated_errors += "named reform effect used with an invalid option name (" + err.file_name + ", line " + std::to_string(line) + ")\n";
				return;
			}
		} else {
			err.accumulated_errors +=
			"unknown effect " + str_label + " encountered (" + err.file_name + ", line " + std::to_string(line) + ")\n";
			return;
		}
	}

	void ef_change_party_name::ideology(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
		if(is_fixed_token_ci(v.data(), v.data() + v.length(), "ruling_party")) {
			// leave invalid
		} else if(auto it = context.outer_context.map_of_ideologies.find(std::string(v)); it != context.outer_context.map_of_ideologies.end()) {
			ideology_ = it->second.id;
		} else {
			err.accumulated_errors += "Invalid ideology " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	}
	void ef_change_party_position::ideology(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
		if(is_fixed_token_ci(v.data(), v.data() + v.length(), "ruling_party")) {
			// leave invalid
		} else if(auto it = context.outer_context.map_of_ideologies.find(std::string(v)); it != context.outer_context.map_of_ideologies.end()) {
			ideology_ = it->second.id;
		} else {
			err.accumulated_errors += "Invalid ideology " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	}
	void ef_change_party_position::position(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
		if(auto it = context.outer_context.map_of_ioptions.find(std::string(v)); it != context.outer_context.map_of_ioptions.end()) {
			opt_ = it->second.id;
		} else {
			err.accumulated_errors += "Invalid issue option " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	}
	void ef_trigger_crisis::type(association_type t, std::string_view v, error_handler& err, int32_t line, effect_building_context& context) {
		if(parsers::is_fixed_token(v.data(), v.data() + v.length(), "claim")) {
			type_ = sys::crisis_type::claim;
		} else if(parsers::is_fixed_token(v.data(), v.data() + v.length(), "colonial")) {
			type_ = sys::crisis_type::colonial;
		} else if(parsers::is_fixed_token(v.data(), v.data() + v.length(), "influence")) {
			type_ = sys::crisis_type::influence;
		} else if(parsers::is_fixed_token(v.data(), v.data() + v.length(), "liberation")) {
			type_ = sys::crisis_type::liberation;
		} else {
			err.accumulated_errors += "Invalid crisis type " + std::string(v) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	}

	void ef_trigger_revolt::culture(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(auto it = context.outer_context.map_of_culture_names.find(std::string(value));
			it != context.outer_context.map_of_culture_names.end()) {
			culture_ = it->second;
		} else {
			err.accumulated_errors += "Invalid culture " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	}
	void ef_trigger_revolt::religion(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(auto it = context.outer_context.map_of_religion_names.find(std::string(value));
			it != context.outer_context.map_of_religion_names.end()) {
			religion_ = it->second;
		} else {
			err.accumulated_errors += "Invalid religion " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	}
	void ef_trigger_revolt::ideology(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(auto it = context.outer_context.map_of_ideologies.find(std::string(value));
			it != context.outer_context.map_of_ideologies.end()) {
			ideology_ = it->second.id;
		} else {
			err.accumulated_errors += "Invalid ideology " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	}
	void ef_trigger_revolt::type(association_type t, std::string_view value, error_handler& err, int32_t line, effect_building_context& context) {
		if(auto it = context.outer_context.map_of_rebeltypes.find(std::string(value));
			it != context.outer_context.map_of_rebeltypes.end()) {
			type_ = it->second.id;
		} else {
			err.accumulated_errors += "Invalid rebel type " + std::string(value) + " (" + err.file_name + " line " + std::to_string(line) + ")\n";
		}
	}

	void scan_province_event(token_generator& gen, error_handler& err, scenario_building_context& context) {
		token_generator scan_copy = gen;
		auto scan_result = parse_scan_event(scan_copy, err, context);
		if(scan_result.is_triggered_only) {
			if(auto it = context.map_of_provincial_events.find(scan_result.id); it != context.map_of_provincial_events.end()) {
				if(it->second.text_assigned) {
					err.accumulated_errors += "More than one event given id " + std::to_string(scan_result.id) + " (" + err.file_name + ")\n";
				} else {
					it->second.generator_state = gen;
					it->second.text_assigned = true;
					it->second.original_file = err.file_name;
				}
			} else {
				context.map_of_provincial_events.insert_or_assign(scan_result.id,
					pending_prov_event{ err.file_name, dcon::provincial_event_id(), trigger::slot_contents::empty, trigger::slot_contents::empty,
							trigger::slot_contents::empty, gen, false});
			}
			gen = scan_copy;
		} else {
			if(auto it = context.map_of_provincial_events.find(scan_result.id); it != context.map_of_provincial_events.end()) {
				if(it->second.text_assigned) {
					err.accumulated_errors += "More than one event given id " + std::to_string(scan_result.id) + " (" + err.file_name + ")\n";
				} else {
					it->second.generator_state = gen;
					it->second.text_assigned = true;
					it->second.original_file = err.file_name;
				}
			} else {
				context.map_of_provincial_events.insert_or_assign(scan_result.id,
					pending_prov_event{ err.file_name, dcon::provincial_event_id(), trigger::slot_contents::empty, trigger::slot_contents::empty,
							trigger::slot_contents::empty, gen, true });
			}

			event_building_context e_context{context, trigger::slot_contents::province, trigger::slot_contents::province,
				trigger::slot_contents::empty};
			auto event_result = parse_generic_event(gen, err, e_context);
			auto new_id = context.state.world.create_free_provincial_event();
			auto fid = fatten(context.state.world, new_id);
			fid.set_description(event_result.desc_);
			fid.set_immediate_effect(event_result.immediate_);
			fid.set_news_title(event_result.news_title_);
			fid.set_news_picture(event_result.news_picture_);
			fid.set_news_long_desc(event_result.news_long_desc_);
			fid.set_news_medium_desc(event_result.news_medium_desc_);
			fid.set_news_short_desc(event_result.news_short_desc_);
			fid.set_name(event_result.title_);
			fid.set_mtth(event_result.mean_time_to_happen);
			fid.set_only_once(event_result.fire_only_once);
			fid.set_trigger(event_result.trigger);
			fid.get_options() = event_result.options;
			fid.set_legacy_id(uint32_t(event_result.id));
			fid.set_window_type(context.state.add_key_win1252(event_result.window_type));
		}
	}
	void scan_country_event(token_generator& gen, error_handler& err, scenario_building_context& context) {
		token_generator scan_copy = gen;
		auto scan_result = parse_scan_event(scan_copy, err, context);
		if(scan_result.is_triggered_only) {
			if(auto it = context.map_of_national_events.find(scan_result.id); it != context.map_of_national_events.end()) {
				if(it->second.text_assigned) {
					err.accumulated_errors += "More than one event given id " + std::to_string(scan_result.id) + " (" + err.file_name + ")\n";
				} else {
					it->second.generator_state = gen;
					it->second.text_assigned = true;
					it->second.original_file = err.file_name;
				}
			} else {
				context.map_of_national_events.insert_or_assign(scan_result.id,
					pending_nat_event{ err.file_name, dcon::national_event_id(), trigger::slot_contents::empty, trigger::slot_contents::empty,
							trigger::slot_contents::empty, gen, false});
			}
			gen = scan_copy;
		} else {
			if(auto it = context.map_of_national_events.find(scan_result.id); it != context.map_of_national_events.end()) {
				if(it->second.text_assigned) {
					err.accumulated_errors += "More than one event given id " + std::to_string(scan_result.id) + " (" + err.file_name + ")\n";
				} else {
					it->second.generator_state = gen;
					it->second.text_assigned = true;
					it->second.original_file = err.file_name;
				}
			} else {
				context.map_of_national_events.insert_or_assign(scan_result.id,
					pending_nat_event{ err.file_name, dcon::national_event_id(), trigger::slot_contents::empty, trigger::slot_contents::empty,
							trigger::slot_contents::empty, gen, true });
			}

			event_building_context e_context{context, trigger::slot_contents::nation, trigger::slot_contents::nation,
				trigger::slot_contents::empty};
			auto event_result = parse_generic_event(gen, err, e_context);
			auto new_id = context.state.world.create_free_national_event();
			auto fid = fatten(context.state.world, new_id);
			fid.set_description(event_result.desc_);
			fid.set_name(event_result.title_);
			fid.set_image(event_result.picture_);
			fid.set_immediate_effect(event_result.immediate_);
			fid.set_news_title(event_result.news_title_);
			fid.set_news_picture(event_result.news_picture_);
			fid.set_news_long_desc(event_result.news_long_desc_);
			fid.set_news_medium_desc(event_result.news_medium_desc_);
			fid.set_news_short_desc(event_result.news_short_desc_);
			fid.set_is_major(event_result.major);
			fid.set_mtth(event_result.mean_time_to_happen);
			fid.set_only_once(event_result.fire_only_once);
			fid.set_trigger(event_result.trigger);
			fid.get_options() = event_result.options;
			fid.set_legacy_id(uint32_t(event_result.id));
			fid.set_window_type(context.state.add_key_win1252(event_result.window_type));
		}
	}

	void lambda_country_event(token_generator& gen, error_handler& err, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension lambda_country_event but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}
		event_building_context e_context{ context.outer_context, trigger::slot_contents::nation, trigger::slot_contents::nation, context.this_slot };
		auto event_result = parse_generic_event(gen, err, e_context);
		auto id = context.outer_context.state.world.create_national_event();
		auto fid = dcon::fatten(context.outer_context.state.world, id);
		fid.set_description(event_result.desc_);
		fid.set_name(event_result.title_);
		fid.set_image(event_result.picture_);
		fid.set_allow_multiple_instances(event_result.allow_multiple_instances);
		fid.set_immediate_effect(event_result.immediate_);
		fid.set_news_title(event_result.news_title_);
		fid.set_news_picture(event_result.news_picture_);
		fid.set_news_long_desc(event_result.news_long_desc_);
		fid.set_news_medium_desc(event_result.news_medium_desc_);
		fid.set_news_short_desc(event_result.news_short_desc_);
		fid.set_is_major(event_result.major);
		fid.get_options() = event_result.options;
		fid.set_window_type(context.outer_context.state.add_key_win1252(event_result.window_type));
		//Effect
		ef_country_event value;
		value.days = 0;
		value.id_ = id;
		if(context.main_slot == trigger::slot_contents::nation) {
			if(value.days <= 0) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::country_event_immediate_this_nation));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::country_event_immediate_this_province));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(uint16_t(effect::country_event_immediate_this_state));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_effect.push_back(uint16_t(effect::country_event_immediate_this_pop));
				else {
					err.accumulated_errors +=
					"lambda_country_event effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
					return;
				}
				context.compiled_effect.push_back(trigger::payload(value.id_).value);
			} else {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::country_event_this_nation));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::country_event_this_province));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(uint16_t(effect::country_event_this_state));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_effect.push_back(uint16_t(effect::country_event_this_pop));
				else {
					err.accumulated_errors +=
					"lambda_country_event effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
					return;
				}
				context.compiled_effect.push_back(trigger::payload(value.id_).value);
				context.compiled_effect.push_back(uint16_t(value.days));
			}
		} else if(context.main_slot == trigger::slot_contents::province) {
			if(value.days <= 0) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::country_event_immediate_province_this_nation));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::country_event_immediate_province_this_province));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(uint16_t(effect::country_event_immediate_province_this_state));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_effect.push_back(uint16_t(effect::country_event_immediate_province_this_pop));
				else {
					err.accumulated_errors +=
					"lambda_country_event effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
					return;
				}
				context.compiled_effect.push_back(trigger::payload(value.id_).value);
			} else {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::country_event_province_this_nation));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::country_event_province_this_province));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(uint16_t(effect::country_event_province_this_state));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_effect.push_back(uint16_t(effect::country_event_province_this_pop));
				else {
					err.accumulated_errors +=
					"lambda_country_event effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
					return;
				}
				context.compiled_effect.push_back(trigger::payload(value.id_).value);
				context.compiled_effect.push_back(uint16_t(value.days));
			}
		} else {
			err.accumulated_errors +=
			"lambda_country_event effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
	}

	void lambda_province_event(token_generator& gen, error_handler& err, effect_building_context& context) {
		if(!context.outer_context.use_extensions) {
			err.accumulated_errors += "Usage of effect extension lambda_province_event but parser isn't in extension mode (" + err.file_name + ")\n";
			return;
		}
		event_building_context e_context{ context.outer_context, trigger::slot_contents::province, trigger::slot_contents::province, context.this_slot };
		auto event_result = parse_generic_event(gen, err, e_context);
		auto id = context.outer_context.state.world.create_provincial_event();
		auto fid = dcon::fatten(context.outer_context.state.world, id);
		fid.set_description(event_result.desc_);
		fid.set_immediate_effect(event_result.immediate_);
		fid.set_news_title(event_result.news_title_);
		fid.set_news_picture(event_result.news_picture_);
		fid.set_news_long_desc(event_result.news_long_desc_);
		fid.set_news_medium_desc(event_result.news_medium_desc_);
		fid.set_news_short_desc(event_result.news_short_desc_);
		fid.set_allow_multiple_instances(event_result.allow_multiple_instances);
		fid.set_name(event_result.title_);
		fid.get_options() = event_result.options;
		fid.set_window_type(context.outer_context.state.add_key_win1252(event_result.window_type));
		//Effect
		ef_province_event value;
		value.days = 0;
		value.id_ = id;
		if(context.main_slot == trigger::slot_contents::province) {
			if(value.days <= 0) {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::province_event_immediate_this_nation));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::province_event_immediate_this_province));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(uint16_t(effect::province_event_immediate_this_state));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_effect.push_back(uint16_t(effect::province_event_immediate_this_pop));
				else {
					err.accumulated_errors += "lambda_province_event effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
					return;
				}
				context.compiled_effect.push_back(trigger::payload(value.id_).value);
			} else {
				if(context.this_slot == trigger::slot_contents::nation)
				context.compiled_effect.push_back(uint16_t(effect::province_event_this_nation));
				else if(context.this_slot == trigger::slot_contents::province)
				context.compiled_effect.push_back(uint16_t(effect::province_event_this_province));
				else if(context.this_slot == trigger::slot_contents::state)
				context.compiled_effect.push_back(uint16_t(effect::province_event_this_state));
				else if(context.this_slot == trigger::slot_contents::pop)
				context.compiled_effect.push_back(uint16_t(effect::province_event_this_pop));
				else {
					err.accumulated_errors += "lambda_province_event effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
					return;
				}
				context.compiled_effect.push_back(trigger::payload(value.id_).value);
				context.compiled_effect.push_back(uint16_t(value.days));
			}
		} else {
			err.accumulated_errors +=
			"lambda_province_event effect used in an incorrect scope type " + slot_contents_to_string(context.main_slot) + " (" + err.file_name + ")\n";
			return;
		}
	}
} // namespace parsers
