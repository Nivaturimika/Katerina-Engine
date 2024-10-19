project (ParserGenerator LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20 CACHE STRING "The C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(ParserGenerator "parser_generator.cpp")
