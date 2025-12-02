# CMakeLists.txt for interpreter examples

message(STATUS "Building Draw Language Interpreter")

find_package(OpenGL REQUIRED)

# ImGui库（如果还没有构建）
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/libs/imgui)
if(NOT TARGET imgui)
    add_library(imgui STATIC
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
    )
    target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
    )
endif()

# 源文件
set(INTERPRETER_SOURCES
    # 词法分析器
    ${CMAKE_SOURCE_DIR}/src/lexer/InputSource.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/TableDrivenDFA.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/HardCodedDFA.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/SimpleLexer.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/DrawLangLexer.cpp
    # 语法分析器
    ${CMAKE_SOURCE_DIR}/src/parser/DrawLangAST.cpp
    ${CMAKE_SOURCE_DIR}/src/parser/DrawLangParser.cpp
    # 语义分析器
    ${CMAKE_SOURCE_DIR}/src/semantics/DrawLangSemantic.cpp
    # 错误日志
    ${CMAKE_SOURCE_DIR}/src/errlog/ErrorLog.cpp
    # UI
    ${CMAKE_SOURCE_DIR}/src/gui/DrawLangUI.cpp
    ${CMAKE_SOURCE_DIR}/src/gui/DrawLangImGuiUI.cpp
    # 解释器
    ${CMAKE_SOURCE_DIR}/src/interpreter/DrawLangInterpreter.cpp
)

# Draw语言解释器
add_executable(draw_lang_interpreter
    ${CMAKE_CURRENT_SOURCE_DIR}/draw_lang_main.cc
    ${INTERPRETER_SOURCES}
)

target_include_directories(draw_lang_interpreter PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/lexer
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_SOURCE_DIR}/src/semantics
    ${CMAKE_SOURCE_DIR}/src/errlog
    ${CMAKE_SOURCE_DIR}/src/gui
    ${CMAKE_SOURCE_DIR}/src/interpreter
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# 链接库
target_link_libraries(draw_lang_interpreter PRIVATE 
    spdlog::spdlog 
    imgui 
    glfw 
    OpenGL::GL
)

# 设置输出目录
# set_target_properties(draw_lang_interpreter PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
# )
