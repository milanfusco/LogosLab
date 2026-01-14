cmake_minimum_required(VERSION 3.14)
project(LogosLab VERSION 0.1.0 LANGUAGES CXX)

# ============================================================
# CONFIGURATION SANITY (PRESET-FIRST)
# ============================================================

# Presets must define the build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(FATAL_ERROR
        "CMAKE_BUILD_TYPE is not set.\n"
        "Configure this project using a CMake preset.")
endif()

# Export compile commands for tooling (clangd, clang-tidy)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================
# PROJECT OPTIONS
# ============================================================

# Testing and benchmarking
option(ENABLE_PROPERTY_TESTS "Enable property-based testing with RapidCheck" ON)
option(ENABLE_BENCHMARKS "Enable benchmarking with Google Benchmark" ON)

# Static analysis
option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)
option(ENABLE_CPPCHECK "Enable cppcheck static analysis" OFF)

# Sanitizers (mutually exclusive; validated below)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_MSAN "Enable MemorySanitizer (Clang only)" OFF)

# ============================================================
# OPTION VALIDATION
# ============================================================

# Sanitizer compatibility
if(ENABLE_TSAN AND ENABLE_ASAN)
    message(FATAL_ERROR "TSAN cannot be combined with ASAN")
endif()

if(ENABLE_MSAN AND (ENABLE_ASAN OR ENABLE_TSAN))
    message(FATAL_ERROR "MSAN cannot be combined with ASAN or TSAN")
endif()

if(ENABLE_MSAN AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR
        "MSAN requires Clang. "
        "Current compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

# Benchmarks must be optimized and sanitizer-free
if(ENABLE_BENCHMARKS
   AND (ENABLE_ASAN OR ENABLE_UBSAN OR ENABLE_TSAN OR ENABLE_MSAN)
   AND CMAKE_BUILD_TYPE STREQUAL "Release")
    message(FATAL_ERROR
        "Benchmarks must not be built with sanitizers enabled.")
endif()

# ============================================================
# TOOLING CONFIGURATION
# ============================================================

# ---- clang-tidy ----
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE
        NAMES clang-tidy clang-tidy-18 clang-tidy-17 clang-tidy-16 clang-tidy-15
        HINTS /opt/homebrew/opt/llvm/bin /usr/local/opt/llvm/bin
    )
    if(CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    else()
        message(FATAL_ERROR
            "ENABLE_CLANG_TIDY is ON but clang-tidy was not found")
    endif()
endif()

# ---- cppcheck ----
if(ENABLE_CPPCHECK)
    find_program(CPPCHECK_EXE NAMES cppcheck)
    if(NOT CPPCHECK_EXE)
        message(FATAL_ERROR
            "ENABLE_CPPCHECK is ON but cppcheck was not found")
    endif()
endif()

# ============================================================
# EXTERNAL DEPENDENCIES
# ============================================================

include(FetchContent)

if(ENABLE_PROPERTY_TESTS)
    FetchContent_Declare(
        rapidcheck
        GIT_REPOSITORY https://github.com/emil-e/rapidcheck.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(rapidcheck)
endif()

if(ENABLE_BENCHMARKS)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.3
    )
    FetchContent_MakeAvailable(benchmark)
endif()

# ============================================================
# HELPER FUNCTIONS
# ============================================================

function(logoslab_set_cxx_standard target)
    target_compile_features(${target} PUBLIC cxx_std_17)
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
endfunction()

function(logoslab_add_sanitizers target)
    if(ENABLE_ASAN)
        target_compile_options(${target} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address)
    endif()

    if(ENABLE_UBSAN)
        target_compile_options(${target} PRIVATE -fsanitize=undefined -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=undefined)
    endif()

    if(ENABLE_TSAN)
        target_compile_options(${target} PRIVATE -fsanitize=thread -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=thread)
    endif()

    if(ENABLE_MSAN)
        target_compile_options(${target} PRIVATE
            -fsanitize=memory
            -fno-omit-frame-pointer
            -fsanitize-memory-track-origins
        )
        target_link_options(${target} PRIVATE -fsanitize=memory)
    endif()
endfunction()

function(logoslab_add_executable target)
    cmake_parse_arguments(ARG "" "" "SOURCES;LIBRARIES" ${ARGN})
    add_executable(${target} ${ARG_SOURCES})
    target_link_libraries(${target} PRIVATE LogosLabLib ${ARG_LIBRARIES})
    logoslab_set_cxx_standard(${target})
    logoslab_add_sanitizers(${target})
endfunction()

# ============================================================
# CORE LIBRARY
# ============================================================

add_library(LogosLabLib STATIC
    src/Proposition.cpp
    src/Expression.cpp
    src/Lexer.cpp
    src/Parser.cpp
    src/InferenceEngine.cpp
    src/Ratiocinator.cpp
)

target_include_directories(LogosLabLib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

logoslab_set_cxx_standard(LogosLabLib)
logoslab_add_sanitizers(LogosLabLib)

# ============================================================
# MAIN EXECUTABLE
# ============================================================

logoslab_add_executable(main
    SOURCES src/main.cpp
)

# ============================================================
# TESTING
# ============================================================

include(CTest)
enable_testing()

logoslab_add_executable(testProposition SOURCES tests/testProposition.cpp)
add_test(NAME testProposition COMMAND testProposition)

logoslab_add_executable(testExpression SOURCES tests/testExpression.cpp)
add_test(NAME testExpression COMMAND testExpression)

logoslab_add_executable(testLexer SOURCES tests/testLexer.cpp)
add_test(NAME testLexer COMMAND testLexer)

logoslab_add_executable(testRatiocinator SOURCES tests/testRatiocinator.cpp)
target_compile_definitions(testRatiocinator
    PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
add_test(NAME testRatiocinator COMMAND testRatiocinator)

if(ENABLE_PROPERTY_TESTS)
    logoslab_add_executable(testExpressionProperties
        SOURCES tests/testExpressionProperties.cpp
        LIBRARIES rapidcheck
    )
    add_test(NAME testExpressionProperties COMMAND testExpressionProperties)
endif()

# ============================================================
# BENCHMARKS (BUILD ONLY)
# ============================================================

if(ENABLE_BENCHMARKS)
    logoslab_add_executable(benchmarkInference
        SOURCES benchmarks/benchmarkInference.cpp
        LIBRARIES benchmark::benchmark
    )
endif()

# ============================================================
# CPPCHECK TARGETS (MANUAL)
# ============================================================

if(ENABLE_CPPCHECK)
    add_custom_target(cppcheck
        COMMAND ${CPPCHECK_EXE}
            --enable=warning,performance,portability
            --std=c++17
            --error-exitcode=1
            --quiet
            --inline-suppr
            --suppress=missingIncludeSystem
            -I ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running cppcheck"
        EXCLUDE_FROM_ALL
    )
endif()

# ============================================================
# CONFIGURATION SUMMARY
# ============================================================

message(STATUS "")
message(STATUS "=== LogosLab Configuration ===")
message(STATUS "  Version:          ${PROJECT_VERSION}")
message(STATUS "  Build type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler:         ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Property tests:   ${ENABLE_PROPERTY_TESTS}")
message(STATUS "  Benchmarks:       ${ENABLE_BENCHMARKS}")
message(STATUS "  clang-tidy:       ${ENABLE_CLANG_TIDY}")
message(STATUS "  cppcheck:         ${ENABLE_CPPCHECK}")
message(STATUS "  ASAN:             ${ENABLE_ASAN}")
message(STATUS "  UBSAN:            ${ENABLE_UBSAN}")
message(STATUS "  TSAN:             ${ENABLE_TSAN}")
message(STATUS "  MSAN:             ${ENABLE_MSAN}")
message(STATUS "==============================")
message(STATUS "")
