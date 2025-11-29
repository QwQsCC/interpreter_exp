message(STATUS "Building library tests spdlog_test")

add_executable(spdlog_test ${CMAKE_CURRENT_SOURCE_DIR}/spdlog_example.cc)
target_link_libraries(spdlog_test PRIVATE spdlog::spdlog)