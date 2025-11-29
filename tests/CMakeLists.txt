enable_testing()

include(GoogleTest)

add_executable(simple_test ${CMAKE_CURRENT_SOURCE_DIR}/simple_test/simple_test.cc)
target_link_libraries(simple_test PRIVATE GTest::gtest GTest::gtest_main)
target_compile_definitions(simple_test PRIVATE GTEST_COLOR=1)
gtest_discover_tests(simple_test)
