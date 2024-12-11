#include "system_state.hpp"
#include "parsers.hpp"
#include "parsers_declarations.hpp"
#include <random>

static sys::state game_state; // too big for the stack

native_string produce_mod_path(std::vector<parsers::mod_file>& mod_list) {
	simple_fs::file_system dummy;
	simple_fs::add_root(dummy, NATIVE("."));
	for(int32_t i = 0; i < int32_t(mod_list.size()); ++i) {
		mod_list[i].add_to_file_system(dummy);
	}
	return simple_fs::extract_state(dummy);
}

native_string make_scenario(simple_fs::file_system& fs_root, parsers::error_handler& err, native_string_view path) {
	auto root = simple_fs::get_root(fs_root);
	auto common = simple_fs::open_directory(root, NATIVE("common"));
	parsers::bookmark_context bookmark_context;
	if(auto f = simple_fs::open_file(common, NATIVE("bookmarks.txt")); f) {
		auto bookmark_content = simple_fs::view_contents(*f);
		err.file_name = "bookmarks.txt";
		parsers::token_generator gen(bookmark_content.data, bookmark_content.data + bookmark_content.file_size);
		parsers::parse_bookmark_file(gen, err, bookmark_context);
		assert(!bookmark_context.bookmark_dates.empty());
	} else {
		err.accumulated_errors += "File common/bookmarks.txt could not be opened\n";
	}
	native_string selected_scenario_file;
	sys::checksum_key scenario_key;
	for(uint32_t date_index = 0; date_index < uint32_t(bookmark_context.bookmark_dates.size()); date_index++) {
		err.accumulated_errors.clear();
		err.accumulated_warnings.clear();
		//
		auto book_game_state = std::make_unique<sys::state>();
		simple_fs::restore_state(book_game_state->common_fs, path);
		book_game_state->load_scenario_data(err, bookmark_context.bookmark_dates[date_index].date_);
		if(err.fatal)
			break;
		if(date_index == 0) {
			auto sdir = simple_fs::get_or_create_scenario_directory();
			native_string base_name;
			for(auto const c : path) {
				if(c == NATIVE('?')) {
					break;
				} else if(!(c == NATIVE(';') || c == NATIVE('.') || c == NATIVE('/') || c == NATIVE('\\'))) {
					base_name.push_back(c);
				}
			}
			int32_t append = 0;
			auto time_stamp = uint64_t(std::time(0));
			while(simple_fs::peek_file(sdir, base_name + NATIVE("-") + to_native_string(append) + NATIVE(".bin"))) {
				++append;
			}
			selected_scenario_file = base_name + NATIVE("-") + to_native_string(append) + NATIVE(".bin");
			sys::write_scenario_file(*book_game_state, selected_scenario_file, 1);
			scenario_key = book_game_state->scenario_checksum;
		} else {
			//sys::write_scenario_file(*book_game_state, to_native_string(date_index) + NATIVE(".bin"), 0);
			book_game_state->scenario_checksum = scenario_key;
			sys::write_save_file(*book_game_state, sys::save_type::bookmark, bookmark_context.bookmark_dates[date_index].name_);
		}
	}
	return selected_scenario_file;
}

native_string find_matching_scenario(native_string_view path) {
	native_string selected_scenario_file;
	uint32_t max_scenario_count = 0;
	auto sdir = simple_fs::get_or_create_scenario_directory();
	for(auto& f : simple_fs::list_files(sdir, NATIVE(".bin"))) {
		if(auto of = simple_fs::open_file(f); of) {
			auto content = view_contents(*of);
			auto desc = sys::extract_mod_information(reinterpret_cast<uint8_t const*>(content.data), content.file_size);
			reports::write_debug("Scenario: '" + text::native_to_utf8(desc.mod_path) + "',count=" + std::to_string(desc.count) + "\n");
			if(desc.mod_path == path && desc.count >= max_scenario_count) {
				selected_scenario_file = simple_fs::get_file_name(f);
				max_scenario_count = desc.count;
			}
		}
	}
	return selected_scenario_file;
}

bool scenario_modify_time_is_outdated(native_string path, uint64_t scenario_time) {
	simple_fs::file_system fs_root;
	simple_fs::add_root(fs_root, NATIVE("."));
	simple_fs::restore_state(fs_root, path);
	uint64_t max_time = 0;
	auto root = simple_fs::get_root(fs_root);
	auto scan_files = [&](auto const dir) {
		for(const auto& f : simple_fs::list_files(dir, NATIVE(".txt"))) {
			if(auto of = simple_fs::open_file(f); of) {
				auto file_time = simple_fs::get_write_time(*of);
				max_time = std::max(max_time, file_time);
				if(file_time > scenario_time) {
					reports::write_debug("File " + text::native_to_utf8(simple_fs::get_full_name(f)) + " changed " + std::to_string(max_time) + "\n");
				}
			}
		}
	};
	native_string scan_dirs[] = {
		NATIVE("assets"),
		NATIVE("battleplans"),
		NATIVE("common"),
		NATIVE("decisions"),
		NATIVE("events"),
		//NATIVE("gfx"), -- graphics
		NATIVE("history"),
		NATIVE("inventions"),
		//NATIVE("launcher"), -- not relevant to the game
		//NATIVE("localisation"), -- loaded in-game (not on scenario creation)
		NATIVE("map"),
		NATIVE("news"),
		NATIVE("poptypes"),
		NATIVE("script"),
		NATIVE("technologies"),
		NATIVE("tutorial"),
		NATIVE("units")
	};
	for(const auto& sc_dir : scan_dirs) {
		auto dir = simple_fs::open_directory(root, sc_dir);
		reports::write_debug("Checksum directory: " + text::native_to_utf8(sc_dir) + "\n");
		// history/provinces/africa/thing.txt
		scan_files(dir); //history
		for(const auto& s2 : simple_fs::list_subdirectories(dir)) {
			scan_files(s2); //provinces
			for(const auto& s3 : simple_fs::list_subdirectories(s2)) {
				scan_files(s3); //africa
			}
		}
	}
	// Check also against interface
	reports::write_debug("Checksum interface directory (*.gfx/*.gui)\n");
	auto interface_dir = simple_fs::open_directory(root, NATIVE("interface"));
	for(const auto& f : simple_fs::list_files(interface_dir, NATIVE(".gui"))) {
		if(auto of = simple_fs::open_file(f); of) {
			auto file_time = simple_fs::get_write_time(*of);
			max_time = std::max(max_time, file_time);
			if(file_time > scenario_time) {
				reports::write_debug("File " + text::native_to_utf8(simple_fs::get_full_name(f)) + " changed " + std::to_string(max_time) + "\n");
			}
		}
	}
	for(const auto& f : simple_fs::list_files(interface_dir, NATIVE(".gfx"))) {
		if(auto of = simple_fs::open_file(f); of) {
			auto file_time = simple_fs::get_write_time(*of);
			max_time = std::max(max_time, file_time);
			if(file_time > scenario_time) {
				reports::write_debug("File " + text::native_to_utf8(simple_fs::get_full_name(f)) + " changed " + std::to_string(max_time) + "\n");
			}
		}
	}
	return max_time > scenario_time;
}

int scenario_validate(simple_fs::file_system& fs_root, native_string path) {
	reports::write_debug("Validating scenario\n");
	//
	parsers::error_handler err("");
	simple_fs::restore_state(fs_root, path);
	native_string selected_scenario_file = make_scenario(fs_root, err, path);
	if(!err.accumulated_errors.empty() || !err.accumulated_warnings.empty()) {
		auto assembled_file = std::string("You can still play the mod, but it might be unstable\r\nThe following problems were encountered while creating the scenario:\r\n\r\nErrors:\r\n") + err.accumulated_errors + "\r\n\r\nWarnings:\r\n" + err.accumulated_warnings;
		auto pdir = simple_fs::get_or_create_settings_directory();
		simple_fs::write_file(pdir, NATIVE("scenario_errors.txt"), assembled_file.data(), uint32_t(assembled_file.length()));
		std::printf("%s", assembled_file.c_str());
		if(!err.accumulated_errors.empty()) {
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

/*	Main processing of command line arguments occur here
	@param argv Vector list of arguments, not including the first argument on the command line (the program)
	@return The error code, EXIT_SUCCESS if no error was generated.
*/
int process_command_line(std::vector<native_string>& argv) {
	uint32_t default_seed = uint32_t(std::random_device()());
	int32_t headless_speed = 6;
	int32_t headless_ticks = 0; //0 -- infinite, >1 run for n specified amount
	bool headless_repeat = false;
	bool headless = false;

	simple_fs::identify_global_system_properties(); // -- globals startup

	network::port_forwarder forwarding_apparatus;
	bool loaded_scenario = false;
	bool validate_only = false;
	bool scenario_autofind = true;

	std::vector<parsers::mod_file> mod_list;
	simple_fs::file_system fs_root;
	simple_fs::add_root(fs_root, NATIVE("."));

	native_string opt_fs_path;
	for(size_t i = 0; i < argv.size(); ++i) {
		if(native_string(argv[i]) == NATIVE("-host")) {
			forwarding_apparatus.start_forwarding();
			game_state.network_mode = sys::network_mode_type::host;
		} else if(native_string(argv[i]) == NATIVE("-join")) {
			game_state.network_mode = sys::network_mode_type::client;
			game_state.network_state.ip_address = "127.0.0.1";
			if(i + 1 < argv.size()) {
				game_state.network_state.ip_address = text::native_to_utf8(native_string(argv[i + 1]));
				i++;
			}
		} else if(native_string(argv[i]) == NATIVE("-name")) {
			if(i + 1 < argv.size()) {
				std::string nickname = text::native_to_utf8(native_string(argv[i + 1]));
				std::memcpy(game_state.network_state.nickname.data, nickname.c_str(), std::min<size_t>(nickname.length(), 8));
				i++;
			}
		} else if(native_string(argv[i]) == NATIVE("-password")) {
			if(i + 1 < argv.size()) {
				auto str = text::native_to_utf8(native_string(argv[i + 1]));
				std::memset(game_state.network_state.password, '\0', sizeof(game_state.network_state.password));
				std::memcpy(game_state.network_state.password, str.c_str(), std::min(sizeof(game_state.network_state.password), str.length()));
				i++;
			}
		} else if(native_string(argv[i]) == NATIVE("-v6")) {
			game_state.network_state.as_v6 = true;
		} else if(native_string(argv[i]) == NATIVE("-v4")) {
			game_state.network_state.as_v6 = false;
		} else if(native_string(argv[i]) == NATIVE("-headless")) {
			headless = true;
		} else if(native_string(argv[i]) == NATIVE("-repeat")) {
			headless_repeat = true;
		} else if(native_string(argv[i]) == NATIVE("-gamedir")) {
			if(i + 1 < argv.size()) {
				auto path = native_string(argv[i + 1]);
				simple_fs::set_steam_path(path);
			}
		} else if(native_string(argv[i]) == NATIVE("-mod")) {
			if(i + 1 < argv.size()) {
				auto root = get_root(fs_root);
				auto mod_dir = simple_fs::open_directory(root, NATIVE("mod"));
				parsers::error_handler err("");
				auto of = simple_fs::open_file(mod_dir, native_string(argv[i + 1]));
				if(of) {
					auto content = view_contents(*of);
					parsers::token_generator gen(content.data, content.data + content.file_size);
					mod_list.push_back(parsers::parse_mod_file(gen, err, parsers::mod_file_context{}));
				}
				i++;
			}
		} else if(native_string(argv[i]) == NATIVE("-scenario")) {
			if(i + 1 < argv.size()) {
				auto str = argv[i + 1];
				if(sys::try_read_scenario_and_save_file(game_state, argv[i + 1])) {
					game_state.fill_unsaved_data();
					loaded_scenario = true;
				} else {
					auto msg = std::string("Scenario file ") + text::native_to_utf8(argv[i + 1]) + " could not be read";
					window::emit_error_message(msg, true);
				}
			}
		} else if(native_string(argv[i]) == NATIVE("-autofind")) {
			scenario_autofind = true;
		} else if(native_string(argv[i]) == NATIVE("-speed")) {
			if(i + 1 < argv.size()) {
				auto str = text::native_to_utf8(native_string(argv[i + 1]));
				headless_speed = int32_t(std::atoi(str.c_str()));
				i++;
			}
		} else if(native_string(argv[i]) == NATIVE("-ticks")) {
			if(i + 1 < argv.size()) {
				auto str = text::native_to_utf8(native_string(argv[i + 1]));
				headless_ticks = int32_t(std::atoi(str.c_str()));
				i++;
			}
		} else if(native_string(argv[i]) == NATIVE("-seed")) {
			if(i + 1 < argv.size()) {
				auto str = text::native_to_utf8(native_string(argv[i + 1]));
				default_seed = int32_t(std::atoi(str.c_str()));
				i++;
			}
		} else if(native_string(argv[i]) == NATIVE("-path")) {
			if(i + 1 < argv.size()) {
				opt_fs_path = native_string(argv[i + 1]);
				i++;
			}
		} else if(native_string(argv[i]) == NATIVE("-validate")) {
			validate_only = true;
		} else if(native_string(argv[i]) == NATIVE("-daily-oos")) {
			game_state.cheat_data.daily_oos_check = true;
		} else if(native_string(argv[i]) == NATIVE("-ext-script-ui")) {
			game_state.cheat_data.extension_use_scripted_ui = true;
		} else if(native_string(argv[i]) == NATIVE("-vanilla-parser")) {
			game_state.cheat_data.extension_use_vanilla_parser = true;
		}
	}

	// if the optional fs path is defined use that one, otherwise infer from mod list
	auto path = opt_fs_path.empty() ? produce_mod_path(mod_list) : opt_fs_path;
	native_string selected_scenario_file = NATIVE("");
	//
	// Validator only mode
	if(validate_only) {
		return scenario_validate(fs_root, path);
	}
	//
	// No validator mode -- check if headless to even init the window
	if(!headless) {
		window::create_window(game_state, window::creation_parameters{ 1024, 780, window::window_state::maximized, game_state.user_settings.prefer_fullscreen });
	}
	//
	// Normal mode (default)
	if(!loaded_scenario) {
		reports::write_debug("Produced scenario path: " + text::native_to_utf8(path) + "\n");
		reports::write_debug("Optional scenario path: " + text::native_to_utf8(opt_fs_path) + "\n");
		if(scenario_autofind) { //find scenario
			selected_scenario_file = find_matching_scenario(path);
			// Automatically force creation of scenario when a file changes
			auto s_dir = simple_fs::get_or_create_scenario_directory();
			if(auto f = simple_fs::open_file(s_dir, selected_scenario_file); f) {
				auto scenario_time = simple_fs::get_write_time(*f);
				if(scenario_modify_time_is_outdated(path, scenario_time)) {
					reports::write_debug("Forcing re-creation of scenario due to file changes\n");
					loaded_scenario = false;
				} else {
					reports::write_debug("Using scenario " + text::native_to_utf8(selected_scenario_file) + "\n");
					if(sys::try_read_scenario_and_save_file(game_state, selected_scenario_file)) {
						game_state.fill_unsaved_data();
						loaded_scenario = true;
					}
				}
			}
		}
		if(!loaded_scenario) { //create scenario if we couldn't find it
			parsers::error_handler err("");
			simple_fs::restore_state(fs_root, path);
			selected_scenario_file = make_scenario(fs_root, err, path);
			if(!err.accumulated_errors.empty() || !err.accumulated_warnings.empty()) {
				auto assembled_file = std::string("You can still play the mod, but it might be unstable\r\nThe following problems were encountered while creating the scenario:\r\n\r\nErrors:\r\n") + err.accumulated_errors + "\r\n\r\nWarnings:\r\n" + err.accumulated_warnings;
				auto pdir = simple_fs::get_or_create_settings_directory();
				simple_fs::write_file(pdir, NATIVE("scenario_errors.txt"), assembled_file.data(), uint32_t(assembled_file.length()));
				std::printf("%s", assembled_file.c_str());
			}
			if(sys::try_read_scenario_and_save_file(game_state, selected_scenario_file)) {
				game_state.fill_unsaved_data();
				loaded_scenario = true;
			} else {
				auto msg = std::string("Scenario file ") + text::native_to_utf8(selected_scenario_file) + " could not be read";
				window::emit_error_message(msg, true);
			}
		}
	}

	// Likely overriden by network
	game_state.game_seed = default_seed;
	network::init(game_state);
	
	// UI won't be baked onto scenario whuen not using scripted UI, nor can it be used
	if(!game_state.cheat_data.extension_use_scripted_ui) {
		parsers::building_gfx_context context(game_state, game_state.ui_defs);
		parsers::error_handler err("");
		ui::load_text_gui_definitions(game_state, context, err);
		ui::load_terrain_gfx(game_state, context, err);
	}

	// scenario loading functions (would have to run these even when scenario is pre-built)
	game_state.load_user_settings();
	ui::populate_definitions_map(game_state);

	if(headless) {
		game_state.actual_game_speed = int32_t(headless_speed);
		game_state.ui_pause.store(false, std::memory_order::release);
		game_scene::switch_scene(game_state, game_scene::scene_id::in_game_basic);
		game_state.local_player_nation = dcon::nation_id{};
		if(headless_repeat) {
			std::thread update_thread([&]() { game_state.game_loop(); });
			while(!game_state.quit_signaled.load(std::memory_order::acquire)) {
				while(game_state.current_scene.game_in_progress) {
					std::this_thread::sleep_for(std::chrono::milliseconds(15));
				}
				//Reload savefile
				network::finish(game_state, true);
				game_state.actual_game_speed = 0;
				//
				if(sys::try_read_scenario_and_save_file(game_state, selected_scenario_file)) {
					game_state.fill_unsaved_data();
				} else {
					auto msg = std::string("Scenario file ") + text::native_to_utf8(selected_scenario_file) + " could not be read";
					window::emit_error_message(msg, true);
					return 0;
				}
				//
				network::init(game_state);
				game_scene::switch_scene(game_state, game_scene::scene_id::in_game_basic);
				game_state.actual_game_speed = int32_t(headless_speed);
			};
			update_thread.join();
		} else {
			if(headless_ticks == 0) { //indefinitely
				game_state.game_loop();
			} else { //fixed amount, for benchmarks likely
				for(int32_t i = 0; i < headless_ticks; ++i) {
					game_state.single_game_tick();
				}
			}
		}
	} else {
		std::thread update_thread([&]() { game_state.game_loop(); });
		// entire game runs during this line
		window::initialize_window(game_state);
		game_state.quit_signaled.store(true, std::memory_order_release);
		update_thread.join();
	}

	network::finish(game_state, true);
	return EXIT_SUCCESS;
}
