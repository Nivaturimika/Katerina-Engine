#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/catch_session.hpp>

#ifndef DCON_TRAP_INVALID_STORE
#define DCON_TRAP_INVALID_STORE 1
#endif

#include "system_state.hpp"
#include "serialization.hpp"

static sys::state game_state;

sys::state& load_testing_scenario_file() {
	//add_root(game_state.common_fs, NATIVE(".")); // for the moment this lets us find the shader files
	if(sys::try_read_scenario_and_save_file(game_state, NATIVE("tests_scenario.bin"))) {
		game_state.fill_unsaved_data();
	} else {
		std::abort();
	}
	return game_state;
}

TEST_CASE("Dummy test", "[dummy test instance]") {
	REQUIRE(1 + 1 == 2);
}


int main( int argc, char* argv[] ) {
	return Catch::Session().run( argc, argv );
}
