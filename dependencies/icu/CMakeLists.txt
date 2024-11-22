message(STATUS "Fetching icu...")

set(BUILD_ICU ON)
set(ICU_BUILD_VERSION "70.1")
if(CROSS_COMPILE)
	message(STATUS "Using icu-cmake for cross compiled ICU")
	set(ICU_CROSS_ARCH "aarch64-linux-gnu")
	set(CMAKE_C_FLAGS "-lstdc++")
	set(CMAKE_CXX_FLAGS "-lstdc++")
	#set(CMAKE_CXX_FLAGS "-lstdc++")
endif()

FetchContent_Declare(icu
	GIT_REPOSITORY "https://github.com/viaduck/icu-cmake"
	GIT_TAG 9701bcac520c7596ce51829347f4b522ae8346e5
	GIT_SHALLOW 1
	GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(icu)

add_library(dependency_icu INTERFACE)
target_link_libraries(dependency_icu INTERFACE icu)