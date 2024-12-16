# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#

cmake_minimum_required (VERSION 3.15)

project (ProjectKatEngine LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20 CACHE STRING "The C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Turn on compile_commands.json export for wider editor/IDE support.
# This option is ignored on anything but Unix makefiles/Ninja.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)

option(ARCHITECTURE "Target architecture (valid values: x64, x86, aarch64, arm, riscv64)" "x64")
if(NOT(
	ARCHITECTURE STREQUAL "x64"
	OR ARCHITECTURE STREQUAL "x86"
	OR ARCHITECTURE STREQUAL "aarch64"
	OR ARCHITECTURE STREQUAL "riscv64"
	OR ARCHITECTURE STREQUAL "arm"
))
	message(FATAL_ERROR "Architecture is not valid, specify with -DARCHITECTURE=<arch>")
endif()

if(NOT(
	CMAKE_BUILD_TYPE STREQUAL "Debug"
	OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"
	OR CMAKE_BUILD_TYPE STREQUAL "Release"
))
	message(FATAL_ERROR "Build type is not valid, specify with -DCMAKE_BUILD_TYPE=<Debug, Release or RelWithDebInfo>")
endif()

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

list(APPEND ALICE_CORE_SOURCES_LIST
	"${PROJECT_SOURCE_DIR}/src/ai/ai.cpp"
	"${PROJECT_SOURCE_DIR}/src/common_types/common_types.cpp"
	"${PROJECT_SOURCE_DIR}/src/common_types/blake2.cpp"
	"${PROJECT_SOURCE_DIR}/src/common_types/date_interface.cpp"
	"${PROJECT_SOURCE_DIR}/src/common_types/prng.cpp"
	"${PROJECT_SOURCE_DIR}/src/common_types/reports.cpp"
	"${PROJECT_SOURCE_DIR}/src/culture/culture.cpp"
	"${PROJECT_SOURCE_DIR}/src/culture/politics.cpp"
	"${PROJECT_SOURCE_DIR}/src/culture/rebels.cpp"
	"${PROJECT_SOURCE_DIR}/src/economy/demographics.cpp"
	"${PROJECT_SOURCE_DIR}/src/economy/economy.cpp"
	"${PROJECT_SOURCE_DIR}/src/economy/economy_estimations.cpp"
 	"${PROJECT_SOURCE_DIR}/src/economy/economy_factory.cpp"
	"${PROJECT_SOURCE_DIR}/src/economy/economy_rgo.cpp"
	"${PROJECT_SOURCE_DIR}/src/gamestate/cheats.cpp"
	"${PROJECT_SOURCE_DIR}/src/gamestate/commands.cpp"
	"${PROJECT_SOURCE_DIR}/src/gamestate/diplomatic_messages.cpp"
	"${PROJECT_SOURCE_DIR}/src/gamestate/modifiers.cpp"
	"${PROJECT_SOURCE_DIR}/src/gamestate/notifications.cpp"
	"${PROJECT_SOURCE_DIR}/src/gamestate/serialization.cpp"
	"${PROJECT_SOURCE_DIR}/src/gamestate/system_state.cpp"
	"${PROJECT_SOURCE_DIR}/src/graphics/opengl_wrapper.cpp"
	"${PROJECT_SOURCE_DIR}/src/graphics/texture.cpp"
	"${PROJECT_SOURCE_DIR}/src/graphics/xac.cpp"
	"${PROJECT_SOURCE_DIR}/src/graphics/glad.cpp"
	"${PROJECT_SOURCE_DIR}/src/graphics/color.cpp"
	"${PROJECT_SOURCE_DIR}/src/graphics/stb_image.cpp"
	"${PROJECT_SOURCE_DIR}/src/graphics/stb_image_write.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/gui_state.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/gui_common_elements.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/gui_element_types.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/gui_main_menu.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/gui_console.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/gui_event.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/gui_diplomacy_request_topbar.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/gui_message_settings_window.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/gui_message_window.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/gui_nation_picker.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/gui_effect_tooltips.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/gui_graphics.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/gui_leader_select.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/gui_leader_tooltip.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/gui_modifier_tooltips.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/gui_province_window.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/gui_trigger_tooltips.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/gui_topbar.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/gui_error_window.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/map_tooltip.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/topbar_subwindows/gui_diplomacy_window.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/topbar_subwindows/gui_politics_window.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/topbar_subwindows/gui_population_window.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/topbar_subwindows/gui_production_window.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/topbar_subwindows/gui_technology_window.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/topbar_subwindows/gui_budget_window.cpp"
	"${PROJECT_SOURCE_DIR}/src/gui/unit_tooltip.cpp"
	"${PROJECT_SOURCE_DIR}/src/map/map_modes.cpp"
	"${PROJECT_SOURCE_DIR}/src/map/map_state.cpp"
	"${PROJECT_SOURCE_DIR}/src/map/map_data_loading.cpp"
	"${PROJECT_SOURCE_DIR}/src/map/map_borders.cpp"
	"${PROJECT_SOURCE_DIR}/src/map/map.cpp"
	"${PROJECT_SOURCE_DIR}/src/military/military.cpp"
	"${PROJECT_SOURCE_DIR}/src/military/military_cb.cpp"
	"${PROJECT_SOURCE_DIR}/src/military/military_units.cpp"
	"${PROJECT_SOURCE_DIR}/src/military/military_wars.cpp"
	"${PROJECT_SOURCE_DIR}/src/nations/nations.cpp"
	"${PROJECT_SOURCE_DIR}/src/network/network.cpp"
	"${PROJECT_SOURCE_DIR}/src/parsing/float_from_chars.cpp"
	"${PROJECT_SOURCE_DIR}/src/parsing/parsers.cpp"
	"${PROJECT_SOURCE_DIR}/src/platform_specific.cpp"
	"${PROJECT_SOURCE_DIR}/src/provinces/province.cpp"
	"${PROJECT_SOURCE_DIR}/src/scripting/effects.cpp"
	"${PROJECT_SOURCE_DIR}/src/scripting/events.cpp"
	"${PROJECT_SOURCE_DIR}/src/scripting/triggers.cpp"
	"${PROJECT_SOURCE_DIR}/src/text/bmfont.cpp"
	"${PROJECT_SOURCE_DIR}/src/text/fonts.cpp"
	"${PROJECT_SOURCE_DIR}/src/text/text.cpp"
	"${PROJECT_SOURCE_DIR}/src/zstd/zstd.cpp"
	"${PROJECT_SOURCE_DIR}/src/news/news.cpp"
	"${PROJECT_SOURCE_DIR}/src/network/pcp.cpp"
	"${PROJECT_SOURCE_DIR}/src/gamestate/game_scene.cpp"
)
if(WIN32)
	list(APPEND ALICE_CORE_SOURCES_LIST
		"${PROJECT_SOURCE_DIR}/src/filesystem/simple_fs_win.cpp"
		"${PROJECT_SOURCE_DIR}/src/window/window_win.cpp"
		"${PROJECT_SOURCE_DIR}/src/sound/sound_win.cpp"
		"${PROJECT_SOURCE_DIR}/src/graphics/opengl_wrapper_win.cpp"
	)
else()
	list(APPEND ALICE_CORE_SOURCES_LIST
		"${PROJECT_SOURCE_DIR}/src/filesystem/simple_fs_nix.cpp"
		"${PROJECT_SOURCE_DIR}/src/window/window_nix.cpp"
		"${PROJECT_SOURCE_DIR}/src/sound/sound_nix.cpp"
		"${PROJECT_SOURCE_DIR}/src/graphics/opengl_wrapper_nix.cpp"
	)
endif()

if(WIN32)
	add_executable(KatEngine WIN32
		${ALICE_CORE_SOURCES_LIST}
		"${PROJECT_SOURCE_DIR}/src/entry_point_win.cpp"
		"${PROJECT_SOURCE_DIR}/src/alice.rc"
		${ASSET_FILES})
else()
	add_executable(KatEngine WIN32
		${ALICE_CORE_SOURCES_LIST}
		"${PROJECT_SOURCE_DIR}/src/entry_point_nix.cpp"
		${ASSET_FILES})
endif()

# Include sub-projects.
add_subdirectory(dependencies)
add_subdirectory(ParserGenerator)
add_subdirectory(DconGenerator)

add_library(KatEngineCommon INTERFACE)
target_compile_definitions(KatEngineCommon INTERFACE "PROJECT_ROOT=\"${PROJECT_SOURCE_DIR}\"")
if(ARCHITECTURE STREQUAL "x64")
	target_compile_definitions(KatEngineCommon INTERFACE __x86_64__)
elseif(ARCHITECTURE STREQUAL "x86")
	target_compile_definitions(KatEngineCommon INTERFACE __i686__)
elseif(ARCHITECTURE STREQUAL "aarch64")
	target_compile_definitions(KatEngineCommon INTERFACE __aarch64__)
elseif(ARCHITECTURE STREQUAL "riscv64")
	target_compile_definitions(KatEngineCommon INTERFACE __riscv64__)
endif()
if(WIN32)
	# string(REPLACE "/GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	# string(REPLACE "/W3" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	# string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	# string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	# string(REPLACE "/MDd" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	# string(REPLACE "/MD" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
	# string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})

	# To build Non-AVX version : change all "/arch:AVX2" to "/d2archSSE42"
	# -march=nehalem for clang release
	set(CMAKE_CXX_FLAGS "")
	set(CMAKE_CXX_FLAGS_DEBUG "")
	set(CMAKE_CXX_FLAGS_RELEASE "")
	#set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		target_compile_options(KatEngineCommon INTERFACE /bigobj /wd4100 /wd4189 /wd4065 /wd4201 /wd4324 /GR- /W4 /permissive- /GF /w34388 /w34389 -Wno-unused-macros -Wno-zero-length-array -Wno-switch-default -Wno-reserved-identifier -Wno-unused-parameter -Wno-unused-variable -Wno-unused-private-field /Z7 -Wno-invalid-offsetof -Wno-deprecated-volatile -Wno-missing-prototypes -Wno-reserved-identifier -Wno-implicit-int-float-conversion -Wno-unsafe-buffer-usage -Wno-float-equal -Wno-shadow-field-in-constructor -Wno-gnu-anonymous-struct -Wno-nested-anon-types -Wno-double-promotion -Wno-shadow-uncaptured-local -Wno-nonportable-system-include-path -Wno-format-nonliteral -Wno-shorten-64-to-32 -Wno-ctad-maybe-unsupported -Wno-implicit-int-conversion -Wno-disabled-macro-expansion -Wno-comma -Wno-cast-function-type-strict -Wno-cast-function-type)
		if(ARCHITECTURE STREQUAL "x64")
			target_compile_options(KatEngineCommon INTERFACE /d2archSSE42)
		else()
			target_compile_options(KatEngineCommon INTERFACE /D__SSE3__)
		endif()
		if(OPTIMIZE_MODE STREQUAL "On")
			message(STATUS "Optimizing with PGO data")
			target_compile_options(KatEngineCommon INTERFACE /DNDEBUG /wd4530 /MT /O2 /Oi /sdl- /GS- /Gy /Gw /Zc:inline -Wno-profile-instr-missing -Wno-profile-instr-out-of-date -fprofile-instr-use=code.profdata)
			target_link_options(KatEngineCommon INTERFACE /OPT:REF /OPT:ICF /LTCG -fprofile-instr-use=code.profdata)
		elseif(PROFILE_MODE STREQUAL "On")
			message(STATUS "Compiling for PGO instrumentation")
			target_compile_options(KatEngineCommon INTERFACE /DNDEBUG /wd4530 /MT /O2 /Oi /sdl- /GS- /Gy /Gw /Zc:inline -fprofile-instr-generate)
			target_link_options(KatEngineCommon INTERFACE /DEBUG:FULL /OPT:REF /OPT:ICF /LTCG -fprofile-instr-generate)
		else()
			target_compile_options(KatEngineCommon INTERFACE
				$<$<CONFIG:Debug>:			/RTC1 /EHsc /MTd /Od>
				$<$<NOT:$<CONFIG:Debug>>: 	/DNDEBUG /wd4530 /MT /O2 /Oi /sdl- /GS- /Gy /Gw /Zc:inline>)
			target_link_options(KatEngineCommon INTERFACE
				$<$<CONFIG:Debug>: 			/DEBUG:FULL>
				$<$<NOT:$<CONFIG:Debug>>: 	/OPT:REF /OPT:ICF /LTCG>)
		endif()
	else()
		set_target_properties(KatEngineCommon PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR})
		target_compile_options(KatEngineCommon INTERFACE /bigobj /wd4100 /wd4189 /wd4065 /wd4201 /wd4324 /wd4723 /GR- /W4 /permissive- /Zc:preprocessor /GF /w34388 /w34389)
		if(ARCHITECTURE STREQUAL "x64")
			target_compile_options(KatEngineCommon INTERFACE /d2archSSE42)
		else()
			target_compile_options(KatEngineCommon INTERFACE /arch:SSE2 /D__SSE3__)
		endif()
		if(OPTIMIZE_MODE STREQUAL "On")
			message(STATUS "Optimizing with PGO data")
			target_compile_options(KatEngineCommon INTERFACE /DNDEBUG /wd4530 /MT /O2 /Oi /GL /sdl- /GS- /Gy /Gw /Zc:inline)
			target_link_options(KatEngineCommon INTERFACE /OPT:REF /OPT:ICF /LTCG /USEPROFILE)
		elseif(PROFILE_MODE STREQUAL "On")
			message(STATUS "Compiling for PGO instrumentation")
			target_compile_options(KatEngineCommon INTERFACE /Z7 /DNDEBUG /wd4530 /MT /O2 /Oi /GL /sdl- /GS- /Gy /Gw /Zc:inline)
			target_link_options(KatEngineCommon INTERFACE /DEBUG:FULL /OPT:REF /OPT:ICF /LTCG /GENPROFILE)
		else()
			message(STATUS "Normal, not PGO, build")
			# for faster debug builds, replace /RTC1 /Od with /O1 -- should be able to use /Ox, but for some reason that crashes ZSTD
			target_compile_options(KatEngineCommon INTERFACE /Z7
				$<$<CONFIG:Debug>:			/EHsc /MTd /O1>
				$<$<NOT:$<CONFIG:Debug>>: 	/DNDEBUG /wd4530 /MT /O2 /Oi /GL /sdl- /GS- /Gy /Gw /Zc:inline>)
			target_link_options(KatEngineCommon INTERFACE
				$<$<CONFIG:Debug>: 			/DEBUG:FULL>
				$<$<NOT:$<CONFIG:Debug>>: 	/DEBUG:FULL /OPT:REF /OPT:ICF /LTCG>)
			if(ARCHITECTURE STREQUAL "x86")
				target_link_options(KatEngineCommon INTERFACE /INCREMENTAL:NO /NODEFAULTLIB:MSVCRT)
			endif()
		endif()
	endif()
else() # GCC or CLANG
	# See for FMA support: https://stackoverflow.com/questions/21001388/fma3-in-gcc-how-to-enable
	target_compile_options(KatEngineCommon INTERFACE -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-variable -Wno-switch -Wno-unused-private-field)
	target_compile_options(KatEngineCommon INTERFACE -fpermissive)
	target_compile_options(KatEngineCommon INTERFACE $<$<CONFIG:Debug>: -O1 -g -pipe>)
	target_compile_options(KatEngineCommon INTERFACE $<$<NOT:$<CONFIG:Debug>>: -O3 -pipe -fno-rtti -DNDEBUG>)
	if(ARCHITECTURE STREQUAL "x64" OR ARCHITECTURE STREQUAL "x86")
		target_compile_options(KatEngineCommon INTERFACE -mfpmath=sse -msse4.2)
	endif()
endif()

target_link_libraries(KatEngineCommon INTERFACE dependency_freetype)
target_link_libraries(KatEngineCommon INTERFACE dependency_harfbuzz)
target_link_libraries(KatEngineCommon INTERFACE glm)
target_link_libraries(KatEngineCommon INTERFACE dependency_icu)
if (WIN32)
	target_link_libraries(KatEngineCommon INTERFACE ws2_32)
	target_link_libraries(KatEngineCommon INTERFACE wsock32)
else()
	if(ARCHITECTURE STREQUAL "x64" OR ARCHITECTURE STREQUAL "x86")
		target_link_libraries(KatEngineCommon INTERFACE dependency_tbb)
		add_compile_definitions(PREFER_ONE_TBB)
	else()
		message(STATUS "Non-x86 based architecture, disabling TBB")
		target_link_libraries(KatEngineCommon INTERFACE stdc++)
	endif()
	target_link_libraries(KatEngineCommon INTERFACE glfw)
	target_link_libraries(KatEngineCommon INTERFACE miniaudio)
endif()
target_include_directories(KatEngineCommon INTERFACE
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/src/ai
	${PROJECT_SOURCE_DIR}/src/common_types
	${PROJECT_SOURCE_DIR}/src/filesystem
	${PROJECT_SOURCE_DIR}/src/gamestate
	${PROJECT_SOURCE_DIR}/src/gui
	${PROJECT_SOURCE_DIR}/src/gui/topbar_subwindows
	${PROJECT_SOURCE_DIR}/src/gui/topbar_subwindows/diplomacy_subwindows
	${PROJECT_SOURCE_DIR}/src/gui/topbar_subwindows/production_subwindows
	${PROJECT_SOURCE_DIR}/src/gui/topbar_subwindows/politics_subwindows
	${PROJECT_SOURCE_DIR}/src/gui/topbar_subwindows/military_subwindows
	${PROJECT_SOURCE_DIR}/src/graphics
	${PROJECT_SOURCE_DIR}/src/parsing
	${PROJECT_SOURCE_DIR}/src/window
	${PROJECT_SOURCE_DIR}/src/text
	${PROJECT_SOURCE_DIR}/src/sound
	${PROJECT_SOURCE_DIR}/src/map
	${PROJECT_SOURCE_DIR}/src/network
	${PROJECT_SOURCE_DIR}/src/nations
	${PROJECT_SOURCE_DIR}/src/provinces
	${PROJECT_SOURCE_DIR}/src/economy
	${PROJECT_SOURCE_DIR}/src/culture
	${PROJECT_SOURCE_DIR}/src/military
	${PROJECT_SOURCE_DIR}/src/scripting
	${PROJECT_SOURCE_DIR}/src/zstd
	${PROJECT_SOURCE_DIR}/src/news
	"ankerl")

target_link_libraries(KatEngine PRIVATE KatEngineCommon)

# System headers
target_precompile_headers(KatEngine
	PRIVATE <stddef.h>
	PRIVATE <stdint.h>
	PRIVATE <assert.h>
	PRIVATE <stdarg.h>
	PRIVATE <stdlib.h>
	PRIVATE <math.h>
	PRIVATE <signal.h>
	PRIVATE <ctype.h>
)
# Standard C++ library headers
target_precompile_headers(KatEngine
	PRIVATE <vector>
	PRIVATE <array>
	PRIVATE <optional>
	PRIVATE <memory>
	PRIVATE <variant>
	PRIVATE <string>
	PRIVATE <string_view>
	PRIVATE <charconv>
	PRIVATE <algorithm>
	PRIVATE <iterator>
	PRIVATE <functional>
	PRIVATE <atomic>
	PRIVATE <chrono>
	PRIVATE <tuple>
	PRIVATE <type_traits>
	PRIVATE <new>
	PRIVATE <limits>
	PRIVATE <iterator>
)
# Aliases for C library
target_precompile_headers(KatEngine
	PRIVATE <cstdlib>
	PRIVATE <cstddef>
	PRIVATE <cstdint>
	PRIVATE <cassert>
	PRIVATE <cstdarg>
	PRIVATE <cmath>
)
# KatEngine provided headers
target_precompile_headers(KatEngine
	PRIVATE [["simple_fs.hpp"]]
	PRIVATE [["constants.hpp"]]
	PRIVATE [["unordered_dense.h"]]
	PRIVATE [["dcon_generated.hpp"]]
	PRIVATE [["container_types.hpp"]]
	PRIVATE [["parsers_declarations.hpp"]]
	PRIVATE [["cyto_any.hpp"]]
	PRIVATE [["date_interface.hpp"]]
	PRIVATE [["stb_image.h"]]
)
target_precompile_headers(KatEngine
	PRIVATE [["system_state.hpp"]]
)

if(WIN32)
else()
	target_precompile_headers(KatEngine
		PRIVATE [["miniaudio.h"]]
	)
endif()

# GENERATE CONTAINER
set(CONTAINER_PATH ${PROJECT_SOURCE_DIR}/src/gamestate/dcon_generated)

# The command to build the generated file
add_custom_command(
	OUTPUT ${CONTAINER_PATH}.hpp
	COMMAND DconGenerator ${CONTAINER_PATH}.txt
	DEPENDS ${CONTAINER_PATH}.txt
	VERBATIM)

if(NOT(CROSS_COMPILE))
	# Sets a dependency on the generated file
	add_custom_target(GENERATE_CONTAINER DEPENDS ${CONTAINER_PATH}.hpp)
	add_dependencies(KatEngine GENERATE_CONTAINER ParserGenerator)
endif()

# The command to build the generated parsers file
add_custom_command(
	OUTPUT ${PROJECT_SOURCE_DIR}/src/parsing/parser_defs_generated.hpp
	COMMAND ParserGenerator ${PROJECT_SOURCE_DIR}/src/parsing/parser_defs.txt
	DEPENDS ${PROJECT_SOURCE_DIR}/src/parsing/parser_defs.txt
	VERBATIM)

add_custom_command(
	OUTPUT ${PROJECT_SOURCE_DIR}/src/parsing/tutorial_parser_defs_generated.hpp
	COMMAND ParserGenerator ${PROJECT_SOURCE_DIR}/src/parsing/tutorial_parser_defs.txt
	DEPENDS ${PROJECT_SOURCE_DIR}/src/parsing/tutorial_parser_defs.txt
	VERBATIM)

add_custom_command(
	OUTPUT ${PROJECT_SOURCE_DIR}/src/parsing/gui_parser_defs_generated.hpp
	COMMAND ParserGenerator ${PROJECT_SOURCE_DIR}/src/parsing/gui_parser_defs.txt
	DEPENDS ${PROJECT_SOURCE_DIR}/src/parsing/gui_parser_defs.txt
	VERBATIM)

add_custom_command(
	OUTPUT ${PROJECT_SOURCE_DIR}/src/parsing/save_parser_defs_generated.hpp
	COMMAND ParserGenerator ${PROJECT_SOURCE_DIR}/src/parsing/save_parser_defs.txt
	DEPENDS ${PROJECT_SOURCE_DIR}/src/parsing/save_parser_defs.txt
	VERBATIM)

add_custom_command(
	OUTPUT ${PROJECT_SOURCE_DIR}/src/parsing/news_parser_defs_generated.hpp
	COMMAND ParserGenerator ${PROJECT_SOURCE_DIR}/src/parsing/news_parser_defs.txt
	DEPENDS ${PROJECT_SOURCE_DIR}/src/parsing/news_parser_defs.txt
	VERBATIM)

add_custom_command(
	OUTPUT ${PROJECT_SOURCE_DIR}/src/parsing/effect_parser_defs_generated.hpp
	COMMAND ParserGenerator ${PROJECT_SOURCE_DIR}/src/parsing/effect_parser_defs.txt
	DEPENDS ${PROJECT_SOURCE_DIR}/src/parsing/effect_parser_defs.txt
	VERBATIM)

add_custom_command(
	OUTPUT ${PROJECT_SOURCE_DIR}/src/parsing/trigger_parser_defs_generated.hpp
	COMMAND ParserGenerator ${PROJECT_SOURCE_DIR}/src/parsing/trigger_parser_defs.txt
	DEPENDS ${PROJECT_SOURCE_DIR}/src/parsing/trigger_parser_defs.txt
	VERBATIM)

add_custom_command(
	OUTPUT ${PROJECT_SOURCE_DIR}/src/parsing/game_rules_parser_defs_generated.hpp
	COMMAND ParserGenerator ${PROJECT_SOURCE_DIR}/src/parsing/game_rules_parser_defs.txt
	DEPENDS ${PROJECT_SOURCE_DIR}/src/parsing/game_rules_parser_defs.txt
	VERBATIM)

add_custom_command(
	OUTPUT ${PROJECT_SOURCE_DIR}/src/text/font_defs_generated.hpp
	COMMAND ParserGenerator ${PROJECT_SOURCE_DIR}/src/text/font_defs.txt
	DEPENDS ${PROJECT_SOURCE_DIR}/src/text/font_defs.txt
	VERBATIM)

add_custom_command(
	OUTPUT ${PROJECT_SOURCE_DIR}/src/text/bmfont_defs_generated.hpp
	COMMAND ParserGenerator ${PROJECT_SOURCE_DIR}/src/text/bmfont_defs.txt
	DEPENDS ${PROJECT_SOURCE_DIR}/src/text/bmfont_defs.txt
	VERBATIM)

# Sets a dependency on the generated file
add_custom_target(GENERATE_PARSERS DEPENDS
	${PROJECT_SOURCE_DIR}/src/parsing/parser_defs_generated.hpp
	${PROJECT_SOURCE_DIR}/src/parsing/save_parser_defs_generated.hpp
	${PROJECT_SOURCE_DIR}/src/parsing/tutorial_parser_defs_generated.hpp
	${PROJECT_SOURCE_DIR}/src/parsing/gui_parser_defs_generated.hpp
	${PROJECT_SOURCE_DIR}/src/parsing/news_parser_defs_generated.hpp
	${PROJECT_SOURCE_DIR}/src/parsing/effect_parser_defs_generated.hpp
	${PROJECT_SOURCE_DIR}/src/parsing/trigger_parser_defs_generated.hpp
	${PROJECT_SOURCE_DIR}/src/parsing/game_rules_parser_defs_generated.hpp
	${PROJECT_SOURCE_DIR}/src/text/bmfont_defs_generated.hpp
	${PROJECT_SOURCE_DIR}/src/text/font_defs_generated.hpp)
add_dependencies(KatEngine GENERATE_PARSERS)

if(ARCHITECTURE STREQUAL "x64" OR ARCHITECTURE STREQUAL "x86")
	add_subdirectory(SaveEditor)
	if(BUILD_TESTING)
		enable_testing()
		add_subdirectory(tests)
	endif()
	if(WIN32)
		add_subdirectory(DbgKatEngine)
		add_subdirectory(Launcher)
	endif()
endif()

# Installation
if(NOT WIN32)
    install(FILES ${PROJECT_SOURCE_DIR}/src/alice.desktop DESTINATION /usr/share/applications)
    install(FILES ${PROJECT_SOURCE_DIR}/src/alice.png DESTINATION /usr/share/icons/hicolor/128x128/apps)
endif()

