add_executable(save_editor
	${ALICE_CORE_SOURCES_LIST}
	"${PROJECT_SOURCE_DIR}/SaveEditor/save_editor_main.cpp")

target_link_libraries(save_editor PRIVATE KatEngineCommon)

add_dependencies(save_editor GENERATE_PARSERS)
add_dependencies(save_editor GENERATE_CONTAINER ParserGenerator)

target_precompile_headers(save_editor REUSE_FROM KatEngine)
