message(STATUS "Finding icu...")

if(WIN32)
	if(ARCHITECTURE STREQUAL "x64")
		FetchContent_Declare(icu
			URL "https://github.com/unicode-org/icu/releases/download/release-75-1/icu4c-75_1-Win64-MSVC2022.zip")
		FetchContent_MakeAvailable(icu)
		# icudt
		add_library(icudt SHARED IMPORTED)
		set_target_properties(icudt PROPERTIES
			IMPORTED_LOCATION "${icu_SOURCE_DIR}/bin64/icudt75.dll"
			IMPORTED_IMPLIB "${icu_SOURCE_DIR}/lib64/icudt.lib")
		# icuuc
		add_library(icuuc SHARED IMPORTED)
		set_target_properties(icuuc PROPERTIES
			IMPORTED_LOCATION "${icu_SOURCE_DIR}/bin64/icuuc75.dll"
			IMPORTED_IMPLIB "${icu_SOURCE_DIR}/lib64/icuuc.lib")
		# icuin
		add_library(icuin SHARED IMPORTED)
		set_target_properties(icuin PROPERTIES
			IMPORTED_LOCATION "${icu_SOURCE_DIR}/bin64/icuin75.dll"
			IMPORTED_IMPLIB "${icu_SOURCE_DIR}/lib64/icuin.lib")
	else()
		FetchContent_Declare(icu
			URL "https://github.com/unicode-org/icu/releases/download/release-75-1/icu4c-75_1-Win32-MSVC2022.zip")
		FetchContent_MakeAvailable(icu)
		# icudt
		add_library(icudt SHARED IMPORTED)
		set_target_properties(icudt PROPERTIES
			IMPORTED_LOCATION "${icu_SOURCE_DIR}/bin/icudt75.dll"
			IMPORTED_IMPLIB "${icu_SOURCE_DIR}/lib/icudt.lib")
		# icuuc
		add_library(icuuc SHARED IMPORTED)
		set_target_properties(icuuc PROPERTIES
			IMPORTED_LOCATION "${icu_SOURCE_DIR}/bin/icuuc75.dll"
			IMPORTED_IMPLIB "${icu_SOURCE_DIR}/lib/icuuc.lib")
		# icuin
		add_library(icuin SHARED IMPORTED)
		set_target_properties(icuin PROPERTIES
			IMPORTED_LOCATION "${icu_SOURCE_DIR}/bin/icuin75.dll"
			IMPORTED_IMPLIB "${icu_SOURCE_DIR}/lib/icuin.lib")
	endif()
	#
	# interface target
	add_library(dependency_icu INTERFACE)
	target_include_directories(dependency_icu SYSTEM INTERFACE "${icu_SOURCE_DIR}/include")
	target_link_libraries(dependency_icu INTERFACE icudt icuuc icuin)
else()
	find_package(ICU COMPONENTS uc dt in REQUIRED)
	add_library(dependency_icu INTERFACE)
	target_link_libraries(dependency_icu INTERFACE ICU::uc ICU::dt ICU::in)
endif()
