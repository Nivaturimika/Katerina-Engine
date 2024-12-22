# Scripting
<!-- TOC -->

- [Scripting](#scripting)
	- [Simple effects](#simple-effects)
		- [capital = <province id>](#capital--province-id)
		- [add_core = <THIS/FROM/TAG>](#add_core--thisfromtag)
		- [remove_core = <THIS/FROM/TAG>](#remove_core--thisfromtag)
		- [change_region_name = <name>](#change_region_name--name)
		- [trade_goods = <good>](#trade_goods--good)
		- [add_accepted_culture = <THIS/FROM/culture>](#add_accepted_culture--thisfromculture)
		- [remove_accepted_culture = <culture>](#remove_accepted_culture--culture)
		- [primary_culture = <culture>](#primary_culture--culture)
		- [life_rating = <n>](#life_rating--n)
		- [religion = <religion>](#religion--religion)
		- [is_slave = <yes/no>](#is_slave--yesno)
		- [research_points = <n>](#research_points--n)
		- [tech_school = <tech_school>](#tech_school--tech_school)
		- [government = <government>](#government--government)
		- [treasury = <n>](#treasury--n)
		- [war_exhaustion = <n>](#war_exhaustion--n)
		- [prestige = <n>](#prestige--n)
		- [change_tag = <TAG/culture>](#change_tag--tagculture)
		- [change_tag_no_core_switch = <TAG/culture>](#change_tag_no_core_switch--tagculture)
		- [set_country_flag = <flag>](#set_country_flag--flag)
		- [clr_country_flag = <flag>](#clr_country_flag--flag)
		- [set_news_flag = <flag>](#set_news_flag--flag)
		- [clear_news_flag = <flag>](#clear_news_flag--flag)
		- [country_event = <id>](#country_event--id)
		- [province_event = <id>](#province_event--id)
		- [military_access = <THIS/FROM/TAG>](#military_access--thisfromtag)
		- [badboy = <n>](#badboy--n)
		- [secede_province = <THIS/FROM/TAG>](#secede_province--thisfromtag)
		- [inherit = <THIS/FROM/TAG>](#inherit--thisfromtag)
		- [annex_to = <THIS/FROM/TAG>](#annex_to--thisfromtag)
		- [release = <THIS/FROM/TAG>](#release--thisfromtag)
		- [change_controller = <THIS/FROM/TAG>](#change_controller--thisfromtag)
		- [infrastructure = <n>](#infrastructure--n)
		- [money = <n>](#money--n)
		- [prestige_factor = <n>](#prestige_factor--n)
		- [leadership = <n>](#leadership--n)
		- [create_vassal = <THIS/FROM/TAG>](#create_vassal--thisfromtag)
		- [end_military_access = <THIS/FROM/TAG>](#end_military_access--thisfromtag)
		- [leave_alliance = <THIS/FROM/TAG>](#leave_alliance--thisfromtag)
		- [end_war = <THIS/FROM/TAG>](#end_war--thisfromtag)
		- [enable_ideology = <ideology>](#enable_ideology--ideology)
		- [ruling_party_ideology = <ideology>](#ruling_party_ideology--ideology)
		- [plurality = <n>](#plurality--n)
		- [remove_province_modifier = <modifier>](#remove_province_modifier--modifier)
		- [remove_country_modifier = <modifier>](#remove_country_modifier--modifier)
		- [create_alliance = <THIS/FROM/TAG>](#create_alliance--thisfromtag)
		- [release_vassal = <THIS/FROM/TAG/random>](#release_vassal--thisfromtagrandom)
		- [change_province_name = <name>](#change_province_name--name)
		- [enable_canal = <id>](#enable_canal--id)
		- [set_global_flag = <flag>](#set_global_flag--flag)
		- [clr_global_flag = <flag>](#clr_global_flag--flag)
		- [nationalvalue = <national value>](#nationalvalue--national-value)
		- [civilized = <yes/no>](#civilized--yesno)
		- [election = <yes/no>](#election--yesno)
		- [social_reform = <reform>](#social_reform--reform)
		- [political_reform = <reform>](#political_reform--reform)
		- [add_tax_relative_income = <n>](#add_tax_relative_income--n)
		- [neutrality = <yes/no>](#neutrality--yesno)
		- [reduce_pop = <n>](#reduce_pop--n)
		- [move_pop = <province id>](#move_pop--province-id)
		- [pop_type = <type>](#pop_type--type)
		- [years_of_research = <n>](#years_of_research--n)
		- [military_reform = <reform>](#military_reform--reform)
		- [economic_reform = <reform>](#economic_reform--reform)
		- [remove_random_military_reforms = <n>](#remove_random_military_reforms--n)
		- [remove_random_economic_reforms = <n>](#remove_random_economic_reforms--n)
		- [add_crime = <crime>](#add_crime--crime)
		- [nationalize = <yes/no>](#nationalize--yesno)
		- [build_factory_in_capital_state = <type>](#build_factory_in_capital_state--type)
		- [activate_technology = <tech>](#activate_technology--tech)
		- [great_wars_enabled = <yes/no>](#great_wars_enabled--yesno)
		- [world_wars_enabled = <yes/no>](#world_wars_enabled--yesno)
		- [assimilate = <yes/no>](#assimilate--yesno)
		- [literacy = <n>](#literacy--n)
		- [add_crisis_interest = <THIS/FROM/TAG>](#add_crisis_interest--thisfromtag)
		- [flashpoint_tension = <n>](#flashpoint_tension--n)
		- [add_crisis_temperature = <n>](#add_crisis_temperature--n)
		- [consciousness = <n>](#consciousness--n)
		- [militancy = <n>](#militancy--n)
		- [rgo_size = <n>](#rgo_size--n)
		- [fort = <n>](#fort--n)
		- [naval_base = <n>](#naval_base--n)
		- [railroad = <n>](#railroad--n)
		- [[building type] = <n>](#building-type--n)
		- [kill_leader = <name>](#kill_leader--name)
		- [set_province_flag = <flag>](#set_province_flag--flag)
		- [clr_province_flag = <flag>](#clr_province_flag--flag)
	- [Effects with parameters](#effects-with-parameters)
		- [trigger_revolt = { ... }](#trigger_revolt----)
		- [diplomatic_influence = { ... }](#diplomatic_influence----)
		- [relation = { ... }](#relation----)
		- [modify_relation = { ... }](#modify_relation----)
		- [add_province_modifier = { ... }](#add_province_modifier----)
		- [add_country_modifier = { ... }](#add_country_modifier----)
		- [casus_belli = { ... }](#casus_belli----)
		- [add_casus_belli = { ... }](#add_casus_belli----)
		- [remove_casus_belli = { ... }](#remove_casus_belli----)
		- [this_remove_casus_belli = { ... }](#this_remove_casus_belli----)
		- [war = { ... }](#war----)
		- [country_event = { ... }](#country_event----)
		- [province_event = { ... }](#province_event----)
		- [sub_unit = { ... }](#sub_unit----)
		- [set_variable = { ... }](#set_variable----)
		- [change_variable = { ... }](#change_variable----)
		- [ideology = { ... }](#ideology----)
		- [dominant_issue = { ... }](#dominant_issue----)
		- [upper_house = { ... }](#upper_house----)
		- [scaled_militancy = { ... }](#scaled_militancy----)
		- [scaled_consciousness = { ... }](#scaled_consciousness----)
		- [define_general = { ... }](#define_general----)
		- [define_admiral = { ... }](#define_admiral----)
		- [add_war_goal = { ... }](#add_war_goal----)
		- [move_issue_percentage = { ... }](#move_issue_percentage----)
		- [party_loyalty = { ... }](#party_loyalty----)
		- [build_railway_in_capital = { ... }](#build_railway_in_capital----)
		- [build_fort_in_capital = { ... }](#build_fort_in_capital----)
	- [Effect scopes](#effect-scopes)
		- [hidden_tooltip = { ... }](#hidden_tooltip----)
		- [any_neighbor_province = { ... }](#any_neighbor_province----)
		- [any_neighbor_country = { ... }](#any_neighbor_country----)
		- [any_country = { ... }](#any_country----)
		- [random_country = { ... }](#random_country----)
		- [random_neighbor_province = { ... }](#random_neighbor_province----)
		- [random_empty_neighbor_province = { ... }](#random_empty_neighbor_province----)
		- [any_greater_power = { ... }](#any_greater_power----)
		- [poor_strata = { ... }](#poor_strata----)
		- [middle_strata = { ... }](#middle_strata----)
		- [rich_strata = { ... }](#rich_strata----)
		- [random_pop = { ... }](#random_pop----)
		- [random_owned = { ... }](#random_owned----)
		- [random_province = { ... }](#random_province----)
		- [all_core = { ... }](#all_core----)
		- [any_owned = { ... }](#any_owned----)
		- [any_state = { ... }](#any_state----)
		- [random_state = { ... }](#random_state----)
		- [any_pop = { ... }](#any_pop----)
		- [owner = { ... }](#owner----)
		- [controller = { ... }](#controller----)
		- [location = { ... }](#location----)
		- [country = { ... }](#country----)
		- [capital_scope = { ... }](#capital_scope----)
		- [THIS = { ... }](#this----)
		- [FROM = { ... }](#from----)
		- [sea_zone = { ... }](#sea_zone----)
		- [cultural_union = { ... }](#cultural_union----)
		- [overlord = { ... }](#overlord----)
		- [sphere_owner = { ... }](#sphere_owner----)
		- [independence = { ... }](#independence----)
		- [flashpoint_tag_scope = { ... }](#flashpoint_tag_scope----)
		- [crisis_state_scope = { ... }](#crisis_state_scope----)
		- [state_scope = { ... }](#state_scope----)
		- [random = { ... }](#random----)
		- [random_list = { ... }](#random_list----)
		- [clear_news_scopes = { ... }](#clear_news_scopes----)
		- [[TAG] = { ... }](#tag----)
		- [[region name] = { ... }](#region-name----)
		- [[province id] = { ... }](#province-id----)
	- [Broken effects](#broken-effects)
		- [guarentee = <THIS/FROM/TAG>](#guarentee--thisfromtag)
		- [fow = <THIS/FROM/TAG>](#fow--thisfromtag)
		- [disable_fow = <THIS/FROM/TAG>](#disable_fow--thisfromtag)
- [Scripting triggers](#scripting-triggers)
	- [Simple triggers](#simple-triggers)
		- [total_sunk_by_us = <n>](#total_sunk_by_us--n)
		- [ai = <yes/no>](#ai--yesno)
		- [tag = <THIS/FROM/TAG>](#tag--thisfromtag)
		- [war = <yes/no>](#war--yesno)
		- [owns = <province id>](#owns--province-id)
		- [port = <yes/no>](#port--yesno)
		- [rank = <n>](#rank--n)
		- [type = <pop type>](#type--pop-type)
		- [has_pop_type = <pop type>](#has_pop_type--pop-type)
		- [year = <n>](#year--n)
		- [empty = <yes/no>](#empty--yesno)
		- [money = <n>](#money--n)
		- [month = <n>](#month--n)
		- [always = <yes/no>](#always--yesno)
		- [badboy = <n>](#badboy--n)
		- [exists = <THIS/FROM/TAG/yes/no>](#exists--thisfromtagyesno)
		- [region = <region name>](#region--region-name)
		- [strata = <poor/middle/rich>](#strata--poormiddlerich)
		- [capital = <province id>](#capital--province-id)
		- [culture = <culture>](#culture--culture)
		- [is_culture = <culture>](#is_culture--culture)
		- [is_core = <THIS/FROM/TAG>](#is_core--thisfromtag)
		- [blockade = <yes/no>](#blockade--yesno)
		- [controls = <province id>](#controls--province-id)
		- [election = <yes/no>](#election--yesno)
		- [is_slave = <yes/no>](#is_slave--yesno)
		- [literacy = <n>](#literacy--n)
		- [neighbor = <THIS/FROM/TAG>](#neighbor--thisfromtag)
		- [owned_by = <THIS/FROM/TAG>](#owned_by--thisfromtag)
		- [poor_tax = <n>](#poor_tax--n)
		- [low_tax = <n>](#low_tax--n)
		- [pop_type = <pop type>](#pop_type--pop-type)
		- [prestige = <n>](#prestige--n)
		- [produces = <goods>](#produces--goods)
		- [religion = <religion>](#religion--religion)
		- [rich_tax = <n>](#rich_tax--n)
		- [state_id = <province id>](#state_id--province-id)
		- [treasury = <n>](#treasury--n)
		- [war_with = <THIS/FROM/TAG>](#war_with--thisfromtag)
		- [civilized = <yes/no>](#civilized--yesno)
		- [continent = <continent>](#continent--continent)
		- [has_crime = <crime>](#has_crime--crime)
		- [in_sphere = <THIS/FROM/TAG>](#in_sphere--thisfromtag)
		- [invention = <invention>](#invention--invention)
		- [is_vassal = <yes/no>](#is_vassal--yesno)
		- [is_subject = <yes/no>](#is_subject--yesno)
		- [militancy = <n>](#militancy--n)
		- [pop_militancy = <n>](#pop_militancy--n)
		- [plurality = <n>](#plurality--n)
		- [vassal_of = <THIS/FROM/TAG>](#vassal_of--thisfromtag)
		- [war_score = <n>](#war_score--n)
		- [corruption = <n>](#corruption--n)
		- [government = <government>](#government--government)
		- [has_leader = <name>](#has_leader--name)
		- [in_default = <THIS/FROM/TAG/yes/no>](#in_default--thisfromtagyesno)
		- [is_capital = <yes/no>](#is_capital--yesno)
		- [is_coastal = <yes/no>](#is_coastal--yesno)
		- [life_needs = <n>](#life_needs--n)
		- [middle_tax = <n>](#middle_tax--n)
		- [minorities = <n>](#minorities--n)
		- [revanchism = <n>](#revanchism--n)
		- [total_pops = <n>](#total_pops--n)
		- [truce_with = <THIS/FROM/TAG>](#truce_with--thisfromtag)
		- [casus_belli = <THIS/FROM/TAG>](#casus_belli--thisfromtag)
		- [is_colonial = <yes/no>](#is_colonial--yesno)
		- [is_disarmed = <yes/no>](#is_disarmed--yesno)
		- [is_overseas = <yes/no>](#is_overseas--yesno)
		- [is_substate = <yes/no>](#is_substate--yesno)
		- [life_rating = <n>](#life_rating--n)
		- [nationalism = <n>](#nationalism--n)
		- [province_id = <province id>](#province_id--province-id)
		- [substate_of = <THIS/FROM/TAG>](#substate_of--thisfromtag)
		- [tech_school = <tech_school>](#tech_school--tech_school)
		- [trade_goods = <goods>](#trade_goods--goods)
		- [big_producer = <goods>](#big_producer--goods)
		- [crisis_exist = <yes/no>](#crisis_exist--yesno)
		- [has_building = <building type>](#has_building--building-type)
		- [is_blockaded = <yes/no>](#is_blockaded--yesno)
		- [is_mobilised = <yes/no>](#is_mobilised--yesno)
		- [luxury_needs = <n>](#luxury_needs--n)
		- [num_of_ports = <n>](#num_of_ports--n)
		- [ruling_party = <ruling party>](#ruling_party--ruling-party)
		- [unemployment = <n>](#unemployment--n)
		- [alliance_with = <THIS/FROM/TAG>](#alliance_with--thisfromtag)
		- [cash_reserves = <n>](#cash_reserves--n)
		- [consciousness = <n>](#consciousness--n)
		- [controlled_by = <THIS/FROM/TAG>](#controlled_by--thisfromtag)
		- [culture_group = <culture group>](#culture_group--culture-group)
		- [has_factories = <yes/no>](#has_factories--yesno)
		- [is_our_vassal = <THIS/FROM/TAG>](#is_our_vassal--thisfromtag)
		- [lost_national = <n>](#lost_national--n)
		- [nationalvalue = <national value>](#nationalvalue--national-value)
		- [num_of_allies = <n>](#num_of_allies--n)
		- [num_of_cities = <n>](#num_of_cities--n)
		- [crime_fighting = <n>](#crime_fighting--n)
		- [everyday_needs = <n>](#everyday_needs--n)
		- [has_flashpoint = <yes/no>](#has_flashpoint--yesno)
		- [is_independant = <yes/no>](#is_independant--yesno)
		- [is_next_reform = <reform>](#is_next_reform--reform)
		- [military_score = <n>](#military_score--n)
		- [num_of_revolts = <n>](#num_of_revolts--n)
		- [num_of_vassals = <n>](#num_of_vassals--n)
		- [part_of_sphere = <yes/no>](#part_of_sphere--yesno)
		- [unit_in_battle = <yes/no>](#unit_in_battle--yesno)
		- [unit_has_leader = <yes/no>](#unit_has_leader--yesno)
		- [war_exhaustion = <n>](#war_exhaustion--n)
		- [can_nationalize = <yes/no>](#can_nationalize--yesno)
		- [colonial_nation = <yes/no>](#colonial_nation--yesno)
		- [constructing_cb = <THIS/FROM/TAG/cb type>](#constructing_cb--thisfromtagcb-type)
		- [has_global_flag = <flag>](#has_global_flag--flag)
		- [is_claim_crisis = <yes/no>](#is_claim_crisis--yesno)
		- [is_colonial_crisis = <yes/no>](#is_colonial_crisis--yesno)
		- [is_influence_crisis = <yes/no>](#is_influence_crisis--yesno)
		- [military_access = <THIS/FROM/TAG>](#military_access--thisfromtag)
		- [primary_culture = <THIS/FROM/TAG/culture>](#primary_culture--thisfromtagculture)
		- [social_movement = <REB/yes/no>](#social_movement--rebyesno)
		- [social_spending = <n>](#social_spending--n)
		- [accepted_culture = <culture>](#accepted_culture--culture)
		- [brigades_compare = <n>](#brigades_compare--n)
		- [has_country_flag = <flag>](#has_country_flag--flag)
		- [has_news_flag = <flag>](#has_news_flag--flag)
		- [has_culture_core = <yes/no>](#has_culture_core--yesno)
		- [has_pop_religion = <religion>](#has_pop_religion--religion)
		- [industrial_score = <n>](#industrial_score--n)
		- [stronger_army_than = <THIS/FROM/TAG>](#stronger_army_than--thisfromtag)
		- [is_canal_enabled = <id>](#is_canal_enabled--id)
		- [is_culture_group = <culture group>](#is_culture_group--culture-group)
		- [is_greater_power = <yes/no>](#is_greater_power--yesno)
		- [is_state_capital = <yes/no>](#is_state_capital--yesno)
		- [num_of_substates = <n>](#num_of_substates--n)
		- [number_of_states = <n>](#number_of_states--n)
		- [average_militancy = <n>](#average_militancy--n)
		- [can_build_factory = <yes/no>](#can_build_factory--yesno)
		- [is_cultural_union = <THIS/TAG/yes/no>](#is_cultural_union--thistagyesno)
		- [is_state_religion = <yes/no>](#is_state_religion--yesno)
		- [military_spending = <n>](#military_spending--n)
		- [mobilisation_size = <n>](#mobilisation_size--n)
		- [revolt_percentage = <n>](#revolt_percentage--n)
		- [units_in_province = <THIS/FROM/province id/yes/no>](#units_in_province--thisfromprovince-idyesno)
		- [country_units_in_state = <THIS/FROM/state id/yes/no>](#country_units_in_state--thisfromstate-idyesno)
		- [can_create_vassals = <yes/no>](#can_create_vassals--yesno)
		- [crisis_temperature = <n>](#crisis_temperature--n)
		- [education_spending = <n>](#education_spending--n)
		- [flashpoint_tension = <n>](#flashpoint_tension--n)
		- [great_wars_enabled = <yes/no>](#great_wars_enabled--yesno)
		- [involved_in_crisis = <yes/no>](#involved_in_crisis--yesno)
		- [is_possible_vassal = <THIS/FROM/TAG>](#is_possible_vassal--thisfromtag)
		- [is_primary_culture = <THIS/FROM/culture>](#is_primary_culture--thisfromculture)
		- [is_secondary_power = <yes/no>](#is_secondary_power--yesno)
		- [political_movement = <REB/yes/no>](#political_movement--rebyesno)
		- [pop_majority_issue = <issue>](#pop_majority_issue--issue)
		- [social_reform_want = <n>](#social_reform_want--n)
		- [this_culture_union = <THIS/FROM/TAG>](#this_culture_union--thisfromtag)
		- [total_num_of_ports = <n>](#total_num_of_ports--n)
		- [world_wars_enabled = <yes/no>](#world_wars_enabled--yesno)
		- [has_cultural_sphere = <yes/no>](#has_cultural_sphere--yesno)
		- [has_unclaimed_cores = <yes/no>](#has_unclaimed_cores--yesno)
		- [is_accepted_culture = <THIS/culture/yes/no>](#is_accepted_culture--thiscultureyesno)
		- [is_ideology_enabled = <ideology>](#is_ideology_enabled--ideology)
		- [is_sphere_leader_of = <THIS/FROM/TAG>](#is_sphere_leader_of--thisfromtag)
		- [rich_tax_above_poor = <yes/no>](#rich_tax_above_poor--yesno)
		- [constructing_cb_type = <THIS/FROM/TAG/cb type>](#constructing_cb_type--thisfromtagcb-type)
		- [controlled_by_rebels = <yes/no>](#controlled_by_rebels--yesno)
		- [has_country_modifier = <modifier>](#has_country_modifier--modifier)
		- [is_liberation_crisis = <yes/no>](#is_liberation_crisis--yesno)
		- [is_releasable_vassal = <THIS/FROM/TAG/yes/no>](#is_releasable_vassal--thisfromtagyesno)
		- [pop_majority_culture = <culture>](#pop_majority_culture--culture)
		- [rebel_power_fraction = <n>](#rebel_power_fraction--n)
		- [recruited_percentage = <n>](#recruited_percentage--n)
		- [trade_goods_in_state = <goods>](#trade_goods_in_state--goods)
		- [average_consciousness = <n>](#average_consciousness--n)
		- [civilization_progress = <n>](#civilization_progress--n)
		- [culture_has_union_tag = <yes/no>](#culture_has_union_tag--yesno)
		- [has_national_minority = <yes/no>](#has_national_minority--yesno)
		- [has_province_modifier = <modifier>](#has_province_modifier--modifier)
		- [has_recent_imigration = <yes/no>](#has_recent_imigration--yesno)
		- [has_recently_lost_war = <yes/no>](#has_recently_lost_war--yesno)
		- [political_reform_want = <n>](#political_reform_want--n)
		- [poor_strata_militancy = <n>](#poor_strata_militancy--n)
		- [pop_majority_ideology = <ideology>](#pop_majority_ideology--ideology)
		- [pop_majority_religion = <religion>](#pop_majority_religion--religion)
		- [province_control_days = <n>](#province_control_days--n)
		- [rich_strata_militancy = <n>](#rich_strata_militancy--n)
		- [ruling_party_ideology = <ideology>](#ruling_party_ideology--ideology)
		- [total_amount_of_ships = <n>](#total_amount_of_ships--n)
		- [poor_strata_life_needs = <n>](#poor_strata_life_needs--n)
		- [rich_strata_life_needs = <n>](#rich_strata_life_needs--n)
		- [administration_spending = <n>](#administration_spending--n)
		- [agree_with_ruling_party = <n>](#agree_with_ruling_party--n)
		- [middle_strata_militancy = <n>](#middle_strata_militancy--n)
		- [constructing_cb_progress = <n>](#constructing_cb_progress--n)
		- [has_empty_adjacent_state = <yes/no>](#has_empty_adjacent_state--yesno)
		- [middle_strata_life_needs = <n>](#middle_strata_life_needs--n)
		- [poor_strata_luxury_needs = <n>](#poor_strata_luxury_needs--n)
		- [rich_strata_luxury_needs = <n>](#rich_strata_luxury_needs--n)
		- [social_movement_strength = <n>](#social_movement_strength--n)
		- [country_units_in_province = <THIS/FROM/TAG/province id>](#country_units_in_province--thisfromtagprovince-id)
		- [total_amount_of_divisions = <n>](#total_amount_of_divisions--n)
		- [middle_strata_luxury_needs = <n>](#middle_strata_luxury_needs--n)
		- [poor_strata_everyday_needs = <n>](#poor_strata_everyday_needs--n)
		- [rich_strata_everyday_needs = <n>](#rich_strata_everyday_needs--n)
		- [constructing_cb_discovered = <yes/no>](#constructing_cb_discovered--yesno)
		- [someone_can_form_union_tag = <FROM/other>](#someone_can_form_union_tag--fromother)
		- [crime_higher_than_education = <yes/no>](#crime_higher_than_education--yesno)
		- [has_empty_adjacent_province = <yes/no>](#has_empty_adjacent_province--yesno)
		- [national_provinces_occupied = <n>](#national_provinces_occupied--n)
		- [num_of_vassals_no_substates = <n>](#num_of_vassals_no_substates--n)
		- [political_movement_strength = <n>](#political_movement_strength--n)
		- [middle_strata_everyday_needs = <n>](#middle_strata_everyday_needs--n)
		- [can_build_factory_in_capital_state = <factory name>](#can_build_factory_in_capital_state--factory-name)
		- [news_printing_count = <n>](#news_printing_count--n)
		- [[pop type] = <n>](#pop-type--n)
		- [[issue option] = <n>](#issue-option--n)
		- [[issue] = <n>](#issue--n)
		- [[reform] = <n>](#reform--n)
		- [[technology] = <1>](#technology--1)
		- [[invention] = <1>](#invention--1)
		- [[ideology] = <n>](#ideology--n)
		- [[goods] = <n>](#goods--n)
	- [Triggers with multiple parameters](#triggers-with-multiple-parameters)
		- [diplomatic_influence = { ... }](#diplomatic_influence----)
		- [pop_unemployment = { ... }](#pop_unemployment----)
		- [relation = { ... }](#relation----)
		- [check_variable = { ... }](#check_variable----)
		- [upper_house = { ... }](#upper_house----)
		- [unemployment_by_type = { ... }](#unemployment_by_type----)
		- [party_loyalty = { ... }](#party_loyalty----)
		- [can_build_in_province = { ... }](#can_build_in_province----)
		- [can_build_railway_in_capital = { ... }](#can_build_railway_in_capital----)
		- [can_build_fort_in_capital = { ... }](#can_build_fort_in_capital----)
		- [work_available = { ... }](#work_available----)
		- [tags_eq = { ... }](#tags_eq----)
		- [values_eq = { ... }](#values_eq----)
		- [strings_eq = { ... }](#strings_eq----)
		- [dates_eq = { ... }](#dates_eq----)
		- [tags_greater = { ... }](#tags_greater----)
		- [values_greater = { ... }](#values_greater----)
		- [strings_greater = { ... }](#strings_greater----)
		- [dates_greater = { ... }](#dates_greater----)
		- [tags_match = { ... }](#tags_match----)
		- [values_match = { ... }](#values_match----)
		- [strings_match = { ... }](#strings_match----)
		- [dates_match = { ... }](#dates_match----)
		- [tags_contains = { ... }](#tags_contains----)
		- [values_contains = { ... }](#values_contains----)
		- [strings_contains = { ... }](#strings_contains----)
		- [dates_contains = { ... }](#dates_contains----)
		- [length_greater = { ... }](#length_greater----)
	- [Trigger scopes](#trigger-scopes)
		- [AND = { ... }](#and----)
		- [OR = { ... }](#or----)
		- [NOT = { ... }](#not----)
		- [any_neighbor_province = { ... }](#any_neighbor_province----)
		- [any_neighbor_country = { ... }](#any_neighbor_country----)
		- [war_countries = { ... }](#war_countries----)
		- [any_greater_power = { ... }](#any_greater_power----)
		- [any_owned_province = { ... }](#any_owned_province----)
		- [any_core = { ... }](#any_core----)
		- [all_core = { ... }](#all_core----)
		- [any_state = { ... }](#any_state----)
		- [any_substate = { ... }](#any_substate----)
		- [any_sphere_member = { ... }](#any_sphere_member----)
		- [any_pop = { ... }](#any_pop----)
		- [owner = { ... }](#owner----)
		- [controller = { ... }](#controller----)
		- [location = { ... }](#location----)
		- [country = { ... }](#country----)
		- [capital_scope = { ... }](#capital_scope----)
		- [THIS = { ... }](#this----)
		- [FROM = { ... }](#from----)
		- [sea_zone = { ... }](#sea_zone----)
		- [cultural_union = { ... }](#cultural_union----)
		- [overlord = { ... }](#overlord----)
		- [sphere_owner = { ... }](#sphere_owner----)
		- [independence = { ... }](#independence----)
		- [flashpoint_tag_scope = { ... }](#flashpoint_tag_scope----)
		- [crisis_state_scope = { ... }](#crisis_state_scope----)
		- [state_scope = { ... }](#state_scope----)
		- [any_substate = { ... }](#any_substate----)
	- [Broken triggers](#broken-triggers)
		- [unit_in_siege = <province id>](#unit_in_siege--province-id)
		- [have_core_in = <text>](#have_core_in--text)
		- [has_province_flag = <flag>](#has_province_flag--flag)
- [Extensions](#extensions)
- [Logic scripting tutorial](#logic-scripting-tutorial)
	- [Scope overview and optimizations](#scope-overview-and-optimizations)
	- [Double negation NOT NOT](#double-negation-not-not)
	- [Logical scopes](#logical-scopes)
	- [Verifying](#verifying)
	- [Notes](#notes)
	- [Invalid trigger behaviour](#invalid-trigger-behaviour)
	- [Order for variably named triggers](#order-for-variably-named-triggers)

<!-- /TOC -->

## Simple effects

Effects that take a single parameter.

### `capital = <province id>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `add_core = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: `add_core = FROM` can be buggy

### `remove_core = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `change_region_name = <name>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `trade_goods = <good>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `add_accepted_culture = <THIS/FROM/culture>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `remove_accepted_culture = <culture>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `primary_culture = <culture>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Set primary culture to `<culture>`

### `life_rating = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Increase or decrease life rating by `<n>`

### `religion = <religion>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Set state religion to `<religion>`

### `is_slave = <yes/no>`

- **Version**: Base
- **Scopes**: Province, State, Pop
- **Comment**: Makes a province/state/pop into a slave

### `research_points = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `tech_school = <tech_school>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `government = <government>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `treasury = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `war_exhaustion = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `prestige = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `change_tag = <TAG/culture>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `change_tag_no_core_switch = <TAG/culture>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `set_country_flag = <flag>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `clr_country_flag = <flag>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `set_news_flag = <flag>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `clear_news_flag = <flag>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `country_event = <id>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `province_event = <id>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `military_access = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `badboy = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `secede_province = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `inherit = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `annex_to = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `release = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `change_controller = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `infrastructure = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `money = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `prestige_factor = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `leadership = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `create_vassal = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `end_military_access = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `leave_alliance = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `end_war = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `enable_ideology = <ideology>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `ruling_party_ideology = <ideology>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `plurality = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `remove_province_modifier = <modifier>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `remove_country_modifier = <modifier>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `create_alliance = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `release_vassal = <THIS/FROM/TAG/random>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Random will select the tag that appears first in the file. Releasing `REB` will release the defection tag from the `FROM` scope.

### `change_province_name = <name>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `enable_canal = <id>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Enable the canal `<id>` defined in the adjacencies file.

### `set_global_flag = <flag>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `clr_global_flag = <flag>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `nationalvalue = <national value>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `civilized = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `election = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `social_reform = <reform>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `political_reform = <reform>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `add_tax_relative_income = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `neutrality = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `reduce_pop = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Pop size is multiplied by `<n>`: 0.5 is 50% of the original size, 1.5 is 150% times the original size.

### `move_pop = <province id>`

- **Version**: Base
- **Scopes**: Pop
- **Comment**: N/A

### `pop_type = <type>`

- **Version**: Base
- **Scopes**: Pop
- **Comment**: N/A

### `years_of_research = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `military_reform = <reform>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `economic_reform = <reform>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `remove_random_military_reforms = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Remove up to `<n>` military reforms

### `remove_random_economic_reforms = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Remove up to `<n>` economic reforms

### `add_crime = <crime>`

- **Version**: Base
- **Scopes**: Province
- **Comment**: N/A

### `nationalize = <yes/no>`

- **Version**: Base
- **Scopes**: Country
- **Comment**: N/A

### `build_factory_in_capital_state = <type>`

- **Version**: Base
- **Scopes**: State
- **Comment**: N/A

### `activate_technology = <tech>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `great_wars_enabled = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `world_wars_enabled = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `assimilate = <yes/no>`

- **Version**: Base
- **Scopes**: Pop
- **Comment**: N/A

### `literacy = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `add_crisis_interest = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `flashpoint_tension = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `add_crisis_temperature = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `consciousness = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `militancy = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `rgo_size = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `fort = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `naval_base = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `railroad = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `[building type] = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Add `<n>` levels to a given building type (defined in `buildings.txt`)

### `kill_leader = <name>`

- **Version**: Base
- **Scopes**: Country
- **Comment**: kills a leader (admiral or general) belonging to the country in scope with the given name. Note that this will only reliably function if you have explicitly created a leader with that name via effect or via definition in the history files.

### `set_province_flag = <flag>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Will only display a 3D model on the map associated with such province flag.

### `clr_province_flag = <flag>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

## Effects with parameters

Effects that take multiple parameters.

### `trigger_revolt = { ... }`

```
trigger_revolt = {
	culture = <culture>
	religion = <religion>
	ideology = <ideology>
	type = <rebel type>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `diplomatic_influence = { ... }`

```
diplomatic_influence = {
	who = <THIS/FROM/TAG>
	value = <n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `relation = { ... }`

```
relation = {
	who = <THIS/FROM/TAG>
	value = <n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `modify_relation = { ... }`

```
modify_relation = {
	who = <THIS/FROM/TAG>
	value = <n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `add_province_modifier = { ... }`

```
add_province_modifier = {
	name = <name>
	duration = <n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `add_country_modifier = { ... }`

```
add_country_modifier = {
	name = <name>
	duration = <n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `casus_belli = { ... }`

```
casus_belli = {
	type = <name>
	target = <THIS/FROM/TAG>
	months = <n>
}
```

- **Version**: Base
- **Scopes**: Country
- **Comment**: Gives the `<target>` country a casus belli against the country in scope

### `add_casus_belli = { ... }`

```
add_casus_belli = {
	type = <name>
	target = <THIS/FROM/TAG>
	months = <n>
}
```

- **Version**: Base
- **Scopes**: Country
- **Comment**: Gives the country in scope a casus belli against another

### `remove_casus_belli = { ... }`

```
remove_casus_belli = {
	type = <name>
	target = <THIS/FROM/TAG>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `this_remove_casus_belli = { ... }`

```
this_remove_casus_belli = {
	type = <name>
	target = <THIS/FROM/TAG>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `war = { ... }`

```
war = {
	name = <name>
	target = <THIS/FROM/TAG>
	attacker_goal = { 
		target = <THIS/FROM/TAG>
		casus_belli = <cb type>
		state_province_id = <province id>
	}
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `country_event = { ... }`

```
country_event = {
	id = <id>
	days = <n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `province_event = { ... }`

```
province_event = {
	id = <id>
	days = <n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: Saving the game with a pending province event will result in a crash

### `sub_unit = { ... }`

```
sub_unit = {
	type = <type>
	value = <current/n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: If value is `<current>`, then it will spawn a stack with 1 regiment, if it is `<n>` it will spawn a thousand x `<n>` sized regiments.

### `set_variable = { ... }`

```
set_variable = {
	which = <variable>
	value = <n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `change_variable = { ... }`

```
change_variable = {
	which = <variable>
	value = <n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `ideology = { ... }`

```
ideology = {
	value = <ideology>
	factor = <n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `dominant_issue = { ... }`

```
dominant_issue = {
	value = <issue>
	factor = <n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `upper_house = { ... }`

```
upper_house = {
	value = <issue>
	factor = <n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `scaled_militancy = { ... }`

```
scaled_militancy = {
	ideology = <ideology>
	issue = <issue>
	unemployment = <n>
	factor = <n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `scaled_consciousness = { ... }`

```
scaled_consciousness = {
	ideology = <ideology>
	issue = <issue>
	unemployment = <n>
	factor = <n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `define_general = { ... }`

```
define_general = {
	name = <name>
	picture = <gfx picture>
	background = <background trait>
	personality = <personality trait>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `define_admiral = { ... }`

```
define_admiral = {
	name = <name>
	picture = <gfx picture>
	background = <background trait>
	personality = <personality trait>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `add_war_goal = { ... }`

```
add_war_goal = {
	casus_belli = <cb type>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `move_issue_percentage = { ... }`

```
move_issue_percentage = {
	to = <issue>
	from = <issue>
	value = <n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `party_loyalty = { ... }`

```
party_loyalty = {
	ideology = <ideology>
	loyalty_value = <n>
	province_id = <province id>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `build_railway_in_capital = { ... }`

```
build_railway_in_capital = {
	in_whole_capital_state = <yes/no>
	limit_to_world_greatest_level = <yes/no>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: Does not go higher than what the scoped country tech level allows

### `build_fort_in_capital = { ... }`

```
build_fort_in_capital = {
	in_whole_capital_state = <yes/no>
	limit_to_world_greatest_level = <yes/no>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: Does not go higher than what the scoped country tech level allows

## Effect scopes

Unless otherwise noted, any scope laking a `limit = { ... }` is NOT able to use `limit` (for example, `THIS = {}`)

### `hidden_tooltip = { ... }`

```
hidden_tooltip = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: Doesn't actually hide the tooltip

### `any_neighbor_province = { ... }`

```
any_neighbor_province = {
	...
}
```

- **Version**: Base
- **Scopes**: Province
- **Comment**: N/A

### `any_neighbor_country = { ... }`

```
any_neighbor_country = {
	...
}
```

- **Version**: Base
- **Scopes**: Country
- **Comment**: N/A

### `any_country = { ... }`

```
any_country = {
	limit = { ... }
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: Differs in events and decisions, in decisions it only can reference existing countries (i.e has atleast a province), in events it references all countries.

### `random_country = { ... }`

```
random_country = {
	limit = { ... }
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: Same as `any_country` but stops when `limit` is satisfied (or first-random if none is present)

### `random_neighbor_province = { ... }`

```
random_neighbor_province = {
	...
}
```

- **Version**: Base
- **Scopes**: Province
- **Comment**: Same as `any_neighbor_province` but stops when `limit` is satisfied (or first-random if none is present)

### `random_empty_neighbor_province = { ... }`

```
random_empty_neighbor_province = {
	...
}
```

- **Version**: Base
- **Scopes**: Province
- **Comment**: N/A

### `any_greater_power = { ... }`

```
any_greater_power = {
	limit = { ... }
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: Will list all that matches

### `poor_strata = { ... }`

```
poor_strata = {
	limit = { ... }
	...
}
```

- **Version**: Base
- **Scopes**: Country, State, Province
- **Comment**: N/A

### `middle_strata = { ... }`

```
middle_strata = {
	limit = { ... }
	...
}
```

- **Version**: Base
- **Scopes**: Country, State, Province
- **Comment**: N/A

### `rich_strata = { ... }`

```
rich_strata = {
	limit = { ... }
	...
}
```

- **Version**: Base
- **Scopes**: Country, State, Province
- **Comment**: N/A

### `random_pop = { ... }`

```
random_pop = {
	limit = { ... }
	...
}
```

- **Version**: Base
- **Scopes**: Country, State, Province
- **Comment**: Same as `any_pop` but stops when `limit` is satisfied (or first-random if none is present)

### `random_owned = { ... }`

```
random_owned = {
	limit = { ... }
	...
}
```

- **Version**: Base
- **Scopes**: Country
- **Comment**: Same as `any_owned` but stops when `limit` is satisfied (or first-random if none is present)

### `random_province = { ... }`

```
random_province = {
	limit = { ... }
	...
}
```

- **Version**: Base
- **Scopes**: Country
- **Comment**: Same as `any_owned` but stops when `limit` is satisfied (or first-random if none is present)

### `all_core = { ... }`

```
all_core = {
	limit = { ... }
	...
}
```

- **Version**: Base
- **Scopes**: Country
- **Comment**: Scopes all cores of this country (regardless if it exists or not)

### `any_owned = { ... }`

```
any_owned = {
	limit = { ... }
	...
}
```

- **Version**: Base
- **Scopes**: Country
- **Comment**: N/A

### `any_state = { ... }`

```
any_state = {
	limit = { ... }
	...
}
```

- **Version**: Base
- **Scopes**: Country
- **Comment**: N/A

### `random_state = { ... }`

```
random_state = {
	limit = { ... }
	...
}
```

- **Version**: Base
- **Scopes**: Country
- **Comment**: N/A

### `any_pop = { ... }`

```
any_pop = {
	limit = { ... }
	...
}
```

- **Version**: Base
- **Scopes**: Country, State, Province
- **Comment**: N/A

### `owner = { ... }`

```
owner = {
	...
}
```

- **Version**: Base
- **Scopes**: State, Province
- **Comment**: N/A

### `controller = { ... }`

```
controller = {
	...
}
```

- **Version**: Base
- **Scopes**: State, Province
- **Comment**: If controller is `REB` (rebels) it will scope into the `REB` tag.

### `location = { ... }`

```
location = {
	...
}
```

- **Version**: Base
- **Scopes**: Pop
- **Comment**: N/A

### `country = { ... }`

```
country = {
	...
}
```

- **Version**: Base
- **Scopes**: Pop
- **Comment**: N/A

### `capital_scope = { ... }`

```
capital_scope = {
	...
}
```

- **Version**: Base
- **Scopes**: Country, Province
- **Comment**: Scopes into the capital of a nation

### `THIS = { ... }`

```
THIS = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: Scopes into `THIS`, it skips the effects (but evaluates the triggers) if the tag doesn't exist

### `FROM = { ... }`

```
FROM = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: Scopes into `FROM`, it skips the effects (but evaluates the triggers) if the tag doesn't exist

### `sea_zone = { ... }`

```
sea_zone = {
	...
}
```

- **Version**: Base
- **Scopes**: Province
- **Comment**: Scopes all adjacent sea tiles (similar to `any_neighbor_province` but for sea)

### `cultural_union = { ... }`

```
cultural_union = {
	...
}
```

- **Version**: Base
- **Scopes**: Country
- **Comment**: Scopes into the cultural union of the primary culture of the country in scope, for example `GER` for germanic culturs (defined in `common/cultures.txt`)

### `overlord = { ... }`

```
overlord = {
	...
}
```

- **Version**: Base
- **Scopes**: Country
- **Comment**: Scopes into overlord of the current subject country, if no such country exists it's skipped

### `sphere_owner = { ... }`

```
sphere_owner = {
	...
}
```

- **Version**: Base
- **Scopes**: Country
- **Comment**: Scopes into the sphere owner of the given country

### `independence = { ... }`

```
independence = {
	...
}
```

- **Version**: Base
- **Scopes**: Rebel
- **Comment**: Scopes into the independence tag of the rebel in scope (for example `ROM` for Romanian Pan-Nationalists)

### `flashpoint_tag_scope = { ... }`

```
flashpoint_tag_scope = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: The tag using a flashpoint focus on a given state (see `common/national_focus.txt`)

### `crisis_state_scope = { ... }`

```
crisis_state_scope = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: The state the crisis is taking in (can be uncolonized, for colonial crisis)

### `state_scope = { ... }`

```
state_scope = {
	...
}
```

- **Version**: Base
- **Scopes**: Province, Pop
- **Comment**: Scopes into the state of a province, or pop

### `random = { ... }`

```
random = {
	limit = { ... }
	chance = <n>
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: Seed is per-event and per-decision, chance is a weight relative to 100 (50 = 50%)

### `random_list = { ... }`

```
random = {
	<n> = { ... }
	<n> = { ... }
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: Seed is per-event and per-decision, chance is weighted accross the sum of all weights (n1 + n2 + n3 + ...), then they are normalized to 0-1 (for example, n1 = 5, n2 = 5, means 10 total weight, divide 5 by 10, you get 0.5). So n1 = 50%, and n3 = 50% (the 50% is because 0.5 * 100 = 50%). Basically basic probabilistic math, not much needed to explain here.

### `clear_news_scopes = { ... }`

```
clear_news_scopes = {
	limit = { ... }
	type = <scope type>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: Will clear news scopes of the given type, the types can be:
 - `peace_offer_accept`
 - `game_event`
 - `province_change_controller`
 - `province_change_owner`
 - `construction_complete`
 - `research_complete`
 - `battle_over`
 - `rebel_break_country`
 - `new_party`
 - `war_declared`
 - `crisis_started`
 - `crisis_backer`
 - `crisis_side_joined`
 - `crisis_resolved`
 - `decision`
 - `goods_price_change`
 - `ai_afraid_of`
 - `ai_likes_very_much`
 - `fake`
 - `invention`

### `[TAG] = { ... }`

```
[TAG] = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: Can also scope into non-existing tags, or even `REB`. Use with caution, because upon scoping into a province scope, the tag scope is bugged out., for example the following code is broken:
```
BRZ = {
	100 = { reduce_pop = 1.1 }
	set_country_flag = test
}
```
The tag scope will only work until you scope into something else, so instead do:
```
100 = { reduce_pop = 1.1 }
BRZ = { set_country_flag = test }
```

### `[region name] = { ... }`

```
[region name] = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: As defined in `region.txt`

### `[province id] = { ... }`

```
[province id] = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: Finnicky, use with caution as multiple effects inside can crash the game

## Broken effects

### `guarentee = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Country
- **Comment**: Only shows a tooltip, no known in-game effect

### `fow = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Only shows a tooltip, no known in-game effect

### `disable_fow = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Only shows a tooltip, no known in-game effect

# Scripting (triggers)

## Simple triggers

### `total_sunk_by_us = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Total amount of sunk ships

### `ai = <yes/no>`

- **Version**: Base
- **Scopes**: Country
- **Comment**: Is the current scoped country AI controlled?

### `tag = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Country
- **Comment**: Is the current scoped country the tag?

### `war = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Is the current country at war?

### `owns = <province id>`

- **Version**: Base
- **Scopes**: Country
- **Comment**: Whetever the given province is owned by the country at scope

### `port = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Has path to capital and has a naval base

### `rank = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Rank higher than n (higher means the number is lower, "rank = 16" means higher than 16, and rank 15 is > 16). It's better to think of ranks in the game as negative natural numbers, rather than positive ones.

### `type = <pop type>`

- **Version**: Base
- **Scopes**: Pop
- **Comment**: N/A

### `has_pop_type = <pop type>`

- **Version**: Base
- **Scopes**: Country, State, Province
- **Comment**: N/A

### `year = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Year greater or equal than `<n>`

### `empty = <yes/no>`

- **Version**: Base
- **Scopes**: State, Province
- **Comment**: N/A

### `money = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `month = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `always = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `badboy = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Factor `<n>` is % of infamy relative to the infamy limit, 0.1 = 10%, 1 = 100%

### `exists = <THIS/FROM/TAG/yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: It's faster to do `exists = TAG` than to do `TAG = { exists = yes }` due to the extra indirection

### `region = <region name>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: As defined in `regions.txt`

### `strata = <poor/middle/rich>`

- **Version**: Base
- **Scopes**: Pop
- **Comment**: Checks if the pop is in the given strata

### `capital = <province id>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Equivalent to `<province id> = { is_capital = yes }`

### `culture = <culture>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_culture = <culture>`

- **Version**: Base
- **Scopes**: Pop
- **Comment**: N/A

### `is_core = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: State, Province
- **Comment**: N/A

### `blockade = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Is blockaded? Related to gunboat diplomacy, adjacent provinces to blockaded sea regions are marked as blockaded, see the `blockade` static modifier in `common/static_modifiers.txt`

### `controls = <province id>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `election = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Has election ongoing?

### `is_slave = <yes/no>`

- **Version**: Base
- **Scopes**: State, Province
- **Comment**: Is a slave state/province?

### `literacy = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `neighbor = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `owned_by = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `poor_tax = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `low_tax = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `pop_type = <pop type>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `prestige = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `produces = <goods>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `religion = <religion>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `rich_tax = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `state_id = <province id>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `treasury = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `war_with = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `civilized = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `continent = <continent>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `has_crime = <crime>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `in_sphere = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `invention = <invention>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_vassal = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_subject = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `militancy = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `pop_militancy = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `plurality = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `vassal_of = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `war_score = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Average of all war scores

### `corruption = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `government = <government>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `has_leader = <name>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Case sensitive, must be the matching name of a leader, for example `has_leader = "Paul Hinderburg"`

### `in_default = <THIS/FROM/TAG/yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Returns if the given country has defaulted on debts, for `yes/no` is only valid in country scope

### `is_capital = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_coastal = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `life_needs = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `middle_tax = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `minorities = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `revanchism = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `total_pops = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `truce_with = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `casus_belli = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Country
- **Comment**: Is fabricating casus belli against a given nation?

### `is_colonial = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_disarmed = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_overseas = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_substate = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `life_rating = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `nationalism = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `province_id = <province id>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `substate_of = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `tech_school = <tech_school>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `trade_goods = <goods>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `big_producer = <goods>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `crisis_exist = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `has_building = <building type>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_blockaded = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_mobilised = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `luxury_needs = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `num_of_ports = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `ruling_party = <ruling party>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `unemployment = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `alliance_with = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `cash_reserves = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `consciousness = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `controlled_by = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `culture_group = <culture group>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: As defined in `cultures.txt`

### `has_factories = <yes/no>`

- **Version**: Base
- **Scopes**: State, Province
- **Comment**: N/A

### `is_our_vassal = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `lost_national = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `nationalvalue = <national value>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `num_of_allies = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `num_of_cities = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `crime_fighting = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `everyday_needs = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `has_flashpoint = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_independant = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_next_reform = <reform>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `military_score = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `num_of_revolts = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `num_of_vassals = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `part_of_sphere = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `unit_in_battle = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Only checks for country units, rebels are ignored. `unit_in_battle = no` is equivalent to `unit_in_battle = yes`, use a `NOT = {}` instead

### `unit_has_leader = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: `unit_has_leader = no` is equivalent to `unit_has_leader = yes`, use a `NOT = {}` instead

### `war_exhaustion = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `can_nationalize = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `colonial_nation = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `constructing_cb = <THIS/FROM/TAG/cb type>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Localisation is broken with the TAG variant

### `has_global_flag = <flag>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_claim_crisis = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_colonial_crisis = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_influence_crisis = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `military_access = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `primary_culture = <THIS/FROM/TAG/culture>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `social_movement = <REB/yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Checks if the pop is part of a social movement, or in the case of REB, check if the rebel faction is part of a social movement

### `social_spending = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `accepted_culture = <culture>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `brigades_compare = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `has_country_flag = <flag>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `has_news_flag = <flag>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `has_culture_core = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `has_pop_religion = <religion>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `industrial_score = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `stronger_army_than = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_canal_enabled = <id>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_culture_group = <culture group>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_greater_power = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Not equivalent to `rank = 8`, since it also holds for GPs who are falling

### `is_state_capital = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: State capital of the capital of a nation may differ from the capital itself (for example Berlin could be the capital, but an adjacent province could be the state capital).

### `num_of_substates = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `number_of_states = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `average_militancy = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `can_build_factory = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_cultural_union = <THIS/TAG/yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_state_religion = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `military_spending = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `mobilisation_size = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `revolt_percentage = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `units_in_province = <THIS/FROM/province id/yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `country_units_in_state = <THIS/FROM/state id/yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `can_create_vassals = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `crisis_temperature = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `education_spending = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `flashpoint_tension = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `great_wars_enabled = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `involved_in_crisis = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_possible_vassal = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_primary_culture = <THIS/FROM/culture>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_secondary_power = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `political_movement = <REB/yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Checks if the pop is part of a politcal movement, or in the case of REB, check if the rebel faction is part of a political movement

### `pop_majority_issue = <issue>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `social_reform_want = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `this_culture_union = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `total_num_of_ports = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `world_wars_enabled = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `has_cultural_sphere = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `has_unclaimed_cores = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_accepted_culture = <THIS/culture/yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_ideology_enabled = <ideology>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_sphere_leader_of = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `rich_tax_above_poor = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `constructing_cb_type = <THIS/FROM/TAG/cb type>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `controlled_by_rebels = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `has_country_modifier = <modifier>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_liberation_crisis = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_releasable_vassal = <THIS/FROM/TAG/yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `pop_majority_culture = <culture>`

- **Version**: Base
- **Scopes**: Country, State, Province, Pop
- **Comment**: N/A

### `rebel_power_fraction = <n>`

- **Version**: Base
- **Scopes**: Country
- **Comment**: Returns true if any rebellion within the country is atleast `<n>` percent strong

### `recruited_percentage = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `trade_goods_in_state = <goods>`

- **Version**: Base
- **Scopes**: State, Province, Pop
- **Comment**: N/A

### `average_consciousness = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `civilization_progress = <n>`

- **Version**: Base
- **Scopes**: Country
- **Comment**: N/A

### `culture_has_union_tag = <yes/no>`

- **Version**: Base
- **Scopes**: Country, Pop
- **Comment**: N/A

### `has_national_minority = <yes/no>`

- **Version**: Base
- **Scopes**: Country, State, Province
- **Comment**: N/A

### `has_province_modifier = <modifier>`

- **Version**: Base
- **Scopes**: Province
- **Comment**: N/A

### `has_recent_imigration = <yes/no>`

- **Version**: Base
- **Scopes**: Province
- **Comment**: N/A

### `has_recently_lost_war = <yes/no>`

- **Version**: Base
- **Scopes**: Country, Pop
- **Comment**: N/A

### `political_reform_want = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `poor_strata_militancy = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `pop_majority_ideology = <ideology>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `pop_majority_religion = <religion>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `province_control_days = <n>`

- **Version**: Base
- **Scopes**: Province
- **Comment**: N/A

### `rich_strata_militancy = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `ruling_party_ideology = <ideology>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `total_amount_of_ships = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `poor_strata_life_needs = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `rich_strata_life_needs = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `administration_spending = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `agree_with_ruling_party = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `middle_strata_militancy = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `constructing_cb_progress = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `has_empty_adjacent_state = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `middle_strata_life_needs = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `poor_strata_luxury_needs = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `rich_strata_luxury_needs = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `social_movement_strength = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Check if there is any movement with atleast `<n>` % strength

### `country_units_in_province = <THIS/FROM/TAG/province id>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `total_amount_of_divisions = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `middle_strata_luxury_needs = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `poor_strata_everyday_needs = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `rich_strata_everyday_needs = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `constructing_cb_discovered = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `someone_can_form_union_tag = <FROM/other>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `crime_higher_than_education = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `has_empty_adjacent_province = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `national_provinces_occupied = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `num_of_vassals_no_substates = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `political_movement_strength = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `middle_strata_everyday_needs = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `can_build_factory_in_capital_state = <factory name>`

- **Version**: Base
- **Scopes**: Country
- **Comment**: Checks if the given nation can build a factory in its capital state

### `news_printing_count = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `[pop type] = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Checks if there is atleast `<n>` pops of a given type.

### `[issue option] = <n>`

- **Version**: Base
- **Scopes**: Country, State, Province, Pop
- **Comment**: Checks if there is atleast `<n>` support for the given issue option.

### `[issue] = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Checks if there is atleast `<n>` support for the given issue.

### `[reform] = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Checks if there is atleast `<n>` support for the given reform.

### `[technology] = <1>`

- **Version**: Base
- **Scopes**: Country, State, Province, Pop
- **Comment**: Checks if the country on scope has technology

### `[invention] = <1>`

- **Version**: Base
- **Scopes**: Country, State, Province, Pop
- **Comment**: Checks if the country on scope has technology

### `[ideology] = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Checks if there is atleast `<n>` support for the given ideology.

### `[goods] = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Checks if there is atleast `<n>` of a given good on the stockpile.

## Triggers with multiple parameters

### `diplomatic_influence = { ... }`

```
diplomatic_influence = {
	who = <THIS/FROM/TAG>
	value = <n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `pop_unemployment = { ... }`

```
pop_unemployment = {
	type = <pop type>
	value = <n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `relation = { ... }`

```
relation = {
	who = <THIS/FROM/TAG>
	value = <n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `check_variable = { ... }`

```
check_variable = {
	which = <variable>
	value = <n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `upper_house = { ... }`

```
upper_house = {
	ideology = <ideology>
	value = <n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `unemployment_by_type = { ... }`

```
unemployment_by_type = {
	type = <type>
	value = <n>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `party_loyalty = { ... }`

```
party_loyalty = {
	ideology = <ideology>
	value = <n>
	province_id = <province id>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `can_build_in_province = { ... }`

```
can_build_in_province = {
	building = <type>
	limit_to_world_greatest_level = <yes/no>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `can_build_railway_in_capital = { ... }`

```
can_build_railway_in_capital = {
	in_whole_capital_state = <yes/no>
	limit_to_world_greatest_level = <yes/no>
}
```

- **Version**: Base
- **Scopes**: Country
- **Comment**: N/A

### `can_build_fort_in_capital = { ... }`

```
can_build_fort_in_capital = {
	in_whole_capital_state = <yes/no>
	limit_to_world_greatest_level = <yes/no>
}
```

- **Version**: Base
- **Scopes**: Country
- **Comment**: N/A

### `work_available = { ... }`

```
work_available = {
	worker = <pop type>
}
```

- **Version**: Base
- **Scopes**: Country, State, Province
- **Comment**: N/A

### `tags_eq = { ... }`

```
tags_eq = { <n> <m> <tag> }
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `values_eq = { ... }`

```
values_eq = { <n> <m> <value> }
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `strings_eq = { ... }`

```
strings_eq = { <n> <m> <string> }
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `dates_eq = { ... }`

```
dates_eq = { <n> <m> <date> }
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `tags_greater = { ... }`

```
tags_greater = { <n> <m> <tag> }
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `values_greater = { ... }`

```
values_greater = { <n> <m> <value> }
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `strings_greater = { ... }`

```
strings_greater = { <n> <m> <string> }
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `dates_greater = { ... }`

```
dates_greater = { <n> <m> <date> }
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `tags_match = { ... }`

```
tags_match = { <n> <m> <o> <p> }
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `values_match = { ... }`

```
values_match = { <n> <m> <o> <p> }
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `strings_match = { ... }`

```
strings_match = { <n> <m> <o> <p> }
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `dates_match = { ... }`

```
dates_match = { <n> <m> <o> <p> }
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `tags_contains = { ... }`

```
tags_contains = { <n> <tag> }
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `values_contains = { ... }`

```
values_contains = { <n> <value> }
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `strings_contains = { ... }`

```
strings_contains = { <n> <string> }
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `dates_contains = { ... }`

```
dates_contains = { <n> <date> }
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `length_greater = { ... }`

```
length_greater = { <tags/strings/values/dates> <n> <m> }
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: Newspaper trigger, must actually say the type, for example `length_greater = { tags 0 0 }`.

## Trigger scopes

### `AND = { ... }`

```
AND = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: Most scopes listed are an implicit `AND`, unless otherwise stated, Faster performance wise if the triggers within are false often

### `OR = { ... }`

```
OR = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: Faster performance wise if the triggers within are true often

### `NOT = { ... }`

```
NOT = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: Negates triggers within, see below for scripting logical statments

### `any_neighbor_province = { ... }`

```
any_neighbor_province = {
	...
}
```

- **Version**: Base
- **Scopes**: Province
- **Comment**: N/A

### `any_neighbor_country = { ... }`

```
any_neighbor_country = {
	...
}
```

- **Version**: Base
- **Scopes**: Country
- **Comment**: N/A

### `war_countries = { ... }`

```
war_countries = {
	...
}
```

- **Version**: Base
- **Scopes**: Country
- **Comment**: N/A

### `any_greater_power = { ... }`

```
any_greater_power = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `any_owned_province = { ... }`

```
any_owned_province = {
	...
}
```

- **Version**: Base
- **Scopes**: Country
- **Comment**: N/A

### `any_core = { ... }`

```
any_core = {
	...
}
```

- **Version**: Base
- **Scopes**: Country
- **Comment**: N/A

### `all_core = { ... }`

```
all_core = {
	...
}
```

- **Version**: Base
- **Scopes**: Country
- **Comment**: N/A

### `any_state = { ... }`

```
any_state = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `any_substate = { ... }`

```
any_substate = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `any_sphere_member = { ... }`

```
any_sphere_member = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `any_pop = { ... }`

```
any_pop = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `owner = { ... }`

```
owner = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `controller = { ... }`

```
controller = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `location = { ... }`

```
location = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `country = { ... }`

```
country = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `capital_scope = { ... }`

```
capital_scope = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `THIS = { ... }`

```
THIS = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `FROM = { ... }`

```
FROM = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `sea_zone = { ... }`

```
sea_zone = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `cultural_union = { ... }`

```
cultural_union = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `overlord = { ... }`

```
overlord = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `sphere_owner = { ... }`

```
sphere_owner = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `independence = { ... }`

```
independence = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `flashpoint_tag_scope = { ... }`

```
flashpoint_tag_scope = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `crisis_state_scope = { ... }`

```
crisis_state_scope = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `state_scope = { ... }`

```
state_scope = {
	...
}
```

- **Version**: Base
- **Scopes**: Province, Pop
- **Comment**: Scopes into the state instance of the given province and POP

### `any_substate = { ... }`

```
any_substate = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comments**: Scope any substate, not vassals

## Broken triggers

### `unit_in_siege = <province id>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Leftover, doesn't work

### `have_core_in = <text>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Doesn't work in base.

### `has_province_flag = <flag>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Doesn't work in base.

# Extensions

See here [scripting_extensions.md](scripting_extensions.md) and [extensions.md](extensions.md)

# Logic scripting tutorial

Suppose the following trigger:

```
any_core = {
	owned_by = XXX
}
```

This returns false when there is no core that is owned by XXX.

```
all_core = {
	owned_by = XXX
}
```

This returns false when there is also no core that is owned by XXX.

The `NOT = { all_core = { ... } }` can be read as "No core satisfies P" (where P is the predicament).

The `all_core = { NOT = { ... } }` can be read as "In all cores, none satisfied P" (where P is the predicament).

## Scope overview and optimizations

To optimize we must first know our systems, hence, let's start with the basics:

What's the similarity between `AND` and `OR`?

`AND = { ... }` encapsulates all members and asks "All of the conditions within, satisfy all their predicaments?" - in plain English: "Does all the triggers inside the AND evaluate to `True`?"

Whereas `OR = { ... }` asks "All of the conditions within, there is atleast one who satisfy their predicaments" - or "Atleast one trigger inside evaluates to `True`".

If you look closely, you may notice a relationship between the negative versions of the scopes:

| Statment | Some `F`/`T` | All `F` | All `T`
|---|---|---|---|
| `AND = { ... }` | `F` | `F` | `T` |
| `OR = { ... }` | `T` | `F` | `T` |
| `NOT = { AND = { ... } }` | `T` | `T` | `F` |
| `NOT = { OR = { ... } }` | `F` | `T` | `F` |

It's important to realize the properties of All `F`/`T` are applied to everry case, including the case with a single element (as some of one, is all - can you take "some" of one? No!).

For the case of no elements however, none of the properties apply, and an implicit `T` is given (as opposed to iterative scopes such as `any_core`/`all_core` - who give a `F` when there are no elements).

Let's now insert a predicate `P`, assume `P = { exists = yes tag = PRU }` for example, so wehn we say `AND = { P }`. It's interpreted as `AND = { exists = yes tag = PRU }`

With that said, let's now begin using this imaginary predicate to uncover the relationships and convergences of logical statments:

## Double negation (`NOT NOT`)

First off, `NOT = { P }` is the same as `NOT = { NOT = { P } }`, double negation cancels out.

## Logical scopes

Now let's use the table above and insert a `NOT = { P }` in the `...` part of the scopes:

| Statment | Some `F`/`T` | All `F` | All `T`
|---|---|---|---|
| `AND = { NOT = { P } }` | `F` | `F` | `T` |
| `OR = { NOT = { P } }` | `T` | `F` | `T` |
| `NOT = { AND = { NOT = { P } } }` | `T` | `T` | `F` |
| `NOT = { OR = { NOT = { P } } }` | `F` | `T` | `F` |

Huh? This is still true, but we might have forgotten that we're asking the final predicament results, rather than the internal composition.

| Statment | P is `F` | P is `T`
|---|---|---|
| `AND = { P }` | `F` | `T` |
| `OR = { P }` | `F` | `T` |
| `NOT = { AND = { P }` | `T` | `F` |
| `NOT = { OR = { P }` | `T` | `F` |

As discussed above, properties of All `T`/`F` converge with a single element. Let's add another predicate, call it `P1` and `P2`.

We'll denote `P1` and `P2` with a slash, first is `P1`, second is `P2`.

| Statment | `F`/`F` | `F`/`T` | `T`/`F` | `T`/`T` |
|---|---|---|---|---|
| `AND = { P1 P2 }` | `F` | `F` | `F` | `T` |
| `OR = { P1 P2 }` | `F` | `T` | `T` | `T` |
| `NOT = { AND = { P1 P2 } }` | `T` | `T` | `T` | `F` |
| `NOT = { OR = { P1 P2 } }` | `T` | `F` | `F` | `F` |

With careful observation, we can observe that double negating the results gives us back the original. This once again proves that double negation in our system will cancel itself out.

When the predicaments are inverted, denoted with `!P1` and `!P2`:

| Statment | `F`/`F` | `F`/`T` | `T`/`F` | `T`/`T` |
|---|---|---|---|---|
| `AND = { !P1 !P2 }` | `T` | `F` | `F` | `F` |
| `OR = { !P1 !P2 }` | `T` | `T` | `T` | `F` |
| `NOT = { AND = { !P1 !P2 } }` | `F` | `T` | `T` | `T` |
| `NOT = { OR = { !P1 !P2 } }` | `F` | `F` | `F` | `T` |

Remember, the header row is "inverted" too (and denotes the result of `P1` and `P2`), this then gets "inverted".

The complete table looks as follows:

| Statment | `F`/`F` | `F`/`T` | `T`/`F` | `T`/`T` |
|---|---|---|---|---|
| `AND = { P1 P2 }` | `F` | `F` | `F` | `T` |
| `OR = { P1 P2 }` | `F` | `T` | `T` | `T` |
| `NOT = { AND = { P1 P2 } }` | `T` | `T` | `T` | `F` |
| `NOT = { OR = { P1 P2 } }` | `T` | `F` | `F` | `F` |
| `AND = { !P1 !P2 }` | `T` | `F` | `F` | `F` |
| `OR = { !P1 !P2 }` | `T` | `T` | `T` | `F` |
| `NOT = { AND = { !P1 !P2 } }` | `F` | `T` | `T` | `T` |
| `NOT = { OR = { !P1 !P2 } }` | `F` | `F` | `F` | `T` |

We can notice a few similar cases:

| Statment | `F`/`F` | `F`/`T` | `T`/`F` | `T`/`T` |
|---|---|---|---|---|
| `AND = { P1 P2 }` | `F` | `F` | `F` | `T` |
| `NOT = { OR = { !P1 !P2 } }` | `F` | `F` | `F` | `T` |

| Statment | `F`/`F` | `F`/`T` | `T`/`F` | `T`/`T` |
|---|---|---|---|---|
| `OR = { P1 P2 }` | `F` | `T` | `T` | `T` |
| `NOT = { AND = { !P1 !P2 } }` | `F` | `T` | `T` | `T` |

| Statment | `F`/`F` | `F`/`T` | `T`/`F` | `T`/`T` |
|---|---|---|---|---|
| `NOT = { AND = { P1 P2 } }` | `T` | `T` | `T` | `F` |
| `OR = { !P1 !P2 }` | `T` | `T` | `T` | `F` |

| Statment | `F`/`F` | `F`/`T` | `T`/`F` | `T`/`T` |
|---|---|---|---|---|
| `NOT = { OR = { P1 P2 } }` | `T` | `F` | `F` | `F` |
| `AND = { !P1 !P2 }` | `T` | `F` | `F` | `F` |

We have that `AND = { P1 P2 }` is the same as `NOT = { OR = { !P1 !P2 } }`, let's see if that's true:

## Verifying

Assume `P1 = { exists = yes }` and `P2 = { tag = PRU }`, hence, let's construct the `OR` logical scope:

```
NOT = {
	OR = {
		NOT = { exists = yes }
		NOT = { tag = PRU }
	}
}
```

And for the `AND` scope:

```
AND = { exists = yes tag = PRU }
```

Let's do some induction then (`T` if the trigger returned `True`, `F` if it returned `False`):
```
NOT = { OR = { NOT = { F } NOT = { F } } } -> NOT = { OR = { T T } } -> NOT = { T } -> F
NOT = { OR = { NOT = { F } NOT = { T } } } -> NOT = { OR = { T F } } -> NOT = { T } -> F
NOT = { OR = { NOT = { T } NOT = { F } } } -> NOT = { OR = { F T } } -> NOT = { T } -> F
NOT = { OR = { NOT = { T } NOT = { T } } } -> NOT = { OR = { F F } } -> NOT = { F } -> T

AND = { F F } -> F
AND = { F T } -> F
AND = { T F } -> F
AND = { T T } -> T
```

Shocker! We were right, those are logically equivalent scopes!

## Notes

- State instance refers to an instance of a state, for example a "state definition" would be ENG_1, a state instance would be the English part of ENG_1, and another instance would be the Russian part of ENG_1 (if the state is split between the two).
- Five scopes exist: Country, State, Province, Pop and Rebels. The rebels scope acts like the country one, for most of the scripting triggers and effects. The tag for rebels is REB.
- Scripting `exists = no` is faster than doing `NOT = { exists = yes }`, except for triggers that can't take negation (such as `unit_has_leader`)

## Invalid trigger behaviour

Triggers for tags, cultures, governments, ideologies, pop types, satates, inventions, culture groups, or technologies that don't exist have the same behaviour of `always = no`. For example if a trigger `culture = FROMpashtun` was found, then it was a) likely a typo, but b) the game tries to find the `FROMpashtun` culture, finds none, returns false. For performance reasons we already know the result beforehand so we can just materialize it into a `always = no`.

List of triggers that get materialized into `always = no` when invalid:

- `has_pop_culture`
- `cultural_group`
- `culture`
- `primary_culture`
- `accepted_culture`
- `pop_majority_culture`
- `have_core_in`
- `culturan_union_of`
- `is_core`
- `owned_by`
- `tag`
- `casus_belli`
- `exists`
- `military_access`
- `stronger_army_than`
- `neighbour`
- `country_units_in_state`
- `units_in_province`
- `war_with`
- `in_sphere`
- `controlled_by`
- `truce_with`
- `is_sphere_leader_of`
- `constructing_cb`
- `vassal_of`
- `substate_of`
- `is_our_vassal`
- `this_culture_union`
- `alliance_with`
- `in_default`
- `industrial_score`
- `military_score`
- `is_possible_vassal`
- `diplomatic_influence`
- `relation`
- `tech_school`

Note that the following have special behaviour:

- `has_province_modifier`: Behaves as `always = no`
- `has_country_modifier`: Behaves as `always = yes`

## Order for variably named triggers

Triggers which depend on the name of an object (such as `artisans = 99`), have to be parsed in the following order (first match, break from the "matcher"). So if an invention is named `aaa` and a issue is named `aaa` the issue comes first.

- issue options
- issues
- reforms
- technologies
- inventions
- ideologies
- pop types
- commodities
