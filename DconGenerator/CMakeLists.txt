if(ARCHITECTURE STREQUAL "x64" OR ARCHITECTURE STREQUAL "x86")
	project (DconGenerator LANGUAGES CXX)
	set(CMAKE_CXX_STANDARD 20 CACHE STRING "The C++ standard to use")
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	add_executable(DconGenerator
		"DataContainerGenerator.cpp"
		"code_fragments.cpp"
		"object_member_fragments.cpp"
		"parsing.cpp"
		"query_fragments.cpp"
		"serialize_fragments.cpp"
	)
	export(TARGETS DconGenerator FILE "${CMAKE_CURRENT_BINARY_DIR}/DconGeneratorTargets.cmake")
else()
	set(IMPORT_PATH "IMPORTFILE-NOTFOUND" CACHE FILEPATH "Point it to the export file path from a native build")
	file(TO_CMAKE_PATH "${IMPORT_PATH}" IMPORT_PATH_CMAKE)
	include(${IMPORT_PATH_CMAKE}/DconGenerator/DconGeneratorTargets.cmake)
endif()
