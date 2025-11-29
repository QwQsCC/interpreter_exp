message(STATUS "Building library tests spdlog_test")

add_executable(spdlog_test spdlog_test.cc)
target_link_libraries(spdlog_test PRIVATE spdlog::spdlog)