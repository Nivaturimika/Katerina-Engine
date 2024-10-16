#include "serialization.hpp"
#include "system_state.hpp"
#include "reports.hpp"

static sys::state game_state; // too big for the stack

int main(int argc, char **argv) {
	simple_fs::identify_global_system_properties(); // -- globals startup
	add_root(game_state.common_fs, NATIVE(".")); // will add the working directory as first root -- for the moment this lets us find the shader files

	if(argc >= 2) {
		reports::write_debug("Loading scenario: " + text::native_to_utf8(argv[1]));
		for(int i = 1; i < argc; ++i) {
			if(native_string(argv[i]) == NATIVE("-host")) {
				game_state.network_mode = sys::network_mode_type::host;
			} else if(native_string(argv[i]) == NATIVE("-join")) {
				game_state.network_mode = sys::network_mode_type::client;
				game_state.network_state.ip_address = "127.0.0.1";
				if(i + 1 < argc) {
					game_state.network_state.ip_address = text::native_to_utf8(native_string(argv[i + 1]));
					i++;
				}
			} else if(native_string(argv[i]) == NATIVE("-name")) {
				if(i + 1 < argc) {
					std::string nickname = text::native_to_utf8(native_string(argv[i + 1]));
					memcpy(game_state.network_state.nickname.data, nickname.data(), std::min<size_t>(nickname.length(), 8));
					i++;
				}
			} else if(native_string(argv[i]) == NATIVE("-password")) {
				if(i + 1 < argc) {
					auto str = text::native_to_utf8(native_string(argv[i + 1]));
					std::memset(game_state.network_state.password, '\0', sizeof(game_state.network_state.password));
					std::memcpy(game_state.network_state.password, str.c_str(), std::min(sizeof(game_state.network_state.password), str.length()));
					i++;
				}
			} else if(native_string(argv[i]) == NATIVE("-v6")) {
				game_state.network_state.as_v6 = true;
			} else if(native_string(argv[i]) == NATIVE("-v4")) {
				game_state.network_state.as_v6 = false;
			}
		}

		if(sys::try_read_scenario_and_save_file(game_state, argv[1])) {
			game_state.fill_unsaved_data();
		} else {
			auto msg = std::string("Scenario file ") + text::native_to_utf8(argv[1]) + " could not be read";
			window::emit_error_message(msg, true);
			return 0;
		}

		network::init(game_state);
	} else {
		if(!sys::try_read_scenario_and_save_file(game_state, NATIVE("development_test_file.bin"))) {
			// scenario making functions
			parsers::error_handler err{ "" };
			game_state.load_scenario_data(err, sys::year_month_day{ 1836, 1, 1 });
			if(!err.accumulated_errors.empty())
				window::emit_error_message(err.accumulated_errors, true);
			sys::write_scenario_file(game_state, NATIVE("development_test_file.bin"), 0);
			game_state.loaded_scenario_file = NATIVE("development_test_file.bin");
		} else {
			game_state.fill_unsaved_data();
		}
	}

	// scenario loading functions (would have to run these even when scenario is pre-built
	game_state.load_user_settings();
	ui::populate_definitions_map(game_state);

	std::thread update_thread([&]() { game_state.game_loop(); });

	window::create_window(game_state, window::creation_parameters{ 1024, 780, window::window_state::maximized, game_state.user_settings.prefer_fullscreen });
	window::initialize_window(game_state);

	game_state.quit_signaled.store(true, std::memory_order_release);
	update_thread.join();

	network::finish(game_state, true);
	return EXIT_SUCCESS;
}
