#include "system_state.hpp"
#include "gui_effect_tooltips.hpp"
#include "dcon_generated.hpp"
#include "gui_event.hpp"
#include "gui_modifier_tooltips.hpp"
#include "gui_trigger_tooltips.hpp"
#include "prng.hpp"
#include "script_constants.hpp"
#include "text.hpp"
#include "triggers.hpp"

namespace ui {

	namespace effect_tooltip {

		inline constexpr int32_t indentation_amount = 15;

		#define EFFECT_DISPLAY_PARAMS                                                                                                    \
		sys::state &ws, uint16_t const *tval, text::layout_base &layout, int32_t primary_slot, int32_t this_slot, int32_t from_slot,   \
			uint32_t r_lo, uint32_t r_hi, int32_t indentation

		uint32_t internal_make_effect_description(EFFECT_DISPLAY_PARAMS);


		void add_adj_this_to_map(sys::state& ws, text::substitution_map& map, dcon::nation_id n) {
			static std::string this_nation = text::produce_simple_string(ws, "this_nation");
			if(n) {
				text::add_to_substitution_map(map, text::variable_type::adj, text::get_adjective(ws, n));
			} else {
				text::add_to_substitution_map(map, text::variable_type::adj, std::string_view{ this_nation });
			}
		}
		void add_adj_this_to_map(sys::state& ws, text::substitution_map& map, dcon::province_id p) {
			static std::string this_nation = text::produce_simple_string(ws, "this_nation");
			if(auto n = ws.world.province_get_nation_from_province_ownership(p); n) {
				text::add_to_substitution_map(map, text::variable_type::adj, text::get_adjective(ws, n));
			} else {
				text::add_to_substitution_map(map, text::variable_type::adj, std::string_view{ this_nation });
			}
		}
		void add_adj_this_to_map(sys::state& ws, text::substitution_map& map, dcon::state_instance_id p) {
			static std::string this_nation = text::produce_simple_string(ws, "this_nation");
			if(auto n = ws.world.state_instance_get_nation_from_state_ownership(p); n) {
				text::add_to_substitution_map(map, text::variable_type::adj, text::get_adjective(ws, n));
			} else {
				text::add_to_substitution_map(map, text::variable_type::adj, std::string_view{ this_nation });
			}
		}
		void add_adj_this_to_map(sys::state& ws, text::substitution_map& map, dcon::pop_id p) {
			static std::string this_nation = text::produce_simple_string(ws, "this_nation");
			if(auto n = nations::owner_of_pop(ws, p); n) {
				text::add_to_substitution_map(map, text::variable_type::adj, text::get_adjective(ws, n));
			} else {
				text::add_to_substitution_map(map, text::variable_type::adj, std::string_view{ this_nation });
			}
		}
		void add_adj_from_to_map(sys::state& ws, text::substitution_map& map, dcon::rebel_faction_id p) {
			static std::string this_nation = text::produce_simple_string(ws, "from_nation");
			auto fp = fatten(ws.world, p);
			if(auto n = fp.get_defection_target().get_nation_from_identity_holder(); n) {
				text::add_to_substitution_map(map, text::variable_type::adj, text::get_adjective(ws, n));
			} else {
				text::add_to_substitution_map(map, text::variable_type::adj, std::string_view{ this_nation });
			}
		}
		void add_adj_from_to_map(sys::state& ws, text::substitution_map& map, dcon::nation_id n) {
			static std::string this_nation = text::produce_simple_string(ws, "from_nation");
			if(n) {
				text::add_to_substitution_map(map, text::variable_type::adj, text::get_adjective(ws, n));
			} else {
				text::add_to_substitution_map(map, text::variable_type::adj, std::string_view{ this_nation });
			}
		}
		void add_adj_from_to_map(sys::state& ws, text::substitution_map& map, dcon::province_id p) {
			static std::string this_nation = text::produce_simple_string(ws, "from_nation");
			if(auto n = ws.world.province_get_nation_from_province_ownership(p); n) {
				text::add_to_substitution_map(map, text::variable_type::adj, text::get_adjective(ws, n));
			} else {
				text::add_to_substitution_map(map, text::variable_type::adj, std::string_view{ this_nation });
			}
		}

		inline uint32_t display_subeffects(EFFECT_DISPLAY_PARAMS) {
			auto const source_size = 1 + effect::get_effect_scope_payload_size(tval);
			auto sub_units_start = tval + 2 + effect::effect_scope_data_payload(tval[0]);
			uint32_t i = 0;
			while(sub_units_start < tval + source_size) {
				i += internal_make_effect_description(ws, sub_units_start, layout, primary_slot, this_slot, from_slot, r_hi, r_lo + i, indentation);
				sub_units_start += 1 + effect::get_generic_effect_payload_size(sub_units_start);
			}
			return i;
		}

		void display_value(int64_t value, bool positive_is_green, sys::state& ws, text::layout_base& layout, text::layout_box& box) {
			if(value >= 0) {
				text::add_to_layout_box(ws, layout, box, "+", positive_is_green ? text::text_color::green : text::text_color::red, std::monostate{});
			}
			if(positive_is_green) {
				text::add_to_layout_box(ws, layout, box, value, value >= 0 ? text::text_color::green : text::text_color::red);
			} else {
				text::add_to_layout_box(ws, layout, box, value, value >= 0 ? text::text_color::red : text::text_color::green);
			}
			text::add_space_to_layout_box(ws, layout, box);
		}
		void display_value(text::fp_currency value, bool positive_is_green, sys::state& ws, text::layout_base& layout, text::layout_box& box) {
			if(value.value >= 0) {
				text::add_to_layout_box(ws, layout, box, "+", positive_is_green ? text::text_color::green : text::text_color::red, std::monostate{});
			}
			if(positive_is_green) {
				text::add_to_layout_box(ws, layout, box, value, value.value >= 0 ? text::text_color::green : text::text_color::red);
			} else {
				text::add_to_layout_box(ws, layout, box, value, value.value >= 0 ? text::text_color::red : text::text_color::green);
			}
			text::add_space_to_layout_box(ws, layout, box);
		}
		void display_value(text::fp_percentage value, bool positive_is_green, sys::state& ws, text::layout_base& layout, text::layout_box& box) {
			if(value.value >= 0) {
				text::add_to_layout_box(ws, layout, box, "+", positive_is_green ? text::text_color::green : text::text_color::red, std::monostate{});
			}
			if(positive_is_green) {
				text::add_to_layout_box(ws, layout, box, value, value.value >= 0 ? text::text_color::green : text::text_color::red);
			} else {
				text::add_to_layout_box(ws, layout, box, value, value.value >= 0 ? text::text_color::red : text::text_color::green);
			}
			text::add_space_to_layout_box(ws, layout, box);
		}
		void display_value(text::fp_one_place value, bool positive_is_green, sys::state& ws, text::layout_base& layout, text::layout_box& box) {
			if(value.value >= 0) {
				text::add_to_layout_box(ws, layout, box, "+", positive_is_green ? text::text_color::green : text::text_color::red, std::monostate{});
			}
			if(positive_is_green) {
				text::add_to_layout_box(ws, layout, box, value, value.value >= 0 ? text::text_color::green : text::text_color::red);
			} else {
				text::add_to_layout_box(ws, layout, box, value, value.value >= 0 ? text::text_color::red : text::text_color::green);
			}
			text::add_space_to_layout_box(ws, layout, box);
		}
		void display_value(text::fp_two_places value, bool positive_is_green, sys::state& ws, text::layout_base& layout, text::layout_box& box) {
			if(value.value >= 0) {
				text::add_to_layout_box(ws, layout, box, "+", positive_is_green ? text::text_color::green : text::text_color::red, std::monostate{});
			}
			if(positive_is_green) {
				text::add_to_layout_box(ws, layout, box, value, value.value >= 0 ? text::text_color::green : text::text_color::red);
			} else {
				text::add_to_layout_box(ws, layout, box, value, value.value >= 0 ? text::text_color::red : text::text_color::green);
			}
			text::add_space_to_layout_box(ws, layout, box);
		}
		void display_value(text::fp_three_places value, bool positive_is_green, sys::state& ws, text::layout_base& layout, text::layout_box& box) {
			if(value.value >= 0) {
				text::add_to_layout_box(ws, layout, box, "+", positive_is_green ? text::text_color::green : text::text_color::red, std::monostate{});
			}
			if(positive_is_green) {
				text::add_to_layout_box(ws, layout, box, value, value.value >= 0 ? text::text_color::green : text::text_color::red);
			} else {
				text::add_to_layout_box(ws, layout, box, value, value.value >= 0 ? text::text_color::red : text::text_color::green);
			}
			text::add_space_to_layout_box(ws, layout, box);
		}

		void tag_type_this_nation_effect(int32_t this_slot, sys::state& ws, text::layout_base& layout, text::layout_box& box) {
			text::add_to_layout_box(ws, layout, box,
			this_slot != -1 ? text::produce_simple_string(ws, text::get_name(ws, trigger::to_nation(this_slot)))
											: text::produce_simple_string(ws, "this_nation"));
		}
		void tag_type_this_state_effect(int32_t this_slot, sys::state& ws, text::layout_base& layout, text::layout_box& box) {
			text::add_to_layout_box(ws, layout, box,
			this_slot != -1
					? text::produce_simple_string(ws,
						text::get_name(ws, ws.world.state_instance_get_nation_from_state_ownership(trigger::to_state(this_slot))))
					: text::produce_simple_string(ws, "this_nation"));
		}
		void tag_type_this_province_effect(int32_t this_slot, sys::state& ws, text::layout_base& layout, text::layout_box& box) {
			text::add_to_layout_box(ws, layout, box,
			this_slot != -1
					? text::produce_simple_string(ws,
						text::get_name(ws, ws.world.province_get_nation_from_province_ownership(trigger::to_prov(this_slot))))
					: text::produce_simple_string(ws, "this_nation"));
		}
		void tag_type_this_pop_effect(int32_t this_slot, sys::state& ws, text::layout_base& layout, text::layout_box& box) {
			text::add_to_layout_box(ws, layout, box,
			this_slot != -1
					? text::produce_simple_string(ws, text::get_name(ws, nations::owner_of_pop(ws, trigger::to_pop(this_slot))))
					: text::produce_simple_string(ws, "this_nation"));
		}
		void tag_type_from_nation_effect(int32_t this_slot, sys::state& ws, text::layout_base& layout, text::layout_box& box) {
			text::add_to_layout_box(ws, layout, box,
			this_slot != -1 ? text::produce_simple_string(ws, text::get_name(ws, trigger::to_nation(this_slot)))
											: text::produce_simple_string(ws, "from_nation"));
		}
		void tag_type_from_province_effect(int32_t this_slot, sys::state& ws, text::layout_base& layout, text::layout_box& box) {
			text::add_to_layout_box(ws, layout, box,
			this_slot != -1
					? text::produce_simple_string(ws,
						text::get_name(ws, ws.world.province_get_nation_from_province_ownership(trigger::to_prov(this_slot))))
					: text::produce_simple_string(ws, "from_nation"));
		}

		uint32_t es_generic_scope(EFFECT_DISPLAY_PARAMS) {
			return display_subeffects(ws, tval, layout, primary_slot, this_slot, from_slot, r_lo, r_hi, indentation);
		}

		inline auto random_or_every(uint16_t tval) {
			return (tval & effect::is_random_scope) != 0 ? "random" : "every";
		}

		void show_limit(sys::state& ws, uint16_t const* tval, text::layout_base& layout, int32_t primary_slot, int32_t this_slot, int32_t from_slot,
		int32_t indentation) {
			if((tval[0] & effect::scope_has_limit) != 0) {
				auto limit = trigger::payload(tval[2]).tr_id;
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "where"));
				text::close_layout_box(layout, box);
				trigger_tooltip::make_trigger_description(ws, layout, ws.trigger_data.data() + ws.trigger_data_indices[limit.index() + 1], primary_slot,
				this_slot, from_slot, indentation + 2 * indentation_amount, primary_slot != -1);
			}
		}

		uint32_t es_if_scope(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "et_if"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, primary_slot, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout, primary_slot, this_slot, from_slot, r_lo, r_hi, indentation + 1);
		}
		uint32_t es_else_if_scope(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "et_else_if"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, primary_slot, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout, primary_slot, this_slot, from_slot, r_lo, r_hi, indentation + 1);
		}

		uint32_t es_x_neighbor_province_scope(EFFECT_DISPLAY_PARAMS) {
			if((tval[0] & effect::is_random_scope) != 0) {
				if(primary_slot != -1) {
					auto neighbor_range = ws.world.province_get_province_adjacency(trigger::to_prov(primary_slot));

					std::vector<dcon::province_id> rlist;

					if((tval[0] & effect::scope_has_limit) != 0) {
						auto limit = trigger::payload(tval[2]).tr_id;
						for(auto p : neighbor_range) {
							auto other = p.get_connected_provinces(p.get_connected_provinces(0) == trigger::to_prov(primary_slot) ? 1 : 0);
							if(other.get_nation_from_province_ownership() && trigger::evaluate(ws, limit, trigger::to_generic(other.id), this_slot, from_slot)) {
								rlist.push_back(other.id);
							}
						}
					} else {
						for(auto p : neighbor_range) {
							auto other = p.get_connected_provinces(p.get_connected_provinces(0) == trigger::to_prov(primary_slot) ? 1 : 0);
							if(other.get_nation_from_province_ownership()) {
								rlist.push_back(other.id);
							}
						}
					}

					if(rlist.size() != 0) {
						auto r = uint32_t(rng::get_random(ws, r_hi, r_lo) % rlist.size());
						auto box = text::open_layout_box(layout, indentation);
						text::add_to_layout_box(ws, layout, box, rlist[r]);
						text::close_layout_box(layout, box);
						show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
						return 1 + display_subeffects(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, r_hi, r_lo + 1, indentation + indentation_amount);
					}
					return 0;
				}
			}

			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "neighboring_province"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo,
																																	r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0),
																																	indentation + indentation_amount);
		}
		uint32_t es_x_neighbor_country_scope(EFFECT_DISPLAY_PARAMS) {
			if((tval[0] & effect::is_random_scope) != 0) {
				if(primary_slot != -1) {
					auto neighbor_range = ws.world.nation_get_nation_adjacency(trigger::to_nation(primary_slot));
					std::vector<dcon::nation_id> rlist;

					if((tval[0] & effect::scope_has_limit) != 0) {
						auto limit = trigger::payload(tval[2]).tr_id;
						for(auto p : neighbor_range) {
							auto other = p.get_connected_nations(0) == trigger::to_nation(primary_slot) ? p.get_connected_nations(1)
							: p.get_connected_nations(0);
							if(trigger::evaluate(ws, limit, trigger::to_generic(other.id), this_slot, from_slot)) {
								rlist.push_back(other.id);
							}
						}
					} else {
						for(auto p : neighbor_range) {
							auto other = p.get_connected_nations(0) == trigger::to_nation(primary_slot) ? p.get_connected_nations(1)
							: p.get_connected_nations(0);
							rlist.push_back(other.id);
						}
					}

					if(rlist.size() != 0) {
						auto r = uint32_t(rng::get_random(ws, r_hi, r_lo) % rlist.size());
						auto box = text::open_layout_box(layout, indentation);
						text::add_to_layout_box(ws, layout, box, rlist[r]);
						text::close_layout_box(layout, box);
						show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
						return 1 + display_subeffects(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, r_hi, r_lo + 1, indentation + indentation_amount);
					}
					return 0;
				}
			}
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "neighboring_nation"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo,
			r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0),
			indentation + indentation_amount);
		}
		uint32_t es_x_country_scope_nation(EFFECT_DISPLAY_PARAMS) {
			if((tval[0] & effect::is_random_scope) != 0) {
				std::vector<dcon::nation_id> rlist;
				if((tval[0] & effect::scope_has_limit) != 0) {
					auto limit = trigger::payload(tval[2]).tr_id;
					for(auto n : ws.world.in_nation) {
						if(n.get_owned_province_count() != 0 && trigger::evaluate(ws, limit, trigger::to_generic(n.id), this_slot, from_slot))
						rlist.push_back(n.id);
					}
				} else {
					for(auto n : ws.world.in_nation) {
						if(n.get_owned_province_count() != 0)
						rlist.push_back(n.id);
					}
				}
				if(rlist.size() != 0) {
					auto r = uint32_t(rng::get_random(ws, r_hi, r_lo) % rlist.size());
					auto box = text::open_layout_box(layout, indentation);
					text::add_to_layout_box(ws, layout, box, rlist[r]);
					text::close_layout_box(layout, box);
					show_limit(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, indentation);
					return 1 + display_subeffects(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, r_hi, r_lo + 1, indentation + indentation_amount);
				}
				return 0;
			}
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "nation"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo,
			r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0),
			indentation + indentation_amount);
		}
		uint32_t es_x_event_country_scope_nation(EFFECT_DISPLAY_PARAMS) {
			if((tval[0] & effect::is_random_scope) != 0) {
				std::vector<dcon::nation_id> rlist;
				if((tval[0] & effect::scope_has_limit) != 0) {
					auto limit = trigger::payload(tval[2]).tr_id;
					for(auto n : ws.world.in_nation) {
						if(n != trigger::to_nation(primary_slot) && trigger::evaluate(ws, limit, trigger::to_generic(n.id), this_slot, from_slot))
						rlist.push_back(n.id);
					}
				} else {
					for(auto n : ws.world.in_nation) {
						if(n != trigger::to_nation(primary_slot))
						rlist.push_back(n.id);
					}
				}
				if(rlist.size() != 0) {
					auto r = uint32_t(rng::get_random(ws, r_hi, r_lo) % rlist.size());
					auto box = text::open_layout_box(layout, indentation);
					text::add_to_layout_box(ws, layout, box, rlist[r]);
					text::close_layout_box(layout, box);
					show_limit(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, indentation);
					return 1 + display_subeffects(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, r_hi, r_lo + 1,
					indentation + indentation_amount);
				}
				return 0;
			}
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "nation"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo,
			r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0),
			indentation + indentation_amount);
		}
		uint32_t es_x_decision_country_scope_nation(EFFECT_DISPLAY_PARAMS) {
			if((tval[0] & effect::is_random_scope) != 0) {
				std::vector<dcon::nation_id> rlist;
				if((tval[0] & effect::scope_has_limit) != 0) {
					auto limit = trigger::payload(tval[2]).tr_id;
					for(auto n : ws.world.in_nation) {
						if(n != trigger::to_nation(primary_slot) && n.get_owned_province_count() != 0 && trigger::evaluate(ws, limit, trigger::to_generic(n.id), this_slot, from_slot))
						rlist.push_back(n.id);
					}
				} else {
					for(auto n : ws.world.in_nation) {
						if(n != trigger::to_nation(primary_slot) && n.get_owned_province_count() != 0)
						rlist.push_back(n.id);
					}
				}
				if(rlist.size() != 0) {
					auto r = uint32_t(rng::get_random(ws, r_hi, r_lo) % rlist.size());
					auto box = text::open_layout_box(layout, indentation);
					text::add_to_layout_box(ws, layout, box, rlist[r]);
					text::close_layout_box(layout, box);
					show_limit(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, indentation);
					return 1 + display_subeffects(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, r_hi, r_lo + 1,
					indentation + indentation_amount);
				}
				return 0;
			}
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "nation"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo,
			r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0),
			indentation + indentation_amount);
		}
		uint32_t es_from_bounce_scope(EFFECT_DISPLAY_PARAMS) {
			//NOTE: The entire point is to bounce FROM, so the repeated primary_slot on from_slot is intentional
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "becomes_from"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, primary_slot, this_slot, primary_slot, indentation);
			return display_subeffects(ws, tval, layout, primary_slot, this_slot, primary_slot, r_lo, r_hi + 1, indentation + indentation_amount);
		}
		uint32_t es_this_bounce_scope(EFFECT_DISPLAY_PARAMS) {
			//See es_from_bounce_scope, similarly here, but for THIS
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "becomes_this"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, primary_slot, primary_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout, primary_slot, primary_slot, from_slot, r_lo, r_hi + 1, indentation + indentation_amount);
		}

		uint32_t es_loop_bounded_scope(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::x, text::pretty_integer{ int32_t(tval[2]) });
			text::localised_format_box(ws, layout, box, "repeats_x_times", m);
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, primary_slot, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout, primary_slot, this_slot, from_slot, r_lo, r_hi + 1, indentation + indentation_amount);
		}

		uint32_t es_x_country_scope(EFFECT_DISPLAY_PARAMS) {
		return es_x_country_scope_nation(ws, tval, layout, trigger::to_generic(dcon::nation_id{}), this_slot, from_slot, r_lo, r_hi, indentation);
		}
		uint32_t es_x_event_country_scope(EFFECT_DISPLAY_PARAMS) {
		return es_x_event_country_scope_nation(ws, tval, layout, trigger::to_generic(dcon::nation_id{}), this_slot, from_slot, r_hi, r_lo, indentation);
		}
		uint32_t es_x_decision_country_scope(EFFECT_DISPLAY_PARAMS) {
		return es_x_decision_country_scope_nation(ws, tval, layout, trigger::to_generic(dcon::nation_id{}), this_slot, from_slot, r_hi, r_lo, indentation);
		}
		uint32_t es_x_empty_neighbor_province_scope(EFFECT_DISPLAY_PARAMS) {
			if((tval[0] & effect::is_random_scope) != 0) {
				if(primary_slot != -1) {
					auto neighbor_range = ws.world.province_get_province_adjacency(trigger::to_prov(primary_slot));
					std::vector<dcon::province_id> rlist;

					if((tval[0] & effect::scope_has_limit) != 0) {
						auto limit = trigger::payload(tval[2]).tr_id;
						for(auto p : neighbor_range) {
							auto other = p.get_connected_provinces(p.get_connected_provinces(0) == trigger::to_prov(primary_slot) ? 1 : 0);
							if(!other.get_nation_from_province_ownership() &&
							trigger::evaluate(ws, limit, trigger::to_generic(other.id), this_slot, from_slot)) {
								rlist.push_back(other.id);
							}
						}
					} else {
						for(auto p : neighbor_range) {
							auto other = p.get_connected_provinces(p.get_connected_provinces(0) == trigger::to_prov(primary_slot) ? 1 : 0);
							if(!other.get_nation_from_province_ownership()) {
								rlist.push_back(other.id);
							}
						}
					}

					if(rlist.size() != 0) {
						auto r = uint32_t(rng::get_random(ws, r_hi, r_lo) % rlist.size());
						auto box = text::open_layout_box(layout, indentation);
						text::add_to_layout_box(ws, layout, box, rlist[r]);
						text::close_layout_box(layout, box);
						show_limit(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, indentation);
						return 1 + display_subeffects(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, r_hi, r_lo + 1,
											 indentation + indentation_amount);
					}
					return 0;
				}
			}
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "empty_neighboring_province"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo,
																																	r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0),
																																	indentation + indentation_amount);
		}
		uint32_t es_x_greater_power_scope(EFFECT_DISPLAY_PARAMS) {
			if((tval[0] & effect::is_random_scope) != 0) {
				std::vector<dcon::nation_id> rlist;
				if((tval[0] & effect::scope_has_limit) != 0) {
					auto limit = trigger::payload(tval[2]).tr_id;
					for(auto& n : ws.great_nations) {
						if(trigger::evaluate(ws, limit, trigger::to_generic(n.nation), this_slot, from_slot)) {
							rlist.push_back(n.nation);
						}
					}
				} else {
					for(auto& n : ws.great_nations) {
						rlist.push_back(n.nation);
					}
				}

				if(rlist.size() != 0) {
					auto r = uint32_t(rng::get_random(ws, r_hi, r_lo) % rlist.size());
					auto box = text::open_layout_box(layout, indentation);
					text::add_to_layout_box(ws, layout, box, rlist[r]);
					text::close_layout_box(layout, box);
					show_limit(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, indentation);
					return 1 + display_subeffects(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, r_hi, r_lo + 1,
										 indentation + indentation_amount);
				}
				return 0;
			}

			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "great_power"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo,
			r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0),
			indentation + indentation_amount);
		}
		uint32_t es_poor_strata_scope_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "poor_strata_pop"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);

			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo,
			r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0),
			indentation + indentation_amount);
		}
		uint32_t es_poor_strata_scope_state(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "poor_strata_pop"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo,
			r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0),
			indentation + indentation_amount);
		}
		uint32_t es_poor_strata_scope_province(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "poor_strata_pop"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo,
			r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0),
			indentation + indentation_amount);
		}
		uint32_t es_middle_strata_scope_nation(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "middle_strata_pop"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo,
			r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0),
			indentation + indentation_amount);
		}
		uint32_t es_middle_strata_scope_state(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "middle_strata_pop"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo,
			r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0),
			indentation + indentation_amount);
		}
		uint32_t es_middle_strata_scope_province(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "middle_strata_pop"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo,
			r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0),
			indentation + indentation_amount);
		}
		uint32_t es_rich_strata_scope_nation(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "rich_strata_pop"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo,
			r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0),
			indentation + indentation_amount);
		}
		uint32_t es_rich_strata_scope_state(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "rich_strata_pop"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo,
			r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0),
			indentation + indentation_amount);
		}
		uint32_t es_rich_strata_scope_province(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "rich_strata_pop"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo,
			r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0),
			indentation + indentation_amount);
		}
		uint32_t es_x_pop_scope_nation(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "pop"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo,
			r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0),
			indentation + indentation_amount);
		}
		uint32_t es_x_pop_scope_state(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "pop"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo,
			r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0),
			indentation + indentation_amount);
		}
		uint32_t es_x_pop_scope_province(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "pop"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo,
			r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0),
			indentation + indentation_amount);
		}
		uint32_t es_x_owned_scope_nation(EFFECT_DISPLAY_PARAMS) {
			if((tval[0] & effect::is_random_scope) != 0) {
				if(primary_slot != -1) {
					std::vector<dcon::province_id> rlist;

					if((tval[0] & effect::scope_has_limit) != 0) {
						auto limit = trigger::payload(tval[2]).tr_id;
						for(auto p : ws.world.nation_get_province_ownership(trigger::to_nation(primary_slot))) {
							if(trigger::evaluate(ws, limit, trigger::to_generic(p.get_province().id), this_slot, from_slot)) {
								rlist.push_back(p.get_province().id);
							}
						}
					} else {
						for(auto p : ws.world.nation_get_province_ownership(trigger::to_nation(primary_slot))) {
							rlist.push_back(p.get_province().id);
						}
					}

					if(rlist.size() != 0) {
						auto r = uint32_t(rng::get_random(ws, r_hi, r_lo) % rlist.size());
						auto i_amount = 0;
						if(ws.user_settings.spoilers) {
							i_amount = indentation_amount;
							auto box = text::open_layout_box(layout, indentation);
							text::add_to_layout_box(ws, layout, box, rlist[r]);
							text::close_layout_box(layout, box);
							show_limit(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, indentation);
							/* So that SirRunner's random_owned + has_country_flag trick works properly! */
							if((tval[0] & effect::scope_has_limit) != 0 && primary_slot != -1) {
								auto limit = trigger::payload(tval[2]).tr_id;
								if(!trigger::evaluate(ws, limit, primary_slot, this_slot, from_slot)) {
									return 0;
								}
							}
						}
						return 1 + display_subeffects(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, r_hi, r_lo + 1, indentation + i_amount);
					}
					return 0;
				}
			}

			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "owned_province"));
			text::add_space_to_layout_box(ws, layout, box);
			if(primary_slot != -1)
			text::add_to_layout_box(ws, layout, box, trigger::to_nation(primary_slot));
			else
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "singular_nation"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo,
			r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0),
			indentation + indentation_amount);
		}
		uint32_t es_x_owned_scope_state(EFFECT_DISPLAY_PARAMS) {
			if((tval[0] & effect::is_random_scope) != 0) {
				if(primary_slot != -1) {
					std::vector<dcon::province_id> rlist;

					if((tval[0] & effect::scope_has_limit) != 0) {
						auto limit = trigger::payload(tval[2]).tr_id;
						auto d = ws.world.state_instance_get_definition(trigger::to_state(primary_slot));
						auto o = ws.world.state_instance_get_nation_from_state_ownership(trigger::to_state(primary_slot));
						for(auto p : ws.world.state_definition_get_abstract_state_membership(d)) {
							if(p.get_province().get_nation_from_province_ownership() == o) {
								if(trigger::evaluate(ws, limit, trigger::to_generic(p.get_province().id), this_slot, from_slot)) {
									rlist.push_back(p.get_province().id);
								}
							}
						}
					} else {
						auto d = ws.world.state_instance_get_definition(trigger::to_state(primary_slot));
						auto o = ws.world.state_instance_get_nation_from_state_ownership(trigger::to_state(primary_slot));
						for(auto p : ws.world.state_definition_get_abstract_state_membership(d)) {
							if(p.get_province().get_nation_from_province_ownership() == o) {
								rlist.push_back(p.get_province().id);
							}
						}
					}

					if(rlist.size() != 0) {
						auto r = uint32_t(rng::get_random(ws, r_hi, r_lo) % rlist.size());
						auto i_amount = 0;
						if(ws.user_settings.spoilers) {
							i_amount = indentation_amount;
							auto box = text::open_layout_box(layout, indentation);
							text::add_to_layout_box(ws, layout, box, rlist[r]);
							text::close_layout_box(layout, box);
							show_limit(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, indentation);
							/* So that SirRunner's random_owned + has_country_flag trick works properly! */
							if((tval[0] & effect::scope_has_limit) != 0 && primary_slot != -1) {
								auto limit = trigger::payload(tval[2]).tr_id;
								if(!trigger::evaluate(ws, limit, primary_slot, this_slot, from_slot))
								return 0;
							}
						}
						return 1 + display_subeffects(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, r_hi, r_lo + 1,
						indentation + i_amount);
					}
					return 0;
				}
			}

			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "owned_province"));
				text::add_space_to_layout_box(ws, layout, box);
				if(primary_slot != -1)
				text::add_to_layout_box(ws, layout, box, trigger::to_state(primary_slot));
				else
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "singular_state"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);

			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo,
																																	r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0),
																																	indentation + indentation_amount);
		}
		uint32_t es_x_core_scope(EFFECT_DISPLAY_PARAMS) {
			if((tval[0] & effect::is_random_scope) != 0) {
				if(primary_slot != -1) {
					auto tag = ws.world.nation_get_identity_from_identity_holder(trigger::to_nation(primary_slot));
					auto cores_range = ws.world.national_identity_get_core(tag);

					std::vector<dcon::province_id> rlist;

					if((tval[0] & effect::scope_has_limit) != 0) {
						auto limit = trigger::payload(tval[2]).tr_id;
						for(auto p : cores_range) {
							if(trigger::evaluate(ws, limit, trigger::to_generic(p.get_province().id), this_slot, from_slot))
							rlist.push_back(p.get_province().id);
						}
					} else {
						for(auto p : cores_range) {
							rlist.push_back(p.get_province().id);
						}
					}

					if(rlist.size() != 0) {
						auto r = uint32_t(rng::get_random(ws, r_hi, r_lo) % rlist.size());
						auto i_amount = 0;
						if(ws.user_settings.spoilers) {
							i_amount = indentation_amount;
							auto box = text::open_layout_box(layout, indentation);
							text::add_to_layout_box(ws, layout, box, rlist[r]);
							text::close_layout_box(layout, box);
							show_limit(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, indentation);
						}
						return 1 + display_subeffects(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, r_hi, r_lo + 1,
						indentation + i_amount);
					}
					return 0;
				}
			}

			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "core_of"));
				text::add_space_to_layout_box(ws, layout, box);
				if(primary_slot != -1)
				text::add_to_layout_box(ws, layout, box, trigger::to_nation(primary_slot));
				else
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "singular_nation"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);

			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo, r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0), indentation + indentation_amount);
		}
		uint32_t es_x_core_scope_province(EFFECT_DISPLAY_PARAMS) {
			if((tval[0] & effect::is_random_scope) != 0) {
				if(primary_slot != -1) {
					auto prov = trigger::to_prov(primary_slot);
					auto cores_range = ws.world.province_get_core(prov);

					std::vector<dcon::nation_id> rlist;

					if((tval[0] & effect::scope_has_limit) != 0) {
						auto limit = trigger::payload(tval[2]).tr_id;
						for(auto p : cores_range) {
							auto h = p.get_identity().get_nation_from_identity_holder();
							if(h && trigger::evaluate(ws, limit, trigger::to_generic(h.id), this_slot, from_slot))
							rlist.push_back(h.id);
						}
					} else {
						for(auto p : cores_range) {
							auto h = p.get_identity().get_nation_from_identity_holder();
							if(h)
							rlist.push_back(h.id);
						}
					}

					if(rlist.size() != 0) {
						auto r = uint32_t(rng::get_random(ws, r_hi, r_lo) % rlist.size());

						auto box = text::open_layout_box(layout, indentation);
						text::add_to_layout_box(ws, layout, box, rlist[r]);
						text::close_layout_box(layout, box);
						show_limit(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, indentation);
						return 1 + display_subeffects(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, r_hi, r_lo + 1,
											 indentation + indentation_amount);
					}
					return 0;
				}
			}

			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "core_in"));
				text::add_space_to_layout_box(ws, layout, box);
				if(primary_slot != -1)
				text::add_to_layout_box(ws, layout, box, trigger::to_prov(primary_slot));
				else
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "singular_province"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);

			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo, r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0), indentation + indentation_amount);
		}
		uint32_t es_x_substate_scope(EFFECT_DISPLAY_PARAMS) {
			if((tval[0] & effect::is_random_scope) != 0) {
				std::vector<dcon::nation_id> rlist;

				if((tval[0] & effect::scope_has_limit) != 0) {
					auto limit = trigger::payload(tval[2]).tr_id;
					for(auto si : ws.world.nation_get_overlord_as_ruler(trigger::to_nation(primary_slot))) {
						if(si.get_subject().get_is_substate() && trigger::evaluate(ws, limit, trigger::to_generic(si.get_subject().id), this_slot, from_slot))
						rlist.push_back(si.get_subject().id);
					}
				} else {
					for(auto si : ws.world.nation_get_overlord_as_ruler(trigger::to_nation(primary_slot))) {
						if(si.get_subject().get_is_substate())
						rlist.push_back(si.get_subject().id);
					}
				}
				if(rlist.size() != 0) {
					auto r = uint32_t(rng::get_random(ws, r_hi, r_lo) % rlist.size());

					auto box = text::open_layout_box(layout, indentation);
					text::add_to_layout_box(ws, layout, box, rlist[r]);
					text::close_layout_box(layout, box);
					show_limit(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, indentation);
					return 1 + display_subeffects(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, r_hi, r_lo + 1, indentation + indentation_amount);
				}
				return 0;
			}

			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "substate_of"));
				text::add_space_to_layout_box(ws, layout, box);
				if(primary_slot != -1)
				text::add_to_layout_box(ws, layout, box, trigger::to_nation(primary_slot));
				else
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "singular_nation"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);

			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo, r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0), indentation + indentation_amount);

		}
		uint32_t es_x_state_scope(EFFECT_DISPLAY_PARAMS) {
			if((tval[0] & effect::is_random_scope) != 0) {
				if(primary_slot != -1) {
					std::vector<dcon::state_instance_id> rlist;

					if((tval[0] & effect::scope_has_limit) != 0) {
						auto limit = trigger::payload(tval[2]).tr_id;
						for(auto si : ws.world.nation_get_state_ownership(trigger::to_nation(primary_slot))) {
							if(trigger::evaluate(ws, limit, trigger::to_generic(si.get_state().id), this_slot, from_slot))
							rlist.push_back(si.get_state().id);
						}
					} else {
						for(auto si : ws.world.nation_get_state_ownership(trigger::to_nation(primary_slot))) {
							rlist.push_back(si.get_state().id);
						}
					}

					if(rlist.size() != 0) {
						auto r = uint32_t(rng::get_random(ws, r_hi, r_lo) % rlist.size());

						auto box = text::open_layout_box(layout, indentation);
						text::add_to_layout_box(ws, layout, box, rlist[r]);
						text::close_layout_box(layout, box);
						show_limit(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, indentation);
						return 1 + display_subeffects(ws, tval, layout, trigger::to_generic(rlist[r]), this_slot, from_slot, r_hi, r_lo + 1,
											 indentation + indentation_amount);
					}
					return 0;
				}
			}

			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, random_or_every(tval[0])));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "state_of"));
				text::add_space_to_layout_box(ws, layout, box);
				if(primary_slot != -1)
				text::add_to_layout_box(ws, layout, box, trigger::to_nation(primary_slot));
				else
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "singular_nation"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);

			return ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0) + display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_lo, r_hi + ((tval[0] & effect::is_random_scope) != 0 ? 1 : 0), indentation + indentation_amount);
		}
		uint32_t es_random_list_scope(EFFECT_DISPLAY_PARAMS) {

			auto const source_size = 1 + effect::get_effect_scope_payload_size(tval);
			auto chances_total = tval[2];

			auto sub_units_start = tval + 3; // [code] + [payload size] + [chances total] + [first sub effect chance]

			auto r = int32_t(rng::get_random(ws, r_hi, r_lo) % chances_total);
			uint32_t rval = 0;
			bool found_res = false;

			while(sub_units_start < tval + source_size) {
				auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::fp_percentage{ float(*sub_units_start) / float(chances_total) });
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "chance_of"));
				text::close_layout_box(layout, box);

				r -= *sub_units_start;
				auto tresult = 1 + internal_make_effect_description(ws, sub_units_start + 1, layout, primary_slot, this_slot, from_slot, r_hi, r_lo + 1,
				indentation + indentation_amount);
				if(r < 0 && !found_res) {
					found_res = true;
					rval = tresult;
				}
				sub_units_start += 2 + effect::get_generic_effect_payload_size(sub_units_start + 1); // each member preceded by uint16_t
			}
			return rval;
		}
		uint32_t es_random_scope(EFFECT_DISPLAY_PARAMS) {
			auto chance = tval[2];
			auto box = text::open_layout_box(layout, indentation);
		text::add_to_layout_box(ws, layout, box, text::fp_percentage{ float(chance) / 100.f });
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "chance_of"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return 1 + display_subeffects(ws, tval, layout, primary_slot, this_slot, from_slot, r_hi, r_lo + 1, indentation + indentation_amount);
		}
		uint32_t es_random_by_modifier_scope(EFFECT_DISPLAY_PARAMS) {
		auto mod_k = dcon::value_modifier_key{ dcon::value_modifier_key::value_base_t(tval[2]) };
			if(primary_slot != -1) {
				auto chance = trigger::evaluate_multiplicative_modifier(ws, mod_k, primary_slot, this_slot, from_slot);
				assert(chance >= 0.f);
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::fp_percentage{ float(chance) / 100.f });
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "chance_of"));
				text::close_layout_box(layout, box);
			}
			ui::multiplicative_value_modifier_description(ws, layout, mod_k, primary_slot, this_slot, from_slot);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return 1 + display_subeffects(ws, tval, layout, primary_slot, this_slot, from_slot, r_hi, r_lo + 1, indentation + indentation_amount);
		}
		uint32_t es_owner_scope_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "owner_of"));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box,
				primary_slot != -1 ? text::get_dynamic_state_name(ws, trigger::to_state(primary_slot))
													 : text::produce_simple_string(ws, "singular_state"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout,
			primary_slot != -1
					? trigger::to_generic(ws.world.state_instance_get_nation_from_state_ownership(trigger::to_state(primary_slot)))
					: -1,
			this_slot, from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_owner_scope_province(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "owner_of"));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, primary_slot != -1 ? text::produce_simple_string(ws, ws.world.province_get_name(trigger::to_prov(primary_slot))) : text::produce_simple_string(ws, "singular_province"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout,
			primary_slot != -1
					? trigger::to_generic(ws.world.province_get_nation_from_province_ownership(trigger::to_prov(primary_slot)))
					: -1,
			this_slot, from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_controller_scope(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "controller_of"));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box,
				primary_slot != -1 ? text::produce_simple_string(ws, ws.world.province_get_name(trigger::to_prov(primary_slot)))
													 : text::produce_simple_string(ws, "singular_province"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout,
			primary_slot != -1 ? trigger::to_generic(ws.world.province_get_nation_from_province_control(trigger::to_prov(primary_slot)))
												 : -1,
			this_slot, from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_location_scope(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "location_of_pop"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout,
			primary_slot != -1 ? trigger::to_generic(ws.world.pop_get_province_from_pop_location(trigger::to_pop(primary_slot))) : -1,
			this_slot, from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_country_scope_pop(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "nation_of_pop"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout,
			primary_slot != -1 ? trigger::to_generic(nations::owner_of_pop(ws, trigger::to_pop(primary_slot))) : -1, this_slot,
			from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_country_scope_state(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "owner_of"));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box,
			primary_slot != -1 ? text::get_dynamic_state_name(ws, trigger::to_state(primary_slot))
													: text::produce_simple_string(ws, "singular_state"));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout,
			primary_slot != -1
					? trigger::to_generic(ws.world.state_instance_get_nation_from_state_ownership(trigger::to_state(primary_slot)))
					: -1,
			this_slot, from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_capital_scope(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "capital_of"));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box,
				primary_slot != -1 ? text::produce_simple_string(ws, text::get_name(ws, trigger::to_nation(primary_slot)))
													 : text::produce_simple_string(ws, "singular_nation"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout,
			primary_slot != -1 ? trigger::to_generic(ws.world.nation_get_capital(trigger::to_nation(primary_slot))) : -1, this_slot,
			from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_capital_scope_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "capital_of"));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box,
				primary_slot != -1 ? text::produce_simple_string(ws, text::get_name(ws, ws.world.province_get_nation_from_province_ownership(trigger::to_prov(primary_slot))))
													 : text::produce_simple_string(ws, "singular_nation"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout,
			primary_slot != -1 ? trigger::to_generic(ws.world.nation_get_capital(ws.world.province_get_nation_from_province_ownership(trigger::to_prov(primary_slot)))) : -1, this_slot,
			from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_this_scope_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box,
				this_slot != -1 ? text::produce_simple_string(ws, text::get_name(ws, trigger::to_nation(this_slot)))
												: text::produce_simple_string(ws, "this_nation"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout, this_slot, this_slot, from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_this_scope_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box,
				this_slot != -1 ? text::get_dynamic_state_name(ws, trigger::to_state(this_slot))
												: text::produce_simple_string(ws, "this_state"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout, this_slot, this_slot, from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_this_scope_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box,
				this_slot != -1 ? text::produce_simple_string(ws, ws.world.province_get_name(trigger::to_prov(this_slot)))
												: text::produce_simple_string(ws, "this_province"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout, this_slot, this_slot, from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_this_scope_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "this_pop"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout, this_slot, this_slot, from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_from_scope_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box,
				from_slot != -1 ? text::produce_simple_string(ws, text::get_name(ws, trigger::to_nation(from_slot)))
												: text::produce_simple_string(ws, "from_nation"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout, from_slot, this_slot, from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_from_scope_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box,
				from_slot != -1 ? text::get_dynamic_state_name(ws, trigger::to_state(from_slot))
												: text::produce_simple_string(ws, "from_state"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout, from_slot, this_slot, from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_from_scope_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box,
				from_slot != -1 ? text::produce_simple_string(ws, ws.world.province_get_name(trigger::to_prov(from_slot)))
												: text::produce_simple_string(ws, "from_province"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout, from_slot, this_slot, from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_from_scope_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "from_pop"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout, from_slot, this_slot, from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_sea_zone_scope(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "adjacent_sea"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);

			auto sea_zone = primary_slot != -1 ? [&ws](int32_t p_slot) {
				auto pid = fatten(ws.world, trigger::to_prov(p_slot));
				for(auto adj : pid.get_province_adjacency()) {
					if(adj.get_connected_provinces(0).id.index() >= ws.province_definitions.first_sea_province.index()) {
						return adj.get_connected_provinces(0).id;
					} else if(adj.get_connected_provinces(1).id.index() >= ws.province_definitions.first_sea_province.index()) {
						return adj.get_connected_provinces(1).id;
					}
				}
			return dcon::province_id{};
			}(primary_slot)
			: dcon::province_id{};

			return display_subeffects(ws, tval, layout, sea_zone ? trigger::to_generic(sea_zone) : -1, this_slot, from_slot, r_hi, r_lo,
				indentation + indentation_amount);
		}
		uint32_t es_cultural_union_scope(EFFECT_DISPLAY_PARAMS) {
			auto prim_culture =
		primary_slot != -1 ? ws.world.nation_get_primary_culture(trigger::to_nation(primary_slot)) : dcon::culture_id{};
			auto cg = ws.world.culture_get_group_from_culture_group_membership(prim_culture);

			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "cultural_union_of"));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box,
				cg ? text::produce_simple_string(ws, ws.world.culture_group_get_name(cg))
					 : text::produce_simple_string(ws, "singular_nation"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);

			auto union_tag = ws.world.culture_group_get_identity_from_cultural_union_of(cg);
			auto group_holder = ws.world.national_identity_get_nation_from_identity_holder(union_tag);

			return display_subeffects(ws, tval, layout, trigger::to_generic(group_holder), this_slot, from_slot, r_hi, r_lo,
			indentation + indentation_amount);
		}
		uint32_t es_overlord_scope(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "overlord_of"));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box,
				primary_slot != -1 ? text::produce_simple_string(ws, text::get_name(ws, trigger::to_nation(primary_slot)))
													 : text::produce_simple_string(ws, "singular_nation"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);

			auto olr = ws.world.nation_get_overlord_as_subject(trigger::to_nation(primary_slot));
			return display_subeffects(ws, tval, layout, trigger::to_generic(ws.world.overlord_get_ruler(olr)), this_slot, from_slot, r_hi,
			r_lo, indentation + indentation_amount);
		}
		uint32_t es_sphere_owner_scope(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "sphere_leader_of"));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box,
				primary_slot != -1 ? text::produce_simple_string(ws, text::get_name(ws, trigger::to_nation(primary_slot)))
													 : text::produce_simple_string(ws, ""));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);

			return display_subeffects(ws, tval, layout,
			primary_slot != -1 ? trigger::to_generic(ws.world.nation_get_in_sphere_of((trigger::to_nation(primary_slot)))) : -1,
			this_slot, from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_independence_scope(EFFECT_DISPLAY_PARAMS) {
			auto rtag =
		from_slot != -1 ? ws.world.rebel_faction_get_defection_target(trigger::to_rebel(from_slot)) : dcon::national_identity_id{};
			auto r_holder = ws.world.national_identity_get_nation_from_identity_holder(rtag);

			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "reb_independence_nation"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);

			return display_subeffects(ws, tval, layout, r_holder ? trigger::to_generic(r_holder) : -1, this_slot, from_slot, r_hi, r_lo,
			indentation + indentation_amount);
		}
		uint32_t es_flashpoint_tag_scope(EFFECT_DISPLAY_PARAMS) {
			auto ctag = ws.world.state_instance_get_flashpoint_tag(trigger::to_state(primary_slot));
			auto fp_nation = ws.world.national_identity_get_nation_from_identity_holder(ctag);

			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "flashpoint_nation"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout, fp_nation ? trigger::to_generic(fp_nation) : -1, this_slot, from_slot, r_hi, r_lo,
			indentation + indentation_amount);
		}
		uint32_t es_crisis_state_scope(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "crisis_state"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			auto cstate = ws.crisis_state ? trigger::to_generic(ws.crisis_state) : -1;
			return display_subeffects(ws, tval, layout, cstate, this_slot, from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_state_scope_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "containing_state"));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box,
				primary_slot != -1 ? text::produce_simple_string(ws, ws.world.province_get_name(trigger::to_prov(primary_slot)))
													 : text::produce_simple_string(ws, "singular_province"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
		auto st = primary_slot != -1 ? ws.world.province_get_state_membership(trigger::to_prov(primary_slot)) : dcon::state_instance_id{};
			return display_subeffects(ws, tval, layout, st ? trigger::to_generic(st) : -1, this_slot, from_slot, r_hi, r_lo,
			indentation + indentation_amount);
		}
		uint32_t es_state_scope_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "containing_state"));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "singular_pop"));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
		auto st = primary_slot != -1 ? ws.world.province_get_state_membership(ws.world.pop_get_province_from_pop_location(trigger::to_pop(primary_slot))) : dcon::state_instance_id{};
			return display_subeffects(ws, tval, layout, st ? trigger::to_generic(st) : -1, this_slot, from_slot, r_hi, r_lo,
			indentation + indentation_amount);
		}
		uint32_t es_tag_scope(EFFECT_DISPLAY_PARAMS) {
			auto tag = trigger::payload(tval[2]).tag_id;
			auto tag_holder = ws.world.national_identity_get_nation_from_identity_holder(tag);
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, tag_holder ? text::get_name(ws, tag_holder) : ws.world.national_identity_get_name(tag)));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout, tag_holder ? trigger::to_generic(tag_holder) : -1, this_slot, from_slot, r_hi, r_lo,
			indentation + indentation_amount);
		}
		uint32_t es_integer_scope(EFFECT_DISPLAY_PARAMS) {
			auto p = trigger::payload(tval[2]).prov_id;

			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, ws.world.province_get_name(p)));
				text::close_layout_box(layout, box);
			}
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout, trigger::to_generic(p), this_slot, from_slot, r_hi, r_lo,
			indentation + indentation_amount);
		}

		uint32_t es_pop_type_scope_nation(EFFECT_DISPLAY_PARAMS) {
			auto type = (tval[0] & effect::scope_has_limit) != 0 ? trigger::payload(tval[3]).popt_id : trigger::payload(tval[2]).popt_id;

			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "every"));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, ws.world.pop_type_get_name(type)));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "pop"));
				text::close_layout_box(layout, box);
			}

			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);

			return display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_pop_type_scope_state(EFFECT_DISPLAY_PARAMS) {
			auto type = (tval[0] & effect::scope_has_limit) != 0 ? trigger::payload(tval[3]).popt_id : trigger::payload(tval[2]).popt_id;

			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "every"));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, ws.world.pop_type_get_name(type)));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "pop"));
				text::close_layout_box(layout, box);
			}

			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);

			return display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_pop_type_scope_province(EFFECT_DISPLAY_PARAMS) {
			auto type = (tval[0] & effect::scope_has_limit) != 0 ? trigger::payload(tval[3]).popt_id : trigger::payload(tval[2]).popt_id;

			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "every"));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, ws.world.pop_type_get_name(type)));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "pop"));
				text::close_layout_box(layout, box);
			}

			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);

			return display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_region_scope(EFFECT_DISPLAY_PARAMS) {
			auto region = trigger::payload(tval[(tval[0] & effect::scope_has_limit) != 0 ? 3 : 2]).state_id;
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, region);
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t es_region_proper_scope(EFFECT_DISPLAY_PARAMS) {
			auto region = (tval[0] & effect::scope_has_limit) != 0 ? trigger::payload(tval[3]).reg_id : trigger::payload(tval[2]).reg_id;
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, ws.world.region_get_name(region));
			text::close_layout_box(layout, box);
			show_limit(ws, tval, layout, -1, this_slot, from_slot, indentation);
			return display_subeffects(ws, tval, layout, -1, this_slot, from_slot, r_hi, r_lo, indentation + indentation_amount);
		}
		uint32_t ef_none(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "no_effect"));
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_capital(EFFECT_DISPLAY_PARAMS) {
			auto new_capital = trigger::payload(tval[1]).prov_id;
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "move_capital_to"));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, new_capital);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_add_core_tag(EFFECT_DISPLAY_PARAMS) {
			auto tag = trigger::payload(tval[1]).tag_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::adj, ws.world.national_identity_get_adjective(tag));
				text::localised_format_box(ws, layout, box, "add_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_core_tag_state(EFFECT_DISPLAY_PARAMS) {
			auto tag = trigger::payload(tval[1]).tag_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::adj, ws.world.national_identity_get_adjective(tag));
				text::localised_format_box(ws, layout, box, "add_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}

		uint32_t ef_add_core_state_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_this_to_map(ws, m, trigger::to_nation(this_slot));
				text::localised_format_box(ws, layout, box, "add_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_core_state_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_this_to_map(ws, m, trigger::to_prov(this_slot));
				text::localised_format_box(ws, layout, box, "add_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_core_state_this_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_this_to_map(ws, m, trigger::to_state(this_slot));
				text::localised_format_box(ws, layout, box, "add_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_core_state_this_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_this_to_map(ws, m, trigger::to_pop(this_slot));
				text::localised_format_box(ws, layout, box, "add_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_core_state_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_from_to_map(ws, m, trigger::to_prov(from_slot));
				text::localised_format_box(ws, layout, box, "add_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_core_state_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_from_to_map(ws, m, trigger::to_nation(from_slot));
				text::localised_format_box(ws, layout, box, "add_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_core_state_reb(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_from_to_map(ws, m, trigger::to_rebel(from_slot));
				text::localised_format_box(ws, layout, box, "add_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}

		uint32_t ef_add_core_int(EFFECT_DISPLAY_PARAMS) {
			auto prov = trigger::payload(tval[1]).prov_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "add_core_to"));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, prov);
				text::close_layout_box(layout, box);
			}
			return 0;
		}

		uint32_t ef_add_core_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_this_to_map(ws, m, trigger::to_nation(this_slot));
				text::localised_format_box(ws, layout, box, "add_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_core_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_this_to_map(ws, m, trigger::to_prov(this_slot));
				text::localised_format_box(ws, layout, box, "add_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_core_this_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_this_to_map(ws, m, trigger::to_state(this_slot));
				text::localised_format_box(ws, layout, box, "add_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_core_this_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_this_to_map(ws, m, trigger::to_pop(this_slot));
				text::localised_format_box(ws, layout, box, "add_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_core_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_from_to_map(ws, m, trigger::to_nation(from_slot));
				text::localised_format_box(ws, layout, box, "add_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_core_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_from_to_map(ws, m, trigger::to_prov(from_slot));
				text::localised_format_box(ws, layout, box, "add_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_core_reb(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_from_to_map(ws, m, trigger::to_rebel(from_slot));
				text::localised_format_box(ws, layout, box, "add_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_tag(EFFECT_DISPLAY_PARAMS) {
			auto tag = trigger::payload(tval[1]).tag_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::adj, ws.world.national_identity_get_adjective(tag));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_tag_state(EFFECT_DISPLAY_PARAMS) {
			auto tag = trigger::payload(tval[1]).tag_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::adj, ws.world.national_identity_get_adjective(tag));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_int(EFFECT_DISPLAY_PARAMS) {
			auto prov = trigger::payload(tval[1]).prov_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "remove_core_from"));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, prov);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_this_to_map(ws, m, trigger::to_nation(this_slot));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_this_to_map(ws, m, trigger::to_prov(this_slot));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_this_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_this_to_map(ws, m, trigger::to_state(this_slot));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_this_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_this_to_map(ws, m, trigger::to_pop(this_slot));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_from_to_map(ws, m, trigger::to_nation(from_slot));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_from_to_map(ws, m, trigger::to_prov(from_slot));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_reb(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_from_to_map(ws, m, trigger::to_rebel(from_slot));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}

		uint32_t ef_remove_core_state_tag(EFFECT_DISPLAY_PARAMS) {
			auto tag = trigger::payload(tval[1]).tag_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::adj, ws.world.national_identity_get_adjective(tag));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_state_tag_state(EFFECT_DISPLAY_PARAMS) {
			auto tag = trigger::payload(tval[1]).tag_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::adj, ws.world.national_identity_get_adjective(tag));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_state_int(EFFECT_DISPLAY_PARAMS) {
			auto prov = trigger::payload(tval[1]).prov_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "remove_core_from"));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, prov);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_state_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_this_to_map(ws, m, trigger::to_nation(this_slot));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_state_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_this_to_map(ws, m, trigger::to_prov(this_slot));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_state_this_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_this_to_map(ws, m, trigger::to_state(this_slot));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_state_this_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_this_to_map(ws, m, trigger::to_pop(this_slot));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_state_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_from_to_map(ws, m, trigger::to_nation(from_slot));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_state_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_from_to_map(ws, m, trigger::to_prov(from_slot));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_state_reb(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_from_to_map(ws, m, trigger::to_rebel(from_slot));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}

		uint32_t ef_remove_core_tag_nation(EFFECT_DISPLAY_PARAMS) {
			auto tag = trigger::payload(tval[1]).tag_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::adj, ws.world.national_identity_get_adjective(tag));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_nation_tag_state(EFFECT_DISPLAY_PARAMS) {
			auto tag = trigger::payload(tval[1]).tag_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::adj, ws.world.national_identity_get_adjective(tag));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_nation_int(EFFECT_DISPLAY_PARAMS) {
			auto prov = trigger::payload(tval[1]).prov_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "remove_core_from"));
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, prov);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_nation_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_this_to_map(ws, m, trigger::to_nation(this_slot));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_nation_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_this_to_map(ws, m, trigger::to_prov(this_slot));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_nation_this_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_this_to_map(ws, m, trigger::to_state(this_slot));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_nation_this_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_this_to_map(ws, m, trigger::to_pop(this_slot));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_nation_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_from_to_map(ws, m, trigger::to_nation(from_slot));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_nation_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_from_to_map(ws, m, trigger::to_prov(from_slot));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_core_nation_reb(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_adj_from_to_map(ws, m, trigger::to_rebel(from_slot));
				text::localised_format_box(ws, layout, box, "remove_x_core", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}

		uint32_t ef_change_region_name_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				dcon::text_key name{ dcon::text_key::value_base_t(trigger::read_int32_t_from_payload(tval + 1)) };
				text::add_to_substitution_map(m, text::variable_type::text, name);
				text::localised_format_box(ws, layout, box, "change_name_to", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_change_region_name_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				dcon::text_key name{ dcon::text_key::value_base_t(trigger::read_int32_t_from_payload(tval + 1)) };
				text::add_to_substitution_map(m, text::variable_type::text, name);
				text::localised_format_box(ws, layout, box, "change_state_name_to", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_trade_goods(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.commodity_get_name(trigger::payload(tval[1]).com_id));
				if(primary_slot != -1) {
					text::add_to_substitution_map(m, text::variable_type::x, trigger::to_prov(primary_slot));
					text::localised_format_box(ws, layout, box, "province_change_rgo_production_to", m);
				} else {
					text::localised_format_box(ws, layout, box, "change_rgo_production_to", m);
				}
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_accepted_culture(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.culture_get_name(trigger::payload(tval[1]).cul_id));
				text::localised_format_box(ws, layout, box, "make_accepted_culture", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_accepted_culture_union(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "union_culture_accepted", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_accepted_culture_union_this(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "union_culture_accepted_this", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_accepted_culture_union_from(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "union_culture_accepted_from", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}

		uint32_t ef_add_accepted_culture_this(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.nation_get_primary_culture(trigger::to_nation(this_slot)).get_name());
				text::localised_format_box(ws, layout, box, "make_accepted_culture", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_accepted_culture_from(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.nation_get_primary_culture(trigger::to_nation(from_slot)).get_name());
				text::localised_format_box(ws, layout, box, "make_accepted_culture", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}

		uint32_t ef_primary_culture(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.culture_get_name(trigger::payload(tval[1]).cul_id));
				text::localised_format_box(ws, layout, box, "primary_culture_changes_to", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		dcon::nation_id convert_this(sys::state& ws, dcon::nation_id n) {
			return n;
		}
		dcon::nation_id convert_this(sys::state& ws, dcon::state_instance_id p) {
			return ws.world.state_instance_get_nation_from_state_ownership(p);
		}
		dcon::nation_id convert_this(sys::state& ws, dcon::province_id p) {
			return ws.world.province_get_nation_from_province_ownership(p);
		}
		dcon::nation_id convert_this(sys::state& ws, dcon::pop_id p) {
			return nations::owner_of_pop(ws, p);
		}
		dcon::nation_id convert_this(sys::state& ws, dcon::rebel_faction_id p) {
			auto fp = fatten(ws.world, p);
			return fp.get_defection_target().get_nation_from_identity_holder();
		}
		template<typename T, typename F>
		void add_to_map(sys::state& ws, text::substitution_map& map, T v, std::string_view alt, F const& f) {
			if(v) {
				text::add_to_substitution_map(map, text::variable_type::text, f(v));
			} else {
				if(auto k = ws.lookup_key(alt); k) {
					text::add_to_substitution_map(map, text::variable_type::text, k);
				} else {
					text::add_to_substitution_map(map, text::variable_type::text, alt);
				}
			}
		}
		uint32_t ef_primary_culture_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_to_map(ws, m, trigger::to_nation(this_slot), "this_nation_culture",
				[&](auto id) { return ws.world.culture_get_name(ws.world.nation_get_primary_culture(id)); });
				text::localised_format_box(ws, layout, box, "primary_culture_changes_to", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_primary_culture_this_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_to_map(ws, m, convert_this(ws, trigger::to_state(this_slot)), "this_nation_culture",
				[&](auto id) { return ws.world.culture_get_name(ws.world.nation_get_primary_culture(id)); });
				text::localised_format_box(ws, layout, box, "primary_culture_changes_to", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_primary_culture_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation_culture",
				[&](auto id) { return ws.world.culture_get_name(ws.world.nation_get_primary_culture(id)); });
				text::localised_format_box(ws, layout, box, "primary_culture_changes_to", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_primary_culture_this_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_to_map(ws, m, convert_this(ws, trigger::to_pop(this_slot)), "this_nation_culture",
				[&](auto id) { return ws.world.culture_get_name(ws.world.nation_get_primary_culture(id)); });
				text::localised_format_box(ws, layout, box, "primary_culture_changes_to", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_primary_culture_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation_culture",
				[&](auto id) { return ws.world.culture_get_name(ws.world.nation_get_primary_culture(id)); });
				text::localised_format_box(ws, layout, box, "primary_culture_changes_to", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_accepted_culture(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.culture_get_name(trigger::payload(tval[1]).cul_id));
				text::localised_format_box(ws, layout, box, "alice_remove_accepted_culture", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_life_rating(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "life_rating", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(trigger::payload(tval[1]).signed_value), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_life_rating_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "life_rating", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(trigger::payload(tval[1]).signed_value), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_religion(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.religion_get_name(trigger::payload(tval[1]).rel_id));
				text::localised_format_box(ws, layout, box, "make_national_religion", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_is_slave_state_yes(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "make_slave_state", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_is_slave_pop_yes(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "make_slave_pop", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_is_slave_province_yes(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "make_slave_state", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}

		uint32_t ef_research_points(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "research_points", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(trigger::payload(tval[1]).signed_value), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_tech_school(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, ws.world.modifier_get_name(trigger::payload(tval[1]).mod_id));
			text::localised_format_box(ws, layout, box, "change_tech_school", m);
			text::close_layout_box(layout, box);
			if(ws.user_settings.spoilers) {
				modifier_description(ws, layout, trigger::payload(tval[1]).mod_id, indentation + +indentation_amount);
			}
			return 0;
		}
		uint32_t ef_government(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.government_type_get_name(trigger::payload(tval[1]).gov_id));
				text::localised_format_box(ws, layout, box, "change_government_to", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_government_reb(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				if(from_slot != -1) {
					auto new_gov = ws.world.rebel_faction_get_type(trigger::to_rebel(from_slot)).get_government_change(ws.world.nation_get_government_type(trigger::to_nation(primary_slot)));
					if(new_gov)
					text::add_to_substitution_map(m, text::variable_type::text, ws.world.government_type_get_name(new_gov));
				}
				text::localised_format_box(ws, layout, box, "change_government_to", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_treasury(EFFECT_DISPLAY_PARAMS) {
			auto amount = trigger::read_float_from_payload(tval + 1);
			{
				auto box = text::open_layout_box(layout, indentation);
			display_value(text::fp_currency{ amount }, true, ws, layout, box);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "add_to_treasury", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_war_exhaustion(EFFECT_DISPLAY_PARAMS) {
			auto amount = trigger::read_float_from_payload(tval + 1) / 100.0f;

			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "military_war_exhaustion", m);
				text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_percentage{ amount }, false, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_prestige(EFFECT_DISPLAY_PARAMS) {
			auto delta = trigger::read_float_from_payload(tval + 1);
			float change = delta;
			if(primary_slot != -1) {
				auto prestige_multiplier =
				1.0f + ws.world.nation_get_modifier_values(trigger::to_nation(primary_slot), sys::national_mod_offsets::prestige);
				auto new_prestige = std::max(0.0f,
				ws.world.nation_get_prestige(trigger::to_nation(primary_slot)) + (delta > 0 ? (delta * prestige_multiplier) : delta));
				change = (new_prestige - ws.world.nation_get_prestige(trigger::to_nation(primary_slot)));
			}
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "prestige", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(change), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_change_tag(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, trigger::payload(tval[1]).tag_id);
				text::localised_format_box(ws, layout, box, "become_blank", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_change_tag_culture(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				if(primary_slot != -1) {
					auto prim_culture = ws.world.nation_get_primary_culture(trigger::to_nation(primary_slot));
					auto cg = ws.world.culture_get_group_from_culture_group_membership(prim_culture);
					auto u = ws.world.culture_group_get_identity_from_cultural_union_of(cg);
					text::add_to_substitution_map(m, text::variable_type::text, u);
				} else {
					if(auto k = ws.lookup_key(std::string_view("cultural_union_nation")); k) {
						text::add_to_substitution_map(m, text::variable_type::text, k);
					}
				}
				text::localised_format_box(ws, layout, box, "become_blank", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_change_tag_no_core_switch(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, trigger::payload(tval[1]).tag_id);
				text::localised_format_box(ws, layout, box, "player_control_change", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_change_tag_no_core_switch_culture(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				if(primary_slot != -1) {
					auto prim_culture = ws.world.nation_get_primary_culture(trigger::to_nation(primary_slot));
					auto cg = ws.world.culture_get_group_from_culture_group_membership(prim_culture);
					auto u = ws.world.culture_group_get_identity_from_cultural_union_of(cg);
					text::add_to_substitution_map(m, text::variable_type::text, u);
				} else {
					if(auto k = ws.lookup_key(std::string_view("cultural_union_nation")); k) {
						text::add_to_substitution_map(m, text::variable_type::text, k);
					}
				}
				text::localised_format_box(ws, layout, box, "player_control_change", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_set_country_flag(EFFECT_DISPLAY_PARAMS) {
			if(ws.user_settings.spoilers) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.national_definitions.flag_variable_names[trigger::payload(tval[1]).natf_id]);
				text::localised_format_box(ws, layout, box, "set_national_flag", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_clr_country_flag(EFFECT_DISPLAY_PARAMS) {
			if(ws.user_settings.spoilers) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.national_definitions.flag_variable_names[trigger::payload(tval[1]).natf_id]);
				text::localised_format_box(ws, layout, box, "remove_national_flag", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_military_access(EFFECT_DISPLAY_PARAMS) {
			auto target = ws.world.national_identity_get_nation_from_identity_holder(trigger::payload(tval[1]).tag_id);
			if(target) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, target);
				text::localised_format_box(ws, layout, box, "get_access_with", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_military_access_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "get_access_with", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_military_access_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "get_access_with", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_military_access_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "get_access_with", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_military_access_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "get_access_with", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_badboy(EFFECT_DISPLAY_PARAMS) {
			auto amount = trigger::read_float_from_payload(tval + 1);
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "infamy", m);
				text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_one_place{ amount }, false, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_secede_province(EFFECT_DISPLAY_PARAMS) {
			auto target = ws.world.national_identity_get_nation_from_identity_holder(trigger::payload(tval[1]).tag_id);
			if(target) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, target);
				text::localised_format_box(ws, layout, box, "change_province_owner", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_secede_province_state(EFFECT_DISPLAY_PARAMS) {
			auto target = ws.world.national_identity_get_nation_from_identity_holder(trigger::payload(tval[1]).tag_id);
			if(target) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, target);
				text::localised_format_box(ws, layout, box, "change_state_owner", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_secede_province_state_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_state_owner", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_secede_province_state_this_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_state(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_state_owner", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_secede_province_state_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_state_owner", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_secede_province_state_this_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_pop(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_state_owner", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_secede_province_state_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_state_owner", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_secede_province_state_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_state_owner", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_secede_province_state_reb(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_rebel(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_state_owner", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_secede_province_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_province_owner", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_secede_province_this_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_state(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_province_owner", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_secede_province_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_province_owner", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_secede_province_this_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_pop(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_province_owner", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_secede_province_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_province_owner", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_secede_province_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_province_owner", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_secede_province_reb(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_rebel(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_province_owner", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_inherit(EFFECT_DISPLAY_PARAMS) {
			auto target = ws.world.national_identity_get_nation_from_identity_holder(trigger::payload(tval[1]).tag_id);
			if(target) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, target);
				text::localised_format_box(ws, layout, box, "annex_effect", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_inherit_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "annex_effect", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_inherit_this_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_state(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "annex_effect", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_inherit_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "annex_effect", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_inherit_this_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_pop(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "annex_effect", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_inherit_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "annex_effect", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_inherit_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "annex_effect", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_annex_to(EFFECT_DISPLAY_PARAMS) {
			auto target = ws.world.national_identity_get_nation_from_identity_holder(trigger::payload(tval[1]).tag_id);
			if(target) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, target);
				text::localised_format_box(ws, layout, box, "annexed_by_effect", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_annex_to_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "annexed_by_effect", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_annex_to_this_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_state(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "annexed_by_effect", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_annex_to_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "annexed_by_effect", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_annex_to_this_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_pop(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "annexed_by_effect", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_annex_to_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "annexed_by_effect", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_annex_to_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "annexed_by_effect", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_release(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, trigger::payload(tval[1]).tag_id);
				text::localised_format_box(ws, layout, box, "core_return", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_release_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "core_return", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_release_this_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_state(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "core_return", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_release_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "core_return", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_release_this_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_pop(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "core_return", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_release_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "core_return", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_release_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "core_return", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_change_controller(EFFECT_DISPLAY_PARAMS) {
			auto target = ws.world.national_identity_get_nation_from_identity_holder(trigger::payload(tval[1]).tag_id);
			if(target) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, target);
				text::localised_format_box(ws, layout, box, "change_province_controller", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_change_controller_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_province_controller", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_change_controller_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_province_controller", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_change_controller_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_province_controller", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_change_controller_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_province_controller", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_change_controller_state(EFFECT_DISPLAY_PARAMS) {
			auto target = ws.world.national_identity_get_nation_from_identity_holder(trigger::payload(tval[1]).tag_id);
			if(target) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, target);
				text::localised_format_box(ws, layout, box, "change_state_controller", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_change_controller_state_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_state_controller", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_change_controller_state_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_state_controller", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_change_controller_state_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_state_controller", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_change_controller_state_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "change_state_controller", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_money(EFFECT_DISPLAY_PARAMS) {
			auto amount = trigger::read_float_from_payload(tval + 1);
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "pop_savings", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(text::fp_currency{ amount }, true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_leadership(EFFECT_DISPLAY_PARAMS) {
			auto amount = trigger::payload(tval[1]).signed_value;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "leadership_points", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(amount), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_create_vassal(EFFECT_DISPLAY_PARAMS) {
			auto target = ws.world.national_identity_get_nation_from_identity_holder(trigger::payload(tval[1]).tag_id);
			if(target) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, target);
				text::localised_format_box(ws, layout, box, "release_as_vassal", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_create_vassal_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "release_as_vassal", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_create_vassal_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "release_as_vassal", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_create_vassal_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "release_as_vassal", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_create_vassal_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "release_as_vassal", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_end_military_access(EFFECT_DISPLAY_PARAMS) {
			auto target = ws.world.national_identity_get_nation_from_identity_holder(trigger::payload(tval[1]).tag_id);
			if(target) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, target);
				text::localised_format_box(ws, layout, box, "lose_access_with", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_end_military_access_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "lose_access_with", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_end_military_access_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "lose_access_with", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_end_military_access_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "lose_access_with", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_end_military_access_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "lose_access_with", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_leave_alliance(EFFECT_DISPLAY_PARAMS) {
			auto target = ws.world.national_identity_get_nation_from_identity_holder(trigger::payload(tval[1]).tag_id);
			if(target) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, target);
				text::localised_format_box(ws, layout, box, "end_alliance", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_leave_alliance_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "end_alliance", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_leave_alliance_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "end_alliance", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_leave_alliance_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "end_alliance", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_leave_alliance_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "end_alliance", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_end_war(EFFECT_DISPLAY_PARAMS) {
			auto target = ws.world.national_identity_get_nation_from_identity_holder(trigger::payload(tval[1]).tag_id);
			if(target) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, target);
				text::localised_format_box(ws, layout, box, "end_war_with", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_end_war_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "end_war_with", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_end_war_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "end_war_with", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_end_war_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "end_war_with", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_end_war_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "end_war_with", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_enable_ideology(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, ws.world.ideology_get_name(trigger::payload(tval[1]).ideo_id));
			text::localised_format_box(ws, layout, box, "enable_ideology", m);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_ruling_party_ideology(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, ws.world.ideology_get_name(trigger::payload(tval[1]).ideo_id));
			text::localised_format_box(ws, layout, box, "change_ruling_party", m);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_plurality(EFFECT_DISPLAY_PARAMS) {
			auto amount = trigger::read_float_from_payload(tval + 1) / 100.0f;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "plurality", m);
				text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_percentage{ amount }, true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_province_modifier(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, ws.world.modifier_get_name(trigger::payload(tval[1]).mod_id));
			text::localised_format_box(ws, layout, box, "remove_prov_mod", m);
			text::close_layout_box(layout, box);
			if(ws.user_settings.spoilers) {
				modifier_description(ws, layout, trigger::payload(tval[1]).mod_id, indentation + indentation_amount);
			}
			return 0;
		}
		uint32_t ef_remove_province_modifier_state(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, ws.world.modifier_get_name(trigger::payload(tval[1]).mod_id));
			text::localised_format_box(ws, layout, box, "remove_prov_mod", m);
			text::close_layout_box(layout, box);
			if(ws.user_settings.spoilers) {
				modifier_description(ws, layout, trigger::payload(tval[1]).mod_id, indentation + indentation_amount);
			}
			return 0;
		}
		uint32_t ef_remove_country_modifier(EFFECT_DISPLAY_PARAMS) {
			auto mod = trigger::payload(tval[1]).mod_id;
			bool has_mod = (primary_slot == -1);
			if(!has_mod) {
				for(auto m : ws.world.nation_get_current_modifiers(trigger::to_nation(primary_slot))) {
					if(m.mod_id == mod)
					has_mod = true;
				}
			}
			if(ws.user_settings.spoilers || has_mod) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.modifier_get_name(mod));
				text::localised_format_box(ws, layout, box, "remove_nat_mod", m);
				text::close_layout_box(layout, box);
			}
			if(ws.user_settings.spoilers) {
				modifier_description(ws, layout, mod, indentation + indentation_amount);
			}
			return 0;
		}
		uint32_t ef_create_alliance(EFFECT_DISPLAY_PARAMS) {
			auto target = ws.world.national_identity_get_nation_from_identity_holder(trigger::payload(tval[1]).tag_id);
			if(target) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, target);
				text::localised_format_box(ws, layout, box, "make_alliance", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_create_alliance_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "make_alliance", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_create_alliance_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "make_alliance", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_create_alliance_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "make_alliance", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_create_alliance_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "make_alliance", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_release_vassal(EFFECT_DISPLAY_PARAMS) {
			auto holder = ws.world.national_identity_get_nation_from_identity_holder(trigger::payload(tval[1]).tag_id);
			if(holder) {
				auto hprovs = ws.world.nation_get_province_ownership(holder);
				if(hprovs.begin() == hprovs.end()) {
					auto box = text::open_layout_box(layout, indentation);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, holder);
					text::localised_format_box(ws, layout, box, "release_as_vassal", m);
					text::close_layout_box(layout, box);
				} else {
					auto box = text::open_layout_box(layout, indentation);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, holder);
					text::localised_format_box(ws, layout, box, "becomes_independent", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_release_vassal_this_nation(EFFECT_DISPLAY_PARAMS) {
			auto holder = trigger::to_nation(this_slot);
			auto hprovs = ws.world.nation_get_province_ownership(holder);
			if(hprovs.begin() == hprovs.end()) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, holder);
				text::localised_format_box(ws, layout, box, "release_as_vassal", m);
				text::close_layout_box(layout, box);
			} else {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "becomes_independent", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_release_vassal_this_province(EFFECT_DISPLAY_PARAMS) {
			auto holder = ws.world.province_get_nation_from_province_ownership(trigger::to_prov(this_slot));
			auto hprovs = ws.world.nation_get_province_ownership(holder);
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			if(hprovs.begin() == hprovs.end()) {
				text::add_to_substitution_map(m, text::variable_type::text, holder);
				text::localised_format_box(ws, layout, box, "release_as_vassal", m);
			} else {
				add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "becomes_independent", m);
			}
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_release_vassal_from_nation(EFFECT_DISPLAY_PARAMS) {
			auto holder = trigger::to_nation(from_slot);
			auto hprovs = ws.world.nation_get_province_ownership(holder);
			if(hprovs.begin() == hprovs.end()) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, holder);
				text::localised_format_box(ws, layout, box, "release_as_vassal", m);
				text::close_layout_box(layout, box);
			} else {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "becomes_independent", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_release_vassal_from_province(EFFECT_DISPLAY_PARAMS) {
			auto holder = ws.world.province_get_nation_from_province_ownership(trigger::to_prov(from_slot));
			auto hprovs = ws.world.nation_get_province_ownership(holder);
			if(hprovs.begin() == hprovs.end()) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, holder);
				text::localised_format_box(ws, layout, box, "release_as_vassal", m);
				text::close_layout_box(layout, box);
			} else {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "becomes_independent", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_release_vassal_reb(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_rebel(from_slot)), "from_nation", [](auto x) { return x; });
			text::localised_format_box(ws, layout, box, "release_as_independent", m);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_release_vassal_random(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::localised_format_box(ws, layout, box, "release_random_vassal");
			text::close_layout_box(layout, box);
			return 0;
		}

		uint32_t ef_release_vassal_province(EFFECT_DISPLAY_PARAMS) {
			auto holder = ws.world.national_identity_get_nation_from_identity_holder(trigger::payload(tval[1]).tag_id);
			if(holder) {
				auto hprovs = ws.world.nation_get_province_ownership(holder);
				if(hprovs.begin() == hprovs.end()) {
					auto box = text::open_layout_box(layout, indentation);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, holder);
					text::localised_format_box(ws, layout, box, "release_as_vassal", m);
					text::close_layout_box(layout, box);
				} else {
					auto box = text::open_layout_box(layout, indentation);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, holder);
					text::localised_format_box(ws, layout, box, "becomes_independent", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_release_vassal_province_this_nation(EFFECT_DISPLAY_PARAMS) {
			auto holder = trigger::to_nation(this_slot);
			auto hprovs = ws.world.nation_get_province_ownership(holder);
			if(hprovs.begin() == hprovs.end()) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, holder);
				text::localised_format_box(ws, layout, box, "release_as_vassal", m);
				text::close_layout_box(layout, box);
			} else {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "becomes_independent", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_release_vassal_province_this_province(EFFECT_DISPLAY_PARAMS) {
			auto holder = ws.world.province_get_nation_from_province_ownership(trigger::to_prov(this_slot));
			auto hprovs = ws.world.nation_get_province_ownership(holder);
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			if(hprovs.begin() == hprovs.end()) {
				text::add_to_substitution_map(m, text::variable_type::text, holder);
				text::localised_format_box(ws, layout, box, "release_as_vassal", m);
			} else {
				add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "becomes_independent", m);
			}
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_release_vassal_province_from_nation(EFFECT_DISPLAY_PARAMS) {
			auto holder = trigger::to_nation(from_slot);
			auto hprovs = ws.world.nation_get_province_ownership(holder);
			if(hprovs.begin() == hprovs.end()) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, holder);
				text::localised_format_box(ws, layout, box, "release_as_vassal", m);
				text::close_layout_box(layout, box);
			} else {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "becomes_independent", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_release_vassal_province_from_province(EFFECT_DISPLAY_PARAMS) {
			auto holder = ws.world.province_get_nation_from_province_ownership(trigger::to_prov(from_slot));
			auto hprovs = ws.world.nation_get_province_ownership(holder);
			if(hprovs.begin() == hprovs.end()) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, holder);
				text::localised_format_box(ws, layout, box, "release_as_vassal", m);
				text::close_layout_box(layout, box);
			} else {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "becomes_independent", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_release_vassal_province_reb(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_rebel(from_slot)), "from_nation", [](auto x) { return x; });
			text::localised_format_box(ws, layout, box, "release_as_independent", m);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_release_vassal_province_random(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::localised_format_box(ws, layout, box, "no_effect");
			text::close_layout_box(layout, box);
			return 0;
		}

		uint32_t ef_change_province_name(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
		dcon::text_key name{ dcon::text_key::value_base_t(trigger::read_int32_t_from_payload(tval + 1)) };
			text::add_to_substitution_map(m, text::variable_type::text, name);
			text::localised_format_box(ws, layout, box, "change_name_to", m);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_enable_canal(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			auto canal_name = text::produce_simple_string(ws, std::string("canal_") + std::to_string(tval[1]));
		text::add_to_substitution_map(m, text::variable_type::name, std::string_view{ canal_name });
			text::localised_format_box(ws, layout, box, "enable_canal", m);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_set_global_flag(EFFECT_DISPLAY_PARAMS) {
			if(ws.user_settings.spoilers) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.national_definitions.global_flag_variable_names[trigger::payload(tval[1]).glob_id]);
				text::localised_format_box(ws, layout, box, "set_global_flag", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_clr_global_flag(EFFECT_DISPLAY_PARAMS) {
			if(ws.user_settings.spoilers) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.national_definitions.global_flag_variable_names[trigger::payload(tval[1]).glob_id]);
				text::localised_format_box(ws, layout, box, "remove_global_flag", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_nationalvalue_province(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, ws.world.modifier_get_name(trigger::payload(tval[1]).mod_id));
			text::localised_format_box(ws, layout, box, "change_national_value", m);
			text::close_layout_box(layout, box);
			if(ws.user_settings.spoilers) {
				modifier_description(ws, layout, trigger::payload(tval[1]).mod_id, indentation + indentation_amount);
			}
			return 0;
		}
		uint32_t ef_nationalvalue_nation(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, ws.world.modifier_get_name(trigger::payload(tval[1]).mod_id));
			text::localised_format_box(ws, layout, box, "change_national_value", m);
			text::close_layout_box(layout, box);
			if(ws.user_settings.spoilers) {
				modifier_description(ws, layout, trigger::payload(tval[1]).mod_id, indentation + indentation_amount);
			}
			return 0;
		}
		uint32_t ef_civilized_yes(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::localised_format_box(ws, layout, box, "become_civ");
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_civilized_no(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::localised_format_box(ws, layout, box, "become_unciv");
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_is_slave_state_no(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::localised_format_box(ws, layout, box, "free_slave_state");
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_is_slave_pop_no(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::localised_format_box(ws, layout, box, "free_slave_pop");
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_is_slave_province_no(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::localised_format_box(ws, layout, box, "free_slave_state");
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_election(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::localised_format_box(ws, layout, box, "hold_election");
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_social_reform(EFFECT_DISPLAY_PARAMS) {
			auto n = trigger::to_nation(primary_slot);
			auto opt = fatten(ws.world, trigger::payload(tval[1]).opt_id);
			if(ws.user_settings.spoilers || ws.world.nation_get_issues(n, opt.get_parent_issue()) != opt) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::issue, opt.get_parent_issue().get_name());
				text::add_to_substitution_map(m, text::variable_type::text, opt.get_name());
				text::localised_format_box(ws, layout, box, "issue_change", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_political_reform(EFFECT_DISPLAY_PARAMS) {
			auto n = trigger::to_nation(primary_slot);
			auto opt = fatten(ws.world, trigger::payload(tval[1]).opt_id);
			if(ws.user_settings.spoilers || ws.world.nation_get_issues(n, opt.get_parent_issue()) != opt) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::issue, opt.get_parent_issue().get_name());
				text::add_to_substitution_map(m, text::variable_type::text, opt.get_name());
				text::localised_format_box(ws, layout, box, "issue_change", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_social_reform_province(EFFECT_DISPLAY_PARAMS) {
			auto p = trigger::to_prov(primary_slot);
			auto n = ws.world.province_get_nation_from_province_ownership(p);
			auto opt = fatten(ws.world, trigger::payload(tval[1]).opt_id);
			if(ws.user_settings.spoilers || ws.world.nation_get_issues(n, opt.get_parent_issue()) != opt) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::issue, opt.get_parent_issue().get_name());
				text::add_to_substitution_map(m, text::variable_type::text, opt.get_name());
				text::localised_format_box(ws, layout, box, "issue_change", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_political_reform_province(EFFECT_DISPLAY_PARAMS) {
			auto p = trigger::to_prov(primary_slot);
			auto n = ws.world.province_get_nation_from_province_ownership(p);
			auto opt = fatten(ws.world, trigger::payload(tval[1]).opt_id);
			if(ws.user_settings.spoilers || ws.world.nation_get_issues(n, opt.get_parent_issue()) != opt) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::issue, opt.get_parent_issue().get_name());
				text::add_to_substitution_map(m, text::variable_type::text, opt.get_name());
				text::localised_format_box(ws, layout, box, "issue_change", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_tax_relative_income(EFFECT_DISPLAY_PARAMS) {
			auto amount = trigger::read_float_from_payload(tval + 1);
			if(primary_slot != -1) {
				auto income = ws.world.nation_get_total_poor_income(trigger::to_nation(primary_slot))
					+ ws.world.nation_get_total_middle_income(trigger::to_nation(primary_slot))
					+ ws.world.nation_get_total_rich_income(trigger::to_nation(primary_slot));
				auto combined_amount = income * amount;
				auto box = text::open_layout_box(layout, indentation);
				display_value(text::fp_currency{ combined_amount }, true, ws, layout, box);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "add_to_treasury", m);
				text::close_layout_box(layout, box);
			} else {
				auto box = text::open_layout_box(layout, indentation);
				display_value(text::fp_percentage{ amount }, true, ws, layout, box);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "add_relative_income", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_neutrality(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::localised_format_box(ws, layout, box, "make_neutral");
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_change_pop_size(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "pop_size", m);
			text::add_space_to_layout_box(ws, layout, box);
			auto const amount = trigger::read_int32_t_from_payload(tval + 1);
			display_value(text::fp_two_places{ float(amount) }, true, ws, layout, box);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_add_or_create_pop(EFFECT_DISPLAY_PARAMS) {
			auto const amount = trigger::read_int32_t_from_payload(tval + 1);
			auto const type = trigger::payload(tval[3]).popt_id;
			auto const cul = trigger::payload(tval[4]).cul_id;
			auto const rel = trigger::payload(tval[5]).rel_id;
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::x, text::pretty_integer{ amount });
			text::add_to_substitution_map(m, text::variable_type::type, ws.world.pop_type_get_name(type));
			text::add_to_substitution_map(m, text::variable_type::culture, ws.world.culture_get_name(cul));
			text::add_to_substitution_map(m, text::variable_type::religion, ws.world.religion_get_name(rel));
			text::localised_format_box(ws, layout, box, "add_or_create_pop", m);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_reduce_pop_nation(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "pop_size", m);
			text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_percentage{ trigger::read_float_from_payload(tval + 1) - 1.0f }, true, ws, layout, box);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_reduce_pop_state(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "pop_size", m);
			text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_percentage{ trigger::read_float_from_payload(tval + 1) - 1.0f }, true, ws, layout, box);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_reduce_pop_province(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "pop_size", m);
			text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_percentage{ trigger::read_float_from_payload(tval + 1) - 1.0f }, true, ws, layout, box);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_reduce_pop(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "pop_size", m);
			text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_percentage{ trigger::read_float_from_payload(tval + 1) - 1.0f }, true, ws, layout, box);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_move_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, trigger::payload(tval[1]).prov_id);
				text::localised_format_box(ws, layout, box, "move_pop_to", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_pop_type(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.pop_type_get_name(trigger::payload(tval[1]).popt_id));
				text::localised_format_box(ws, layout, box, "change_pop_type", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_years_of_research(EFFECT_DISPLAY_PARAMS) {
			if(primary_slot != -1) {
				auto amount = trigger::read_float_from_payload(tval + 1);
				auto result = nations::daily_research_points(ws, trigger::to_nation(primary_slot)) * 365.0f * amount;

				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "research_points", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(result), true, ws, layout, box);
				text::close_layout_box(layout, box);
				return 0;
			} else {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 1) }, true, ws, layout, box);
				text::localised_format_box(ws, layout, box, "years_of_research", m);
				text::close_layout_box(layout, box);
				return 0;
			}
		}
		uint32_t ef_prestige_factor_positive(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "prestige", m);
			text::add_space_to_layout_box(ws, layout, box);
		display_value(text::fp_percentage{ trigger::read_float_from_payload(tval + 1) }, true, ws, layout, box);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_prestige_factor_negative(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "prestige", m);
			text::add_space_to_layout_box(ws, layout, box);
		display_value(text::fp_percentage{ trigger::read_float_from_payload(tval + 1) }, true, ws, layout, box);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_military_reform(EFFECT_DISPLAY_PARAMS) {
			{
				auto opt = fatten(ws.world, trigger::payload(tval[1]).ropt_id);

				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::issue, opt.get_parent_reform().get_name());
				text::add_to_substitution_map(m, text::variable_type::text, opt.get_name());
				text::localised_format_box(ws, layout, box, "issue_change", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_economic_reform(EFFECT_DISPLAY_PARAMS) {
			{
				auto opt = fatten(ws.world, trigger::payload(tval[1]).ropt_id);

				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::issue, opt.get_parent_reform().get_name());
				text::add_to_substitution_map(m, text::variable_type::text, opt.get_name());
				text::localised_format_box(ws, layout, box, "issue_change", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_random_military_reforms(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::value, int64_t(tval[1]));
				text::localised_format_box(ws, layout, box, "remove_mil_reforms", m);
				text::close_layout_box(layout, box);
			}
			return tval[1];
		}
		uint32_t ef_remove_random_economic_reforms(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::value, int64_t(tval[1]));
				text::localised_format_box(ws, layout, box, "remove_econ_reforms", m);
				text::close_layout_box(layout, box);
			}
			return tval[1];
		}
		uint32_t ef_add_crime(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.culture_definitions.crimes[trigger::payload(tval[1]).crm_id].name);
				text::localised_format_box(ws, layout, box, "add_crime", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_crime_none(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "remove_crime", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_nationalize(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "perform_nationalization", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_build_factory_in_capital_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.factory_type_get_name(trigger::payload(tval[1]).fac_id));
				text::localised_format_box(ws, layout, box, "build_factory_in_capital", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_activate_technology(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.technology_get_name(trigger::payload(tval[1]).tech_id));
				text::localised_format_box(ws, layout, box, "enable_technology", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_activate_invention(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.invention_get_name(trigger::payload(tval[1]).invt_id));
				text::localised_format_box(ws, layout, box, "enable_invention", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_great_wars_enabled_yes(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "enable_great_wars", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_great_wars_enabled_no(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "disable_great_wars", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_world_wars_enabled_yes(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "enable_world_wars", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_world_wars_enabled_no(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "disable_world_wars", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_assimilate_province(EFFECT_DISPLAY_PARAMS) {
			if(auto owner = ws.world.province_get_nation_from_province_ownership(trigger::to_prov(primary_slot)); owner) {
				auto owner_c = ws.world.nation_get_primary_culture(owner);

				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.culture_get_name(owner_c));
				text::localised_format_box(ws, layout, box, "assimilate_province", m);
				text::close_layout_box(layout, box);

				return 0;
			} else {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				std::string t = text::produce_simple_string(ws, "owner_primary_culture");
			text::add_to_substitution_map(m, text::variable_type::text, std::string_view{ t });
				text::localised_format_box(ws, layout, box, "assimilate_province", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_assimilate_state(EFFECT_DISPLAY_PARAMS) {
			if(auto owner = ws.world.state_instance_get_nation_from_state_ownership(trigger::to_state(primary_slot)); owner) {
				auto owner_c = ws.world.nation_get_primary_culture(owner);

				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.culture_get_name(owner_c));
				text::localised_format_box(ws, layout, box, "assimilate_province", m);
				text::close_layout_box(layout, box);

				return 0;
			} else {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				std::string t = text::produce_simple_string(ws, "owner_primary_culture");
			text::add_to_substitution_map(m, text::variable_type::text, std::string_view{ t });
				text::localised_format_box(ws, layout, box, "assimilate_province", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_assimilate_pop(EFFECT_DISPLAY_PARAMS) {
			if(auto owner = nations::owner_of_pop(ws, trigger::to_pop(primary_slot)); owner) {
				auto owner_c = ws.world.nation_get_primary_culture(owner);

				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.culture_get_name(owner_c));
				text::localised_format_box(ws, layout, box, "assimilate_pop", m);
				text::close_layout_box(layout, box);

				return 0;
			} else {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				std::string t = text::produce_simple_string(ws, "owner_primary_culture");
			text::add_to_substitution_map(m, text::variable_type::text, std::string_view{ t });
				text::localised_format_box(ws, layout, box, "assimilate_pop", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_literacy(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "literacy", m);
			text::add_space_to_layout_box(ws, layout, box);
		display_value(text::fp_percentage{ trigger::read_float_from_payload(tval + 1) }, true, ws, layout, box);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_add_crisis_interest(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "add_crisis_interest", m);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_flashpoint_tension(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "flashpoint_tension_label", m);
			text::add_space_to_layout_box(ws, layout, box);
			display_value(int64_t(trigger::read_float_from_payload(tval + 1)), true, ws, layout, box);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_flashpoint_tension_province(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "flashpoint_tension_label", m);
			text::add_space_to_layout_box(ws, layout, box);
			display_value(int64_t(trigger::read_float_from_payload(tval + 1)), true, ws, layout, box);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_add_crisis_temperature(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "crisis_temperature_plain", m);
			text::add_space_to_layout_box(ws, layout, box);
			display_value(int64_t(trigger::read_float_from_payload(tval + 1)), true, ws, layout, box);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_consciousness_nation(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "consciousness", m);
			text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 1) }, true, ws, layout, box);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_consciousness_state(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "consciousness", m);
			text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 1) }, true, ws, layout, box);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_consciousness_province(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "consciousness", m);
			text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 1) }, true, ws, layout, box);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_consciousness(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "consciousness", m);
			text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 1) }, true, ws, layout, box);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_militancy_nation(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "militancy", m);
			text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 1) }, false, ws, layout, box);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_militancy_state(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "militancy", m);
			text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 1) }, false, ws, layout, box);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_militancy_province(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "militancy", m);
			text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 1) }, false, ws, layout, box);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_militancy(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "militancy", m);
			text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 1) }, false, ws, layout, box);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_rgo_size(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "rgo_size", m);
			text::add_space_to_layout_box(ws, layout, box);
			display_value(int64_t(trigger::payload(tval[1]).signed_value), false, ws, layout, box);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_trigger_revolt_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "trigger_every_revolt", m);
				text::close_layout_box(layout, box);
			}
			if(trigger::payload(tval[1]).reb_id) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.rebel_type_get_name(trigger::payload(tval[1]).reb_id));
				text::localised_format_box(ws, layout, box, "of_type", m);
				text::close_layout_box(layout, box);
			}
			if(trigger::payload(tval[2]).cul_id) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.culture_get_name(trigger::payload(tval[2]).cul_id));
				text::localised_format_box(ws, layout, box, "where_culture_is", m);
				text::close_layout_box(layout, box);
			}
			if(trigger::payload(tval[3]).rel_id) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.religion_get_name(trigger::payload(tval[3]).rel_id));
				text::localised_format_box(ws, layout, box, "where_religion_is", m);
				text::close_layout_box(layout, box);
			}
			if(trigger::payload(tval[4]).ideo_id) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.ideology_get_name(trigger::payload(tval[4]).ideo_id));
				text::localised_format_box(ws, layout, box, "where_ideology_is", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_trigger_revolt_state(EFFECT_DISPLAY_PARAMS) {
			return ef_trigger_revolt_nation(ws, tval, layout, 0, 0, 0, r_lo, r_hi, indentation);
		}
		uint32_t ef_trigger_revolt_province(EFFECT_DISPLAY_PARAMS) {
			return ef_trigger_revolt_nation(ws, tval, layout, 0, 0, 0, r_lo, r_hi, indentation);
		}
		uint32_t ef_diplomatic_influence(EFFECT_DISPLAY_PARAMS) {
			auto target = ws.world.national_identity_get_nation_from_identity_holder(trigger::payload(tval[1]).tag_id);
			if(target) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, target);
				text::localised_format_box(ws, layout, box, "diplomatic_influence_with", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(trigger::payload(tval[2]).signed_value), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_diplomatic_influence_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "diplomatic_influence_with", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(trigger::payload(tval[1]).signed_value), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_diplomatic_influence_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "diplomatic_influence_with", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(trigger::payload(tval[1]).signed_value), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_diplomatic_influence_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "diplomatic_influence_with", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(trigger::payload(tval[1]).signed_value), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_diplomatic_influence_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "diplomatic_influence_with", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(trigger::payload(tval[1]).signed_value), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_relation(EFFECT_DISPLAY_PARAMS) {
			auto target = ws.world.national_identity_get_nation_from_identity_holder(trigger::payload(tval[1]).tag_id);
			if(target) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, target);
				text::localised_format_box(ws, layout, box, "relations_with", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(trigger::payload(tval[2]).signed_value), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_relation_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "relations_with", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(trigger::payload(tval[1]).signed_value), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_relation_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "relations_with", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(trigger::payload(tval[1]).signed_value), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_relation_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "relations_with", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(trigger::payload(tval[1]).signed_value), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_relation_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "relations_with", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(trigger::payload(tval[1]).signed_value), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_province_modifier(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, ws.world.modifier_get_name(trigger::payload(tval[1]).mod_id));
			text::add_to_substitution_map(m, text::variable_type::date, ws.current_date + trigger::payload(tval[2]).signed_value);
			text::localised_format_box(ws, layout, box, "add_modifier_until", m);
			text::close_layout_box(layout, box);
			modifier_description(ws, layout, trigger::payload(tval[1]).mod_id, indentation + indentation_amount);
			return 0;
		}
		uint32_t ef_add_province_modifier_state(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, ws.world.modifier_get_name(trigger::payload(tval[1]).mod_id));
			text::add_to_substitution_map(m, text::variable_type::date, ws.current_date + trigger::payload(tval[2]).signed_value);
			text::localised_format_box(ws, layout, box, "add_modifier_until", m);
			text::close_layout_box(layout, box);
			modifier_description(ws, layout, trigger::payload(tval[1]).mod_id, indentation + indentation_amount);
			return 0;
		}
		uint32_t ef_add_province_modifier_no_duration(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, ws.world.modifier_get_name(trigger::payload(tval[1]).mod_id));
			text::localised_format_box(ws, layout, box, "add_modifier", m);
			text::close_layout_box(layout, box);
			modifier_description(ws, layout, trigger::payload(tval[1]).mod_id, indentation + indentation_amount);
			return 0;
		}
		uint32_t ef_add_province_modifier_state_no_duration(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, ws.world.modifier_get_name(trigger::payload(tval[1]).mod_id));
			text::localised_format_box(ws, layout, box, "add_modifier", m);
			text::close_layout_box(layout, box);
			modifier_description(ws, layout, trigger::payload(tval[1]).mod_id, indentation + indentation_amount);
			return 0;
		}
		uint32_t ef_add_country_modifier(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, ws.world.modifier_get_name(trigger::payload(tval[1]).mod_id));
			text::add_to_substitution_map(m, text::variable_type::date, ws.current_date + trigger::payload(tval[2]).signed_value);
			text::localised_format_box(ws, layout, box, "add_modifier_until", m);
			text::close_layout_box(layout, box);
			modifier_description(ws, layout, trigger::payload(tval[1]).mod_id, indentation + indentation_amount);
			return 0;
		}
		uint32_t ef_add_country_modifier_no_duration(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, ws.world.modifier_get_name(trigger::payload(tval[1]).mod_id));
			text::localised_format_box(ws, layout, box, "add_modifier", m);
			text::close_layout_box(layout, box);
			modifier_description(ws, layout, trigger::payload(tval[1]).mod_id, indentation + indentation_amount);
			return 0;
		}
		uint32_t ef_casus_belli_tag(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			auto months = trigger::payload(tval[2]).signed_value;
			auto tag_target = trigger::payload(tval[3]).tag_id;

			auto target = ws.world.national_identity_get_nation_from_identity_holder(tag_target);
			if(target) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, target);
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::add_to_substitution_map(m, text::variable_type::date, ws.current_date + 31 * months);
				text::localised_format_box(ws, layout, box, months != 0 ? "add_cb_months" : "add_cb_no_months", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_casus_belli_int(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			auto months = trigger::payload(tval[2]).signed_value;
			if(auto holder = ws.world.province_get_nation_from_province_ownership(trigger::payload(tval[3]).prov_id); holder) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, holder);
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::add_to_substitution_map(m, text::variable_type::date, ws.current_date + 31 * months);
				text::localised_format_box(ws, layout, box, months != 0 ? "add_cb_months" : "add_cb_no_months", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_casus_belli_this_nation(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			auto months = trigger::payload(tval[2]).signed_value;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::add_to_substitution_map(m, text::variable_type::date, ws.current_date + 31 * months);
				text::localised_format_box(ws, layout, box, months != 0 ? "add_cb_months" : "add_cb_no_months", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_casus_belli_this_state(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			auto months = trigger::payload(tval[2]).signed_value;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_state(this_slot)), "this_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::add_to_substitution_map(m, text::variable_type::date, ws.current_date + 31 * months);
				text::localised_format_box(ws, layout, box, months != 0 ? "add_cb_months" : "add_cb_no_months", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_casus_belli_this_province(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			auto months = trigger::payload(tval[2]).signed_value;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::add_to_substitution_map(m, text::variable_type::date, ws.current_date + 31 * months);
				text::localised_format_box(ws, layout, box, months != 0 ? "add_cb_months" : "add_cb_no_months", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_casus_belli_this_pop(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			auto months = trigger::payload(tval[2]).signed_value;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_pop(this_slot)), "this_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::add_to_substitution_map(m, text::variable_type::date, ws.current_date + 31 * months);
				text::localised_format_box(ws, layout, box, months != 0 ? "add_cb_months" : "add_cb_no_months", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_casus_belli_from_nation(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			auto months = trigger::payload(tval[2]).signed_value;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::add_to_substitution_map(m, text::variable_type::date, ws.current_date + 31 * months);
				text::localised_format_box(ws, layout, box, months != 0 ? "add_cb_months" : "add_cb_no_months", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_casus_belli_from_province(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			auto months = trigger::payload(tval[2]).signed_value;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::add_to_substitution_map(m, text::variable_type::date, ws.current_date + 31 * months);
				text::localised_format_box(ws, layout, box, months != 0 ? "add_cb_months" : "add_cb_no_months", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_casus_belli_tag(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			auto months = trigger::payload(tval[2]).signed_value;
			auto tag_target = trigger::payload(tval[3]).tag_id;

			auto target = ws.world.national_identity_get_nation_from_identity_holder(tag_target);
			if(target) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, target);
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::add_to_substitution_map(m, text::variable_type::date, ws.current_date + 31 * months);
				text::localised_format_box(ws, layout, box, months != 0 ? "add_cb_reversed_months" : "add_cb_reversed_no_months", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_casus_belli_int(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			auto months = trigger::payload(tval[2]).signed_value;
			if(auto holder = ws.world.province_get_nation_from_province_ownership(trigger::payload(tval[3]).prov_id); holder) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, holder);
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::add_to_substitution_map(m, text::variable_type::date, ws.current_date + 31 * months);
				text::localised_format_box(ws, layout, box, months != 0 ? "add_cb_reversed_months" : "add_cb_reversed_no_months", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_casus_belli_this_nation(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			auto months = trigger::payload(tval[2]).signed_value;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::add_to_substitution_map(m, text::variable_type::date, ws.current_date + 31 * months);
				text::localised_format_box(ws, layout, box, months != 0 ? "add_cb_reversed_months" : "add_cb_reversed_no_months", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_casus_belli_this_state(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			auto months = trigger::payload(tval[2]).signed_value;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_state(this_slot)), "this_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::add_to_substitution_map(m, text::variable_type::date, ws.current_date + 31 * months);
				text::localised_format_box(ws, layout, box, months != 0 ? "add_cb_reversed_months" : "add_cb_reversed_no_months", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_casus_belli_this_province(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			auto months = trigger::payload(tval[2]).signed_value;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::add_to_substitution_map(m, text::variable_type::date, ws.current_date + 31 * months);
				text::localised_format_box(ws, layout, box, months != 0 ? "add_cb_reversed_months" : "add_cb_reversed_no_months", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_casus_belli_this_pop(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			auto months = trigger::payload(tval[2]).signed_value;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_pop(this_slot)), "this_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::add_to_substitution_map(m, text::variable_type::date, ws.current_date + 31 * months);
				text::localised_format_box(ws, layout, box, months != 0 ? "add_cb_reversed_months" : "add_cb_reversed_no_months", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_casus_belli_from_nation(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			auto months = trigger::payload(tval[2]).signed_value;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::add_to_substitution_map(m, text::variable_type::date, ws.current_date + 31 * months);
				text::localised_format_box(ws, layout, box, months != 0 ? "add_cb_reversed_months" : "add_cb_reversed_no_months", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_casus_belli_from_province(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			auto months = trigger::payload(tval[2]).signed_value;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::add_to_substitution_map(m, text::variable_type::date, ws.current_date + 31 * months);
				text::localised_format_box(ws, layout, box, months != 0 ? "add_cb_reversed_months" : "add_cb_reversed_no_months", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_casus_belli_tag(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			auto tag_target = trigger::payload(tval[2]).tag_id;

			auto target = ws.world.national_identity_get_nation_from_identity_holder(tag_target);
			if(target) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, target);
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::localised_format_box(ws, layout, box, "remove_cb", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_casus_belli_int(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			if(auto holder = ws.world.province_get_nation_from_province_ownership(trigger::payload(tval[2]).prov_id); holder) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, holder);
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::localised_format_box(ws, layout, box, "remove_cb", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_casus_belli_this_nation(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::localised_format_box(ws, layout, box, "remove_cb", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_casus_belli_this_state(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_state(this_slot)), "this_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::localised_format_box(ws, layout, box, "remove_cb", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_casus_belli_this_province(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::localised_format_box(ws, layout, box, "remove_cb", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_casus_belli_this_pop(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_pop(this_slot)), "this_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::localised_format_box(ws, layout, box, "remove_cb", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_casus_belli_from_nation(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::localised_format_box(ws, layout, box, "remove_cb", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_casus_belli_from_province(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::localised_format_box(ws, layout, box, "remove_cb", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_this_remove_casus_belli_tag(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			auto tag_target = trigger::payload(tval[2]).tag_id;

			auto target = ws.world.national_identity_get_nation_from_identity_holder(tag_target);
			if(target) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, target);
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::localised_format_box(ws, layout, box, "remove_cb_reversed", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_this_remove_casus_belli_int(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			if(auto holder = ws.world.province_get_nation_from_province_ownership(trigger::payload(tval[2]).prov_id); holder) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, holder);
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::localised_format_box(ws, layout, box, "remove_cb_reversed", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_this_remove_casus_belli_this_nation(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::localised_format_box(ws, layout, box, "remove_cb_reversed", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_this_remove_casus_belli_this_state(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_state(this_slot)), "this_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::localised_format_box(ws, layout, box, "remove_cb_reversed", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_this_remove_casus_belli_this_province(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::localised_format_box(ws, layout, box, "remove_cb_reversed", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_this_remove_casus_belli_this_pop(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_pop(this_slot)), "this_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::localised_format_box(ws, layout, box, "remove_cb_reversed", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_this_remove_casus_belli_from_nation(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::localised_format_box(ws, layout, box, "remove_cb_reversed", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_this_remove_casus_belli_from_province(EFFECT_DISPLAY_PARAMS) {
			auto type = trigger::payload(tval[1]).cb_id;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::name, ws.world.cb_type_get_name(type));
				text::localised_format_box(ws, layout, box, "remove_cb_reversed", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}

		uint32_t ef_add_truce_tag(EFFECT_DISPLAY_PARAMS) {
			auto target = ws.world.national_identity_get_nation_from_identity_holder(trigger::payload(tval[1]).tag_id);
			if(!target)
			return 0;
			text::add_line(ws, layout, "att_truce_with", text::variable_type::text, target, text::variable_type::y, ws.current_date + int32_t(tval[2] * 30.5), indentation);
			return 0;
		}
		uint32_t ef_add_truce_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::y, ws.current_date + int32_t(tval[1] * 30.5));
				text::localised_format_box(ws, layout, box, "att_truce_with", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_truce_this_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_state(this_slot)), "this_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::y, ws.current_date + int32_t(tval[1] * 30.5));
				text::localised_format_box(ws, layout, box, "att_truce_with", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_truce_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::y, ws.current_date + int32_t(tval[1] * 30.5));
				text::localised_format_box(ws, layout, box, "att_truce_with", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_truce_this_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_pop(this_slot)), "this_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::y, ws.current_date + int32_t(tval[1] * 30.5));
				text::localised_format_box(ws, layout, box, "att_truce_with", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_truce_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::y, ws.current_date + int32_t(tval[1] * 30.5));
				text::localised_format_box(ws, layout, box, "att_truce_with", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_truce_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::add_to_substitution_map(m, text::variable_type::y, ws.current_date + int32_t(tval[1] * 30.5));
				text::localised_format_box(ws, layout, box, "att_truce_with", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_call_allies(EFFECT_DISPLAY_PARAMS) {
			text::add_line(ws, layout, "att_call_allies_effect");
			return 0;
		}

		uint32_t ef_ruling_party_this(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, ws.world.nation_get_ruling_party(trigger::to_nation(this_slot)).get_ideology().get_name());
			text::localised_format_box(ws, layout, box, "change_ruling_party", m);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_ruling_party_from(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, ws.world.nation_get_ruling_party(trigger::to_nation(from_slot)).get_ideology().get_name());
			text::localised_format_box(ws, layout, box, "change_ruling_party", m);
			text::close_layout_box(layout, box);
			return 0;
		}

		uint32_t ef_war_tag(EFFECT_DISPLAY_PARAMS) {
			auto target = ws.world.national_identity_get_nation_from_identity_holder(trigger::payload(tval[1]).tag_id);
			if(!target)
			return 0;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, target);
				text::localised_format_box(ws, layout, box, "declare_war_on", m);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[5]).cb_id));
				text::localised_format_box(ws, layout, box, "attacker_wg_label", m);
				text::close_layout_box(layout, box);
			}
			if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[6]).prov_id); si) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, si);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(auto t = trigger::payload(tval[7]).tag_id; t) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, t);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(trigger::payload(tval[2]).cb_id) {
				{
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[2]).cb_id));
					text::localised_format_box(ws, layout, box, "defender_wg_label", m);
					text::close_layout_box(layout, box);
				}
				if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[3]).prov_id); si) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, si);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
				if(auto t = trigger::payload(tval[4]).tag_id; t) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, t);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_war_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "declare_war_on", m);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[4]).cb_id));
				text::localised_format_box(ws, layout, box, "attacker_wg_label", m);
				text::close_layout_box(layout, box);
			}
			if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[5]).prov_id); si) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, si);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(auto t = trigger::payload(tval[6]).tag_id; t) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, t);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(trigger::payload(tval[1]).cb_id) {
				{
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[1]).cb_id));
					text::localised_format_box(ws, layout, box, "defender_wg_label", m);
					text::close_layout_box(layout, box);
				}
				if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[2]).prov_id); si) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, si);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
				if(auto t = trigger::payload(tval[3]).tag_id; t) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, t);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_war_this_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_state(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "declare_war_on", m);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[4]).cb_id));
				text::localised_format_box(ws, layout, box, "attacker_wg_label", m);
				text::close_layout_box(layout, box);
			}
			if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[5]).prov_id); si) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, si);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(auto t = trigger::payload(tval[6]).tag_id; t) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, t);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(trigger::payload(tval[1]).cb_id) {
				{
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[1]).cb_id));
					text::localised_format_box(ws, layout, box, "defender_wg_label", m);
					text::close_layout_box(layout, box);
				}
				if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[2]).prov_id); si) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, si);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
				if(auto t = trigger::payload(tval[3]).tag_id; t) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, t);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_war_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "declare_war_on", m);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[4]).cb_id));
				text::localised_format_box(ws, layout, box, "attacker_wg_label", m);
				text::close_layout_box(layout, box);
			}
			if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[5]).prov_id); si) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, si);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(auto t = trigger::payload(tval[6]).tag_id; t) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, t);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(trigger::payload(tval[1]).cb_id) {
				{
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[1]).cb_id));
					text::localised_format_box(ws, layout, box, "defender_wg_label", m);
					text::close_layout_box(layout, box);
				}
				if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[2]).prov_id); si) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, si);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
				if(auto t = trigger::payload(tval[3]).tag_id; t) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, t);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_war_this_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_pop(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "declare_war_on", m);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[4]).cb_id));
				text::localised_format_box(ws, layout, box, "attacker_wg_label", m);
				text::close_layout_box(layout, box);
			}
			if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[5]).prov_id); si) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, si);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(auto t = trigger::payload(tval[6]).tag_id; t) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, t);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(trigger::payload(tval[1]).cb_id) {
				{
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[1]).cb_id));
					text::localised_format_box(ws, layout, box, "defender_wg_label", m);
					text::close_layout_box(layout, box);
				}
				if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[2]).prov_id); si) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, si);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
				if(auto t = trigger::payload(tval[3]).tag_id; t) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, t);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_war_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "declare_war_on", m);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[4]).cb_id));
				text::localised_format_box(ws, layout, box, "attacker_wg_label", m);
				text::close_layout_box(layout, box);
			}
			if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[5]).prov_id); si) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, si);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(auto t = trigger::payload(tval[6]).tag_id; t) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, t);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(trigger::payload(tval[1]).cb_id) {
				{
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[1]).cb_id));
					text::localised_format_box(ws, layout, box, "defender_wg_label", m);
					text::close_layout_box(layout, box);
				}
				if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[2]).prov_id); si) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, si);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
				if(auto t = trigger::payload(tval[3]).tag_id; t) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, t);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_war_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "declare_war_on", m);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[4]).cb_id));
				text::localised_format_box(ws, layout, box, "attacker_wg_label", m);
				text::close_layout_box(layout, box);
			}
			if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[5]).prov_id); si) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, si);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(auto t = trigger::payload(tval[6]).tag_id; t) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, t);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(trigger::payload(tval[1]).cb_id) {
				{
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[1]).cb_id));
					text::localised_format_box(ws, layout, box, "defender_wg_label", m);
					text::close_layout_box(layout, box);
				}
				if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[2]).prov_id); si) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, si);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
				if(auto t = trigger::payload(tval[3]).tag_id; t) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, t);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_war_no_ally_tag(EFFECT_DISPLAY_PARAMS) {
			auto target = ws.world.national_identity_get_nation_from_identity_holder(trigger::payload(tval[1]).tag_id);
			if(!target)
			return 0;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, target);
				text::localised_format_box(ws, layout, box, "declare_war_on", m);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "attacker_no_allies", m);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[5]).cb_id));
				text::localised_format_box(ws, layout, box, "attacker_wg_label", m);
				text::close_layout_box(layout, box);
			}
			if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[6]).prov_id); si) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, si);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(auto t = trigger::payload(tval[7]).tag_id; t) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, t);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(trigger::payload(tval[2]).cb_id) {
				{
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[2]).cb_id));
					text::localised_format_box(ws, layout, box, "defender_wg_label", m);
					text::close_layout_box(layout, box);
				}
				if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[3]).prov_id); si) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, si);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
				if(auto t = trigger::payload(tval[4]).tag_id; t) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, t);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_war_no_ally_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "declare_war_on", m);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "attacker_no_allies", m);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[4]).cb_id));
				text::localised_format_box(ws, layout, box, "attacker_wg_label", m);
				text::close_layout_box(layout, box);
			}
			if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[5]).prov_id); si) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, si);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(auto t = trigger::payload(tval[6]).tag_id; t) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, t);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(trigger::payload(tval[1]).cb_id) {
				{
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[1]).cb_id));
					text::localised_format_box(ws, layout, box, "attacker_wg_label", m);
					text::close_layout_box(layout, box);
				}
				if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[2]).prov_id); si) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, si);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
				if(auto t = trigger::payload(tval[3]).tag_id; t) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, t);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_war_no_ally_this_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_state(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "declare_war_on", m);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "attacker_no_allies", m);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[4]).cb_id));
				text::localised_format_box(ws, layout, box, "attacker_wg_label", m);
				text::close_layout_box(layout, box);
			}
			if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[5]).prov_id); si) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, si);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(auto t = trigger::payload(tval[6]).tag_id; t) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, t);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(trigger::payload(tval[1]).cb_id) {
				{
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[1]).cb_id));
					text::localised_format_box(ws, layout, box, "defender_wg_label", m);
					text::close_layout_box(layout, box);
				}
				if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[2]).prov_id); si) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, si);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
				if(auto t = trigger::payload(tval[3]).tag_id; t) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, t);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_war_no_ally_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "declare_war_on", m);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "attacker_no_allies", m);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[4]).cb_id));
				text::localised_format_box(ws, layout, box, "attacker_wg_label", m);
				text::close_layout_box(layout, box);
			}
			if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[5]).prov_id); si) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, si);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(auto t = trigger::payload(tval[6]).tag_id; t) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, t);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(trigger::payload(tval[1]).cb_id) {
				{
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[1]).cb_id));
					text::localised_format_box(ws, layout, box, "defender_wg_label", m);
					text::close_layout_box(layout, box);
				}
				if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[2]).prov_id); si) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, si);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
				if(auto t = trigger::payload(tval[3]).tag_id; t) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, t);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_war_no_ally_this_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_pop(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "declare_war_on", m);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "attacker_no_allies", m);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[4]).cb_id));
				text::localised_format_box(ws, layout, box, "attacker_wg_label", m);
				text::close_layout_box(layout, box);
			}
			if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[5]).prov_id); si) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, si);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(auto t = trigger::payload(tval[6]).tag_id; t) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, t);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(trigger::payload(tval[1]).cb_id) {
				{
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[1]).cb_id));
					text::localised_format_box(ws, layout, box, "defender_wg_label", m);
					text::close_layout_box(layout, box);
				}
				if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[2]).prov_id); si) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, si);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
				if(auto t = trigger::payload(tval[3]).tag_id; t) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, t);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_war_no_ally_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "declare_war_on", m);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "attacker_no_allies", m);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[4]).cb_id));
				text::localised_format_box(ws, layout, box, "attacker_wg_label", m);
				text::close_layout_box(layout, box);
			}
			if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[5]).prov_id); si) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, si);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(auto t = trigger::payload(tval[6]).tag_id; t) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, t);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(trigger::payload(tval[1]).cb_id) {
				{
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[1]).cb_id));
					text::localised_format_box(ws, layout, box, "defender_wg_label", m);
					text::close_layout_box(layout, box);
				}
				if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[2]).prov_id); si) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, si);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
				if(auto t = trigger::payload(tval[3]).tag_id; t) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, t);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_war_no_ally_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "declare_war_on", m);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "attacker_no_allies", m);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[4]).cb_id));
				text::localised_format_box(ws, layout, box, "attacker_wg_label", m);
				text::close_layout_box(layout, box);
			}
			if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[5]).prov_id); si) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, si);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(auto t = trigger::payload(tval[6]).tag_id; t) {
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, t);
				text::localised_format_box(ws, layout, box, "for_text", m);
				text::close_layout_box(layout, box);
			}
			if(trigger::payload(tval[1]).cb_id) {
				{
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[1]).cb_id));
					text::localised_format_box(ws, layout, box, "defender_wg_label", m);
					text::close_layout_box(layout, box);
				}
				if(auto si = ws.world.province_get_state_membership(trigger::payload(tval[2]).prov_id); si) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, si);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
				if(auto t = trigger::payload(tval[3]).tag_id; t) {
					auto box = text::open_layout_box(layout, indentation + indentation_amount);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, t);
					text::localised_format_box(ws, layout, box, "for_text", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_country_event_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.national_event_get_name(trigger::payload(tval[1]).nev_id));
			populate_event_submap(ws, m, event::pending_human_n_event {r_lo, r_hi + 1, primary_slot, this_slot, ws.current_date, trigger::payload(tval[1]).nev_id, trigger::to_nation(primary_slot), event::slot_type::nation, event::slot_type::nation});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_country_event_immediate_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.national_event_get_name(trigger::payload(tval[1]).nev_id));
			populate_event_submap(ws, m, event::pending_human_n_event {r_lo, r_hi + 1, primary_slot, this_slot, ws.current_date, trigger::payload(tval[1]).nev_id, trigger::to_nation(primary_slot), event::slot_type::nation, event::slot_type::nation});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_province_event_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.provincial_event_get_name(trigger::payload(tval[1]).pev_id));

				populate_event_submap(ws, m, event::pending_human_p_event {r_lo, r_hi + 1,
				this_slot, ws.current_date, trigger::payload(tval[1]).pev_id, trigger::to_prov(primary_slot), event::slot_type::nation});

				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_province_event_immediate_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.provincial_event_get_name(trigger::payload(tval[1]).pev_id));
				populate_event_submap(ws, m, event::pending_human_p_event {r_lo, r_hi + 1,
				this_slot, ws.current_date, trigger::payload(tval[1]).pev_id, trigger::to_prov(primary_slot), event::slot_type::nation});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_country_event_this_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.national_event_get_name(trigger::payload(tval[1]).nev_id));
			populate_event_submap(ws, m, event::pending_human_n_event {r_lo, r_hi + 1, primary_slot, this_slot, ws.current_date, trigger::payload(tval[1]).nev_id, trigger::to_nation(primary_slot), event::slot_type::nation, event::slot_type::state});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_country_event_immediate_this_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.national_event_get_name(trigger::payload(tval[1]).nev_id));
			populate_event_submap(ws, m, event::pending_human_n_event {r_lo, r_hi + 1, primary_slot, this_slot, ws.current_date, trigger::payload(tval[1]).nev_id, trigger::to_nation(primary_slot), event::slot_type::nation, event::slot_type::state});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_province_event_this_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.provincial_event_get_name(trigger::payload(tval[1]).pev_id));
				populate_event_submap(ws, m, event::pending_human_p_event {r_lo, r_hi + 1,
				this_slot, ws.current_date, trigger::payload(tval[1]).pev_id, trigger::to_prov(primary_slot), event::slot_type::state});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_province_event_immediate_this_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.provincial_event_get_name(trigger::payload(tval[1]).pev_id));
				populate_event_submap(ws, m, event::pending_human_p_event {r_lo, r_hi + 1,
				this_slot, ws.current_date, trigger::payload(tval[1]).pev_id, trigger::to_prov(primary_slot), event::slot_type::state});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_country_event_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.national_event_get_name(trigger::payload(tval[1]).nev_id));
			populate_event_submap(ws, m, event::pending_human_n_event {r_lo, r_hi + 1, primary_slot, this_slot, ws.current_date, trigger::payload(tval[1]).nev_id, trigger::to_nation(primary_slot), event::slot_type::nation, event::slot_type::province});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_country_event_immediate_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.national_event_get_name(trigger::payload(tval[1]).nev_id));
			populate_event_submap(ws, m, event::pending_human_n_event {r_lo, r_hi + 1, primary_slot, this_slot, ws.current_date, trigger::payload(tval[1]).nev_id, trigger::to_nation(primary_slot), event::slot_type::nation, event::slot_type::province});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_province_event_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.provincial_event_get_name(trigger::payload(tval[1]).pev_id));
				populate_event_submap(ws, m, event::pending_human_p_event {r_lo, r_hi + 1,
				this_slot, ws.current_date, trigger::payload(tval[1]).pev_id, trigger::to_prov(primary_slot), event::slot_type::province});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_province_event_immediate_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.provincial_event_get_name(trigger::payload(tval[1]).pev_id));
				populate_event_submap(ws, m, event::pending_human_p_event {r_lo, r_hi + 1,
				this_slot, ws.current_date, trigger::payload(tval[1]).pev_id, trigger::to_prov(primary_slot), event::slot_type::province});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_country_event_this_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.national_event_get_name(trigger::payload(tval[1]).nev_id));
			populate_event_submap(ws, m, event::pending_human_n_event {r_lo, r_hi + 1, primary_slot, this_slot, ws.current_date, trigger::payload(tval[1]).nev_id, trigger::to_nation(primary_slot), event::slot_type::nation, event::slot_type::pop});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_country_event_immediate_this_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.national_event_get_name(trigger::payload(tval[1]).nev_id));
			populate_event_submap(ws, m, event::pending_human_n_event {r_lo, r_hi + 1, primary_slot, this_slot, ws.current_date, trigger::payload(tval[1]).nev_id, trigger::to_nation(primary_slot), event::slot_type::nation, event::slot_type::pop});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_province_event_this_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.provincial_event_get_name(trigger::payload(tval[1]).pev_id));
				populate_event_submap(ws, m, event::pending_human_p_event {r_lo, r_hi + 1,
				this_slot, ws.current_date, trigger::payload(tval[1]).pev_id, trigger::to_prov(primary_slot), event::slot_type::pop});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_province_event_immediate_this_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.provincial_event_get_name(trigger::payload(tval[1]).pev_id));
				populate_event_submap(ws, m, event::pending_human_p_event {r_lo, r_hi + 1,
				this_slot, ws.current_date, trigger::payload(tval[1]).pev_id, trigger::to_prov(primary_slot), event::slot_type::pop});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_country_event_province_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.national_event_get_name(trigger::payload(tval[1]).nev_id));
				populate_event_submap(ws, m, event::pending_human_n_event {r_lo, r_hi + 1,
					trigger::to_generic(ws.world.province_get_nation_from_province_ownership(trigger::to_prov(primary_slot))),
				this_slot, ws.current_date, trigger::payload(tval[1]).nev_id, trigger::to_nation(primary_slot), event::slot_type::nation, event::slot_type::nation});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_country_event_immediate_province_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.national_event_get_name(trigger::payload(tval[1]).nev_id));
				populate_event_submap(ws, m, event::pending_human_n_event {r_lo, r_hi + 1,
					trigger::to_generic(ws.world.province_get_nation_from_province_ownership(trigger::to_prov(primary_slot))),
				this_slot, ws.current_date, trigger::payload(tval[1]).nev_id, trigger::to_nation(primary_slot), event::slot_type::nation, event::slot_type::nation});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_country_event_province_this_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.provincial_event_get_name(trigger::payload(tval[1]).pev_id));
				populate_event_submap(ws, m, event::pending_human_n_event {r_lo, r_hi + 1,
					trigger::to_generic(ws.world.province_get_nation_from_province_ownership(trigger::to_prov(primary_slot))),
				this_slot, ws.current_date, trigger::payload(tval[1]).nev_id, trigger::to_nation(primary_slot), event::slot_type::nation, event::slot_type::state});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_country_event_immediate_province_this_state(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.provincial_event_get_name(trigger::payload(tval[1]).pev_id));
				populate_event_submap(ws, m, event::pending_human_n_event {r_lo, r_hi + 1,
					trigger::to_generic(ws.world.province_get_nation_from_province_ownership(trigger::to_prov(primary_slot))),
				this_slot, ws.current_date, trigger::payload(tval[1]).nev_id, trigger::to_nation(primary_slot), event::slot_type::nation, event::slot_type::state});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_country_event_province_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.national_event_get_name(trigger::payload(tval[1]).nev_id));
				populate_event_submap(ws, m, event::pending_human_n_event {r_lo, r_hi + 1,
					trigger::to_generic(ws.world.province_get_nation_from_province_ownership(trigger::to_prov(primary_slot))),
				this_slot, ws.current_date, trigger::payload(tval[1]).nev_id, trigger::to_nation(primary_slot), event::slot_type::nation, event::slot_type::province});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_country_event_immediate_province_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.national_event_get_name(trigger::payload(tval[1]).nev_id));
				populate_event_submap(ws, m, event::pending_human_n_event {r_lo, r_hi + 1,
					trigger::to_generic(ws.world.province_get_nation_from_province_ownership(trigger::to_prov(primary_slot))),
				this_slot, ws.current_date, trigger::payload(tval[1]).nev_id, trigger::to_nation(primary_slot), event::slot_type::nation, event::slot_type::province});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_country_event_province_this_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.provincial_event_get_name(trigger::payload(tval[1]).pev_id));
				populate_event_submap(ws, m, event::pending_human_n_event {r_lo, r_hi + 1,
					trigger::to_generic(ws.world.province_get_nation_from_province_ownership(trigger::to_prov(primary_slot))),
				this_slot, ws.current_date, trigger::payload(tval[1]).nev_id, trigger::to_nation(primary_slot), event::slot_type::nation, event::slot_type::pop});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_country_event_immediate_province_this_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.world.provincial_event_get_name(trigger::payload(tval[1]).pev_id));
				populate_event_submap(ws, m, event::pending_human_n_event {r_lo, r_hi + 1,
					trigger::to_generic(ws.world.province_get_nation_from_province_ownership(trigger::to_prov(primary_slot))),
				this_slot, ws.current_date, trigger::payload(tval[1]).nev_id, trigger::to_nation(primary_slot), event::slot_type::nation, event::slot_type::pop});
				text::localised_format_box(ws, layout, box, "event_fires", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}

		uint32_t ef_sub_unit_int(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "no_effect", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_sub_unit_this(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "no_effect", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_sub_unit_from(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "no_effect", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_sub_unit_current(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "no_effect", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_set_variable(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.national_definitions.variable_names[trigger::payload(tval[1]).natv_id]);
			text::add_to_substitution_map(m, text::variable_type::value, text::fp_one_place{ trigger::read_float_from_payload(tval + 2) });
				text::localised_format_box(ws, layout, box, "set_national_variable_to", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_change_variable(EFFECT_DISPLAY_PARAMS) {
			auto amount = trigger::read_float_from_payload(tval + 2);
			if(amount >= 0) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.national_definitions.variable_names[trigger::payload(tval[1]).natv_id]);
			text::add_to_substitution_map(m, text::variable_type::value, text::fp_one_place{ amount });
				text::localised_format_box(ws, layout, box, "increase_national_variable_by", m);
				text::close_layout_box(layout, box);
			} else {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.national_definitions.variable_names[trigger::payload(tval[1]).natv_id]);
			text::add_to_substitution_map(m, text::variable_type::value, text::fp_one_place{ -amount });
				text::localised_format_box(ws, layout, box, "decrease_national_variable_by", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_ideology(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.ideology_get_name(trigger::payload(tval[1]).ideo_id));
				text::localised_format_box(ws, layout, box, "support_for_blank", m);
				text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_percentage{ trigger::read_float_from_payload(tval + 2) }, true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_upper_house(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.ideology_get_name(trigger::payload(tval[1]).ideo_id));
				text::localised_format_box(ws, layout, box, "uh_support_for_blank", m);
				text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_percentage{ trigger::read_float_from_payload(tval + 2) }, true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_militancy_issue(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				std::string t = text::produce_simple_string(ws, "militancy");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 2) }, false, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.issue_option_get_name(trigger::payload(tval[1]).opt_id));
				text::localised_format_box(ws, layout, box, "scaled_support", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_militancy_ideology(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				std::string t = text::produce_simple_string(ws, "militancy");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 2) }, false, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.ideology_get_name(trigger::payload(tval[1]).ideo_id));
				text::localised_format_box(ws, layout, box, "scaled_support", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_militancy_unemployment(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				std::string t = text::produce_simple_string(ws, "militancy");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 1) }, false, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::localised_format_box(ws, layout, box, "scaled_unemployment");
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_consciousness_issue(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				std::string t = text::produce_simple_string(ws, "consciousness");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 2) }, true, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.issue_option_get_name(trigger::payload(tval[1]).opt_id));
				text::localised_format_box(ws, layout, box, "scaled_support", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_consciousness_ideology(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				std::string t = text::produce_simple_string(ws, "consciousness");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 2) }, true, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.ideology_get_name(trigger::payload(tval[1]).ideo_id));
				text::localised_format_box(ws, layout, box, "scaled_support", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_consciousness_unemployment(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				std::string t = text::produce_simple_string(ws, "consciousness");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 1) }, true, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::localised_format_box(ws, layout, box, "scaled_unemployment");
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_militancy_nation_issue(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				std::string t = text::produce_simple_string(ws, "militancy");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 2) }, false, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.issue_option_get_name(trigger::payload(tval[1]).opt_id));
				text::localised_format_box(ws, layout, box, "scaled_support", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_militancy_nation_ideology(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				std::string t = text::produce_simple_string(ws, "militancy");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 2) }, false, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.ideology_get_name(trigger::payload(tval[1]).ideo_id));
				text::localised_format_box(ws, layout, box, "scaled_support", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_militancy_nation_unemployment(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				std::string t = text::produce_simple_string(ws, "militancy");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 1) }, false, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::localised_format_box(ws, layout, box, "scaled_unemployment");
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_consciousness_nation_issue(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				std::string t = text::produce_simple_string(ws, "consciousness");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 2) }, true, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.issue_option_get_name(trigger::payload(tval[1]).opt_id));
				text::localised_format_box(ws, layout, box, "scaled_support", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_consciousness_nation_ideology(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				std::string t = text::produce_simple_string(ws, "consciousness");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 2) }, true, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.ideology_get_name(trigger::payload(tval[1]).ideo_id));
				text::localised_format_box(ws, layout, box, "scaled_support", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_consciousness_nation_unemployment(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				std::string t = text::produce_simple_string(ws, "consciousness");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 1) }, true, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::localised_format_box(ws, layout, box, "scaled_unemployment");
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_militancy_state_issue(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				std::string t = text::produce_simple_string(ws, "militancy");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 2) }, false, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.issue_option_get_name(trigger::payload(tval[1]).opt_id));
				text::localised_format_box(ws, layout, box, "scaled_support", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_militancy_state_ideology(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				std::string t = text::produce_simple_string(ws, "militancy");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 2) }, false, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.ideology_get_name(trigger::payload(tval[1]).ideo_id));
				text::localised_format_box(ws, layout, box, "scaled_support", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_militancy_state_unemployment(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				std::string t = text::produce_simple_string(ws, "militancy");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 1) }, false, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::localised_format_box(ws, layout, box, "scaled_unemployment");
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_consciousness_state_issue(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				std::string t = text::produce_simple_string(ws, "consciousness");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 2) }, true, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.issue_option_get_name(trigger::payload(tval[1]).opt_id));
				text::localised_format_box(ws, layout, box, "scaled_support", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_consciousness_state_ideology(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				std::string t = text::produce_simple_string(ws, "consciousness");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 2) }, true, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.ideology_get_name(trigger::payload(tval[1]).ideo_id));
				text::localised_format_box(ws, layout, box, "scaled_support", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_consciousness_state_unemployment(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				std::string t = text::produce_simple_string(ws, "consciousness");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 1) }, true, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::localised_format_box(ws, layout, box, "scaled_unemployment");
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_militancy_province_issue(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				std::string t = text::produce_simple_string(ws, "militancy");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 2) }, false, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.issue_option_get_name(trigger::payload(tval[1]).opt_id));
				text::localised_format_box(ws, layout, box, "scaled_support", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_militancy_province_ideology(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				std::string t = text::produce_simple_string(ws, "militancy");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 2) }, false, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.ideology_get_name(trigger::payload(tval[1]).ideo_id));
				text::localised_format_box(ws, layout, box, "scaled_support", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_militancy_province_unemployment(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				std::string t = text::produce_simple_string(ws, "militancy");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 1) }, false, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::localised_format_box(ws, layout, box, "scaled_unemployment");
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_consciousness_province_issue(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				std::string t = text::produce_simple_string(ws, "consciousness");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 2) }, true, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.issue_option_get_name(trigger::payload(tval[1]).opt_id));
				text::localised_format_box(ws, layout, box, "scaled_support", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_consciousness_province_ideology(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				std::string t = text::produce_simple_string(ws, "consciousness");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 2) }, true, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.ideology_get_name(trigger::payload(tval[1]).ideo_id));
				text::localised_format_box(ws, layout, box, "scaled_support", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_scaled_consciousness_province_unemployment(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				std::string t = text::produce_simple_string(ws, "consciousness");
			display_value(text::fp_one_place{ trigger::read_float_from_payload(tval + 1) }, true, ws, layout, box);
				text::add_space_to_layout_box(ws, layout, box);
				text::add_to_layout_box(ws, layout, box, t);
				text::localised_format_box(ws, layout, box, "scaled_unemployment");
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_variable_good_name(EFFECT_DISPLAY_PARAMS) {
			{
				auto amount = trigger::read_float_from_payload(tval + 2);
				auto box = text::open_layout_box(layout, indentation);
			display_value(text::fp_one_place{ amount }, true, ws, layout, box);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.commodity_get_name(trigger::payload(tval[1]).com_id));
				text::localised_format_box(ws, layout, box, "stockpile_of", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_variable_good_name_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto amount = trigger::read_float_from_payload(tval + 2);
				auto box = text::open_layout_box(layout, indentation);
			display_value(text::fp_one_place{ amount }, true, ws, layout, box);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.commodity_get_name(trigger::payload(tval[1]).com_id));
				text::localised_format_box(ws, layout, box, "stockpile_of", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_define_general(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "create_general", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_define_admiral(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "create_admiral", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_dominant_issue(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.issue_option_get_name(trigger::payload(tval[1]).opt_id));
				text::localised_format_box(ws, layout, box, "support_for_blank", m);
				text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_percentage{ trigger::read_float_from_payload(tval + 2) }, true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_dominant_issue_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.issue_option_get_name(trigger::payload(tval[1]).opt_id));
				text::localised_format_box(ws, layout, box, "support_for_blank", m);
				text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_percentage{ trigger::read_float_from_payload(tval + 2) }, true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_add_war_goal(EFFECT_DISPLAY_PARAMS) {
			{
				auto amount = trigger::read_float_from_payload(tval + 2);
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.cb_type_get_name(trigger::payload(tval[1]).cb_id));
				std::string t;
				if(primary_slot != -1)
				text::add_to_substitution_map(m, text::variable_type::nation, trigger::to_nation(primary_slot));
				else {
					t = text::produce_simple_string(ws, "this_nation");
				text::add_to_substitution_map(m, text::variable_type::nation, std::string_view{ t });
				}
				text::localised_format_box(ws, layout, box, "add_war_goal", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_move_issue_percentage_nation(EFFECT_DISPLAY_PARAMS) {
			auto amount = trigger::read_float_from_payload(tval + 3);
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.issue_option_get_name(trigger::payload(tval[1]).opt_id));
				text::localised_format_box(ws, layout, box, "support_for_blank", m);
				text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_percentage{ -amount }, true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.issue_option_get_name(trigger::payload(tval[2]).opt_id));
				text::localised_format_box(ws, layout, box, "support_for_blank", m);
				text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_percentage{ amount }, true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_move_issue_percentage_state(EFFECT_DISPLAY_PARAMS) {
			auto amount = trigger::read_float_from_payload(tval + 3);
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.issue_option_get_name(trigger::payload(tval[1]).opt_id));
				text::localised_format_box(ws, layout, box, "support_for_blank", m);
				text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_percentage{ -amount }, true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.issue_option_get_name(trigger::payload(tval[2]).opt_id));
				text::localised_format_box(ws, layout, box, "support_for_blank", m);
				text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_percentage{ amount }, true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_move_issue_percentage_province(EFFECT_DISPLAY_PARAMS) {
			auto amount = trigger::read_float_from_payload(tval + 3);
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.issue_option_get_name(trigger::payload(tval[1]).opt_id));
				text::localised_format_box(ws, layout, box, "support_for_blank", m);
				text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_percentage{ -amount }, true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.issue_option_get_name(trigger::payload(tval[2]).opt_id));
				text::localised_format_box(ws, layout, box, "support_for_blank", m);
				text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_percentage{ amount }, true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_move_issue_percentage_pop(EFFECT_DISPLAY_PARAMS) {
			auto amount = trigger::read_float_from_payload(tval + 3);
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.issue_option_get_name(trigger::payload(tval[1]).opt_id));
				text::localised_format_box(ws, layout, box, "support_for_blank", m);
				text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_percentage{ -amount }, true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.issue_option_get_name(trigger::payload(tval[2]).opt_id));
				text::localised_format_box(ws, layout, box, "support_for_blank", m);
				text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_percentage{ amount }, true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_party_loyalty_province(EFFECT_DISPLAY_PARAMS) {
			float amount = float(trigger::payload(tval[2]).signed_value) / 100.0f;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.ideology_get_name(trigger::payload(tval[1]).ideo_id));
				text::localised_format_box(ws, layout, box, "blank_loyalty", m);
				text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_percentage{ amount }, true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_party_loyalty(EFFECT_DISPLAY_PARAMS) {
			float amount = float(trigger::payload(tval[3]).signed_value) / 100.0f;
			{
				auto box = text::open_layout_box(layout, indentation);
				text::add_to_layout_box(ws, layout, box, trigger::payload(tval[1]).prov_id);
				text::close_layout_box(layout, box);
			}
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.ideology_get_name(trigger::payload(tval[2]).ideo_id));
				text::localised_format_box(ws, layout, box, "blank_loyalty", m);
				text::add_space_to_layout_box(ws, layout, box);
			display_value(text::fp_percentage{ amount }, true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_build_railway_in_capital_yes_whole_state_yes_limit(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "railroad_in_capital_state", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_build_railway_in_capital_yes_whole_state_no_limit(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "railroad_in_capital_state", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_build_railway_in_capital_no_whole_state_yes_limit(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "railroad_in_capital", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_build_railway_in_capital_no_whole_state_no_limit(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "railroad_in_capital", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_build_fort_in_capital_yes_whole_state_yes_limit(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "fort_in_capital_state", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_build_fort_in_capital_yes_whole_state_no_limit(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "fort_in_capital_state", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_build_fort_in_capital_no_whole_state_yes_limit(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "fort_in_capital", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_build_fort_in_capital_no_whole_state_no_limit(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "fort_in_capital", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_relation_reb(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_rebel(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "relations_with", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(trigger::payload(tval[1]).signed_value), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_variable_tech_name_yes(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.technology_get_name(trigger::payload(tval[1]).tech_id));
				text::localised_format_box(ws, layout, box, "enable_blank", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_variable_tech_name_no(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.technology_get_name(trigger::payload(tval[1]).tech_id));
				text::localised_format_box(ws, layout, box, "disable_blank", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_variable_invention_name_yes(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.invention_get_name(trigger::payload(tval[1]).invt_id));
				text::localised_format_box(ws, layout, box, "enable_blank", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_variable_invention_name_no(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.world.invention_get_name(trigger::payload(tval[1]).invt_id));
				text::localised_format_box(ws, layout, box, "disable_blank", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_set_country_flag_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.national_definitions.flag_variable_names[trigger::payload(tval[1]).natf_id]);
				text::localised_format_box(ws, layout, box, "o_set_national_flag", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_set_country_flag_pop(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text,
				ws.national_definitions.flag_variable_names[trigger::payload(tval[1]).natf_id]);
				text::localised_format_box(ws, layout, box, "o_set_national_flag", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_set_province_flag(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.province_definitions.flag_variable_names[trigger::payload(tval[1]).provf_id]);
				text::localised_format_box(ws, layout, box, "o_set_provincial_flag", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_clr_province_flag(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, ws.province_definitions.flag_variable_names[trigger::payload(tval[1]).provf_id]);
				text::localised_format_box(ws, layout, box, "o_clr_provincial_flag", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_remove_crisis(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "remove_crisis", m);
			text::close_layout_box(layout, box);
			return 0;
		}

		uint32_t ef_trigger_crisis(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::localised_format_box(ws, layout, box, "trigger_crisis", m);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_set_crisis_type(EFFECT_DISPLAY_PARAMS) {
			auto type = sys::crisis_type(tval[1]);
			return 0;
		}
		uint32_t ef_set_crisis_colony(EFFECT_DISPLAY_PARAMS) {
			auto sdef = trigger::payload(tval[1]).state_id;
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, sdef);
			text::localised_format_box(ws, layout, box, "set_crisis_colony", m);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_set_crisis_colony_this(EFFECT_DISPLAY_PARAMS) {
			auto sdef = ws.world.state_instance_get_definition(trigger::to_state(this_slot));
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			if(this_slot != -1)
				text::add_to_substitution_map(m, text::variable_type::x, sdef);
			else {
				auto t = text::produce_simple_string(ws, "this_state");
				text::add_to_substitution_map(m, text::variable_type::x, std::string_view(t));
			}
			text::localised_format_box(ws, layout, box, "set_crisis_colony", m);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_set_crisis_colony_from(EFFECT_DISPLAY_PARAMS) {
			auto sdef = ws.world.state_instance_get_definition(trigger::to_state(from_slot));
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			if(this_slot != -1)
				text::add_to_substitution_map(m, text::variable_type::x, sdef);
			else {
				auto t = text::produce_simple_string(ws, "from_state");
				text::add_to_substitution_map(m, text::variable_type::x, std::string_view(t));
			}
			text::localised_format_box(ws, layout, box, "set_crisis_colony", m);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_set_crisis_liberation_tag(EFFECT_DISPLAY_PARAMS) {
			auto tag = trigger::payload(tval[1]).tag_id;
				auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::x, tag);
			text::localised_format_box(ws, layout, box, "set_crisis_liberation_tag", m);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_set_crisis_liberation_tag_this(EFFECT_DISPLAY_PARAMS) {
			auto tag = ws.world.nation_get_identity_from_identity_holder(trigger::to_nation(this_slot));
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			if(this_slot != -1)
				text::add_to_substitution_map(m, text::variable_type::x, tag);
			else {
				auto t = text::produce_simple_string(ws, "this_nation");
				text::add_to_substitution_map(m, text::variable_type::x, std::string_view(t));
			}
			text::localised_format_box(ws, layout, box, "set_crisis_liberation_tag", m);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_set_crisis_liberation_tag_from(EFFECT_DISPLAY_PARAMS) {
			auto tag = ws.world.nation_get_identity_from_identity_holder(trigger::to_nation(from_slot));
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			if(this_slot != -1)
				text::add_to_substitution_map(m, text::variable_type::x, tag);
			else {
				auto t = text::produce_simple_string(ws, "from_nation");
				text::add_to_substitution_map(m, text::variable_type::x, std::string_view(t));
			}
			text::localised_format_box(ws, layout, box, "set_crisis_liberation_tag", m);
			text::close_layout_box(layout, box);
			return 0;
		}

		uint32_t ef_add_country_modifier_province(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, ws.world.modifier_get_name(trigger::payload(tval[1]).mod_id));
			text::add_to_substitution_map(m, text::variable_type::date, ws.current_date + trigger::payload(tval[2]).signed_value);
			text::localised_format_box(ws, layout, box, "o_add_modifier_until", m);
			text::close_layout_box(layout, box);
			modifier_description(ws, layout, trigger::payload(tval[1]).mod_id, indentation + indentation_amount);
			return 0;
		}
		uint32_t ef_add_country_modifier_province_no_duration(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, ws.world.modifier_get_name(trigger::payload(tval[1]).mod_id));
			text::localised_format_box(ws, layout, box, "o_add_modifier", m);
			text::close_layout_box(layout, box);
			modifier_description(ws, layout, trigger::payload(tval[1]).mod_id, indentation + indentation_amount);
			return 0;
		}
		uint32_t ef_relation_province(EFFECT_DISPLAY_PARAMS) {
			auto target = ws.world.national_identity_get_nation_from_identity_holder(trigger::payload(tval[1]).tag_id);
			if(target) {
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::text, target);
				text::localised_format_box(ws, layout, box, "o_relations_with", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(trigger::payload(tval[2]).signed_value), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_relation_province_this_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "o_relations_with", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(trigger::payload(tval[1]).signed_value), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_relation_province_this_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(this_slot)), "this_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "o_relations_with", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(trigger::payload(tval[1]).signed_value), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_relation_province_from_nation(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_nation(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "o_relations_with", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(trigger::payload(tval[1]).signed_value), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_relation_province_from_province(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_prov(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "o_relations_with", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(trigger::payload(tval[1]).signed_value), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_relation_province_reb(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
			add_to_map(ws, m, convert_this(ws, trigger::to_rebel(from_slot)), "from_nation", [](auto x) { return x; });
				text::localised_format_box(ws, layout, box, "o_relations_with", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(trigger::payload(tval[1]).signed_value), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_treasury_province(EFFECT_DISPLAY_PARAMS) {
			auto amount = trigger::read_float_from_payload(tval + 1);
			{
				auto box = text::open_layout_box(layout, indentation);
			display_value(text::fp_currency{ amount }, true, ws, layout, box);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "o_add_to_treasury", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_kill_leader(EFFECT_DISPLAY_PARAMS) {
		dcon::unit_name_id ename{ dcon::unit_name_id::value_base_t(trigger::read_int32_t_from_payload(tval + 1)) };
			auto esv = ws.to_string_view(ename);
			auto box = text::open_layout_box(layout, indentation);
			text::localised_format_box(ws, layout, box, "e_kill_leader");
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, esv);
			text::close_layout_box(layout, box);
			return 0;
		}

		//
		// Banks
		//
		uint32_t ef_build_bank_in_capital_yes_whole_state_yes_limit(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "bank_in_capital_state", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_build_bank_in_capital_yes_whole_state_no_limit(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "bank_in_capital_state", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_build_bank_in_capital_no_whole_state_yes_limit(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "bank_in_capital", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_build_bank_in_capital_no_whole_state_no_limit(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "bank_in_capital", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}

		//
		// Universities
		//
		uint32_t ef_build_university_in_capital_yes_whole_state_yes_limit(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "university_in_capital_state", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_build_university_in_capital_yes_whole_state_no_limit(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "university_in_capital_state", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_build_university_in_capital_no_whole_state_yes_limit(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "university_in_capital", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_build_university_in_capital_no_whole_state_no_limit(EFFECT_DISPLAY_PARAMS) {
			{
				auto box = text::open_layout_box(layout, indentation + indentation_amount);
				text::substitution_map m;
				text::localised_format_box(ws, layout, box, "university_in_capital", m);
				text::close_layout_box(layout, box);
			}
			return 0;
		}

		uint32_t ef_building(EFFECT_DISPLAY_PARAMS) {
			auto const pbt = trigger::payload(tval[1]).pbt_id;
			auto const amount = int32_t(trigger::payload(tval[2]).signed_value);
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::type, ws.world.province_building_type_get_name(pbt));
				text::localised_format_box(ws, layout, box, "ef_building", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(amount), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}
		uint32_t ef_building_state(EFFECT_DISPLAY_PARAMS) {
			auto const pbt = trigger::payload(tval[1]).pbt_id;
			auto const amount = int32_t(trigger::payload(tval[2]).signed_value);
			{
				auto box = text::open_layout_box(layout, indentation);
				text::substitution_map m;
				text::add_to_substitution_map(m, text::variable_type::type, ws.world.province_building_type_get_name(pbt));
				text::localised_format_box(ws, layout, box, "ef_building_state", m);
				text::add_space_to_layout_box(ws, layout, box);
				display_value(int64_t(amount), true, ws, layout, box);
				text::close_layout_box(layout, box);
			}
			return 0;
		}

		uint32_t ef_annex_to_null_nation(EFFECT_DISPLAY_PARAMS) {
			text::add_line(ws, layout, text::produce_simple_string(ws, "ef_annex_null"));
			return 0;
		}
		uint32_t ef_annex_to_null_province(EFFECT_DISPLAY_PARAMS) {
			text::add_line(ws, layout, text::produce_simple_string(ws, "ef_annex_null"));
			return 0;
		}

		uint32_t ef_fop_clr_global_flag_2(EFFECT_DISPLAY_PARAMS) {
			if(ws.user_settings.spoilers) {
				for(uint32_t i = 0; i < 2; i++) {
					auto box = text::open_layout_box(layout, indentation);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.national_definitions.global_flag_variable_names[trigger::payload(tval[1 + i]).glob_id]);
					text::localised_format_box(ws, layout, box, "remove_global_flag", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_fop_clr_global_flag_3(EFFECT_DISPLAY_PARAMS) {
			if(ws.user_settings.spoilers) {
				for(uint32_t i = 0; i < 3; i++) {
					auto box = text::open_layout_box(layout, indentation);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.national_definitions.global_flag_variable_names[trigger::payload(tval[1 + i]).glob_id]);
					text::localised_format_box(ws, layout, box, "remove_global_flag", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_fop_clr_global_flag_4(EFFECT_DISPLAY_PARAMS) {
			if(ws.user_settings.spoilers) {
				for(uint32_t i = 0; i < 4; i++) {
					auto box = text::open_layout_box(layout, indentation);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.national_definitions.global_flag_variable_names[trigger::payload(tval[1 + i]).glob_id]);
					text::localised_format_box(ws, layout, box, "remove_global_flag", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_fop_clr_global_flag_5(EFFECT_DISPLAY_PARAMS) {
			if(ws.user_settings.spoilers) {
				for(uint32_t i = 0; i < 5; i++) {
					auto box = text::open_layout_box(layout, indentation);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.national_definitions.global_flag_variable_names[trigger::payload(tval[1 + i]).glob_id]);
					text::localised_format_box(ws, layout, box, "remove_global_flag", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_fop_clr_global_flag_6(EFFECT_DISPLAY_PARAMS) {
			if(ws.user_settings.spoilers) {
				for(uint32_t i = 0; i < 6; i++) {
					auto box = text::open_layout_box(layout, indentation);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.national_definitions.global_flag_variable_names[trigger::payload(tval[1 + i]).glob_id]);
					text::localised_format_box(ws, layout, box, "remove_global_flag", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_fop_clr_global_flag_7(EFFECT_DISPLAY_PARAMS) {
			if(ws.user_settings.spoilers) {
				for(uint32_t i = 0; i < 7; i++) {
					auto box = text::open_layout_box(layout, indentation);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.national_definitions.global_flag_variable_names[trigger::payload(tval[1 + i]).glob_id]);
					text::localised_format_box(ws, layout, box, "remove_global_flag", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_fop_clr_global_flag_8(EFFECT_DISPLAY_PARAMS) {
			if(ws.user_settings.spoilers) {
				for(uint32_t i = 0; i < 8; i++) {
					auto box = text::open_layout_box(layout, indentation);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.national_definitions.global_flag_variable_names[trigger::payload(tval[1 + i]).glob_id]);
					text::localised_format_box(ws, layout, box, "remove_global_flag", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_fop_clr_global_flag_9(EFFECT_DISPLAY_PARAMS) {
			if(ws.user_settings.spoilers) {
				for(uint32_t i = 0; i < 9; i++) {
					auto box = text::open_layout_box(layout, indentation);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.national_definitions.global_flag_variable_names[trigger::payload(tval[1 + i]).glob_id]);
					text::localised_format_box(ws, layout, box, "remove_global_flag", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_fop_clr_global_flag_10(EFFECT_DISPLAY_PARAMS) {
			if(ws.user_settings.spoilers) {
				for(uint32_t i = 0; i < 10; i++) {
					auto box = text::open_layout_box(layout, indentation);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.national_definitions.global_flag_variable_names[trigger::payload(tval[1 + i]).glob_id]);
					text::localised_format_box(ws, layout, box, "remove_global_flag", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_fop_clr_global_flag_11(EFFECT_DISPLAY_PARAMS) {
			if(ws.user_settings.spoilers) {
				for(uint32_t i = 0; i < 11; i++) {
					auto box = text::open_layout_box(layout, indentation);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.national_definitions.global_flag_variable_names[trigger::payload(tval[1 + i]).glob_id]);
					text::localised_format_box(ws, layout, box, "remove_global_flag", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_fop_clr_global_flag_12(EFFECT_DISPLAY_PARAMS) {
			if(ws.user_settings.spoilers) {
				for(uint32_t i = 0; i < 11; i++) {
					auto box = text::open_layout_box(layout, indentation);
					text::substitution_map m;
					text::add_to_substitution_map(m, text::variable_type::text, ws.national_definitions.global_flag_variable_names[trigger::payload(tval[1 + i]).glob_id]);
					text::localised_format_box(ws, layout, box, "remove_global_flag", m);
					text::close_layout_box(layout, box);
				}
			}
			return 0;
		}
		uint32_t ef_fop_change_province_name(EFFECT_DISPLAY_PARAMS) {
			auto p = trigger::payload(tval[3]).prov_id;
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, ws.world.province_get_name(p)));
			text::add_space_to_layout_box(ws, layout, box);
			text::substitution_map m;
		dcon::text_key name{ dcon::text_key::value_base_t(trigger::read_int32_t_from_payload(tval + 1)) };
			text::add_to_substitution_map(m, text::variable_type::text, name);
			text::localised_format_box(ws, layout, box, "change_name_to", m);
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_change_terrain_pop(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, ws.world.modifier_get_name(trigger::payload(tval[1]).mod_id));
			text::localised_format_box(ws, layout, box, "ett_change_pop_terrain_to", m);
			text::close_layout_box(layout, box);
			if(ws.user_settings.spoilers) {
				modifier_description(ws, layout, trigger::payload(tval[1]).mod_id, indentation + indentation_amount);
			}
			return 0;
		}
		uint32_t ef_change_terrain_province(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, ws.world.modifier_get_name(trigger::payload(tval[1]).mod_id));
			text::localised_format_box(ws, layout, box, "ett_change_province_terrain_to", m);
			text::close_layout_box(layout, box);
			if(ws.user_settings.spoilers) {
				modifier_description(ws, layout, trigger::payload(tval[1]).mod_id, indentation + indentation_amount);
			}
			return 0;
		}
		uint32_t ef_masquerade_as_nation_this(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "masquerade_as_nation"));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::get_name(ws, trigger::to_nation(this_slot)));
			text::close_layout_box(layout, box);
			return 0;
		}
		uint32_t ef_masquerade_as_nation_from(EFFECT_DISPLAY_PARAMS) {
			auto box = text::open_layout_box(layout, indentation);
			text::add_to_layout_box(ws, layout, box, text::produce_simple_string(ws, "masquerade_as_nation"));
			text::add_space_to_layout_box(ws, layout, box);
			text::add_to_layout_box(ws, layout, box, text::get_name(ws, trigger::to_nation(from_slot)));
			text::close_layout_box(layout, box);
			return 0;
		}

		uint32_t ef_change_party_name(EFFECT_DISPLAY_PARAMS) {
			auto ideology = trigger::payload(tval[1]).ideo_id;
			dcon::text_key new_name{ dcon::text_key::value_base_t(trigger::read_int32_t_from_payload(tval + 2)) };
			//
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, new_name);
			if(ideology) {
				auto id_name = ws.world.ideology_get_name(ideology);
				text::add_to_substitution_map(m, text::variable_type::x, id_name);
				text::localised_format_box(ws, layout, box, "change_pname_to", m);
			} else {
				text::localised_format_box(ws, layout, box, "change_rpname_to", m);
			}
			text::close_layout_box(layout, box);
			return 0;
		}

		uint32_t ef_change_party_position(EFFECT_DISPLAY_PARAMS) {
			auto ideology = trigger::payload(tval[1]).ideo_id;
			dcon::issue_option_id new_opt = trigger::payload(tval[2]).opt_id;
			auto opt_name = ws.world.issue_option_get_name(new_opt);
			//
			auto box = text::open_layout_box(layout, indentation);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::text, opt_name);
			if(ideology) {
				auto id_name = ws.world.ideology_get_name(ideology);
				text::add_to_substitution_map(m, text::variable_type::x, id_name);
				text::localised_format_box(ws, layout, box, "change_pposition_to", m);
			} else {
				text::localised_format_box(ws, layout, box, "change_rpposition_to", m);
			}
			text::close_layout_box(layout, box);
			return 0;
		}

#define EFFECT_UNUSED(x) uint32_t ef_unused_##x(EFFECT_DISPLAY_PARAMS) { return 0; }
	EFFECT_UNUSED(1)
	EFFECT_UNUSED(2)
	EFFECT_UNUSED(3)
	EFFECT_UNUSED(4)
	EFFECT_UNUSED(5)
	EFFECT_UNUSED(6)
	EFFECT_UNUSED(7)
	EFFECT_UNUSED(8)
	EFFECT_UNUSED(9)
#undef EFFECT_UNUSED

		inline constexpr uint32_t(*effect_functions[effect::first_invalid_code])(EFFECT_DISPLAY_PARAMS) = {
			ef_none,
#define EFFECT_BYTECODE_ELEMENT(code, name, arg) ef_##name,
			EFFECT_BYTECODE_LIST
#undef EFFECT_BYTECODE_ELEMENT
			// scopes
#define EFFECT_SCOPE_BYTECODE_ELEMENT(name, code) es_##name,
			EFFECT_SCOPE_BYTECODE_LIST
#undef EFFECT_SCOPE_BYTECODE_ELEMENT
		};

		uint32_t internal_make_effect_description(EFFECT_DISPLAY_PARAMS) {
			return effect_functions[*tval & effect::code_mask](ws, tval, layout, primary_slot, this_slot, from_slot, r_lo, r_hi,
			indentation);
		}

		#undef EFFECT_DISPLAY_PARAMS

	} // namespace effect_tooltip

	void effect_description(sys::state& state, text::layout_base& layout, dcon::effect_key k, int32_t primary_slot, int32_t this_slot,
		int32_t from_slot, uint32_t r_lo, uint32_t r_hi) {
		if(!k)
		text::add_line(state, layout, "no_effect");
		else
		effect_tooltip::internal_make_effect_description(state, state.effect_data.data() + state.effect_data_indices[k.index() + 1], layout, primary_slot, this_slot, from_slot, r_lo, r_hi, 0);
	}

} // namespace ui
