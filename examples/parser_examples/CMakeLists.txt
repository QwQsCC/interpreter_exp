# CMakeLists.txt for parser examples

include_directories(${CMAKE_SOURCE_DIR}/include)

# 源文件
set(PARSER_SOURCES
    ${CMAKE_SOURCE_DIR}/src/lexer/InputSource.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/TableDrivenDFA.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/HardCodedDFA.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/SimpleLexer.cpp
    ${CMAKE_SOURCE_DIR}/src/parser/DrawLangAST.cpp
    ${CMAKE_SOURCE_DIR}/src/parser/DrawLangParser.cpp
    ${CMAKE_SOURCE_DIR}/src/semantics/DrawLangSemantic.cpp
    ${CMAKE_SOURCE_DIR}/src/errlog/ErrorLog.cpp
)

# Parser测试示例
add_executable(parser_example 
    ${CMAKE_CURRENT_SOURCE_DIR}/parser_example.cc
    ${PARSER_SOURCES}
)

target_include_directories(parser_example PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/lexer
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_SOURCE_DIR}/src/semantics
    ${CMAKE_SOURCE_DIR}/src/errlog
)

# 链接spdlog
target_link_libraries(parser_example PRIVATE spdlog::spdlog)
