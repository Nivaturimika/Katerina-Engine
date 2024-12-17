# Scripting

## Simple effects

Effects that take a single parameter.

### `capital = <province id>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `add_core = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

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
- **Scopes**: Country
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
- **Comment**: N/A

### `overlord = { ... }`

```
overlord = {
	...
}
```

- **Version**: Base
- **Scopes**: Country
- **Comment**: N/A

### `sphere_owner = { ... }`

```
sphere_owner = {
	...
}
```

- **Version**: Base
- **Scopes**: Country
- **Comment**: N/A

### `independence = { ... }`

```
independence = {
	...
}
```

- **Version**: Base
- **Scopes**: Country
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
- **Comment**: N/A

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
- **Comment**: Seed is per-event and per-decision

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
- **Comment**: Seed is per-event and per-decision

### `clear_news_scopes = { ... }`

```
clear_news_scopes = {
	limit = { ... }
	type = <scope type>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: Will clear news scopes of the given type

### `[TAG] = { ... }`

```
[TAG] = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

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
- **Comment**: N/A

## Broken effects

### `guarentee = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Country
- **Comment**: Only shows a tooltip, no known in-game effect

### `fow = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Only shows a tooltip, no known in-game effect

# Scripting (triggers)

## Simple triggers

### `total_sunk_by_us = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

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
- **Scopes**: Any
- **Comment**: N/A

### `port = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: Has path to capital and has a naval base

### `rank = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

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
- **Comment**: N/A

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
- **Comment**: Factor `<n>` is % of infamy relative to the infamy limit

### `exists = <THIS/FROM/TAG/yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `region = <region name>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: As defined in `regions.txt`

### `strata = <poor/middle/rich>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `capital = <province id>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

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
- **Comment**: N/A

### `controls = <province id>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `election = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `is_slave = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

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
- **Comment**: Case sensitive

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
- **Comment**: Gives the `<target>` country a casus belli against the country in scope

### `add_casus_belli = <THIS/FROM/TAG>`

- **Version**: Base
- **Scopes**: Country
- **Comment**: Gives the country in scope a casus belli against another

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

### `tech_school = <tech school>`

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
- **Comment**: Only checks for country units, rebels are ignored.

### `unit_has_leader = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

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
- **Comment**: N/A

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

### `social_movement = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

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

### `political_movement = <yes/no>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

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
- **Comment**: N/A

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
- **Comment**: N/A

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
- **Scopes**: Any
- **Comment**: N/A
### `news_printing_count = <n>`

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

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
- **Scopes**: Any
- **Comment**: N/A

### `can_build_fort_in_capital = { ... }`

```
can_build_fort_in_capital = {
	in_whole_capital_state = <yes/no>
	limit_to_world_greatest_level = <yes/no>
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `work_available = { ... }`

```
work_available = {
	worker = <pop type>
}
```

- **Version**: Base
- **Scopes**: Any
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
- **Comment**: N/A

### `OR = { ... }`

```
OR = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

### `NOT = { ... }`

```
NOT = {
	...
}
```

- **Version**: Base
- **Scopes**: Any
- **Comment**: N/A

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
- **Scopes**: Any
- **Comment**: N/A

### `state_scope = { ... }`

```
state_scope = {
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

# Scripting (extended effects)

All scopes (that can hold effects) can use `limit`, unless otherwise specified.

### `add_or_create_pop = { ... }`

```
add_or_create_pop = {
	size = <n>
	type = <pop type>
	culture = <culture>
	religion = <religion>
}
```

- **Version**: 0.1.0
- **Scopes**: Province
- **Comments**: Adds or creates a given POP with the defined type/culture/religion. If it already exists then this effect acts the same as `change_pop_size = <n>`.

### `trigger_crisis = { ... }`

```
trigger_crisis = {
	overwrite = yes/no
	type = claim/liberation/colonial/influence
	liberation_tag = TAG/THIS/FROM
	colony = State/THIS/FROM
}
```

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: If the colony specified is on the ownership of a nation that is a great power, said nation becomes the primary defender automatically.

### `change_party_name = { ... }`

```
change_party_name = {
	name = <name>
	ideology = <ideology/ruling_party>
}
```

- **Version**: 0.1.0
- **Scopes**: Country
- **Comments**: This effect is defined using a block with two members. The first member is `ideology = ...` where `...` is either the name of an ideology or `ruling_party`. This member determines which party the effect will apply to. If an ideology is provided, it will apply to the first active political party with that ideology (generally the "first" party is the political party defined earlier in the files). If `ruling_party` is provided, the current ruling party of the nation in scope will be affected. The second member is `name = ...`. This value is the new name that the party will be given. This effect can be used only within a national scope.

### `change_party_position = { ... }`

```
change_party_position = {
	position = <issue>
	ideology = <ideology/ruling_party>
}
```

- **Version**: 0.1.0
- **Scopes**: Country
- **Comments**: This effect is defined using a block with two members. The first member is `ideology = ...` where `...` is either the name of an ideology or `ruling_party`, which determines which party the effect applies to as described above. The second member is `position = ...` where `...` is one of the options for a party issue or a political or social reform. The effect will cause the political party determined by the `ideology` member to change their party position to the given `position`, if it is a party issue or it will give them a voting preference in favor of that particular reform and remove any voting preference in favor of another reform in that group (see below for more on the extension of party preferences). This effect can be used only within a national scope.

### `build_[building type]_in_capital = { ... }`

```
build_[building type]_in_capital = {
	in_whole_capital_state = yes/no
	limit_to_world_greatest_level = yes/no
}
```

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: The game doesn't allow custom defined buildings to be used in this mode as an effect. But hence syntax for `build_bank_in_capital` and it's university counterpart, `build_university_in_capital`, which is also the same.

### `loop_bounded = { ... }`

```
loop_bounded = {
	loop_iterations = <n>
	...
}
```

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Loops over the effects `<n>` times.

### `random_by_modifier = { ... }`

```
random_by_modifier = {
	chance = {
		base = <n>
		factor = { ... }
		...
	}
	...
}
```

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Randomly executes the effects inside with weights given by the chance modifier

### `custom_tooltip = <text key>`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: N/A

### `remove_crisis = <yes/no>`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Removes the current active crisis (if any)

### `custom_tooltip = { ... }`

```
custom_tooltip = {
	x = <variable>
	y = <variable>
	text = <text key>
}
```

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: N/A

### `increment_variable = <variable>`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Shorthand to increment by 1

### `decrement_variable = <variable>`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Shorthand to decrement by 1

### `set_variable_to_zero = <variable>`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Shorthand to set a variable to 0

### `ruling_party_ideology = <THIS/FROM>`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Appoints the ruling party with an ideology of `THIS` or `FROM`

### `add_accepted_culture = <THIS/FROM/this_union/from_union>`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Now with `THIS/FROM` adds the PRIMARY culture of `THIS/FROM` to the nation in scope. For `this_union/from_union` it adds the entire culture union of the primary culture as accepted to the nation in scope.

### `annex_to = <null/--->`

- **Version**: 0.1.0
- **Scopes**: Country
- **Comments**: Uncolonizes an entire country, this turns all the provinces owned by the nation in scope into unowned provinces (which defeats the nation, liberates its puppets, etc). The syntax `annex_to = ---` is supported but discouraged.

### `secede_province = <null/--->`

- **Version**: 0.1.0
- **Scopes**: Province
- **Comments**: Turns the province in scope into an unowned province. This is to replace some of the tricks mods did to turn provinces unowned, such as seceding them to nonexistent tags, etc. The syntax `annex_to = ---` is supported but discouraged.

### `random_greater_power = { ... }`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Like `any_greater_power`, but only one random element that matches the limit (if any).

### `any_empty_neighbor_province = { ... }`

- **Version**: 0.1.0
- **Scopes**: Province
- **Comments**: Like `random_empty_neighbor_province`, but only one random element that matches the limit (if any).

### `change_terrain = <terrain>`

- **Version**: 0.1.0
- **Scopes**: Province, Pop
- **Comments**: Changes the terrain of the province on scope, can be used on pop scopes too (will default to the location of the pop)

### `any_existing_country_except_scoped = { ... }`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Same behaviour of `any_country` on decisions, any existing nation except the one scoped

### `any_defined_country = { ... }`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Same behaviour of `any_country` on events, scope all countries even those that don't exist and includes the current country

### `random_neighbor_country = { ... }`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: N/A

### `all_core = { ... }`

- **Version**: 0.1.0
- **Scopes**: Country, Province
- **Comments**: The ability to use it within provinces to scope all the cores of the province in question, for example if England and Russia have cores on Houston, then this will scope both England and Russia

### `any_core = { ... }`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Effects dont have `any_core`, but triggers do, so for consistency its supported too.

### `from_bounce = { ... }`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Forward whatever is in the current scope to be "bounced" to `FROM`, until the end of this scope

### `this_bounce = { ... }`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Forward whatever is in the current scope to be "bounced" to `THIS`, until the end of this scope

### `province_immigrator = <n>`

- **Version**: 0.1.0
- **Scopes**: Province
- **Comments**: Can only be `1` or `-1`. This is the "province selector" used in various mods.

### `immigrator = <n>`

- **Version**: 0.1.0
- **Scopes**: Province
- **Comments**: Can only be `1` or `-1`. This is the "province selector" used in various mods.

### `immigrator_selector = <n>`

- **Version**: 0.1.0
- **Scopes**: Province
- **Comments**: Can only be `1` or `-1`. This is the "province selector" used in various mods.

### `if = { ... }`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: See [extensions.md](extensions.md) for full usage explaination

### `else_if = { ... }`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: See [extensions.md](extensions.md) for full usage explaination

### `else = { ... }`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: See [extensions.md](extensions.md) for full usage explaination

### `add_country_modifier = <modifier>`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Shorthand for `add_country_modifier = { name = modifier duration = -1 }`

### `add_province_modifier = <modifier>`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Shorthand for `add_province_modifier = { name = modifier duration = -1 }`

### `masquerade_as_nation = <THIS/FROM>`

- **Version**: 0.1.0
- **Scopes**: Country
- **Comments**: Use the visual flag of `THIS/FROM` as the flag of the nation, very useful for dynamic rebellions.

### `set_province_flag = <flag>`

- **Version**: 0.1.0
- **Scopes**: Province
- **Comments**: Didn't work in the original but now it does

### `clr_province_flag = <flag>`

- **Version**: 0.1.0
- **Scopes**: Province
- **Comments**: Didn't work in the original but now it does

# Scripting (extended triggers)

### `test = <scripted trigger>`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Evaluates a scripted trigger from `scripted triggers/`, see [extensions.md](extensions.md) for more usage info

### `any_country = { ... }`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Tests whether any existent country satisfies the conditions given in `...`. This is essentially just mirroring how the existing `any_country` effect scope works but for trigger conditions.

### `every_country = { ... }`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Like `any_country`, but truly applies to EVERY country, even those who do not exist.

### `has_global_flag = katerina_engine`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: True if playing on Katerina Engine, False if not

### `all_war_countries = { ... }`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: All countries we're at war with MUST fullfill the condition, as opposed to `war_countries` were only one country has to fullfill the condition

### `all_state = { ... }`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: All states must fulfill condition, similar to `any_state`

### `all_substate = { ... }`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: All substates must fulfill condition, similar to `any_substate`

### `all_sphere_member = { ... }`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: All sphere members must fulfill condition, similar to `any_sphere_member`

### `all_pop = { ... }`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: All POPs must fulfill condition, similar to `any_pop`

### `all_greater_power = { ... }`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: All greater powers must fullfill condition.

### `has_national_focus = <focus>`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Checks if a state (or province) in scope has a focus `focus`.

### `have_core_in = <THIS/FROM/TAG>`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Doesn't work in vanilla, but will work here.

### `treasury = <n>`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Alias of `money`.

### `has_province_flag = <flag>`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Didn't work in the original but now it does

### `crisis_attacker_scope = { ... }`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Scopes into the primary attacker of a crisis

### `crisis_defender_scope = { ... }`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Scopes into the primary defender of a crisis

### `is_overseas_culture = <yes/no>`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Checks if the pop in scope pertains to a culture that is overseas.

### `is_banned_from = <THIS/FROM/TAG>`

- **Version**: 0.1.0
- **Scopes**: Any
- **Comments**: Checks if the embassy of the country on scope is banned on the given country.
