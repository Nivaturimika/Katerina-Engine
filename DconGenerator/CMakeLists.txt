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

