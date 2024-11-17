message(STATUS "Fetching icu...")

if(WIN32)
	add_library(dependency_icu INTERFACE)
	target_include_directories(dependency_icu SYSTEM INTERFACE .)
	target_link_directories(dependency_icu INTERFACE .)
	target_link_libraries(dependency_icu INTERFACE icuin icuuc)
elseif(NOT(ARCHITECTURE STREQUAL "x64" OR ARCHITECTURE STREQUAL "x86"))
	message(STATUS "Using icu-cmake for cross compiled ICU")

	set(BUILD_ICU ON)
	set(ICU_BUILD_VERSION "70.1")
	set(ICU_CROSS_ARCH "aarch64-linux-gnu")
	set(CMAKE_C_FLAGS "-lstdc++")
	set(CMAKE_CXX_FLAGS "-lstdc++")
	#set(CMAKE_CXX_FLAGS "-lstdc++")
	FetchContent_Declare(icu
		GIT_REPOSITORY "https://github.com/viaduck/icu-cmake"
		GIT_TAG 9701bcac520c7596ce51829347f4b522ae8346e5
		GIT_SHALLOW 1
		GIT_PROGRESS TRUE
	)
	FetchContent_MakeAvailable(icu)

	add_library(dependency_icu INTERFACE)
	target_link_libraries(dependency_icu INTERFACE icu)
else()
	find_package(ICU 70.1 COMPONENTS uc i18n REQUIRED)
	add_library(dependency_icu INTERFACE)
	target_link_libraries(dependency_icu INTERFACE ICU::uc ICU::i18n)
endif()
