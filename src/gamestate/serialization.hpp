#pragma once
#include <vector>
#include "container_types.hpp"
#include "unordered_dense.h"
#include "text.hpp"
#include "simple_fs.hpp"

namespace sys {
	constexpr inline uint32_t save_file_version = 79;
	constexpr inline uint32_t scenario_file_version = 131 + save_file_version;

	struct scenario_header {
		uint32_t version = scenario_file_version;
		uint32_t count = 0;
		uint64_t timestamp = 0;
		checksum_key checksum;
	};

	struct save_header {
		uint32_t version = save_file_version;
		uint32_t count = 0;
		uint64_t timestamp = 0;
		checksum_key checksum;
		dcon::national_identity_id tag;
		dcon::government_type_id cgov;
		sys::date d;
		char save_name[32] = { 0 };
	};

	struct mod_identifier {
		native_string mod_path;
		uint64_t timestamp = 0;
		uint32_t count = 0;
	};

	void read_mod_path(uint8_t const* ptr_in, uint8_t const* lim, native_string& path_out);
	uint8_t const* load_mod_path(uint8_t const* ptr_in, sys::state& state);
	uint8_t* write_mod_path(uint8_t* ptr_in, native_string const& path_in);
	size_t sizeof_mod_path(native_string const& path_in);

	uint8_t const* read_scenario_header(uint8_t const* ptr_in, scenario_header& header_out);
	uint8_t const* read_save_header(uint8_t const* ptr_in, save_header& header_out);
	uint8_t* write_scenario_header(uint8_t* ptr_in, scenario_header const& header_in);
	uint8_t* write_save_header(uint8_t* ptr_in, save_header const& header_in);
	size_t sizeof_scenario_header(scenario_header const& header_in);
	size_t sizeof_save_header(save_header const& header_in);

	mod_identifier extract_mod_information(uint8_t const* ptr_in, uint64_t file_size);

	uint8_t* write_compressed_section(uint8_t* ptr_out, uint8_t const* ptr_in, uint32_t uncompressed_size);

	// Note: these functions are for read / writing the *uncompressed* data
	uint8_t const* read_scenario_section(uint8_t const* ptr_in, uint8_t const* section_end, sys::state& state);
	uint8_t const* read_save_section(uint8_t const* ptr_in, uint8_t const* section_end, sys::state& state);
	uint8_t* write_scenario_section(uint8_t* ptr_in, sys::state& state);
	uint8_t* write_save_section(uint8_t* ptr_in, sys::state& state);
	struct scenario_size {
		uint64_t total_size;
		uint64_t checksum_offset;
	};
	scenario_size sizeof_scenario_section(sys::state& state);
	size_t sizeof_save_section(sys::state& state);

	void write_scenario_file(sys::state& state, native_string_view name, uint32_t count);
	bool try_read_scenario_file(sys::state& state, native_string_view name);
	bool try_read_scenario_and_save_file(sys::state& state, native_string_view name);
	bool try_read_scenario_as_save_file(sys::state& state, native_string_view name);

	void write_save_file(sys::state& state, sys::save_type type = sys::save_type::normal, std::string const& name = std::string(""));
	bool try_read_save_file(sys::state& state, native_string_view name);

} // namespace sys
