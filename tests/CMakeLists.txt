find_package(Catch2 REQUIRED)

file(GLOB_RECURSE TEST_SOURCE_FILES "*.cpp")
add_executable(tests ${TEST_SOURCE_FILES})
target_include_directories(tests PRIVATE "${CMAKE_SOURCE_DIR}/include/feather")
target_link_libraries(tests Catch2::Catch2)

include(CTest)
# include(Catch)
# catch_discover_tests(tests)
include(ParseAndAddCatchTests)
set(PARSE_CATCH_TESTS_VERBOSE ON)
ParseAndAddCatchTests(tests)
