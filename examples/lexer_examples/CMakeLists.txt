include_directories(${CMAKE_SOURCE_DIR}/include)

# SimpleLexer实现源文件
set(SIMPLE_LEXER_SOURCES
    ${CMAKE_SOURCE_DIR}/src/lexer/InputSource.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/TableDrivenDFA.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/HardCodedDFA.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/SimpleLexer.cpp
)

# SimpleLexer测试示例
add_executable(simple_lexer_example 
    ${CMAKE_CURRENT_SOURCE_DIR}/simple_lexer_example.cc
    ${SIMPLE_LEXER_SOURCES}
)

target_include_directories(simple_lexer_example PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/lexer
)