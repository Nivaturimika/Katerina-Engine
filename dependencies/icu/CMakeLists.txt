message(STATUS "Finding icu...")

if(WIN32)
	FetchContent_Declare(icu URL "https://github.com/unicode-org/icu/releases/download/release-76-1/icu4c-76_1-Win64-MSVC2022.zip")
	FetchContent_MakeAvailable(icu)
	# icudt
	add_library(icudt SHARED IMPORTED)
	set_target_properties(icudt PROPERTIES
		IMPORTED_LOCATION "${icu_SOURCE_DIR}/bin64"
		IMPORTED_IMPLIB "${icu_SOURCE_DIR}/lib64")
	# icuuc
	add_library(icuuc SHARED IMPORTED)
	set_target_properties(icuuc PROPERTIES
		IMPORTED_LOCATION "${icu_SOURCE_DIR}/bin64"
		IMPORTED_IMPLIB "${icu_SOURCE_DIR}/lib64")
	# icuin
	add_library(icuin SHARED IMPORTED)
	set_target_properties(icuin PROPERTIES
		IMPORTED_LOCATION "${icu_SOURCE_DIR}/bin64"
		IMPORTED_IMPLIB "${icu_SOURCE_DIR}/lib64")
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
