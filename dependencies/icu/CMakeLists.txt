message(STATUS "Finding icu...")

if(WIN32)
	FetchContent_Declare(icu URL "https://github.com/unicode-org/icu/releases/download/release-76-1/icu4c-76_1-Win64-MSVC2022.zip")
	FetchContent_MakeAvailable(icu)
	add_library(dependency_icu INTERFACE)
	target_include_directories(dependency_icu SYSTEM INTERFACE "${icu_SOURCE_DIR}/include")
	target_link_libraries(dependency_icu INTERFACE
		"${icu_SOURCE_DIR}/lib64/icudt.lib"
		"${icu_SOURCE_DIR}/lib64/icuuc.lib"
		"${icu_SOURCE_DIR}/lib64/icuin.lib"
	)
else()
	find_package(ICU COMPONENTS uc dt in REQUIRED)
	add_library(dependency_icu INTERFACE)
	target_link_libraries(dependency_icu INTERFACE ICU::uc ICU::dt ICU::in)
endif()
