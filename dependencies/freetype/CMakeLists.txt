message(STATUS "Fetching freetype ...")
set(FT_DISABLE_ZLIB ON)
set(FT_DISABLE_BZIP2 ON)
set(FT_DISABLE_PNG ON)
set(FT_DISABLE_BROTLI ON)
set(FT_DISABLE_HARFBUZZ ON)
set(SKIP_INSTALL_ALL ON)
FetchContent_Declare(
	freetype
	GIT_REPOSITORY https://github.com/freetype/freetype.git
	GIT_TAG 3f3e3de34ee3613d621b643c58a40b93148e0932
	GIT_SHALLOW VER-2-13-3
	GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(freetype)
if(WIN32)
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		set_target_properties(freetype PROPERTIES COMPILE_FLAGS "/MTd")
	else()
		set_target_properties(freetype PROPERTIES COMPILE_FLAGS "/MT")
	endif()
endif()
add_library(dependency_freetype INTERFACE)
target_include_directories(dependency_freetype SYSTEM INTERFACE "${freetype_SOURCE_DIR}/include")
target_link_libraries(dependency_freetype INTERFACE freetype)

message(STATUS "Fetching harfbuzz...")
set(HB_BUILD_UTILS OFF)
set(HB_BUILD_SUBSET OFF)
set(HB_HAVE_FREETYPE ON)
FetchContent_Declare(
	harfbuzz
	GIT_REPOSITORY https://github.com/harfbuzz/harfbuzz
	GIT_TAG 10.1.0
	GIT_SHALLOW 1
	GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(harfbuzz)
if(WIN32)
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		set_target_properties(harfbuzz PROPERTIES COMPILE_FLAGS "/MTd")
	else()
		set_target_properties(harfbuzz PROPERTIES COMPILE_FLAGS "/MT")
	endif()
endif()
add_library(dependency_harfbuzz INTERFACE)
target_include_directories(dependency_harfbuzz SYSTEM INTERFACE "${harfbuzz_SOURCE_DIR}/include")
target_link_libraries(dependency_harfbuzz INTERFACE harfbuzz)
