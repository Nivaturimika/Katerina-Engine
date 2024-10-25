#pragma once

#include <stdint.h>
#include <optional>
#include <string_view>
#include <string>

#include "constants.hpp"
#include "parsers.hpp"
#include "script_constants.hpp"
#include "nations.hpp"
#include "container_types.hpp"

namespace parsers {
	struct game_rules_action_context {
		scenario_building_context& outer_context;
		sys::static_game_rule action_type = sys::static_game_rule::count;
		game_rules_action_context(scenario_building_context& outer_context);
	};
	struct game_rule_action {
		dcon::trigger_key limit;
		dcon::effect_key effect;
		void type(association_type, std::string_view value, error_handler& err, int32_t line, game_rules_action_context&);
		void finish(game_rules_action_context& context);
	};
	struct game_rules_file {
		void action(game_rule_action value, error_handler& err, int32_t line, scenario_building_context&);
		void finish(scenario_building_context& context);
	};
	game_rule_action meke_game_rule_action(token_generator& gen, error_handler& err, scenario_building_context& context);
	dcon::trigger_key meke_game_rule_trigger(token_generator& gen, error_handler& err, game_rules_action_context& context);
	dcon::effect_key meke_game_rule_effect(token_generator& gen, error_handler& err, game_rules_action_context& context);
}
