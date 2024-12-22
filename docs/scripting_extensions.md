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
- **Comments**: True if playing on Kat's PA Engine, False if not

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
