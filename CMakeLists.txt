cmake_minimum_required(VERSION 3.10)
project(interpreter_exp)

set(CMAKE_CXX_STANDARD 20)

option(BUILD_WITH_GLFW "Build with GLFW" OFF)
option(BUILD_LIB_EXAMPLES "Build library examples" OFF)
option(BUILD_TEST "Build tests" ON)

add_subdirectory(${CMAKE_SOURCE_DIR}/libs/spdlog)

if(BUILD_WITH_GLFW)
    include_directories(${CMAKE_SOURCE_DIR}/libs/glfw/include)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/glfw)
endif()

if(BUILD_LIB_EXAMPLES)
    add_subdirectory(${CMAKE_SOURCE_DIR}/examples/imgui_test)
    add_subdirectory(${CMAKE_SOURCE_DIR}/examples/spdlog_test)
endif()

if(BUILD_TEST)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/gtest)
    add_subdirectory(${CMAKE_SOURCE_DIR}/tests)
endif()