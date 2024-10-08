# Copy everything from the assets directory into the CMAKE_CURRENT_BINARY_DIR/assets directory
file(GLOB_RECURSE TESTING ${PROJECT_SOURCE_DIR}/assets/*)
foreach(file ${TESTING})
	string(LENGTH "${PROJECT_SOURCE_DIR}/" SOURCE_DIR_LENGTH)
	string(SUBSTRING ${file} ${SOURCE_DIR_LENGTH} -1 file)
	list(APPEND LAUNCHER_ASSET_FILES "${CMAKE_CURRENT_BINARY_DIR}/${file}")

	add_custom_command(
			OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file}
			DEPENDS ${PROJECT_SOURCE_DIR}/${file}
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
					${PROJECT_SOURCE_DIR}/${file}
					${CMAKE_CURRENT_BINARY_DIR}/${file})
endforeach()

add_executable(KatEngine_Launcher WIN32
	"${PROJECT_SOURCE_DIR}/src/launcher/launcher_main.cpp"
	"${PROJECT_SOURCE_DIR}/src/map/map_state.cpp"
	"${PROJECT_SOURCE_DIR}/src/map/map_data_loading.cpp"
	"${PROJECT_SOURCE_DIR}/src/map/map_borders.cpp"
	"${PROJECT_SOURCE_DIR}/src/map/map.cpp"
	"${PROJECT_SOURCE_DIR}/src/graphics/xac.cpp"
	"${PROJECT_SOURCE_DIR}/src/graphics/glad.cpp"
	"${PROJECT_SOURCE_DIR}/src/alice.rc"
	${LAUNCHER_ASSET_FILES})

target_compile_definitions(KatEngine_Launcher PUBLIC ALICE_NO_ENTRY_POINT)

target_link_libraries(KatEngine_Launcher PRIVATE KatEngineCommon)
target_link_libraries(KatEngine_Launcher PRIVATE dbghelp)
target_link_libraries(KatEngine_Launcher INTERFACE libglew_static)
target_link_libraries(KatEngine_Launcher INTERFACE stb_image)
target_link_libraries(KatEngine_Launcher INTERFACE freetype)
target_link_libraries(KatEngine_Launcher INTERFACE harfbuzz)

target_precompile_headers(KatEngine_Launcher REUSE_FROM KatEngine)
