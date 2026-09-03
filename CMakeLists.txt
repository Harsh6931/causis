cmake_minimum_required(VERSION 3.20)

project(causis
    VERSION 0.1.0
    DESCRIPTION "A DSL and compiler for 2D grid-based simulations"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(causis_lexer STATIC
    src/lexer/token.cpp
    src/lexer/lexer.cpp
)

target_include_directories(causis_lexer
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(causis_lexer PRIVATE /W4 /permissive-)
else()
    target_compile_options(causis_lexer PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(causis
    src/main.cpp
)

target_link_libraries(causis
    PRIVATE
        causis_lexer
)

if(MSVC)
    target_compile_options(causis PRIVATE /W4 /permissive-)
else()
    target_compile_options(causis PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_test(NAME causis_help COMMAND causis --help)

add_executable(lexer_test
    tests/lexer/lexer_test.cpp
)

target_link_libraries(lexer_test
    PRIVATE
        causis_lexer
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(lexer_test
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
