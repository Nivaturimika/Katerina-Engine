#pragma once

std::vector<uint32_t> migration_map_from(sys::state& state) {
	uint32_t province_size = state.world.province_size();
	uint32_t texture_size = province_size + 256 - province_size % 256;

	std::vector<uint32_t> prov_color(texture_size * 2);

	auto selected = state.map_state.selected_province;
	auto for_nation = state.world.province_get_nation_from_province_ownership(selected);
	if(for_nation) {
		float mx = 0.0f;
		float mn = 0.0f;
		for(auto p : state.world.nation_get_province_ownership(for_nation)) {
			auto v = p.get_province().get_daily_net_migration();
			mn = std::min(mn, v);
			mx = std::max(mx, v);
		}
		if(mx > mn) {
			for(auto p : state.world.nation_get_province_ownership(for_nation)) {
				auto v = p.get_province().get_daily_net_migration();

				uint32_t color = ogl::color_gradient((v - mn) / (mx - mn),
					sys::pack_color(46, 247, 15),	// to green
					sys::pack_color(247, 15, 15)	// from red
				);
				auto i = province::to_map_id(p.get_province());
				prov_color[i] = color;
				prov_color[i + texture_size] = color;
			}
		}
	} else {
		static auto last_checked_date = sys::date{};
		static std::vector<float> nation_pos;
		static std::vector<float> nation_neg;

		if(state.ui_date != last_checked_date) {
			last_checked_date = state.ui_date;

			auto sz = state.world.nation_size();
			if(uint32_t(nation_pos.size()) < sz)
				nation_pos.resize(sz);
			for(uint32_t i = 0; i < sz; ++i)
				nation_pos[i] = 0.0f;
			//
			if(uint32_t(nation_neg.size()) < sz)
				nation_neg.resize(sz);
			for(uint32_t i = 0; i < sz; ++i)
				nation_neg[i] = 0.0f;

			float least_neg = -1.0f;
			float greatest_pos = 1.0f;
			for(auto p : state.world.in_province) {
				auto owner = p.get_nation_from_province_ownership();
				if(owner && uint32_t(owner.id.index()) < sz) {
					auto v = p.get_daily_net_immigration();
					if(v > 0.f)
						nation_pos[owner.id.index()] += v;
					else
						nation_neg[owner.id.index()] += v;
				}
			}
			for(uint32_t i = 0; i < sz; ++i) {
				least_neg = std::min(nation_neg[i], least_neg);
				greatest_pos = std::max(nation_pos[i], greatest_pos);
			}
			for(uint32_t i = 0; i < sz; ++i) {
				if(nation_neg[i] != 0.0f)
					nation_neg[i] = nation_neg[i] / least_neg;
				if(nation_pos[i] != 0.0f)
					nation_pos[i] = nation_pos[i] / greatest_pos;
			}
		}
		for(auto p : state.world.in_province) {
			auto owner = p.get_nation_from_province_ownership();
			if(owner) {
				auto i = province::to_map_id(p);
				if(uint32_t(owner.id.index()) < nation_pos.size()
				&& uint32_t(owner.id.index()) < nation_neg.size()) {
					uint32_t in_color = ogl::color_gradient(nation_pos[owner.id.index()],
						sys::pack_color(15, 255, 15),
						sys::pack_color(15, 64, 15)
					);
					uint32_t em_color = ogl::color_gradient(nation_neg[owner.id.index()],
						sys::pack_color(255, 15, 15),
						sys::pack_color(64, 15, 15)
					);
					if(nation_pos[owner.id.index()] != 0.f) { //has im
						prov_color[i] = in_color;
						if(nation_neg[owner.id.index()] != 0.f) { //has im and em
							prov_color[i + texture_size] = em_color;
						} else { //has only im
							prov_color[i + texture_size] = in_color;
						}
					} else if(nation_neg[owner.id.index()] != 0.f) { //only em -- else has no em or im
						prov_color[i] = em_color;
						prov_color[i + texture_size] = em_color;
					}
				}
			}
		}
	}
	return prov_color;
}
