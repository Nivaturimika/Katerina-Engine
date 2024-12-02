add_executable(KatEngine_Launcher WIN32
	${ALICE_CORE_SOURCES_LIST}
	"${PROJECT_SOURCE_DIR}/src/launcher/launcher_main.cpp"
	"${PROJECT_SOURCE_DIR}/src/alice.rc")

target_compile_definitions(KatEngine_Launcher PUBLIC ALICE_NO_ENTRY_POINT)

target_link_libraries(KatEngine_Launcher PRIVATE KatEngineCommon)
target_link_libraries(KatEngine_Launcher PRIVATE dbghelp)
#target_precompile_headers(KatEngine_Launcher REUSE_FROM KatEngine)
