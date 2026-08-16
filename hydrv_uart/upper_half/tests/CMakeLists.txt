find_package(GTest REQUIRED)
include(GoogleTest)

file(GLOB TESTS *.cpp)
add_executable(TestUART ${TESTS})
set_target_properties(TestUART
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Testing"
)
target_compile_features(TestUART PUBLIC cxx_std_20)
target_compile_options(TestUART PRIVATE -Wall -Wextra -Wpedantic -Wc++20-compat -Wno-format-security
    -Woverloaded-virtual -Wsuggest-override -g)
target_link_libraries(TestUART HydrodriversUART GTest::gtest GTest::gtest_main -pthread)
gtest_discover_tests(TestUART)
