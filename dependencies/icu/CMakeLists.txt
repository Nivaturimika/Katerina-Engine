message(STATUS "Finding icu...")

find_package(ICU COMPONENTS uc dt in REQUIRED)
add_library(dependency_icu INTERFACE)
target_link_libraries(dependency_icu INTERFACE ICU::uc ICU::data ICU::i18n)
