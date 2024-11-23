#pragma once
#include "dcon_generated.hpp"
#include "date_interface.hpp"
#include "constants.hpp"

namespace sys {
	struct state;
}

namespace news {
	enum class news_generator_type {
		none,
		peace_offer_accept,
		game_event,
		province_change_controller,
		province_change_owner,
		construction_complete,
		research_complete,
		battle_over,
		rebel_break_country,
		new_party,
		war_declared,
		crisis_started,
		crisis_backer,
		crisis_side_joined,
		crisis_resolved,
		decision,
		goods_price_change,
		ai_afraid_of,
		ai_likes_very_much,
		fake,
		invention,
		count
	};
	constexpr inline uint32_t max_news_generator_types = uint32_t(news_generator_type::count);
	constexpr inline uint32_t news_type_mask = 0x7f;
	constexpr inline uint8_t news_size_small = 0x00;
	constexpr inline uint8_t news_size_medium = 0x80;
	constexpr inline uint8_t news_size_huge = 0xC0;
	constexpr inline uint8_t news_size_mask = 0x80 | 0xC0;

	struct news_scope {
		dcon::text_key strings[4][6]; //4*6*4
		int32_t values[4][4]; //4*4*4
		dcon::text_key title; //4
		dcon::text_key desc; //4
		sys::date dates[4][4]; //4*4*2
		dcon::national_identity_id tags[4][4]; //4*4*2
		dcon::gfx_object_id picture; //2
		news::news_generator_type type; //1
		uint8_t padding = 0;
	};
	/*
	static_assert(sizeof(news_scope) ==
		sizeof(news_scope::strings)
		+ sizeof(news_scope::values)
		+ sizeof(news_scope::title)
		+ sizeof(news_scope::desc)
		+ sizeof(news_scope::dates)
		+ sizeof(news_scope::tags)
		+ sizeof(news_scope::picture)
		+ sizeof(news_scope::type)
		+ sizeof(news_scope::padding));
	*/

	struct global_news_state {
		news_scope currently_collected; //currently collected
		news_scope article_pool[5 + 2 + 1];
		news_scope small_articles[5];
		news_scope medium_articles[2];
		news_scope large_articles[1];
		uint32_t num_collected = 0;
		dcon::effect_key on_collect_effect[max_news_generator_types];
		dcon::trigger_key on_collect_trigger[max_news_generator_types];
		dcon::effect_key on_printing_effect[max_news_generator_types];
		dcon::trigger_key on_printing_trigger[max_news_generator_types];
		sys::date last_print;
	};
	// TODO: Make memcpyable

	void collect_news_scope(sys::state& state, news_scope& scope);
	void issue_newspaper(sys::state& state, dcon::nation_id reader);
}
