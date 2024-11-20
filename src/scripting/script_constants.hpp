#pragma once

#include <stdint.h>
#include <cstring>

#include "dcon_generated.hpp"
#include "bytecode_defs.hpp"

namespace effect {
	// flags
	constexpr inline uint16_t no_payload = 0x4000;
	constexpr inline uint16_t is_random_scope = 0x2000;
	constexpr inline uint16_t scope_has_limit = 0x1000;

	constexpr inline uint16_t code_mask = 0x0FFF;

#define EFFECT_BYTECODE_ELEMENT(code, name, arg) constexpr inline uint16_t name = code;
	EFFECT_BYTECODE_LIST
#undef EFFECT_BYTECODE_ELEMENT
	constexpr inline uint16_t first_scope_code = 0x01C7;

#define EFFECT_SCOPE_BYTECODE_ELEMENT(name, code) constexpr inline uint16_t name = first_scope_code + code;
	EFFECT_SCOPE_BYTECODE_LIST
#undef EFFECT_SCOPE_BYTECODE_ELEMENT
	constexpr inline uint16_t first_invalid_code = first_scope_code + 0x0045;

	inline constexpr int8_t data_sizes[] = {
		0, // none
#define EFFECT_BYTECODE_ELEMENT(code, name, arg) arg,
		EFFECT_BYTECODE_LIST
#undef EFFECT_BYTECODE_ELEMENT
	};
	static_assert(sizeof(data_sizes) == first_scope_code);

	inline int32_t get_effect_non_scope_payload_size(uint16_t const* data) {
		assert((data[0] & effect::code_mask) < effect::first_invalid_code);
		return effect::data_sizes[data[0] & effect::code_mask];
	}
	inline int32_t get_effect_scope_payload_size(uint16_t const* data) {
		return data[1];
	}
	inline int32_t get_generic_effect_payload_size(uint16_t const* data) {
		return (data[0] & effect::code_mask) >= first_scope_code ? get_effect_scope_payload_size(data)
		: get_effect_non_scope_payload_size(data);
	}
	inline int32_t effect_scope_data_payload(uint16_t code) {
		auto const masked_code = code & effect::code_mask;
		if((masked_code == effect::tag_scope) || (masked_code == effect::integer_scope) ||
		(masked_code == effect::pop_type_scope_nation) || (masked_code == effect::pop_type_scope_state) ||
		(masked_code == effect::pop_type_scope_province) || (masked_code == effect::region_scope) ||
		(masked_code == effect::region_proper_scope) || (masked_code == effect::random_scope) ||
		(masked_code == effect::random_by_modifier_scope) || (masked_code == effect::loop_bounded_scope))
			return 1 + ((code & effect::scope_has_limit) != 0);
		return 0 + ((code & effect::scope_has_limit) != 0);
	}
	inline bool effect_scope_has_single_member(uint16_t const* source) { // precondition: scope known to not be empty
		auto const data_offset = 2 + effect_scope_data_payload(source[0]);
		return get_effect_scope_payload_size(source) == data_offset + get_generic_effect_payload_size(source + data_offset);
	}

	template<typename T>
	uint32_t recurse_over_effects(uint16_t* source, T const& f) {
		f(source);
		assert((source[0] & effect::code_mask) < effect::first_invalid_code || (source[0] & effect::code_mask) == effect::code_mask);
		if((source[0] & effect::code_mask) >= effect::first_scope_code) {
			auto const source_size = 1 + effect::get_effect_scope_payload_size(source);
			if((source[0] & effect::code_mask) == effect::random_list_scope) {
				auto sub_units_start = source + 3; // [code] + [payload size] + [chances total] + [first sub effect chance]
				while(sub_units_start < source + source_size) {
					sub_units_start += 1 + recurse_over_effects(sub_units_start + 1, f); // each member preceeded by uint16_t
				}
			} else {
				auto sub_units_start = source + 2 + effect::effect_scope_data_payload(source[0]);
				while(sub_units_start < source + source_size) {
					sub_units_start += recurse_over_effects(sub_units_start, f);
				}
			}
			return source_size;
		} else {
			return 1 + effect::get_effect_non_scope_payload_size(source);
		}
	}

} // namespace effect

//
// TRIGGERS
//
namespace trigger {
	// flags
	constexpr inline uint16_t is_disjunctive_scope = 0x4000;
	constexpr inline uint16_t is_existence_scope = 0x2000;

	constexpr inline uint16_t association_mask = 0x7000;
	constexpr inline uint16_t association_eq = 0x1000;
	constexpr inline uint16_t association_gt = 0x2000;
	constexpr inline uint16_t association_ge = 0x3000;
	constexpr inline uint16_t association_lt = 0x4000;
	constexpr inline uint16_t association_le = 0x5000;
	constexpr inline uint16_t association_ne = 0x6000;

	constexpr inline uint16_t no_payload = 0x0800;
	constexpr inline uint16_t code_mask = 0x07FF;

#define TRIGGER_BYTECODE_ELEMENT(code, name, arg) constexpr inline uint16_t name = code;
	TRIGGER_BYTECODE_LIST
#undef TRIGGER_BYTECODE_ELEMENT
	constexpr inline uint16_t first_scope_code = 0x02F6;

#define TRIGGER_SCOPE_BYTECODE_ELEMENT(name, code) constexpr inline uint16_t name = first_scope_code + code;
	TRIGGER_SCOPE_BYTECODE_LIST
#undef TRIGGER_SCOPE_BYTECODE_ELEMENT
	constexpr inline uint16_t first_invalid_code = first_scope_code + 0x0036;
	constexpr inline uint16_t placeholder_not_scope = code_mask;

	inline constexpr int8_t data_sizes[] = {
		0, //none
#define TRIGGER_BYTECODE_ELEMENT(code, name, arg) arg,
		TRIGGER_BYTECODE_LIST
#undef TRIGGER_BYTECODE_ELEMENT
	};
	static_assert(sizeof(data_sizes) == first_scope_code);

enum class slot_contents { empty = 0, province = 1, state = 2, pop = 3, nation = 4, rebel = 5 };

	union payload {

		uint16_t value;
		int16_t signed_value;
		bool boolean_value;
		dcon::government_type_id gov_id;
		dcon::religion_id rel_id;
		dcon::commodity_id com_id;
		dcon::rebel_type_id reb_id;
		dcon::factory_type_id fac_id;
		dcon::ideology_id ideo_id;
		dcon::issue_id iss_id;
		dcon::issue_option_id opt_id;
		dcon::cb_type_id cb_id;
		dcon::pop_type_id popt_id;
		dcon::leader_trait_id lead_id;
		dcon::unit_type_id unit_id;
		dcon::modifier_id mod_id;
		dcon::culture_id cul_id;
		dcon::culture_group_id culgrp_id;
		dcon::national_identity_id tag_id;
		dcon::state_definition_id state_id;
		dcon::province_id prov_id;
		dcon::technology_id tech_id;
		dcon::invention_id invt_id;
		dcon::national_variable_id natv_id;
		dcon::national_flag_id natf_id;
		dcon::global_flag_id glob_id;
		dcon::national_event_id nev_id;
		dcon::provincial_event_id pev_id;
		dcon::trigger_key tr_id;
		dcon::crime_id crm_id;
		dcon::political_party_id par_id;
		dcon::reform_id ref_id;
		dcon::reform_option_id ropt_id;
		dcon::region_id reg_id;
		dcon::stored_trigger_id str_id;
		dcon::national_focus_id nf_id;
		dcon::provincial_flag_id provf_id;

		// variables::national_variable_tag nat_var;
		// variables::national_flag_tag nat_flag;
		// variables::global_variable_tag global_var;
		// events::event_tag event;
		// trigger_tag trigger;

	payload(payload const& i) noexcept : value(i.value) { }
	payload(uint16_t i) : value(i) { }
	payload(int16_t i) : signed_value(i) { }
		payload(bool i) {
			memset(this, 0, sizeof(payload));
			boolean_value = i;
		}
		payload(dcon::provincial_flag_id i) {
			memset(this, 0, sizeof(payload));
			provf_id = i;
		}
		payload(dcon::national_focus_id i) {
			memset(this, 0, sizeof(payload));
			nf_id = i;
		}
		payload(dcon::government_type_id i) {
			memset(this, 0, sizeof(payload));
			gov_id = i;
		}
		payload(dcon::region_id i) {
			memset(this, 0, sizeof(payload));
			reg_id = i;
		}
		payload(dcon::religion_id i) {
			memset(this, 0, sizeof(payload));
			rel_id = i;
		}
		payload(dcon::commodity_id i) {
			memset(this, 0, sizeof(payload));
			com_id = i;
		}
		payload(dcon::rebel_type_id i) {
			memset(this, 0, sizeof(payload));
			reb_id = i;
		}
		payload(dcon::factory_type_id i) {
			memset(this, 0, sizeof(payload));
			fac_id = i;
		}
		payload(dcon::ideology_id i) {
			memset(this, 0, sizeof(payload));
			ideo_id = i;
		}
		payload(dcon::issue_id i) {
			memset(this, 0, sizeof(payload));
			iss_id = i;
		}
		payload(dcon::issue_option_id i) {
			memset(this, 0, sizeof(payload));
			opt_id = i;
		}
		payload(dcon::cb_type_id i) {
			memset(this, 0, sizeof(payload));
			cb_id = i;
		}
		payload(dcon::pop_type_id i) {
			memset(this, 0, sizeof(payload));
			popt_id = i;
		}
		payload(dcon::leader_trait_id i) {
			memset(this, 0, sizeof(payload));
			lead_id = i;
		}
		payload(dcon::unit_type_id i) {
			memset(this, 0, sizeof(payload));
			unit_id = i;
		}
		payload(dcon::modifier_id i) {
			memset(this, 0, sizeof(payload));
			mod_id = i;
		}
		payload(dcon::culture_id i) {
			memset(this, 0, sizeof(payload));
			cul_id = i;
		}
		payload(dcon::culture_group_id i) {
			memset(this, 0, sizeof(payload));
			culgrp_id = i;
		}
		payload(dcon::national_identity_id i) {
			memset(this, 0, sizeof(payload));
			tag_id = i;
		}
		payload(dcon::state_definition_id i) {
			memset(this, 0, sizeof(payload));
			state_id = i;
		}
		payload(dcon::province_id i) {
			memset(this, 0, sizeof(payload));
			prov_id = i;
		}
		payload(dcon::technology_id i) {
			memset(this, 0, sizeof(payload));
			tech_id = i;
		}
		payload(dcon::invention_id i) {
			memset(this, 0, sizeof(payload));
			invt_id = i;
		}
		payload(dcon::national_variable_id i) {
			memset(this, 0, sizeof(payload));
			natv_id = i;
		}
		payload(dcon::national_flag_id i) {
			memset(this, 0, sizeof(payload));
			natf_id = i;
		}
		payload(dcon::global_flag_id i) {
			memset(this, 0, sizeof(payload));
			glob_id = i;
		}
		payload(dcon::national_event_id i) {
			memset(this, 0, sizeof(payload));
			nev_id = i;
		}
		payload(dcon::provincial_event_id i) {
			memset(this, 0, sizeof(payload));
			pev_id = i;
		}
		payload(dcon::trigger_key i) {
			memset(this, 0, sizeof(payload));
			tr_id = i;
		}
		payload(dcon::crime_id i) {
			memset(this, 0, sizeof(payload));
			crm_id = i;
		}
		payload(dcon::political_party_id i) {
			memset(this, 0, sizeof(payload));
			par_id = i;
		}
		payload(dcon::reform_id i) {
			memset(this, 0, sizeof(payload));
			ref_id = i;
		}
		payload(dcon::reform_option_id i) {
			memset(this, 0, sizeof(payload));
			ropt_id = i;
		}
		payload(dcon::stored_trigger_id i) {
			memset(this, 0, sizeof(payload));
			str_id = i;
		}
	};

	static_assert(sizeof(payload) == 2);

	inline int32_t get_trigger_non_scope_payload_size(uint16_t const* data) {
		assert((data[0] & trigger::code_mask) < trigger::first_scope_code);
		return trigger::data_sizes[data[0] & trigger::code_mask];
	}
	inline int32_t get_trigger_scope_payload_size(uint16_t const* data) {
		return data[1];
	}
	inline int32_t get_trigger_payload_size(uint16_t const* data) {
		if((data[0] & trigger::code_mask) >= trigger::first_scope_code)
		return get_trigger_scope_payload_size(data);
		else
		return get_trigger_non_scope_payload_size(data);
	}
	inline int32_t trigger_scope_data_payload(uint16_t code) {
		auto const masked_code = code & trigger::code_mask;
		if((masked_code == trigger::x_provinces_in_variable_region) || (masked_code == trigger::x_provinces_in_variable_region_proper) || (masked_code == trigger::tag_scope) ||
			(masked_code == trigger::integer_scope))
		return 1;
		return 0;
	}

	template<typename T>
	uint16_t* recurse_over_triggers(uint16_t* source, T const& f) {
		f(source);
		assert((source[0] & trigger::code_mask) < trigger::first_invalid_code || (source[0] & trigger::code_mask) == trigger::code_mask);

		if((source[0] & trigger::code_mask) >= trigger::first_scope_code) {
			auto const source_size = 1 + get_trigger_scope_payload_size(source);

			auto sub_units_start = source + 2 + trigger_scope_data_payload(source[0]);
			while(sub_units_start < source + source_size) {
				sub_units_start = recurse_over_triggers(sub_units_start, f);
			}
			return source + source_size;
		} else {
			return source + 1 + get_trigger_non_scope_payload_size(source);
		}
	}

	inline uint32_t count_subtriggers(uint16_t const* source) {
		uint32_t count = 0;
		if((source[0] & trigger::code_mask) >= trigger::first_scope_code) {
			auto const source_size = 1 + get_trigger_scope_payload_size(source);
			auto sub_units_start = source + 2 + trigger_scope_data_payload(source[0]);
			while(sub_units_start < source + source_size) {
				++count;
				sub_units_start += 1 + get_trigger_payload_size(sub_units_start);
			}
		}
		return count;
	}

} // namespace trigger
