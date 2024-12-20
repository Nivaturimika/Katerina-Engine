#pragma once

#include "dcon_generated.hpp"
#include "economy.hpp"
#include "gui_common_elements.hpp"
#include "gui_element_types.hpp"

namespace ui {
	class commodity_price_text : public simple_text_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			auto commodity_id = retrieve<dcon::commodity_id>(state, parent);
			set_text(state, text::format_money(state.world.commodity_get_current_price(commodity_id)));
		}
	};

	class commodity_player_availability_text : public simple_text_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			auto commodity_id = retrieve<dcon::commodity_id>(state, parent);
			if(commodity_id)
			set_text(state, text::format_float(state.world.nation_get_demand_satisfaction(state.local_player_nation, commodity_id), 2));
		}
	};

	class commodity_player_real_demand_text : public simple_text_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			auto commodity_id = retrieve<dcon::commodity_id>(state, parent);
			if(commodity_id)
			set_text(state, text::format_float(state.world.nation_get_real_demand(state.local_player_nation, commodity_id), 1));
		}
	};

	class commodity_national_player_stockpile_text : public simple_text_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			auto commodity_id = retrieve<dcon::commodity_id>(state, parent);
			if(commodity_id)
			set_text(state, text::format_float(state.world.nation_get_stockpiles(state.local_player_nation, commodity_id), 2));
		}
	};

	class commodity_player_stockpile_increase_text : public simple_text_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			auto commodity_id = retrieve<dcon::commodity_id>(state, parent);
			float amount = economy::stockpile_commodity_daily_increase(state, commodity_id, state.local_player_nation);
			auto txt = std::string(amount >= 0.f ? "+" : "") + text::format_float(amount, 2);
			set_text(state, txt);
		}
	};

	class commodity_market_increase_text : public simple_text_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			auto commodity_id = retrieve<dcon::commodity_id>(state, parent);
			float amount = economy::global_market_commodity_daily_increase(state, commodity_id);
			auto txt = std::string("(") + text::format_float(amount, 0) + ")";
			set_text(state, txt);
		}
	};

	class commodity_global_market_text : public simple_text_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			auto commodity_id = retrieve<dcon::commodity_id>(state, parent);
			set_text(state, text::format_float(state.world.commodity_get_global_market_pool(commodity_id), 2));
		}
	};

	class commodity_player_domestic_market_text : public simple_text_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			auto commodity_id = retrieve<dcon::commodity_id>(state, parent);
			if(commodity_id)
			set_text(state, text::format_float(commodity_id ? state.world.nation_get_domestic_market_pool(state.local_player_nation, commodity_id) : 0.0f, 2));
		}
	};

	class commodity_player_factory_needs_text : public simple_text_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			auto commodity_id = retrieve<dcon::commodity_id>(state, parent);
			if(commodity_id)
			set_text(state, text::format_float(commodity_id ? economy_factory::nation_factory_consumption(state, state.local_player_nation, commodity_id) : 0.0f, 2));
		}
	};

	class commodity_player_pop_needs_text : public simple_text_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			auto commodity_id = retrieve<dcon::commodity_id>(state, parent);
			if(commodity_id)
			set_text(state, text::format_float(commodity_id ? economy::nation_pop_consumption(state, state.local_player_nation, commodity_id) : 0.0f, 2));
		}
	};

	class commodity_player_government_needs_text : public simple_text_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			auto commodity_id = retrieve<dcon::commodity_id>(state, parent);
			if(commodity_id)
			set_text(state, text::format_float(commodity_id ? economy::government_consumption(state, state.local_player_nation, commodity_id) : 0.0f, 2));
		}
	};

	enum class trade_sort : uint8_t {
		commodity,
		stockpile,
		stockpile_change,
		price,
		demand_satisfaction,
		needs,
		market,
		real_demand,
		domestic_market,
		global_market_pool,
	};
	enum class trade_sort_assoc : uint8_t {
		market_activity,
		stockpile,
		common_market,
		needs_government,
		needs_factories,
		needs_pops
	};
	struct trade_sort_data {
		trade_sort sort;
		trade_sort_assoc assoc;
	};

	template<trade_sort Sort, trade_sort_assoc Assoc>
	class trade_sort_button : public button_element_base {
	public:
		void button_action(sys::state& state) noexcept override {
			send(state, parent, trade_sort_data{ Sort, Assoc });
		}
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			/*auto box = text::open_layout_box(contents);
			text::localised_format_box(state, contents, box, "sort_by");
			text::add_space_to_layout_box(state, contents, box);
			text::localised_format_box(state, contents, box, "trait_speed");
			text::close_layout_box(contents, box);*/
		}
	};

	class trade_market_activity_value : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			auto n = retrieve<dcon::nation_id>(state, parent);
			auto c = retrieve<dcon::commodity_id>(state, parent);
			float produced = state.world.nation_get_domestic_market_pool(n, c);
			float consumed = state.world.nation_get_real_demand(n, c) * state.world.nation_get_demand_satisfaction(n, c);
			auto v = int64_t(produced - consumed);
			set_text(state, text::prettify(v));
		}
	};
	class trade_market_activity_price : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			auto n = retrieve<dcon::nation_id>(state, parent);
			auto c = retrieve<dcon::commodity_id>(state, parent);
			auto v = int64_t(economy::commodity_market_activity(state, n, c) * economy::commodity_effective_price(state, n, c));
			set_text(state, text::prettify(v));
		}
	};
	class trade_market_activity_entry : public listbox_row_element_base<dcon::commodity_id> {
		public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "goods_type") {
				return make_element_by_type<commodity_image>(state, id);
			} else if(name == "cost") {
				return make_element_by_type<trade_market_activity_price>(state, id);
			} else if(name == "activity") {
				return make_element_by_type<trade_market_activity_value>(state, id);
			} else {
				return nullptr;
			}
		}
	};
	class trade_market_activity_listbox : public listbox_element_base<trade_market_activity_entry, dcon::commodity_id> {
	protected:
		std::string_view get_row_element_name() override {
			return "market_activity_entry";
		}
	public:
		trade_sort sort = trade_sort::commodity;
		bool sort_ascend = false;
		void on_create(sys::state& state) noexcept override {
			listbox_element_base::on_create(state);
			on_update(state);
		}
		void on_update(sys::state& state) noexcept override {
			auto n = state.local_player_nation;

			row_contents.clear();
			for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
				dcon::commodity_id c{ dcon::commodity_id::value_base_t(i) };
				if(economy::commodity_market_activity(state, n, c) != 0.f) {
					row_contents.push_back(c);
				}
			}

			std::function<bool(dcon::commodity_id a, dcon::commodity_id b)> fn;
			switch(sort) {
			case trade_sort::commodity:
				fn = ([&](dcon::commodity_id a, dcon::commodity_id b) {
					return a.index() < b.index();
				});
				break;
			case trade_sort::price:
				fn = ([&](dcon::commodity_id a, dcon::commodity_id b) {
					auto av = economy::commodity_market_activity(state, n, a) * economy::commodity_effective_price(state, n, a);
					auto bv = economy::commodity_market_activity(state, n, a) * economy::commodity_effective_price(state, n, b);
					if(av != bv)
						return av > bv;
					return a.index() < b.index();
				});
				break;
			case trade_sort::demand_satisfaction:
				fn = ([&](dcon::commodity_id a, dcon::commodity_id b) {
					auto av = economy::commodity_market_activity(state, n, a);
					auto bv = economy::commodity_market_activity(state, n, b);
					if(av != bv)
						return av > bv;
					return a.index() < b.index();
				});
				break;
			default:
				break;
			}
			sys::merge_sort(row_contents.begin(), row_contents.end(), fn);
			if(!sort_ascend) {
				std::reverse(row_contents.begin(), row_contents.end());
			}
			update(state);
		}
	};

	class trade_stockpile_entry : public listbox_row_element_base<dcon::commodity_id> {
		public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "goods_type") {
				return make_element_by_type<commodity_image>(state, id);
			} else if(name == "value") {
				return make_element_by_type<commodity_national_player_stockpile_text>(state, id);
			} else if(name == "change") {
				return make_element_by_type<commodity_player_stockpile_increase_text>(state, id);
			} else {
				return nullptr;
			}
		}
	};
	class trade_stockpile_listbox : public listbox_element_base<trade_stockpile_entry, dcon::commodity_id> {
		protected:
		std::string_view get_row_element_name() override {
			return "stockpile_entry";
		}

		public:
		trade_sort sort = trade_sort::commodity;
		bool sort_ascend = false;
		void on_create(sys::state& state) noexcept override {
			listbox_element_base::on_create(state);
			on_update(state);
		}
		void on_update(sys::state& state) noexcept override {
			row_contents.clear();
			for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
				dcon::commodity_id c{ dcon::commodity_id::value_base_t(i) };
				if(state.world.nation_get_stockpiles(state.local_player_nation, c) > 0.0f) {
					row_contents.push_back(c);
				}
			}
			std::function<bool(dcon::commodity_id a, dcon::commodity_id b)> fn;
			switch(sort) {
			case trade_sort::commodity:
				fn = ([&](dcon::commodity_id a, dcon::commodity_id b) {
					return a.index() < b.index();
				});
				break;
			case trade_sort::stockpile:
				fn = ([&](dcon::commodity_id a, dcon::commodity_id b) {
					auto av = state.world.nation_get_stockpiles(state.local_player_nation, a);
					auto bv = state.world.nation_get_stockpiles(state.local_player_nation, b);
					if(av != bv)
						return av > bv;
					return a.index() < b.index();
				});
				break;
			case trade_sort::stockpile_change:
				fn = ([&](dcon::commodity_id a, dcon::commodity_id b) {
					auto av = economy::stockpile_commodity_daily_increase(state, a, state.local_player_nation);
					auto bv = economy::stockpile_commodity_daily_increase(state, b, state.local_player_nation);
					if(av != bv)
						return av > bv;
					return a.index() < b.index();
				});
				break;
			default:
				break;
			}
			sys::merge_sort(row_contents.begin(), row_contents.end(), fn);
			if(!sort_ascend) {
				std::reverse(row_contents.begin(), row_contents.end());
			}
			update(state);
		}
	};

	class trade_common_market_entry : public listbox_row_element_base<dcon::commodity_id> {
		public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "goods_type") {
				return make_element_by_type<commodity_image>(state, id);
			} else if(name == "total") {
				return make_element_by_type<commodity_global_market_text>(state, id);
			} else if(name == "produce_change") {
				return make_element_by_type<commodity_player_real_demand_text>(state, id);
			} else if(name == "exported") {
				return make_element_by_type<commodity_player_domestic_market_text>(state, id);
			} else {
				return nullptr;
			}
		}
	};
	class trade_common_market_listbox : public listbox_element_base<trade_common_market_entry, dcon::commodity_id> {
	protected:
		std::string_view get_row_element_name() override {
			return "common_market_entry";
		}
	public:
		trade_sort sort = trade_sort::commodity;
		bool sort_ascend = false;
		void on_create(sys::state& state) noexcept override {
			listbox_element_base::on_create(state);
			on_update(state);
		}
		void on_update(sys::state& state) noexcept override {
			row_contents.clear();
			for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
				dcon::commodity_id c{ dcon::commodity_id::value_base_t(i) };
				if(state.world.commodity_get_global_market_pool(c) != 0.f) {
					row_contents.push_back(c);
				}
			}
			
			std::function<bool(dcon::commodity_id a, dcon::commodity_id b)> fn;
			switch(sort) {
			case trade_sort::commodity:
				fn = ([&](dcon::commodity_id a, dcon::commodity_id b) {
					return a.index() < b.index();
				});
				break;
			case trade_sort::global_market_pool:
				fn = ([&](dcon::commodity_id a, dcon::commodity_id b) {
					auto av = state.world.commodity_get_global_market_pool(a);
					auto bv = state.world.commodity_get_global_market_pool(b);
					if(av != bv)
						return av > bv;
					return a.index() < b.index();
				});
				break;
			case trade_sort::real_demand:
				fn = ([&](dcon::commodity_id a, dcon::commodity_id b) {
					auto av = state.world.nation_get_real_demand(state.local_player_nation, a);
					auto bv = state.world.nation_get_real_demand(state.local_player_nation, b);
					if(av != bv)
						return av > bv;
					return a.index() < b.index();
				});
				break;
			case trade_sort::domestic_market:
				fn = ([&](dcon::commodity_id a, dcon::commodity_id b) {
					auto av = state.world.nation_get_domestic_market_pool(state.local_player_nation, a);
					auto bv = state.world.nation_get_domestic_market_pool(state.local_player_nation, b);
					if(av != bv)
						return av > bv;
					return a.index() < b.index();
				});
				break;
			default:
				break;
			}
			sys::merge_sort(row_contents.begin(), row_contents.end(), fn);
			if(!sort_ascend) {
				std::reverse(row_contents.begin(), row_contents.end());
			}
			update(state);
		}
	};

	template<class T>
	class trade_goods_needs_entry : public listbox_row_element_base<dcon::commodity_id> {
		public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "goods_type") {
				return make_element_by_type<commodity_image>(state, id);
			} else if(name == "value") {
				return make_element_by_type<T>(state, id);
			} else {
				return nullptr;
			}
		}
	};

	class trade_government_needs_listbox : public listbox_element_base<trade_goods_needs_entry<commodity_player_government_needs_text>, dcon::commodity_id> {
	protected:
		std::string_view get_row_element_name() override {
			return "goods_needs_entry";
		}
	public:
		trade_sort sort = trade_sort::commodity;
		bool sort_ascend = false;
		void on_create(sys::state& state) noexcept override {
			listbox_element_base::on_create(state);
			on_update(state);
		}
		void on_update(sys::state& state) noexcept override {
			row_contents.clear();
			for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
				dcon::commodity_id c{ dcon::commodity_id::value_base_t(i) };
				if(economy::government_consumption(state, state.local_player_nation, c) > 0.0f) {
					row_contents.push_back(c);
				}
			}
			std::function<bool(dcon::commodity_id a, dcon::commodity_id b)> fn;
			switch(sort) {
			case trade_sort::commodity:
				fn = ([&](dcon::commodity_id a, dcon::commodity_id b) {
					return a.index() < b.index();
				});
				break;
			case trade_sort::needs:
				fn = ([&](dcon::commodity_id a, dcon::commodity_id b) {
					auto av = economy::government_consumption(state, state.local_player_nation, a);
					auto bv = economy::government_consumption(state, state.local_player_nation, b);
					if(av != bv)
						return av > bv;
					return a.index() < b.index();
				});
				break;
			default:
				break;
			}
			sys::merge_sort(row_contents.begin(), row_contents.end(), fn);
			if(!sort_ascend) {
				std::reverse(row_contents.begin(), row_contents.end());
			}
			update(state);
		}
	};

	class trade_factory_needs_listbox : public listbox_element_base<trade_goods_needs_entry<commodity_player_factory_needs_text>, dcon::commodity_id> {
	protected:
		std::string_view get_row_element_name() override {
			return "goods_needs_entry";
		}
	public:
		trade_sort sort = trade_sort::commodity;
		bool sort_ascend = false;
		void on_create(sys::state& state) noexcept override {
			listbox_element_base::on_create(state);
			on_update(state);
		}
		void on_update(sys::state& state) noexcept override {
			row_contents.clear();
			for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
				dcon::commodity_id c{ dcon::commodity_id::value_base_t(i) };
				if(economy_factory::nation_factory_consumption(state, state.local_player_nation, c) > 0.0f) {
					row_contents.push_back(c);
				}
			}
			std::function<bool(dcon::commodity_id a, dcon::commodity_id b)> fn;
			switch(sort) {
			case trade_sort::commodity:
				fn = ([&](dcon::commodity_id a, dcon::commodity_id b) {
					return a.index() < b.index();
				});
				break;
			case trade_sort::needs:
				fn = ([&](dcon::commodity_id a, dcon::commodity_id b) {
					auto av = economy_factory::nation_factory_consumption(state, state.local_player_nation, a);
					auto bv = economy_factory::nation_factory_consumption(state, state.local_player_nation, b);
					if(av != bv)
						return av > bv;
					return a.index() < b.index();
				});
				break;
			default:
				break;
			}
			sys::merge_sort(row_contents.begin(), row_contents.end(), fn);
			if(!sort_ascend) {
				std::reverse(row_contents.begin(), row_contents.end());
			}
			update(state);
		}
	};

	class trade_pop_needs_listbox : public listbox_element_base<trade_goods_needs_entry<commodity_player_pop_needs_text>, dcon::commodity_id> {
		protected:
		std::string_view get_row_element_name() override {
			return "goods_needs_entry";
		}
	public:
		trade_sort sort = trade_sort::commodity;
		bool sort_ascend = false;
		void on_create(sys::state& state) noexcept override {
			listbox_element_base::on_create(state);
			on_update(state);
		}
		void on_update(sys::state& state) noexcept override {
			row_contents.clear();
			for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
				dcon::commodity_id c{ dcon::commodity_id::value_base_t(i) };
				if(economy::nation_pop_consumption(state, state.local_player_nation, c) > 0.0f) {
					row_contents.push_back(c);
				}
			}
			std::function<bool(dcon::commodity_id a, dcon::commodity_id b)> fn;
			switch(sort) {
			case trade_sort::commodity:
				fn = ([&](dcon::commodity_id a, dcon::commodity_id b) {
					return a.index() < b.index();
				});
				break;
			case trade_sort::needs:
				fn = ([&](dcon::commodity_id a, dcon::commodity_id b) {
					auto av = economy::nation_pop_consumption(state, state.local_player_nation, a);
					auto bv = economy::nation_pop_consumption(state, state.local_player_nation, b);
					if(av != bv)
						return av > bv;
					return a.index() < b.index();
				});
				break;
			default:
				break;
			}
			sys::merge_sort(row_contents.begin(), row_contents.end(), fn);
			if(!sort_ascend) {
				std::reverse(row_contents.begin(), row_contents.end());
			}
			update(state);
		}
	};

	struct trade_details_select_commodity {
		dcon::commodity_id commodity_id{};
	};
	struct trade_details_open_window {
		dcon::commodity_id commodity_id{};
	};

	class trade_commodity_entry_button : public tinted_button_element_base {
	public:
		void on_update(sys::state& state) noexcept override;
		void button_action(sys::state& state) noexcept override;
		void button_right_action(sys::state& state) noexcept override;
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override;
		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
	};

	class commodity_automation_indicator : public image_element_base {
		bool visible = false;
	public:
		void on_update(sys::state& state) noexcept override {
			auto com = retrieve<dcon::commodity_id>(state, parent);
			visible = state.world.nation_get_stockpile_targets(state.local_player_nation, com) > 0;
		}
		void impl_render(sys::state& state, int32_t x, int32_t y) noexcept override {
			if(visible) {
				image_element_base::impl_render(state, x, y);
			}
		}
	};

	class commodity_stockpile_indicator : public image_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			auto com = retrieve<dcon::commodity_id>(state, parent);
			if(state.world.nation_get_drawing_on_stockpiles(state.local_player_nation, com)) {
				if(state.world.nation_get_stockpiles(state.local_player_nation, com) > 0) {
					frame = 2;
				} else {
					frame = 0;
				}
			} else if(state.world.nation_get_stockpiles(state.local_player_nation, com)  < state.world.nation_get_stockpile_targets(state.local_player_nation, com)) {
				frame = 1;
			} else {
				frame = 0;
			}
		}

		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}
		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			auto com = retrieve<dcon::commodity_id>(state, parent);
			if(state.world.nation_get_drawing_on_stockpiles(state.local_player_nation, com)) {
				if(state.world.nation_get_stockpiles(state.local_player_nation, com) > 0) {
					text::add_line(state, contents, "trade_setting_drawing");
				}
			} else if(state.world.nation_get_stockpiles(state.local_player_nation, com) < state.world.nation_get_stockpile_targets(state.local_player_nation, com)) {
				text::add_line(state, contents, "trade_setting_filling");
			}
		}
	};

	class commodity_price_trend : public image_element_base {
		public:
		void on_update(sys::state& state) noexcept override {
			auto com = retrieve<dcon::commodity_id>(state, parent);
			auto current_price = state.world.commodity_get_price_record(com, (state.ui_date.value >> 4) % economy::price_history_length);
			auto previous_price = state.world.commodity_get_price_record(com, ((state.ui_date.value >> 4) + economy::price_history_length - 1) % economy::price_history_length);
			if(current_price > previous_price) {
				frame = 0;
			} else if(current_price < previous_price) {
				frame = 2;
			} else {
				frame = 1;
			}
		}
	};

	class trade_commodity_icon : public button_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			frame = int32_t(state.world.commodity_get_icon(retrieve<dcon::commodity_id>(state, parent)));
		}

		tooltip_behavior has_tooltip(sys::state& state) noexcept override {
			return tooltip_behavior::variable_tooltip;
		}

		void update_tooltip(sys::state& state, int32_t x, int32_t y, text::columnar_layout& contents) noexcept override {
			auto com = retrieve<dcon::commodity_id>(state, parent);
			if(com) {
				auto box = text::open_layout_box(contents, 0);
				text::add_to_layout_box(state, contents, box, text::produce_simple_string(state, state.world.commodity_get_name(com)), text::text_color::yellow);
				text::close_layout_box(contents, box);
			}
		}

		void button_action(sys::state& state) noexcept override {
			trade_details_select_commodity payload{ retrieve<dcon::commodity_id>(state, parent) };
			send<trade_details_select_commodity>(state, state.ui_state.trade_subwindow, payload);
			Cyto::Any dt_payload = trade_details_open_window{ retrieve<dcon::commodity_id>(state, parent) };
			state.ui_state.trade_subwindow->impl_get(state, dt_payload);
		}
	};

	class trade_commodity_entry : public window_element_base {
	public:
		dcon::commodity_id commodity_id{};
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "entry_button") {
				return make_element_by_type<trade_commodity_entry_button>(state, id);
			} else if(name == "goods_type") {
				return make_element_by_type<trade_commodity_icon>(state, id);
			} else if(name == "price") {
				return make_element_by_type<commodity_price_text>(state, id);
			} else if(name == "trend_indicator") {
				return make_element_by_type<commodity_price_trend>(state, id);
			} else if(name == "selling_indicator") {
				return make_element_by_type<commodity_stockpile_indicator>(state, id);
			} else if(name == "automation_indicator") {
				return make_element_by_type<commodity_automation_indicator>(state, id);
			} else {
				return nullptr;
			}
		}

		message_result get(sys::state& state, Cyto::Any& payload) noexcept override {
			if(payload.holds_type<dcon::commodity_id>()) {
				payload.emplace<dcon::commodity_id>(commodity_id);
				return message_result::consumed;
			}
			return message_result::unseen;
		}
	};

	class trade_flow_data {
		public:
		dcon::commodity_id trade_good;
		enum class type : uint8_t {
			factory,
			province,
			pop,
			military_army,
			military_navy,
		} type{};
		enum class value_type : uint8_t { used_by, produced_by, may_be_used_by } value_type{};
		union {
			dcon::factory_id factory_id;			 // factory
			dcon::province_id province_id;		 // province
			dcon::province_id pop_province_id; // pop
			dcon::army_id army_id;						 // army
			dcon::navy_id navy_id;						 // navy
		} data{};

		bool operator==(trade_flow_data const& o) const {
			if(value_type != o.value_type || type != o.type)
				return false;
			switch(type) {
			case type::factory:
				return data.factory_id == o.data.factory_id;
			case type::province:
				return data.province_id == o.data.province_id;
			case type::pop:
				return data.pop_province_id == o.data.pop_province_id;
			case type::military_army:
				return data.army_id == o.data.army_id;
			case type::military_navy:
				return data.navy_id == o.data.navy_id;
			}
			return true;
		}
		bool operator!=(trade_flow_data const& o) const {
			return !(*this == o);
		}
	};
	class trade_flow_entry : public listbox_row_element_base<trade_flow_data> {
		image_element_base* icon = nullptr;
		simple_text_element_base* title = nullptr;
		simple_text_element_base* value = nullptr;
		image_element_base* output_icon = nullptr;

		public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override {
			if(name == "icon") {
				auto ptr = make_element_by_type<image_element_base>(state, id);
				icon = ptr.get();
				return ptr;
			} else if(name == "title") {
				auto ptr = make_element_by_type<simple_text_element_base>(state, id);
				title = ptr.get();
				return ptr;
			} else if(name == "value") {
				auto ptr = make_element_by_type<simple_text_element_base>(state, id);
				value = ptr.get();
				return ptr;
			} else if(name == "output_icon") {
				auto ptr = make_element_by_type<image_element_base>(state, id);
				output_icon = ptr.get();
				return ptr;
			} else {
				return nullptr;
			}
		}

		void on_update(sys::state& state) noexcept override {
			auto commodity_id = retrieve<dcon::commodity_id>(state, parent);

			icon->frame = int32_t(content.type);
			if(content.type == trade_flow_data::type::military_navy) {
				icon->frame = int32_t(trade_flow_data::type::military_army);
			}

			output_icon->set_visible(state, content.value_type != trade_flow_data::value_type::produced_by);
			value->set_visible(state, content.value_type != trade_flow_data::value_type::may_be_used_by);
			float amount = 0.f;
			switch(content.type) {
				case trade_flow_data::type::factory: {
					auto fid = content.data.factory_id;
					auto ftid = state.world.factory_get_building_type(fid);
					switch(content.value_type) {
					case trade_flow_data::value_type::produced_by: {
						amount += state.world.factory_get_actual_production(fid);
					} break;
					case trade_flow_data::value_type::used_by: {
						auto& inputs = state.world.factory_type_get_inputs(ftid);
						for(uint32_t i = 0; i < inputs.set_size; ++i) {
							if(inputs.commodity_type[i] == commodity_id) {
								amount += inputs.commodity_amounts[i];
							}
						}
						output_icon->frame = state.world.commodity_get_icon(state.world.factory_type_get_output(ftid));
					} break;
					case trade_flow_data::value_type::may_be_used_by:
						output_icon->frame = state.world.commodity_get_icon(state.world.factory_type_get_output(ftid));
						break;
					default:
						break;
					}
					auto name = state.world.factory_type_get_name(ftid);
					title->set_text(state, text::produce_simple_string(state, name));
				} break;
				case trade_flow_data::type::province: {
					auto pid = content.data.province_id;
					switch(content.value_type) {
					case trade_flow_data::value_type::produced_by: {
							amount += state.world.province_get_rgo_actual_production_per_good(pid, content.trade_good);
						} break;
					case trade_flow_data::value_type::used_by:
					case trade_flow_data::value_type::may_be_used_by:
					default:
						break;
					}
					auto name = state.world.province_get_name(pid);
					title->set_text(state, text::produce_simple_string(state, name));
				} break;
			case trade_flow_data::type::pop:
			case trade_flow_data::type::military_army:
			case trade_flow_data::type::military_navy:
			default:
				break;
			}
			if(value->is_visible())
			value->set_text(state, text::format_float(amount, 2));
		}
	};
	class trade_flow_listbox_base : public listbox_element_base<trade_flow_entry, trade_flow_data> {
	protected:
		std::string_view get_row_element_name() override {
			return "trade_flow_entry";
		}

		template<typename F>
		void populate_rows(sys::state& state, F&& factory_func, enum trade_flow_data::value_type vt) {
			auto commodity_id = retrieve<dcon::commodity_id>(state, parent);
			auto n = state.local_player_nation;
			for(auto const fat_stown_id : state.world.nation_get_state_ownership(n)) {
				province::for_each_province_in_state_instance(state, fat_stown_id.get_state(), [&](dcon::province_id pid) {
					// factories
					state.world.province_for_each_factory_location_as_province(pid, [&](dcon::factory_location_id flid) {
						auto fid = state.world.factory_location_get_factory(flid);
						if(factory_func(fid)) {
							trade_flow_data td{};
							td.type = trade_flow_data::type::factory;
							td.value_type = vt;
							td.data.factory_id = fid;
							td.trade_good = commodity_id;
							row_contents.push_back(td);
						}
					});
					// rgos
					if(vt == trade_flow_data::value_type::produced_by) {
						if(state.world.province_get_rgo_actual_production_per_good(pid, commodity_id) > 0.f) {
							trade_flow_data td{};
							td.type = trade_flow_data::type::province;
							td.value_type = vt;
							td.data.province_id = pid;
							td.trade_good = commodity_id;
							row_contents.push_back(td);
						}
					}
					// artisans
					if(state.world.nation_get_artisan_actual_production(n, commodity_id) > 0.f) {
						// production of artisans
						//auto total_artisans = 0.f;
						//for(const auto pl : state.world.province_get_pop_location(pid)) {
						//	total_artisans = pl.get_pop().get_size();
						//}
						if(vt == trade_flow_data::value_type::produced_by) {
							trade_flow_data td{};
							td.type = trade_flow_data::type::pop;
							td.value_type = vt;
							td.data.province_id = pid;
							td.trade_good = commodity_id;
							row_contents.push_back(td);
						}
						// ingredients needed by artisans
						if(vt == trade_flow_data::value_type::used_by) {
							auto const& inputs = state.world.commodity_get_artisan_inputs(commodity_id);
							for(uint32_t j = 0; j < economy::commodity_set::set_size; ++j) {
								if(inputs.commodity_type[j]) {
									trade_flow_data td{};
									td.type = trade_flow_data::type::pop;
									td.value_type = vt;
									td.data.province_id = pid;
									td.trade_good = inputs.commodity_type[j];
									row_contents.push_back(td);
								} else {
									break;
								}
							}
						}
					}
				});
			}
		}
	};

	class trade_flow_produced_by_listbox : public trade_flow_listbox_base {
		public:
		void on_update(sys::state& state) noexcept override;
	};
	class trade_flow_used_by_listbox : public trade_flow_listbox_base {
		public:
		void on_update(sys::state& state) noexcept override;
	};
	class trade_flow_may_be_used_by_listbox : public trade_flow_listbox_base {
	public:
		void on_update(sys::state& state) noexcept override;
	};

	class trade_flow_producers_piechart : public piechart<dcon::nation_id> {
	public:
		void on_update(sys::state& state) noexcept override;
	};

	class trade_flow_consumers_piechart : public piechart<dcon::nation_id> {
	public:
		void on_update(sys::state& state) noexcept override;
	};

	class trade_flow_workers_piechart : public piechart<dcon::pop_type_id> {
	public:
		void on_update(sys::state& state) noexcept override;
	};

	class trade_flow_price_graph_window : public window_element_base {
	public:
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
	};

	class trade_flow_total_produced_text : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			auto cid = retrieve<dcon::commodity_id>(state, parent);
			auto n = state.local_player_nation;
			auto amount = state.world.nation_get_artisan_actual_production(n, cid);
			for(auto const fat_stown_id : state.world.nation_get_state_ownership(n)) {
				province::for_each_province_in_state_instance(state, fat_stown_id.get_state(), [&](dcon::province_id pid) {
					auto fat_id = dcon::fatten(state.world, pid);
					fat_id.for_each_factory_location_as_province([&](dcon::factory_location_id flid) {
						auto fid = state.world.factory_location_get_factory(flid);
						auto ftid = state.world.factory_get_building_type(fid);
						if(state.world.factory_type_get_output(ftid) == cid) {
							amount += state.world.factory_get_actual_production(fid);
						}
					});
					if(state.world.province_get_rgo(pid) == cid) {
						amount += state.world.province_get_rgo_actual_production_per_good(pid, cid);
					}
				});
			}
			set_text(state, text::format_float(amount, 2));
		}
	};
	class trade_flow_total_used_text : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			auto commodity_id = retrieve<dcon::commodity_id>(state, parent);
			auto v = economy_factory::nation_factory_consumption(state, state.local_player_nation, commodity_id);
			set_text(state, text::format_float(v, 2));
		}
	};

	class trade_flow_world_production_text : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			auto cid = retrieve<dcon::commodity_id>(state, parent);
			float total = state.world.commodity_get_total_production(cid);
			float amount = state.world.nation_get_domestic_market_pool(state.local_player_nation, cid);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::x, text::fp_two_places{ (total > 0.f ? amount / total : 0.f) * 100.f });
			set_text(state, text::resolve_string_substitution(state, "trade_flow_world_production", m));
		}
	};

	class trade_flow_window : public window_element_base {
	public:
		void on_create(sys::state& state) noexcept override {
			window_element_base::on_create(state);
			set_visible(state, false);
		}
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
	};

	class trade_commodity_group_window : public window_element_base {
		std::vector<trade_commodity_entry*> entries_element;
	public:
		dcon::commodity_group_id content;

		void on_create(sys::state& state) noexcept override;
		void on_update(sys::state& state) noexcept override;
	};

	class trade_details_button : public button_element_base {
		public:
		void button_action(sys::state& state) noexcept override {
			auto commodity_id = retrieve<dcon::commodity_id>(state, parent);
			Cyto::Any dt_payload = trade_details_open_window{commodity_id};
			state.ui_state.trade_subwindow->impl_get(state, dt_payload);
		}
	};

	class trade_confirm_trade_button : public button_element_base {
		public:
		void button_action(sys::state& state) noexcept override {
			// TODO: change stockpile settings option?
			//float famount = retrieve<float>(state, parent);
			//auto cid = retrieve<dcon::commodity_id>(state, parent);
			//command::change_stockpile_settings(state, state.local_player_nation, cid, famount, false);
		}
	};

	class prices_line_graph : public line_graph {
	public:
		prices_line_graph() : line_graph(32) { }
		void on_create(sys::state& state) noexcept override {
			line_graph::on_create(state);
		}
		void on_update(sys::state& state) noexcept override {
			auto com = retrieve<dcon::commodity_id>(state, parent);
			assert(economy::price_history_length >= 32);
			std::vector<float> datapoints(32);
			auto newest_index = economy::most_recent_price_record_index(state);
			for(uint32_t i = 0; i < 32; ++i) {
				datapoints[i] = state.world.commodity_get_price_record(com, (newest_index + i + economy::price_history_length - 32) % economy::price_history_length);
			}
			set_data_points(state, datapoints);
		}
	};

	class price_chart_high : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			auto com = retrieve<dcon::commodity_id>(state, parent);
			auto newest_index = economy::most_recent_price_record_index(state);
			float max_price = state.world.commodity_get_price_record(com, newest_index);
			for(int32_t i = 1; i < 32; ++i) {
				max_price = std::max(state.world.commodity_get_price_record(com, (newest_index + i + economy::price_history_length - 32) % economy::price_history_length), max_price);
			}
			set_text(state, text::format_money(max_price));
		}
	};

	class price_chart_low : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			auto com = retrieve<dcon::commodity_id>(state, parent);
			auto newest_index = economy::most_recent_price_record_index(state);
			float min_price = state.world.commodity_get_price_record(com, newest_index);
			for(int32_t i = 1; i < 32; ++i) {
				min_price = std::min(state.world.commodity_get_price_record(com, (newest_index + i + economy::price_history_length - 32) % economy::price_history_length), min_price);
			}
			set_text(state, text::format_money(min_price));
		}
	};

	class stockpile_sell_button : public button_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			auto com = retrieve<dcon::commodity_id>(state, parent);
			frame = state.world.nation_get_drawing_on_stockpiles(state.local_player_nation, com) ? 1 : 0;
		}

		void button_action(sys::state& state) noexcept override {
			auto com = retrieve<dcon::commodity_id>(state, parent);
			command::change_stockpile_settings(state, state.local_player_nation, com, state.world.nation_get_stockpile_targets(state.local_player_nation, com), !state.world.nation_get_drawing_on_stockpiles(state.local_player_nation, com));
		}
	};

	class stockpile_sell_label : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			auto com = retrieve<dcon::commodity_id>(state, parent);
			set_text(state, text::produce_simple_string(state, state.world.nation_get_drawing_on_stockpiles(state.local_player_nation, com) ? "trade_use" : "trade_fill"));
		}
	};

	class stockpile_slider_label : public simple_text_element_base {
	public:
		void on_create(sys::state& state) noexcept override {
			simple_text_element_base::on_create(state);
			set_text(state, text::produce_simple_string(state, "trade_stockpile_target"));
		}
	};

	class stockpile_amount_label : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			auto com = retrieve<dcon::commodity_id>(state, parent);
			set_text(state, text::produce_simple_string(state, "trade_stockpile_current") + text::format_float(state.world.nation_get_stockpiles(state.local_player_nation, com), 1));
		}
	};

	class detail_domestic_production : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			auto com = retrieve<dcon::commodity_id>(state, parent);
			text::substitution_map m;
			text::add_to_substitution_map(m, text::variable_type::val, text::pretty_integer{ int64_t(state.world.nation_get_domestic_market_pool(state.local_player_nation, com)) });
			set_text(state, text::resolve_string_substitution(state, "produced_detail_remove", m));
		}
	};

	struct stockpile_target_change {
		float value;
	};
	struct get_stockpile_target {
		float value;
	};

	class trade_slider : public scrollbar {
	public:
		void on_value_change(sys::state& state, int32_t v) noexcept final {
			float a = float(v) - 1.0f;
			send(state, parent, stockpile_target_change{ a });
			if(state.ui_state.drag_target != slider) {
				commit_changes(state);
			}
		}

		void on_update(sys::state& state) noexcept final {
			auto com = retrieve<dcon::commodity_id>(state, parent);
			if(com) {
				if(state.ui_state.drag_target == slider) {

				} else {
					auto value = state.world.nation_get_stockpile_targets(state.local_player_nation, com);
					update_raw_value(state, int32_t(value + 1.f));
					send(state, parent, stockpile_target_change{ value + 1.f });
				}
			}
		}
		void on_drag_finish(sys::state& state) noexcept override {
			commit_changes(state);
		}
		void commit_changes(sys::state& state) noexcept {
			auto com = retrieve<dcon::commodity_id>(state, parent);
			float v = float(raw_value()) - 1.0f;
			command::change_stockpile_settings(state, state.local_player_nation, com, v, state.world.nation_get_drawing_on_stockpiles(state.local_player_nation, com));
		}
	};

	class trade_slider_amount : public simple_text_element_base {
	public:
		void on_update(sys::state& state) noexcept override {
			auto com = retrieve<dcon::commodity_id>(state, parent);
			auto val = retrieve<get_stockpile_target>(state, parent);
			set_text(state, text::prettify(int64_t(val.value)));
		}
	};

	class trade_details_window : public window_element_base {
		simple_text_element_base* slider_value_display = nullptr;
	public:
		float trade_amount = 0.0f;
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
		message_result get(sys::state& state, Cyto::Any& payload) noexcept override;
	};

	class trade_window : public window_element_base {
		trade_flow_window* trade_flow_win = nullptr;
		trade_details_window* details_win = nullptr;
		dcon::commodity_id commodity_id{1};
		trade_market_activity_listbox* list_ma = nullptr;
		trade_stockpile_listbox* list_sp = nullptr;
		trade_common_market_listbox* list_cm = nullptr;
		trade_government_needs_listbox* list_gn = nullptr;
		trade_factory_needs_listbox* list_fn = nullptr;
		trade_pop_needs_listbox* list_pn = nullptr;
	public:
		void on_create(sys::state& state) noexcept override;
		std::unique_ptr<element_base> make_child(sys::state& state, std::string_view name, dcon::gui_def_id id) noexcept override;
		message_result get(sys::state& state, Cyto::Any& payload) noexcept override;
	};

} // namespace ui
