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

add_library(causis_ast STATIC
    src/ast/ast_printer.cpp
)

target_link_libraries(causis_ast
    PUBLIC
        causis_lexer
)

if(MSVC)
    target_compile_options(causis_ast PRIVATE /W4 /permissive-)
else()
    target_compile_options(causis_ast PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(causis_parser STATIC
    src/parser/parser.cpp
)

target_link_libraries(causis_parser
    PUBLIC
        causis_ast
)

if(MSVC)
    target_compile_options(causis_parser PRIVATE /W4 /permissive-)
else()
    target_compile_options(causis_parser PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(causis_semantic STATIC
    src/semantic/analyzer.cpp
)

target_link_libraries(causis_semantic
    PUBLIC
        causis_ast
)

if(MSVC)
    target_compile_options(causis_semantic PRIVATE /W4 /permissive-)
else()
    target_compile_options(causis_semantic PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(causis_vm STATIC
    src/vm/vm.cpp
)

target_link_libraries(causis_vm
    PUBLIC
        causis_bytecode
)

if(MSVC)
    target_compile_options(causis_vm PRIVATE /W4 /permissive-)
else()
    target_compile_options(causis_vm PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(causis_runtime STATIC
    src/runtime/types.cpp
    src/runtime/world.cpp
    src/runtime/simulation.cpp
    src/runtime/executor.cpp
    src/runtime/program_runner.cpp
)

target_include_directories(causis_runtime
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(causis_runtime
    PUBLIC
        causis_ast
        causis_vm
        causis_ir
        causis_bytecode
)

if(MSVC)
    target_compile_options(causis_runtime PRIVATE /W4 /permissive-)
else()
    target_compile_options(causis_runtime PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(causis_ir STATIC
    src/ir/ir.cpp
    src/ir/builder.cpp
    src/ir/lower.cpp
    src/ir/optimizer.cpp
)

target_link_libraries(causis_ir
    PUBLIC
        causis_ast
)

if(MSVC)
    target_compile_options(causis_ir PRIVATE /W4 /permissive-)
else()
    target_compile_options(causis_ir PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(causis_bytecode STATIC
    src/bytecode/bytecode.cpp
    src/bytecode/compiler.cpp
    src/bytecode/disassembler.cpp
)

target_link_libraries(causis_bytecode
    PUBLIC
        causis_ir
)

if(MSVC)
    target_compile_options(causis_bytecode PRIVATE /W4 /permissive-)
else()
    target_compile_options(causis_bytecode PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(causis_cli STATIC
    src/cli/frontend.cpp
)

target_link_libraries(causis_cli
    PUBLIC
        causis_semantic
        causis_parser
)

if(MSVC)
    target_compile_options(causis_cli PRIVATE /W4 /permissive-)
else()
    target_compile_options(causis_cli PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(causis
    src/main.cpp
)

target_link_libraries(causis
    PRIVATE
        causis_cli
        causis_vm
        causis_bytecode
        causis_ir
        causis_runtime
        causis_semantic
        causis_parser
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

add_test(NAME causis_tokenize_basic COMMAND causis tokenize ${CMAKE_SOURCE_DIR}/examples/basic_move.ls)
add_test(NAME causis_parse_basic COMMAND causis parse ${CMAKE_SOURCE_DIR}/examples/basic_move.ls)
add_test(NAME causis_semantic_basic COMMAND causis semantic ${CMAKE_SOURCE_DIR}/examples/basic_move.ls)
add_test(NAME causis_ir_basic COMMAND causis ir ${CMAKE_SOURCE_DIR}/examples/basic_move.ls)
add_test(NAME causis_optimize_basic COMMAND causis optimize ${CMAKE_SOURCE_DIR}/examples/basic_move.ls)
add_test(NAME causis_disassemble_basic COMMAND causis disassemble ${CMAKE_SOURCE_DIR}/examples/basic_move.ls)
add_test(NAME causis_run_target COMMAND causis run ${CMAKE_SOURCE_DIR}/examples/target.ls)
add_test(NAME causis_run_ticks COMMAND causis run ${CMAKE_SOURCE_DIR}/examples/basic_move.ls --ticks 3)

add_executable(lexer_test
    tests/lexer/lexer_test.cpp
)

target_link_libraries(lexer_test
    PRIVATE
        causis_lexer
        GTest::gtest_main
)

add_executable(parser_test
    tests/parser/parser_test.cpp
)

target_link_libraries(parser_test
    PRIVATE
        causis_parser
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(lexer_test
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
gtest_discover_tests(parser_test
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

add_executable(semantic_test
    tests/semantic/semantic_test.cpp
)

target_link_libraries(semantic_test
    PRIVATE
        causis_semantic
        causis_parser
        GTest::gtest_main
)

gtest_discover_tests(semantic_test
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

add_executable(simulation_test
    tests/runtime/simulation_test.cpp
)

target_link_libraries(simulation_test
    PRIVATE
        causis_runtime
        GTest::gtest_main
)

gtest_discover_tests(simulation_test
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

add_executable(program_runner_test
    tests/runtime/program_runner_test.cpp
)

target_link_libraries(program_runner_test
    PRIVATE
        causis_runtime
        causis_semantic
        causis_parser
        GTest::gtest_main
)

gtest_discover_tests(program_runner_test
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

add_executable(ir_test
    tests/ir/ir_test.cpp
)

target_link_libraries(ir_test
    PRIVATE
        causis_ir
        causis_semantic
        causis_parser
        GTest::gtest_main
)

gtest_discover_tests(ir_test
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

add_executable(optimizer_test
    tests/ir/optimizer_test.cpp
)

target_link_libraries(optimizer_test
    PRIVATE
        causis_ir
        causis_semantic
        causis_parser
        GTest::gtest_main
)

gtest_discover_tests(optimizer_test
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

add_executable(bytecode_test
    tests/bytecode/bytecode_test.cpp
)

target_link_libraries(bytecode_test
    PRIVATE
        causis_bytecode
        causis_ir
        causis_semantic
        causis_parser
        GTest::gtest_main
)

gtest_discover_tests(bytecode_test
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

add_executable(vm_test
    tests/vm/vm_test.cpp
)

target_link_libraries(vm_test
    PRIVATE
        causis_vm
        causis_runtime
        causis_bytecode
        causis_ir
        causis_semantic
        causis_parser
        GTest::gtest_main
)

gtest_discover_tests(vm_test
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

add_executable(cli_test
    tests/cli/frontend_test.cpp
)

target_link_libraries(cli_test
    PRIVATE
        causis_cli
        GTest::gtest_main
)

gtest_discover_tests(cli_test
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
