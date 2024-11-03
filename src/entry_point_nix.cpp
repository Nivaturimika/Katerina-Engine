#include "serialization.hpp"
#include "system_state.hpp"
#include "reports.hpp"

#include "entry_point.cpp"

int main(int argc, char **argv) {
	std::vector<native_string> cmd_list;
	for(int i = 1; i < argc; ++i) {
		cmd_list.emplace_back(argv[i]);
	}
	return process_command_line(cmd_list);
}
