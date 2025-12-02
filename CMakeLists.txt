cmake_minimum_required(VERSION 3.10)
project(interpreter_exp)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# GLFW编译有点慢，编译过一次之后可以关了，后面编译快一些
option(BUILD_WITH_GLFW "Build with GLFW" ON)
option(BUILD_LIB_EXAMPLES "Build library examples" ON)
option(BUILD_TEST "Build tests" ON)

add_subdirectory(${CMAKE_SOURCE_DIR}/libs/spdlog)

if(BUILD_WITH_GLFW)
    include_directories(${CMAKE_SOURCE_DIR}/libs/glfw/include)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/glfw)
endif()

if(BUILD_LIB_EXAMPLES)
    file(GLOB EXAMPLE_ENTRIES RELATIVE "${CMAKE_SOURCE_DIR}/examples" "${CMAKE_SOURCE_DIR}/examples/*")
    foreach(entry ${EXAMPLE_ENTRIES})
        set(entry_path "${CMAKE_SOURCE_DIR}/examples/${entry}")
        if(IS_DIRECTORY "${entry_path}")
            add_subdirectory("${entry_path}")
        endif()
    endforeach()
endif()

if(BUILD_TEST)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/gtest)
    add_subdirectory(${CMAKE_SOURCE_DIR}/tests)
endif()

# 测试样例资源复制
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/asset DESTINATION ${CMAKE_BINARY_DIR})