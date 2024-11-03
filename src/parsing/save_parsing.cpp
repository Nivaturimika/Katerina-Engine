#include "save_parsing.hpp"

namespace parsers {

void save_rgo::goods_type(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_flags::any_value(std::string_view name, bool v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_variables::any_value(std::string_view name, int32_t v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_pop_ideology::any_value(std::string_view name, float v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_pop_issues::any_value(std::string_view name, float v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_pop::size(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_pop::money(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_pop::ideology(association_type, save_pop_ideology v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_pop::issues(association_type, save_pop_issues v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_pop::con(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_pop::mil(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_pop::literacy(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_pop::bank(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_pop::any_value(std::string_view name, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_war_dated_history::add_attacker(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_war_dated_history::add_defender(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_war_history::any_group(std::string_view name, save_war_dated_history v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_wargoal::casus_belli(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_wargoal::state_province_id(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_wargoal::actor(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_wargoal::receiver(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_wargoal::score(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_wargoal::change(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_wargoal::date(association_type, sys::year_month_day v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_wargoal::is_fulfilled(association_type, bool v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_alliance::first(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_alliance::second(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_vassal::first(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_vassal::second(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_vassal::end_date(association_type, sys::year_month_day v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_substate::first(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_substate::second(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_substate::end_date(association_type, sys::year_month_day v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_casus_belli::type(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_casus_belli::first(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_casus_belli::second(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_casus_belli::end_date(association_type, sys::year_month_day v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_diplomacy::casus_belli(association_type, save_casus_belli v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_diplomacy::alliance(association_type, save_alliance v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_diplomacy::vassal(association_type, save_vassal v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_diplomacy::substate(association_type, save_substate v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_rebel_faction::id(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_rebel_faction::type(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_rebel_faction::name(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_rebel_faction::country(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_rebel_faction::independence(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_rebel_faction::culture(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_rebel_faction::religion(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_rebel_faction::government(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_rebel_faction::province(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_rebel_faction::leader(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_rebel_faction::organization(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_rebel_faction::pop(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_war::name(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_war::history(association_type, save_war_history v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_war::attacker(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_war::defender(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_war::original_attacker(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_war::original_defender(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_war::war_goal(association_type, save_wargoal v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_war::original_wargoal(association_type, save_wargoal v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_war::action(association_type, sys::year_month_day v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_unit_path::free_value(int32_t v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_regiment::id(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_regiment::name(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_regiment::pop(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_regiment::organisation(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_regiment::strength(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_regiment::experience(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_regiment::count(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_regiment::type(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_army::id(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_army::leader(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_army::name(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_army::previous(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_army::movement_progress(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_army::life_rating(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_army::path(association_type, save_unit_path v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_army::location(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_army::dig_in_last_date(association_type, sys::year_month_day v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_army::supplies(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_army::regiment(association_type, save_regiment& v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_ship::id(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_ship::name(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_ship::organisation(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_ship::strength(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_ship::experience(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_ship::type(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_navy::id(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_navy::leader(association_type, save_typed_id v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_navy::name(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_navy::previous(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_navy::movement_progress(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_navy::life_rating(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_navy::path(association_type, save_unit_path v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_navy::location(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_navy::supplies(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_navy::ship(association_type, save_ship& v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_province::name(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_province::owner(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_province::controller(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_province::core(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_province::colonial(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_province::life_rating(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_province::army(association_type, save_army v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_province::navy(association_type, save_navy v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_province::rgo(association_type, save_rgo v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_province::army(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_province::any_group(std::string_view name, save_pop v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_upper_house::any_value(std::string_view name, float v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_technologies::any_value(std::string_view name, float v, error_handler& err, int32_t line, save_parser_context& context) {

}

void save_country::rich_tax(association_type, save_tax& v, error_handler& err, int32_t line, save_parser_context& context) {
	context.state.world.nation_set_rich_tax(context.current, int8_t(v.current * 100.f));
}
void save_country::middle_tax(association_type, save_tax& v, error_handler& err, int32_t line, save_parser_context& context) {
	context.state.world.nation_set_middle_tax(context.current, int8_t(v.current * 100.f));
}
void save_country::poor_tax(association_type, save_tax& v, error_handler& err, int32_t line, save_parser_context& context) {
	context.state.world.nation_set_poor_tax(context.current, int8_t(v.current * 100.f));
}
void save_country::education_spending(association_type, save_tax& v, error_handler& err, int32_t line, save_parser_context& context) {
	context.state.world.nation_set_education_spending(context.current, int8_t(v.current * 100.f));
}
void save_country::crime_fighting(association_type, save_tax& v, error_handler& err, int32_t line, save_parser_context& context) {
	context.state.world.nation_set_administrative_spending(context.current, int8_t(v.current * 100.f));
}
void save_country::social_spending(association_type, save_tax& v, error_handler& err, int32_t line, save_parser_context& context) {
	context.state.world.nation_set_social_spending(context.current, int8_t(v.current * 100.f));
}
void save_country::military_spending(association_type, save_tax& v, error_handler& err, int32_t line, save_parser_context& context) {
	context.state.world.nation_set_military_spending(context.current, int8_t(v.current * 100.f));
}
void save_country::leadership(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {
	context.state.world.nation_set_leadership_points(context.current, v);
}
void save_country::revanchism(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {
	context.state.world.nation_set_revanchism(context.current, v);
}
void save_country::plurality(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {
	context.state.world.nation_set_plurality(context.current, v);
}
void save_country::diplomatic_points(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {
	context.state.world.nation_set_diplomatic_points(context.current, v);
}
void save_country::badboy(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {
	context.state.world.nation_set_infamy(context.current, v);
}
void save_country::prestige(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {
	context.state.world.nation_set_prestige(context.current, v);
}
void save_country::suppression(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {
	context.state.world.nation_set_suppression_points(context.current, v);
}
void save_country::research_points(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {
	context.state.world.nation_set_research_points(context.current, v);
}
void save_country::civilized(association_type, bool v, error_handler& err, int32_t line, save_parser_context& context) {
	context.state.world.nation_set_is_civilized(context.current, v);
}
void save_country::ruling_party(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context) {
	auto id = dcon::political_party_id(dcon::political_party_id::value_base_t(v));
	context.state.world.nation_set_ruling_party(context.current, id);
}
void save_country::nationalvalue(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_country::variables(association_type, save_variables v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_country::technology(association_type, save_technologies v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_country::upper_house(association_type, save_upper_house v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_country::capital(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_country::schools(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_country::primary_culture(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_country::religion(association_type, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_country::money(association_type, float v, error_handler& err, int32_t line, save_parser_context& context) {
	context.state.world.nation_set_stockpiles(context.current, economy::money, v);
}
void save_country::last_election(association_type, sys::year_month_day ymd, error_handler& err, int32_t line, save_parser_context& context) {
	context.state.world.nation_set_election_ends(context.current, sys::date(ymd, context.state.start_date));
}
void save_country::last_bankrupt(association_type, sys::year_month_day ymd, error_handler& err, int32_t line, save_parser_context& context) {
	context.state.world.nation_set_bankrupt_until(context.current, sys::date(ymd, context.state.start_date));
}
void save_country::flags(association_type, save_flags v, error_handler& err, int32_t line, save_parser_context& context) {
	
}
void save_country::varuables(association_type, save_variables v, error_handler& err, int32_t line, save_parser_context& context) {
	
}
void save_country::any_value(std::string_view name, std::string_view v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_country::any_group(std::string_view name, save_relations v, error_handler& err, int32_t line, save_parser_context& context) {
	dcon::nation_id n;
	for(auto id : context.state.world.in_national_identity) {
		if(nations::int_to_tag(id.get_identifying_int()) == name) {
			n = id.get_identity_holder().get_nation();
			if(n) {
				auto rel = context.state.world.get_diplomatic_relation_by_diplomatic_pair(context.current, n);
				if(!rel) {
					rel = context.state.world.force_create_diplomatic_relation(context.current, n);
				}
				context.state.world.diplomatic_relation_set_value(rel, v.value);
			}
			return;
		}
	}
	err.accumulated_errors += "invalid tag " + std::string(name) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
}

void save_file::date(association_type, sys::year_month_day ymd, error_handler& err, int32_t line, save_parser_context& context) {
	context.state.current_date = sys::date(ymd, context.state.start_date);
}
void save_file::player(association_type, uint32_t v, error_handler& err, int32_t line, save_parser_context& context) {
	auto const tag = nations::int_to_tag(v);
	for(const auto id : context.state.world.in_national_identity) {
		if(nations::int_to_tag(id.get_identifying_int()) == tag) {
			context.state.local_player_nation = context.state.world.national_identity_get_nation_from_identity_holder(id);
			return;
		}
	}
	err.accumulated_errors += "invalid tag " + std::string(tag) + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
}
void save_file::government(association_type, int32_t v, error_handler& err, int32_t line, save_parser_context& context) {
	auto const id = dcon::government_type_id(dcon::government_type_id::value_base_t(v));
	context.state.world.nation_set_government_type(context.state.local_player_nation, id);
}

void save_file::flags(association_type, save_flags v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_file::diplomacy(association_type, save_diplomacy v, error_handler& err, int32_t line, save_parser_context& context) {

}
void save_file::active_war(association_type, save_war v, error_handler& err, int32_t line, save_parser_context& context) {

}

} //namespace parsers
