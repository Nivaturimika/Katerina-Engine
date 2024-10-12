#include "system_state.hpp"
#include "serialization.hpp"

#ifndef UNICODE
#define UNICODE
#endif
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <shellapi.h>
#include <shellscalingapi.h>
#include "Objbase.h"
#include "window.hpp"

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Shell32.lib")

#pragma comment(lib, "icudt.lib")
#pragma comment(lib, "icuin.lib")
#pragma comment(lib, "icuuc.lib")

native_string to_hex(uint64_t v) {
	native_string ret;
	constexpr native_char digits[] = NATIVE("0123456789ABCDEF");
	do {
		ret += digits[v & 0x0F];
		v = v >> 4;
	} while(v != 0);
	return ret;
}

native_string produce_mod_path(std::vector<parsers::mod_file>& mod_list) {
	simple_fs::file_system dummy;
	simple_fs::add_root(dummy, NATIVE("."));
	for(int32_t i = 0; i < int32_t(mod_list.size()); ++i) {
		mod_list[i].add_to_file_system(dummy);
	}
	return simple_fs::extract_state(dummy);
}

static sys::state game_state; // too big for the stack
static CRITICAL_SECTION guard_abort_handler;

void signal_abort_handler(int) {
	static bool run_once = false;

	EnterCriticalSection(&guard_abort_handler);
	if(run_once == false) {
		run_once = true;

		STARTUPINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		PROCESS_INFORMATION pi;
		ZeroMemory(&pi, sizeof(pi));
		// Start the child process. 
		if(CreateProcessW(
			L"dbg_alice.exe",   // Module name
			NULL, // Command line
			NULL, // Process handle not inheritable
			NULL, // Thread handle not inheritable
			FALSE, // Set handle inheritance to FALSE
			0, // No creation flags
			NULL, // Use parent's environment block
			NULL, // Use parent's starting directory 
			&si, // Pointer to STARTUPINFO structure
			&pi) == 0) {

			// create process failed
			LeaveCriticalSection(&guard_abort_handler);
			return;

		}
		// Wait until child process exits.
		WaitForSingleObject(pi.hProcess, INFINITE);
		// Close process and thread handles. 
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	LeaveCriticalSection(&guard_abort_handler);
}

LONG WINAPI uef_wrapper(struct _EXCEPTION_POINTERS* lpTopLevelExceptionFilter) {
	signal_abort_handler(0);
	return EXCEPTION_CONTINUE_SEARCH;
}

void generic_wrapper() {
	signal_abort_handler(0);
}
void invalid_parameter_wrapper(
   const wchar_t* expression,
   const wchar_t* function,
   const wchar_t* file,
   unsigned int line,
   uintptr_t pReserved
) {
	signal_abort_handler(0);
}

void EnableCrashingOnCrashes() {
	typedef BOOL(WINAPI* tGetPolicy)(LPDWORD lpFlags);
	typedef BOOL(WINAPI* tSetPolicy)(DWORD dwFlags);
	const DWORD EXCEPTION_SWALLOWING = 0x1;

	HMODULE kernel32 = LoadLibraryA("kernel32.dll");
	if(kernel32) {
		tGetPolicy pGetPolicy = (tGetPolicy)GetProcAddress(kernel32, "GetProcessUserModeExceptionPolicy");
		tSetPolicy pSetPolicy = (tSetPolicy)GetProcAddress(kernel32, "SetProcessUserModeExceptionPolicy");
		if(pGetPolicy && pSetPolicy) {
			DWORD dwFlags;
			if(pGetPolicy(&dwFlags)) {
				// Turn off the filter
				pSetPolicy(dwFlags & ~EXCEPTION_SWALLOWING);
			}
		}
	}
	BOOL insanity = FALSE;
	SetUserObjectInformationA(GetCurrentProcess(), UOI_TIMERPROC_EXCEPTION_SUPPRESSION, &insanity, sizeof(insanity));
}

native_string make_scenario(simple_fs::file_system& fs_root, parsers::error_handler& err, native_string_view path) {
	simple_fs::restore_state(fs_root, path);
	//
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
			int32_t append = 0;
			auto time_stamp = uint64_t(std::time(0));
			auto base_name = to_hex(time_stamp);
			while(simple_fs::peek_file(sdir, base_name + NATIVE("-") + std::to_wstring(append) + NATIVE(".bin"))) {
				++append;
			}
			selected_scenario_file = base_name + NATIVE("-") + std::to_wstring(append) + NATIVE(".bin");
			sys::write_scenario_file(*book_game_state, selected_scenario_file, 1);
			scenario_key = book_game_state->scenario_checksum;
		} else {
			//sys::write_scenario_file(*book_game_state, std::to_wstring(date_index) + NATIVE(".bin"), 0);
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
			reports::write_debug(("Scenario: '" + text::native_to_utf8(desc.mod_path) + "',count=" + std::to_string(desc.count)).c_str());
			if(desc.mod_path == path && desc.count >= max_scenario_count) {
				selected_scenario_file = simple_fs::get_file_name(f);
				max_scenario_count = desc.count;
			}
		}
	}
	return selected_scenario_file;
}

int WINAPI wWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPWSTR /*commandline*/, int /*nCmdShow*/
) {

	#ifdef _DEBUG
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
	#endif

	InitializeCriticalSection(&guard_abort_handler);

	if(!IsDebuggerPresent()) {
		EnableCrashingOnCrashes();
		_set_purecall_handler(generic_wrapper);
		_set_invalid_parameter_handler(invalid_parameter_wrapper);
		_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
		SetUnhandledExceptionFilter(uef_wrapper);
		signal(SIGABRT, signal_abort_handler);
	}

	// Workaround for old machines
	if(HINSTANCE hUser32dll = LoadLibrary(L"User32.dll"); hUser32dll) {
		auto pSetProcessDpiAwarenessContext = (decltype(&SetProcessDpiAwarenessContext))GetProcAddress(hUser32dll, "SetProcessDpiAwarenessContext");
		if(pSetProcessDpiAwarenessContext != NULL) {
			pSetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
		} else {
			// windows 8.1 (not present on windows 8 and only available on desktop apps)
			if(HINSTANCE hShcoredll = LoadLibrary(L"Shcore.dll"); hShcoredll) {
				auto pSetProcessDpiAwareness = (decltype(&SetProcessDpiAwareness))GetProcAddress(hShcoredll, "SetProcessDpiAwareness");
				if(pSetProcessDpiAwareness != NULL) {
					pSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
				} else {
					SetProcessDPIAware(); //vista+
				}
				FreeLibrary(hShcoredll);
			} else {
				SetProcessDPIAware(); //vista+
			}
		}
		FreeLibrary(hUser32dll);
	}

	if(SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED))) {
		// do everything here: create a window, read messages

		int num_params = 0;
		auto parsed_cmd = CommandLineToArgvW(GetCommandLineW(), &num_params);

		int headless_speed = 6;
		bool headless_repeat = false;
		bool headless = false;

		network::port_forwarder forwarding_apparatus;
		simple_fs::identify_global_system_properties(); // -- globals startup

		if(num_params < 2) {
			add_root(game_state.common_fs, NATIVE(".")); // for the moment this lets us find the shader files
			if(!sys::try_read_scenario_and_save_file(game_state, NATIVE("development_test_file.bin"))) {
				// scenario making functions
				parsers::error_handler err{ "" };
				simple_fs::file_system fs_root;
				simple_fs::add_root(fs_root, NATIVE("."));
				auto root = get_root(fs_root);
				auto common = open_directory(root, NATIVE("common"));
				parsers::bookmark_context bookmark_context;
				if(auto f = open_file(common, NATIVE("bookmarks.txt")); f) {
					auto bookmark_content = simple_fs::view_contents(*f);
					err.file_name = "bookmarks.txt";
					parsers::token_generator gen(bookmark_content.data, bookmark_content.data + bookmark_content.file_size);
					parsers::parse_bookmark_file(gen, err, bookmark_context);
					assert(!bookmark_context.bookmark_dates.empty());
				} else {
					err.accumulated_errors += "File common/bookmarks.txt could not be opened\n";
				}
				auto to_hex = [](uint64_t v) {
					native_string ret;
					constexpr native_char digits[] = NATIVE("0123456789ABCDEF");
					do {
						ret += digits[v & 0x0F];
						v = v >> 4;
					} while(v != 0);
					return ret;
				};
				sys::checksum_key scenario_key;
				for(uint32_t date_index = 0; date_index < uint32_t(bookmark_context.bookmark_dates.size()); date_index++) {
					err.accumulated_errors.clear();
					err.accumulated_warnings.clear();
					//
					auto inner_game_state = std::make_unique<sys::state>();
					simple_fs::add_root(inner_game_state->common_fs, NATIVE("."));
					inner_game_state->load_scenario_data(err, bookmark_context.bookmark_dates[date_index].date_);
					if(err.fatal)
						break;
					if(date_index == 0) {
						auto sdir = simple_fs::get_or_create_scenario_directory();
						int32_t append = 0;
						auto time_stamp = uint64_t(std::time(0));
						auto selected_scenario_file = native_string(NATIVE("development_test_file.bin"));
						sys::write_scenario_file(*inner_game_state, selected_scenario_file, 0);
						if(auto of = simple_fs::open_file(sdir, selected_scenario_file); of) {
							auto content = view_contents(*of);
							auto desc = sys::extract_mod_information(reinterpret_cast<uint8_t const*>(content.data), content.file_size);
						}
						scenario_key = inner_game_state->scenario_checksum;
					} else {
						inner_game_state->scenario_checksum = scenario_key;
						sys::write_save_file(*inner_game_state, sys::save_type::bookmark, bookmark_context.bookmark_dates[date_index].name_);
					}
				}
				if(!err.accumulated_errors.empty()) {
					auto assembled_file = std::string("You can still play the mod, but it might be unstable\r\nThe following problems were encountered while creating the scenario:\r\n\r\nErrors:\r\n") + err.accumulated_errors + "\r\n\r\nWarnings:\r\n" + err.accumulated_warnings;
					auto pdir = simple_fs::get_or_create_settings_directory();
					simple_fs::write_file(pdir, NATIVE("scenario_errors.txt"), assembled_file.data(), uint32_t(assembled_file.length()));
					auto fname = simple_fs::get_full_name(pdir) + NATIVE("\\scenario_errors.txt");
				}
				if(!sys::try_read_scenario_and_save_file(game_state, NATIVE("development_test_file.bin"))) {
					return 0;
				}
			}
			game_state.fill_unsaved_data();
		} else {
			bool loaded_scenario = false;
			bool validate_only = false;
			bool scenario_autofind = false;

			std::vector<parsers::mod_file> mod_list;
			simple_fs::file_system fs_root;
			simple_fs::add_root(fs_root, NATIVE("."));

			native_string opt_fs_path;
			for(int i = 1; i < num_params; ++i) {
				if(native_string(parsed_cmd[i]) == NATIVE("-host")) {
					forwarding_apparatus.start_forwarding();
					game_state.network_mode = sys::network_mode_type::host;
				} else if(native_string(parsed_cmd[i]) == NATIVE("-join")) {
					game_state.network_mode = sys::network_mode_type::client;
					game_state.network_state.ip_address = "127.0.0.1";
					if(i + 1 < num_params) {
						game_state.network_state.ip_address = text::native_to_utf8(native_string(parsed_cmd[i + 1]));
						i++;
					}
				} else if(native_string(parsed_cmd[i]) == NATIVE("-name")) {
					if(i + 1 < num_params) {
						std::string nickname = text::native_to_utf8(native_string(parsed_cmd[i + 1]));
						memcpy(game_state.network_state.nickname.data, nickname.data(), std::min<size_t>(nickname.length(), 8));
						i++;
					}
				} else if(native_string(parsed_cmd[i]) == NATIVE("-password")) {
					if(i + 1 < num_params) {
						auto str = text::native_to_utf8(native_string(parsed_cmd[i + 1]));
						std::memset(game_state.network_state.password, '\0', sizeof(game_state.network_state.password));
						std::memcpy(game_state.network_state.password, str.c_str(), std::min(sizeof(game_state.network_state.password), str.length()));
						i++;
					}
				} else if(native_string(parsed_cmd[i]) == NATIVE("-v6")) {
					game_state.network_state.as_v6 = true;
				} else if(native_string(parsed_cmd[i]) == NATIVE("-v4")) {
					game_state.network_state.as_v6 = false;
				} else if(native_string(parsed_cmd[i]) == NATIVE("-headless")) {
					headless = true;
				} else if(native_string(parsed_cmd[i]) == NATIVE("-repeat")) {
					headless_repeat = true;
				} else if(native_string(parsed_cmd[i]) == NATIVE("-gamedir")) {
					if(i + 1 < num_params) {
						auto path = native_string(parsed_cmd[i + 1]);
						simple_fs::set_steam_path(path);
					}
				} else if(native_string(parsed_cmd[i]) == NATIVE("-mod")) {
					if(i + 1 < num_params) {
						auto root = get_root(fs_root);
						auto mod_dir = simple_fs::open_directory(root, NATIVE("mod"));
						parsers::error_handler err("");
						auto of = simple_fs::open_file(mod_dir, native_string(parsed_cmd[i + 1]));
						if(of) {
							auto content = view_contents(*of);
							parsers::token_generator gen(content.data, content.data + content.file_size);
							mod_list.push_back(parsers::parse_mod_file(gen, err, parsers::mod_file_context{}));
						}
						i++;
					}
				} else if(native_string(parsed_cmd[i]) == NATIVE("-scenario")) {
					if(i + 1 < num_params) {
						auto str = parsed_cmd[i + 1];
						if(sys::try_read_scenario_and_save_file(game_state, parsed_cmd[i + 1])) {
							game_state.fill_unsaved_data();
							loaded_scenario = true;
						} else {
							auto msg = std::string("Scenario file ") + text::native_to_utf8(parsed_cmd[i + 1]) + " could not be read";
							window::emit_error_message(msg, true);
						}
					}
				} else if(native_string(parsed_cmd[i]) == NATIVE("-autofind")) {
					scenario_autofind = true;
				} else if(native_string(parsed_cmd[i]) == NATIVE("-speed")) {
					if(i + 1 < num_params) {
						auto str = text::native_to_utf8(native_string(parsed_cmd[i + 1]));
						headless_speed = std::atoi(str.c_str());
						i++;
					}
				} else if(native_string(parsed_cmd[i]) == NATIVE("-path")) {
					if(i + 1 < num_params) {
						opt_fs_path = native_string(parsed_cmd[i + 1]);
						i++;
					}
				} else if(native_string(parsed_cmd[i]) == NATIVE("-validate")) {
					validate_only = true;
				}
			}
			if(!loaded_scenario) {
				// if the optional fs path is defined use that one, otherwise infer from mod list
				auto path = opt_fs_path.empty() ? produce_mod_path(mod_list) : opt_fs_path;
				reports::write_debug(("Produced scenario path: " + text::native_to_utf8(path) + "\n").c_str());
				reports::write_debug(("Optional scenario path: " + text::native_to_utf8(opt_fs_path) + "\n").c_str());
				if(scenario_autofind) { //find scenario
					native_string selected_scenario_file = find_matching_scenario(path);
					reports::write_debug(("Using scenario " + text::native_to_utf8(selected_scenario_file) + "\n").c_str());
					if(sys::try_read_scenario_and_save_file(game_state, selected_scenario_file)) {
						game_state.fill_unsaved_data();
						loaded_scenario = true;
					}
				}
				if(!loaded_scenario) { //create scenario if we couldn't find it
					parsers::error_handler err("");
					native_string selected_scenario_file = make_scenario(fs_root, err, path);
					if(!err.accumulated_errors.empty() || !err.accumulated_warnings.empty()) {
						auto assembled_file = std::string("You can still play the mod, but it might be unstable\r\nThe following problems were encountered while creating the scenario:\r\n\r\nErrors:\r\n") + err.accumulated_errors + "\r\n\r\nWarnings:\r\n" + err.accumulated_warnings;
						auto pdir = simple_fs::get_or_create_settings_directory();
						simple_fs::write_file(pdir, NATIVE("scenario_errors.txt"), assembled_file.data(), uint32_t(assembled_file.length()));
						std::printf("%s", assembled_file.c_str());
						if(validate_only) {
							if(!err.accumulated_errors.empty()) {
								return EXIT_FAILURE;
							}
							return EXIT_SUCCESS;
						}
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
			network::init(game_state);
		}
		LocalFree(parsed_cmd);

		// scenario loading functions (would have to run these even when scenario is pre-built)
		game_state.load_user_settings();
		ui::populate_definitions_map(game_state);

		if(headless) {
			game_state.actual_game_speed = headless_speed;
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
					if(sys::try_read_scenario_and_save_file(game_state, parsed_cmd[1])) {
						game_state.fill_unsaved_data();
					} else {
						auto msg = std::string("Scenario file ") + text::native_to_utf8(parsed_cmd[1]) + " could not be read";
						window::emit_error_message(msg, true);
						return 0;
					}
					//
					network::init(game_state);
					game_scene::switch_scene(game_state, game_scene::scene_id::in_game_basic);
					game_state.actual_game_speed = headless_speed;
				};
				update_thread.join();
			} else {
				game_state.game_loop();
			}
		} else {
			std::thread update_thread([&]() { game_state.game_loop(); });
			// entire game runs during this line
			window::create_window(game_state, window::creation_parameters{ 1024, 780, window::window_state::maximized, game_state.user_settings.prefer_fullscreen });
			game_state.quit_signaled.store(true, std::memory_order_release);
			update_thread.join();
		}

		network::finish(game_state, true);
		CoUninitialize();
	}
	return 0;
}
