enable_testing()

include(GoogleTest)
include_directories(src/lexer)
include_directories(src/parser)
include_directories(src/semantics)
include_directories(src/errlog)
include_directories(src/interpreter)
include_directories(src/gui)

# 简单测试
add_executable(simple_test ${CMAKE_CURRENT_SOURCE_DIR}/simple_test/simple_test.cc)
target_link_libraries(simple_test PRIVATE GTest::gtest GTest::gtest_main)
target_compile_definitions(simple_test PRIVATE GTEST_COLOR=1)
gtest_discover_tests(simple_test)

# 词法分析器测试
set(LEXER_SOURCES
    ${CMAKE_SOURCE_DIR}/src/lexer/InputSource.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/TableDrivenDFA.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/HardCodedDFA.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/SimpleLexer.cpp
)

add_executable(lexer_test 
    ${CMAKE_CURRENT_SOURCE_DIR}/lexer_test/lexer_test.cc
    ${LEXER_SOURCES}
)
target_include_directories(lexer_test PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/lexer
)
target_link_libraries(lexer_test PRIVATE GTest::gtest GTest::gtest_main)
target_compile_definitions(lexer_test PRIVATE GTEST_COLOR=1)
gtest_discover_tests(lexer_test)

# 语法分析器测试
set(PARSER_SOURCES
    ${CMAKE_SOURCE_DIR}/src/lexer/InputSource.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/TableDrivenDFA.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/HardCodedDFA.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/SimpleLexer.cpp
    ${CMAKE_SOURCE_DIR}/src/parser/DrawLangAST.cpp
    ${CMAKE_SOURCE_DIR}/src/parser/DrawLangParser.cpp
    ${CMAKE_SOURCE_DIR}/src/errlog/ErrorLog.cpp
)

add_executable(parser_test 
    ${CMAKE_CURRENT_SOURCE_DIR}/parser_test/parser_test.cc
    ${PARSER_SOURCES}
)
target_include_directories(parser_test PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/lexer
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_SOURCE_DIR}/src/errlog
)
target_link_libraries(parser_test PRIVATE GTest::gtest GTest::gtest_main spdlog::spdlog)
target_compile_definitions(parser_test PRIVATE GTEST_COLOR=1)
gtest_discover_tests(parser_test)

# 语义分析器测试
set(SEMANTIC_SOURCES
    ${CMAKE_SOURCE_DIR}/src/lexer/InputSource.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/TableDrivenDFA.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/HardCodedDFA.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/SimpleLexer.cpp
    ${CMAKE_SOURCE_DIR}/src/parser/DrawLangAST.cpp
    ${CMAKE_SOURCE_DIR}/src/parser/DrawLangParser.cpp
    ${CMAKE_SOURCE_DIR}/src/semantics/DrawLangSemantic.cpp
    ${CMAKE_SOURCE_DIR}/src/errlog/ErrorLog.cpp
)

add_executable(semantic_test 
    ${CMAKE_CURRENT_SOURCE_DIR}/semantic_test/semantic_test.cc
    ${SEMANTIC_SOURCES}
)
target_include_directories(semantic_test PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/lexer
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_SOURCE_DIR}/src/semantics
    ${CMAKE_SOURCE_DIR}/src/errlog
)
target_link_libraries(semantic_test PRIVATE GTest::gtest GTest::gtest_main spdlog::spdlog)
target_compile_definitions(semantic_test PRIVATE GTEST_COLOR=1)
gtest_discover_tests(semantic_test)
