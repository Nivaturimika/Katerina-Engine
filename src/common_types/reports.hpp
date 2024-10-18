#pragma once

#include <string_view>
namespace reports {
	inline void write_debug(std::string_view msg) noexcept {
		if(msg.size() > 0) {
			std::string s = std::string(msg);
#ifdef _WIN32
			static FILE* fp = fopen("game_log.txt", "wt");
			fprintf(fp, "%s", s.c_str());
			OutputDebugStringA(s.c_str());
#else
			std::fprintf(stderr, "%s", s.c_str());
#endif
		}
	}
}
