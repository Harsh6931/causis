cmake_minimum_required(VERSION 3.20)

project(causis
    VERSION 0.1.0
    DESCRIPTION "A DSL and compiler for 2D grid-based simulations"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(causis
    src/main.cpp
)

target_include_directories(causis
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(causis PRIVATE /W4 /permissive-)
else()
    target_compile_options(causis PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()

add_test(NAME causis_help COMMAND causis --help)
