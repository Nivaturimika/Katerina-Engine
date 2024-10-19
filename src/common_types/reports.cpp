#include <stdio.h>
#include "reports.hpp"

namespace reports {
	// Static globals, oooh scary!!! (dont do this)
	static FILE* fp = NULL;
	static bool tried_opening_fp = false;

	void write_debug(std::string_view msg) noexcept {
		if(msg.size() > 0) {
			std::string s = std::string(msg);
#ifdef _WIN32
			if(!fp && !tried_opening_fp) {
				fopen_s(&fp, "game_log.txt", "wt");
				tried_opening_fp = true;
			}
			if(fp) {
				fprintf(fp, "%s", s.c_str());
			}
			OutputDebugStringA(s.c_str());
#else
			std::fprintf(stderr, "%s", s.c_str());
#endif
		}
	}
}
