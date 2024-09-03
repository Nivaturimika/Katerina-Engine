#pragma once

#include "gui_element_types.hpp"
#include "prng.hpp"
#include "gui_leader_tooltip.hpp"

namespace ui {

class leader_portrait : public button_element_base {
	void on_update(sys::state& state) noexcept override {
		auto pculture = state.world.nation_get_primary_culture(state.local_player_nation);
		auto ltype = pculture.get_group_from_culture_group_membership().get_leader();
		if(ltype) {
			auto lid = retrieve<dcon::leader_id>(state, parent);
			auto admiral = state.world.leader_get_is_admiral(lid);
			if(admiral) {
				auto arange = ltype.get_admirals();
				if(arange.size() > 0) {
					auto rval = rng::get_random(state, uint32_t(state.world.leader_get_since(lid).value), uint32_t(lid.value));
					auto in_range = rng::reduce(uint32_t(rval), arange.size());
					base_data.data.image.gfx_object = arange[in_range];
				}
			} else {
				auto grange = ltype.get_generals();
				if(grange.size() > 0) {
					auto rval = rng::get_random(state, uint32_t(state.world.leader_get_since(lid).value), uint32_t(lid.value));
					auto in_range = rng::reduce(uint32_t(rval), grange.size());
					base_data.data.image.gfx_object = grange[in_range];
				}
			}
		}	
	}

	void button_action(sys::state& state) noexcept override {
		auto location = get_absolute_non_mirror_location(state, *this);
		auto lid = retrieve<dcon::leader_id>(state, parent);
		auto admiral = state.world.leader_get_is_admiral(lid);
		if(admiral) {
			auto a = state.world.leader_get_navy_from_navy_leadership(lid);
			if(command::can_change_admiral(state, state.local_player_nation, a, dcon::leader_id{}))
				open_leader_selection(state, dcon::army_id{}, a, location.x, location.y);
		} else {
			auto a = state.world.leader_get_army_from_army_leadership(lid);
			if(command::can_change_general(state, state.local_player_nation, a, dcon::leader_id{}))
				open_leader_selection(state, a, dcon::navy_id{}, location.x, location.y);
		}
	}

	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return tooltip_behavior::variable_tooltip;
	}

	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		display_leader_attributes(state, retrieve<dcon::leader_id>(state, parent), contents, 0);
	}
};

class leader_personality_text : public simple_text_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		auto l = retrieve<dcon::leader_id>(state, parent);
		auto name = state.world.leader_get_personality(l).get_name();
		set_text(state, text::produce_simple_string(state, name));
	}
};
class leader_background_text : public simple_text_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		auto l = retrieve<dcon::leader_id>(state, parent);
		auto name = state.world.leader_get_background(l).get_name();
		set_text(state, text::produce_simple_string(state, name));
	}
};
class leader_name_text : public simple_text_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		auto l = retrieve<dcon::leader_id>(state, parent);
		auto name = state.world.leader_get_name(l);
		set_text(state, std::string(state.to_string_view(name)));
	}
};
class leader_location_text : public simple_text_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		auto l = retrieve<dcon::leader_id>(state, parent);
		auto a = state.world.leader_get_army_from_army_leadership(l);
		dcon::province_id location{};
		if(a) {
			location = state.world.army_get_location_from_army_location(a);
		} else {
			auto n = state.world.leader_get_navy_from_navy_leadership(l);
			if(n) {
				location = state.world.navy_get_location_from_navy_location(n);
			}
		}
		set_text(state, text::produce_simple_string(state, state.world.province_get_name(location)));
	}
};
class leader_unit_name_text : public simple_text_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		auto l = retrieve<dcon::leader_id>(state, parent);
		auto a = state.world.leader_get_army_from_army_leadership(l);
		dcon::unit_name_id unit_name{};
		if(a) {
			unit_name = state.world.army_get_name(a);
		} else {
			auto n = state.world.leader_get_navy_from_navy_leadership(l);
			if(n) {
				unit_name = state.world.navy_get_name(n);
			}
		}
		set_text(state, text::produce_simple_string(state, std::string(state.to_string_view(unit_name))));
	}
};
class leader_prestige_bar : public vertical_progress_bar {
public:
	void on_update(sys::state& state) noexcept override {
		auto l = retrieve<dcon::leader_id>(state, parent);
		progress = state.world.leader_get_prestige(l);
	}
};

class military_leaders : public listbox_row_element_base<dcon::leader_id> {
public:
	ui::simple_text_element_base* leader_name = nullptr;
	ui::simple_text_element_base* army = nullptr;
	ui::simple_text_element_base* location = nullptr;

	void on_create(sys::state& state) noexcept override {
		listbox_row_element_base::on_create(state);
		base_data.position = xy_pair{ 0, 0 };
	}

	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "name") {
			return make_element_by_type<leader_name_text>(state, id);
		} else if (name == "leader") {
			return make_element_by_type<leader_portrait>(state, id);
		} else if(name == "background") {
			return make_element_by_type<leader_background_text>(state, id);
		} else if(name == "personality") {
			return make_element_by_type<leader_personality_text>(state, id);
		} else if(name == "use_leader") {
			return make_element_by_type<invisible_element>(state, id);
		} else if(name == "army") {
			return make_element_by_type<leader_unit_name_text>(state, id);
		} else if(name == "location") {
			return make_element_by_type<leader_location_text>(state, id);
		} else if(name == "leader_prestige_bar") {
			return make_element_by_type<leader_prestige_bar>(state, id);
		} else {
			return nullptr;
		}
	}
};

enum class leader_sort : uint8_t {
	name, prestige, type, army
};

class military_leaders_listbox : public listbox_element_base<military_leaders, dcon::leader_id> {
protected:
	std::string_view get_row_element_name() override {
		return "milview_leader_entry";
	}

	bool is_asc = false;
	leader_sort sort = leader_sort::army;
public:
	void on_update(sys::state& state) noexcept override {
		row_contents.clear();
		for(auto const fat_id : state.world.nation_get_leader_loyalty(state.local_player_nation)) {
			row_contents.push_back(fat_id.get_leader());
		}
		switch(sort) {
		case leader_sort::name:
			std::sort(row_contents.begin(), row_contents.end(), [&](dcon::leader_id a, dcon::leader_id b) {
				auto in_a = state.to_string_view(state.world.leader_get_name(a));
				auto in_b = state.to_string_view(state.world.leader_get_name(b));
				if(in_a != in_b)
					return in_a < in_b;
				return a.index() < b.index();
			});
			break;
		case leader_sort::prestige:
			std::sort(row_contents.begin(), row_contents.end(), [&](dcon::leader_id a, dcon::leader_id b) {
				auto in_a = state.world.leader_get_prestige(a);
				auto in_b = state.world.leader_get_prestige(b);
				if(in_a != in_b)
					return in_a < in_b;
				return a.index() < b.index();
			});
			break;
		case leader_sort::type:
			std::sort(row_contents.begin(), row_contents.end(), [&](dcon::leader_id a, dcon::leader_id b) {
				auto in_a = state.world.leader_get_is_admiral(a);
				auto in_b = state.world.leader_get_is_admiral(b);
				if(in_a != in_b)
					return in_a < in_b;
				auto in_a1 = state.world.leader_trait_get_attack(state.world.leader_get_background(a))
					+ state.world.leader_trait_get_attack(state.world.leader_get_personality(a));
				auto in_b1 = state.world.leader_trait_get_attack(state.world.leader_get_background(b))
					+ state.world.leader_trait_get_attack(state.world.leader_get_personality(b));
				if(in_a1 != in_b1)
					return in_a1 < in_b1;
				return a.index() < b.index();
			});
			break;
		case leader_sort::army:
			std::sort(row_contents.begin(), row_contents.end(), [&](dcon::leader_id a, dcon::leader_id b) {
				auto ar_a = state.world.leader_get_army_from_army_leadership(a);
				auto ar_b = state.world.leader_get_army_from_army_leadership(b);
				auto in_a = state.to_string_view(state.world.army_get_name(ar_a));
				auto in_b = state.to_string_view(state.world.army_get_name(ar_b));
				if(in_a != in_b)
					return in_a < in_b;
				return a.index() < b.index();
			});
			break;
		}
		if(is_asc) {
			std::reverse(row_contents.begin(), row_contents.end());
		}
		update(state);
	}

	message_result get(sys::state& state, Cyto::Any& payload) noexcept override {
		if(payload.holds_type<leader_sort>()) {
			auto s = Cyto::any_cast<leader_sort>(payload);
			if(sort == s) {
				is_asc = !is_asc;
			}
			sort = s;
			on_update(state);
			return message_result::consumed;
		}
		return message_result::unseen;
	}
};

template<bool B>
class military_make_leader_button : public right_click_button_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		disabled = !command::can_make_leader(state, state.local_player_nation, B);
		auto lp = state.world.nation_get_leadership_points(state.local_player_nation);
		int32_t count = int32_t(lp / state.defines.leader_recruit_cost);
		if(B) {
			set_button_text(state, text::produce_simple_string(state, "alice_mw_create_1") + " (" + std::to_string(count) + ")");
		} else {
			set_button_text(state, text::produce_simple_string(state, "alice_mw_create_2") + " (" + std::to_string(count) + ")");
		}
	}
	void button_action(sys::state& state) noexcept override {
		command::make_leader(state, state.local_player_nation, B);
	}
	void button_right_action(sys::state& state) noexcept override {
		auto lp = state.world.nation_get_leadership_points(state.local_player_nation);
		int32_t count = int32_t(lp / state.defines.leader_recruit_cost);
		for(; count > 0; count--) {
			command::make_leader(state, state.local_player_nation, B);
		}
	}

	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return tooltip_behavior::tooltip;
	}
	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		text::add_line(state, contents, "alice_mw_create_lp", text::variable_type::x, text::fp_two_places{ state.defines.leader_recruit_cost });
		text::add_line(state, contents, "alice_mw_create_lpb");
		if(B) {
			text::add_line(state, contents, "alice_mw_controls_1");
		} else {
			text::add_line(state, contents, "alice_mw_controls_2");
		}
	}
};

class leaders_sortby_prestige : public button_element_base {
public:
	void button_action(sys::state& state) noexcept override {
		send(state, parent, leader_sort::prestige);
	}

	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return tooltip_behavior::tooltip;
	}

	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		auto box = text::open_layout_box(contents, 0);
		text::localised_format_box(state, contents, box, std::string_view("sort_by_prestige"));
		text::close_layout_box(contents, box);
	}
};

class leaders_sortby_type : public button_element_base {
public:
	void button_action(sys::state& state) noexcept override {
		send(state, parent, leader_sort::type);
	}

	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return tooltip_behavior::tooltip;
	}

	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		auto box = text::open_layout_box(contents, 0);
		text::localised_format_box(state, contents, box, std::string_view("military_sort_by_type_tooltip"));
		text::close_layout_box(contents, box);
	}
};

class leaders_sortby_name : public button_element_base {
public:
	void button_action(sys::state& state) noexcept override {
		send(state, parent, leader_sort::name);
	}

	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return tooltip_behavior::tooltip;
	}

	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		auto box = text::open_layout_box(contents, 0);
		text::localised_format_box(state, contents, box, std::string_view("military_sort_by_name_tooltip"));
		text::close_layout_box(contents, box);
	}
};

class leaders_sortby_army : public button_element_base {
public:
	void button_action(sys::state& state) noexcept override {
		send(state, parent, leader_sort::army);
	}

	tooltip_behavior has_tooltip(sys::state& state) noexcept override {
		return tooltip_behavior::tooltip;
	}

	void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
		auto box = text::open_layout_box(contents, 0);
		text::localised_format_box(state, contents, box, std::string_view("military_sort_by_assignment_tooltip"));
		text::close_layout_box(contents, box);
	}
};

class military_general_count : public simple_text_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		auto general_count = 0;
		for(auto const fat_id : state.world.nation_get_leader_loyalty(state.local_player_nation)) {
			auto leader_id = fat_id.get_leader();
			if(!state.world.leader_get_is_admiral(leader_id)) {
				++general_count;
			}
		}
		set_text(state, std::to_string(general_count));
	}
};

class military_admiral_count : public simple_text_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		auto admiral_count = 0;
		for(auto const fat_id : state.world.nation_get_leader_loyalty(state.local_player_nation)) {
			auto leader_id = fat_id.get_leader();
			if(state.world.leader_get_is_admiral(leader_id)) {
				++admiral_count;
			}
		}
		set_text(state, std::to_string(admiral_count));
	}
};

class military_auto_assign_text : public simple_text_element_base {
public:
	void on_update(sys::state& state) noexcept override {
		if(!state.world.nation_get_auto_create_admirals(state.local_player_nation)) {
			set_text(state, text::produce_simple_string(state, "military_autocreate_2"));
		} else {
			set_text(state, text::produce_simple_string(state, "military_autocreate"));
		}
	}
};
class military_auto_create_button : public checkbox_button {
public:
	bool is_active(sys::state& state) noexcept override {
		return !state.world.nation_get_auto_create_admirals(state.local_player_nation);
	}
	void button_action(sys::state& state) noexcept override {
		command::toggle_auto_create_generals(state, state.local_player_nation);
	}
};
class military_auto_assign_button : public checkbox_button {
public:
	bool is_active(sys::state& state) noexcept override {
		return state.world.nation_get_auto_assign_leaders(state.local_player_nation);
	}
	void button_action(sys::state& state) noexcept override {
		command::toggle_auto_assign_leaders(state, state.local_player_nation);
	}
};

class leaders_window : public window_element_base {
	military_leaders_listbox* listbox = nullptr;
public:
	std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
		if(name == "sort_leader_prestige") {
			return make_element_by_type<leaders_sortby_prestige>(state, id);
		} else if(name == "sort_prestige_icon") {
			return make_element_by_type<image_element_base>(state, id);
		} else if(name == "sort_leader_type") {
			return make_element_by_type<leaders_sortby_type>(state, id);
		} else if(name == "sort_leader_name") {
			return make_element_by_type<leaders_sortby_name>(state, id);
		} else if(name == "sort_leader_army") {
			return make_element_by_type<leaders_sortby_army>(state, id);
		} else if(name == "leader_listbox") {
			auto ptr = make_element_by_type<military_leaders_listbox>(state, id);
			listbox = ptr.get();
			return ptr;
		} else if(name == "new_general") {
			return make_element_by_type<military_make_leader_button<true>>(state, id);
		} else if(name == "new_admiral") {
			return make_element_by_type<military_make_leader_button<false>>(state, id);
		} else if(name == "generals") {
			return make_element_by_type<military_general_count>(state, id);
		} else if(name == "admirals") {
			return make_element_by_type<military_admiral_count>(state, id);
		} else if(name == "auto_create") {
			return make_element_by_type<military_auto_create_button>(state, id);
		} else if(name == "auto_create_text") {
			return make_element_by_type<military_auto_assign_text>(state, id);
		} else if(name == "auto_assign") {
			return make_element_by_type<military_auto_assign_button>(state, id);
		} else if(name == "auto_assign_text") {
			return make_element_by_type<simple_text_element_base>(state, id);
		} else {
			return nullptr;
		}
	}
	message_result get(sys::state& state, Cyto::Any& payload) noexcept override {
		if(payload.holds_type<leader_sort>()) {
			if(listbox)
				listbox->impl_get(state, payload);
			return message_result::consumed;
		}
		return message_result::unseen;
	}
};

} // namespace ui
