#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/catch_session.hpp>

#ifndef DCON_TRAP_INVALID_STORE
#define DCON_TRAP_INVALID_STORE 1
#endif

#define ALICE_NO_ENTRY_POINT 1
#include "common_types.cpp"

std::unique_ptr<sys::state> load_testing_scenario_file() {
	std::unique_ptr<sys::state> game_state = std::make_unique<sys::state>(); // too big for the stack

	add_root(game_state->common_fs, NATIVE("."));        // for the moment this lets us find the shader files

	if (!sys::try_read_scenario_and_save_file(*game_state, NATIVE("tests_scenario.bin"))) {
		// scenario making functions
		parsers::error_handler err("");
		game_state->load_scenario_data(err, sys::year_month_day{ 1836, 1, 1 });
		sys::write_scenario_file(*game_state, NATIVE("tests_scenario.bin"), 1);
	} else {
		game_state->fill_unsaved_data();
	}

	return game_state;
}

TEST_CASE("Dummy test", "[dummy test instance]") {
	REQUIRE(1 + 1 == 2);
}


int main( int argc, char* argv[] ) {
	return Catch::Session().run( argc, argv );
}
