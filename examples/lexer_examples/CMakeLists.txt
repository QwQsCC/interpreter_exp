include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(dfa_table_driver STATIC 
    ${CMAKE_SOURCE_DIR}/src/lexer/dfa_table_driver.cc
)
target_link_libraries(dfa_table_driver PRIVATE spdlog::spdlog)

add_executable(token_example ${CMAKE_CURRENT_SOURCE_DIR}/lexer_example.cc)
target_link_libraries(token_example PRIVATE spdlog::spdlog dfa_table_driver)