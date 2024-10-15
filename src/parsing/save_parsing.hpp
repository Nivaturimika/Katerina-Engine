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
struct save_parser_context {
	sys::state& state;
	dcon::nation_id current;
	save_parser_context(sys::state& state);
};
struct save_rgo {
	void goods_type(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_typed_id {
	int32_t id = 0;
	int32_t type = 0;
	void finish(save_parser_context& context) { }
};
struct save_flags {
	void any_value(std::string_view name, bool v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_variables {
	void any_value(std::string_view name, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_pop_ideology {
	void any_value(std::string_view name, float v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_pop_issues {
	void any_value(std::string_view name, float v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_pop {
	void size(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void money(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void ideology(association_type, save_pop_ideology v, error_handler& err, int32_t line, save_parser_context& context);
	void issues(association_type, save_pop_issues v, error_handler& err, int32_t line, save_parser_context& context);
	void con(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void mil(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void literacy(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void bank(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void any_value(std::string_view name, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_war_dated_history {
	void add_attacker(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void add_defender(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_war_history {
	void any_group(std::string_view name, save_war_dated_history v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};

struct save_wargoal {
	void casus_belli(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void state_province_id(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void actor(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void receiver(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void score(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void change(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void date(association_type, sys::year_month_day v, error_handler& err, int32_t line, save_parser_context& context);
	void is_fulfilled(association_type, bool v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_alliance {
	void first(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void second(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_vassal {
	void first(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void second(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void end_date(association_type, sys::year_month_day v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_substate {
	void first(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void second(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void end_date(association_type, sys::year_month_day v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_casus_belli {
	void type(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void first(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void second(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void end_date(association_type, sys::year_month_day v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_diplomacy {
	void casus_belli(association_type, save_casus_belli v, error_handler& err, int32_t line, save_parser_context& context);
	void alliance(association_type, save_alliance v, error_handler& err, int32_t line, save_parser_context& context);
	void vassal(association_type, save_vassal v, error_handler& err, int32_t line, save_parser_context& context);
	void substate(association_type, save_substate v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_rebel_faction {
	void id(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context);
	void type(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void name(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void country(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void independence(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void culture(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void religion(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void government(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void province(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void leader(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context);
	void organization(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void pop(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_war {
	void name(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void history(association_type, save_war_history v, error_handler& err, int32_t line, save_parser_context& context);
	void attacker(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void defender(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void original_attacker(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void original_defender(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context);
	void war_goal(association_type, save_wargoal v, error_handler& err, int32_t line, save_parser_context& context);
	void original_wargoal(association_type, save_wargoal v, error_handler& err, int32_t line, save_parser_context& context);
	void action(association_type, sys::year_month_day v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_tax {
	float current = 0.f;
	void finish(save_parser_context& context) { }
};
struct save_relations {
	float value = 0.f;
	void finish(save_parser_context& context) { }
};
struct save_unit_path {
	void free_value(int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_regiment {
	void id(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context);
	void name(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void pop(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context);
	void organisation(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void strength(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void experience(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void count(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void type(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_army {
	void id(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context);
	void leader(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context);
	void name(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void previous(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void movement_progress(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void life_rating(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void path(association_type, save_unit_path v, error_handler& err, int32_t line, save_parser_context& context);
	void location(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void dig_in_last_date(association_type, sys::year_month_day v, error_handler& err, int32_t line, save_parser_context& context);
	void supplies(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void regiment(association_type, save_regiment& v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_ship {
	void id(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context);
	void name(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void organisation(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void strength(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void experience(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void type(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_navy {
	void id(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context);
	void leader(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context);
	void name(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void previous(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void movement_progress(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void life_rating(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void path(association_type, save_unit_path v, error_handler& err, int32_t line, save_parser_context& context);
	void location(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void supplies(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void ship(association_type, save_ship& v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_province {
	void name(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void owner(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void controller(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void core(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void colonial(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void life_rating(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void army(association_type, save_army v, error_handler& err, int32_t line, save_parser_context& context);
	void navy(association_type, save_navy v, error_handler& err, int32_t line, save_parser_context& context);
	void rgo(association_type, save_rgo v, error_handler& err, int32_t line, save_parser_context& context);
	void army(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void any_group(std::string_view name, save_pop v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_upper_house {
	void any_value(std::string_view name, float v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_technologies {
	void any_value(std::string_view name, float v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_country {
	void rich_tax(association_type, save_tax& v, error_handler& err, int32_t line, save_parser_context& context);
	void middle_tax(association_type, save_tax& v, error_handler& err, int32_t line, save_parser_context& context);
	void poor_tax(association_type, save_tax& v, error_handler& err, int32_t line, save_parser_context& context);
	void education_spending(association_type, save_tax& v, error_handler& err, int32_t line, save_parser_context& context);
	void crime_fighting(association_type, save_tax& v, error_handler& err, int32_t line, save_parser_context& context);
	void social_spending(association_type, save_tax& v, error_handler& err, int32_t line, save_parser_context& context);
	void military_spending(association_type, save_tax& v, error_handler& err, int32_t line, save_parser_context& context);
	void leadership(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void revanchism(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void plurality(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void diplomatic_points(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void badboy(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void prestige(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void suppression(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void ruling_party(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void research_points(association_type, float v, error_handler& err, int32_t line, save_parser_context& context);
	void civilized(association_type, bool v, error_handler& err, int32_t line, save_parser_context& context);
	void money(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void nationalvalue(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void variables(association_type, save_variables v, error_handler& err, int32_t line, save_parser_context& context);
	void technology(association_type, save_technologies v, error_handler& err, int32_t line, save_parser_context& context);
	void upper_house(association_type, save_upper_house v, error_handler& err, int32_t line, save_parser_context& context);
	void capital(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void schools(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void primary_culture(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void religion(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void last_election(association_type, sys::year_month_day ymd, error_handler& err, int32_t line, save_parser_context& context);
	void last_bankrupt(association_type, sys::year_month_day ymd, error_handler& err, int32_t line, save_parser_context& context);
	void flags(association_type, save_flags v, error_handler& err, int32_t line, save_parser_context& context);
	void varuables(association_type, save_variables v, error_handler& err, int32_t line, save_parser_context& context);
	void any_value(std::string_view name, std::string_view v, error_handler& err, int32_t line, save_parser_context& context);
	void any_group(std::string_view name, save_relations, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
struct save_file {
	void date(association_type, sys::year_month_day ymd, error_handler& err, int32_t line, save_parser_context& context);
	void player(association_type, uint32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void government(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context);
	void flags(association_type, save_flags v, error_handler& err, int32_t line, save_parser_context& context);
	void diplomacy(association_type, save_diplomacy v, error_handler& err, int32_t line, save_parser_context& context);
	void active_war(association_type, save_war v, error_handler& err, int32_t line, save_parser_context& context);
	void finish(save_parser_context& context) { }
};
} //namespace parsers
