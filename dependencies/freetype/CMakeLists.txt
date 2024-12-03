message(STATUS "Fetching freetype ...")
set(FT_DISABLE_ZLIB ON)
set(FT_DISABLE_BZIP2 ON)
set(FT_DISABLE_PNG ON)
set(FT_DISABLE_BROTLI ON)
if(ARCHITECTURE STREQUAL "x64" OR ARCHITECTURE STREQUAL "x86")
	set(FT_DISABLE_HARFBUZZ OFF)
else()
	set(FT_DISABLE_HARFBUZZ ON)
endif()
set(SKIP_INSTALL_ALL ON)
FetchContent_Declare(
	freetype
	GIT_REPOSITORY https://github.com/freetype/freetype.git
	GIT_TAG VER-2-13-3
	GIT_SHALLOW 1
	GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(freetype)
if(WIN32)
	set(CMAKE_CXX_FLAGS "")
	set(CMAKE_CXX_FLAGS_DEBUG "")
	set(CMAKE_CXX_FLAGS_RELEASE "")
	target_compile_options(freetype INTERFACE /bigobj /wd4100 /wd4189 /wd4065 /wd4201 /wd4324 /wd4723 /GR- /W4 /permissive- /Zc:preprocessor /GF /w34388 /w34389)
	target_compile_options(freetype INTERFACE /Z7
		$<$<CONFIG:Debug>:			/EHsc /MTd /O1>
		$<$<NOT:$<CONFIG:Debug>>: 	/DNDEBUG /wd4530 /MT /O2 /Oi /GL /sdl- /GS- /Gy /Gw /Zc:inline>)
	target_link_options(freetype INTERFACE
		$<$<CONFIG:Debug>: 			/DEBUG:FULL>
		$<$<NOT:$<CONFIG:Debug>>: 	/DEBUG:FULL /OPT:REF /OPT:ICF /LTCG>)
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
	set(CMAKE_CXX_FLAGS "")
	set(CMAKE_CXX_FLAGS_DEBUG "")
	set(CMAKE_CXX_FLAGS_RELEASE "")
	target_compile_options(harfbuzz INTERFACE /bigobj /wd4100 /wd4189 /wd4065 /wd4201 /wd4324 /wd4723 /GR- /W4 /permissive- /Zc:preprocessor /GF /w34388 /w34389)
	target_compile_options(harfbuzz INTERFACE /Z7
		$<$<CONFIG:Debug>:			/EHsc /MTd /O1>
		$<$<NOT:$<CONFIG:Debug>>: 	/DNDEBUG /wd4530 /MT /O2 /Oi /GL /sdl- /GS- /Gy /Gw /Zc:inline>)
	target_link_options(harfbuzz INTERFACE
		$<$<CONFIG:Debug>: 			/DEBUG:FULL>
		$<$<NOT:$<CONFIG:Debug>>: 	/DEBUG:FULL /OPT:REF /OPT:ICF /LTCG>)
endif()
add_library(dependency_harfbuzz INTERFACE)
target_include_directories(dependency_harfbuzz SYSTEM INTERFACE "${harfbuzz_SOURCE_DIR}/include")
target_link_libraries(dependency_harfbuzz INTERFACE harfbuzz)
