cmake_minimum_required(VERSION 3.10)
project(interpreter_exp)

set(CMAKE_CXX_STANDARD 20)

option(BUILD_WITH_GLFW "Build with GLFW" ON)
option(BUILD_LIB_TESTS "Build library tests" ON)

add_subdirectory(${CMAKE_SOURCE_DIR}/libs/spdlog)

if(BUILD_WITH_GLFW)
    include_directories(${CMAKE_SOURCE_DIR}/libs/glfw/include)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/glfw)
endif()

if(BUILD_LIB_TESTS)
    add_subdirectory(${CMAKE_SOURCE_DIR}/examples/imgui_test)
    add_subdirectory(${CMAKE_SOURCE_DIR}/examples/spdlog_test)
endif()


