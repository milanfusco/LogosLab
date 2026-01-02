cmake_minimum_required(VERSION 3.14)
project(LogosLab VERSION 0.1.0 LANGUAGES CXX)

# ============================================================
# GLOBAL SETTINGS 
# ============================================================

# Export compile commands for IDE/tooling support (clangd, clang-tidy, etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type defaults to Debug
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug' as none was specified")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# ============================================================
# PROJECT OPTIONS
# ============================================================

# Testing and benchmarking
option(ENABLE_PROPERTY_TESTS "Enable property-based testing with RapidCheck" ON)
option(ENABLE_BENCHMARKS "Enable benchmarking with Google Benchmark" ON)

# Static analysis
option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)
option(ENABLE_CPPCHECK "Enable cppcheck static analysis" OFF)

# Sanitizers (mutually exclusive constraints enforced below)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_MSAN "Enable MemorySanitizer (Clang only)" OFF)

# ============================================================
# SANITIZER VALIDATION
# ============================================================

# Validate sanitizer combinations
if(ENABLE_TSAN AND ENABLE_ASAN)
    message(FATAL_ERROR "ThreadSanitizer (TSAN) cannot be used with AddressSanitizer (ASAN)")
endif()

if(ENABLE_MSAN AND (ENABLE_ASAN OR ENABLE_TSAN))
    message(FATAL_ERROR "MemorySanitizer (MSAN) cannot be used with ASAN or TSAN")
endif()

if(ENABLE_MSAN AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "MemorySanitizer requires Clang. Current compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

# Warn about sanitizers in Release mode
if((ENABLE_ASAN OR ENABLE_UBSAN OR ENABLE_TSAN OR ENABLE_MSAN) AND CMAKE_BUILD_TYPE STREQUAL "Release")
    message(WARNING "Sanitizers work best with Debug builds. Consider using -DCMAKE_BUILD_TYPE=Debug")
endif()

# ============================================================
# CLANG-TIDY CONFIGURATION
# ============================================================

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE 
        NAMES clang-tidy clang-tidy-18 clang-tidy-17 clang-tidy-16 clang-tidy-15
        HINTS /opt/homebrew/opt/llvm/bin /usr/local/opt/llvm/bin
    )
    if(CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
        # Rely on .clang-tidy config file for check configuration
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    else()
        message(WARNING "clang-tidy requested but not found. Install LLVM or set PATH to include clang-tidy.")
    endif()
endif()

# ============================================================
# CPPCHECK CONFIGURATION
# ============================================================

if(ENABLE_CPPCHECK)
    find_program(CPPCHECK_EXE NAMES cppcheck)
    if(CPPCHECK_EXE)
        message(STATUS "cppcheck found: ${CPPCHECK_EXE}")
    else()
        message(WARNING "cppcheck requested but not found")
    endif()
endif()

# ============================================================
# EXTERNAL DEPENDENCIES (FetchContent)
# ============================================================

include(FetchContent)

if(ENABLE_PROPERTY_TESTS)
    FetchContent_Declare(
        rapidcheck
        GIT_REPOSITORY https://github.com/emil-e/rapidcheck.git
        GIT_TAG        master
    )
    FetchContent_MakeAvailable(rapidcheck)
endif()

if(ENABLE_BENCHMARKS)
    # Disable Google Benchmark's own tests to speed up build
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG        v1.8.3
    )
    FetchContent_MakeAvailable(benchmark)
endif()

# ============================================================
# HELPER FUNCTIONS
# ============================================================

# Apply C++17 standard to a target
function(logoslab_set_cxx_standard target)
    target_compile_features(${target} PUBLIC cxx_std_17)
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
endfunction()

# Apply sanitizer flags to a target
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

# Configure a LogosLab executable (links library, applies standards and sanitizers)
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

set(LOGOSLAB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Proposition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Expression.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Lexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InferenceEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Ratiocinator.cpp
)

add_library(LogosLabLib STATIC ${LOGOSLAB_SOURCES})

# Target-specific include directories (PUBLIC so consumers inherit them)
target_include_directories(LogosLabLib 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Apply C++17 standard
logoslab_set_cxx_standard(LogosLabLib)

# Apply sanitizers to the library
logoslab_add_sanitizers(LogosLabLib)

# ============================================================
# MAIN EXECUTABLE
# ============================================================

logoslab_add_executable(main SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

# ============================================================
# TESTING
# ============================================================

include(CTest)
enable_testing()

# Unit test executables
logoslab_add_executable(testProposition 
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/testProposition.cpp
)
add_test(NAME testProposition COMMAND testProposition)

logoslab_add_executable(testExpression 
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/testExpression.cpp
)
add_test(NAME testExpression COMMAND testExpression)

logoslab_add_executable(testRatiocinator 
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/testRatiocinator.cpp
)
target_compile_definitions(testRatiocinator PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
add_test(NAME testRatiocinator COMMAND testRatiocinator)

logoslab_add_executable(testLexer 
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/testLexer.cpp
)
add_test(NAME testLexer COMMAND testLexer)

# Property-based tests with RapidCheck
if(ENABLE_PROPERTY_TESTS)
    logoslab_add_executable(testExpressionProperties 
        SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/testExpressionProperties.cpp
        LIBRARIES rapidcheck
    )
    add_test(NAME testExpressionProperties COMMAND testExpressionProperties)
endif()

# ============================================================
# BENCHMARKS
# ============================================================

if(ENABLE_BENCHMARKS)
    logoslab_add_executable(benchmarkInference 
        SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/benchmarkInference.cpp
        LIBRARIES benchmark::benchmark
    )
    
    # Register benchmark as a quick-run test
    add_test(NAME benchmarkInference 
        COMMAND benchmarkInference --benchmark_min_time=0.01 --benchmark_format=console
    )
endif()

# ============================================================
# CPPCHECK TARGETS
# ============================================================

if(ENABLE_CPPCHECK AND CPPCHECK_EXE)
    add_custom_target(cppcheck
        COMMAND ${CPPCHECK_EXE}
            --enable=all
            --std=c++17
            --suppress=missingIncludeSystem
            --suppress=unmatchedSuppression
            --suppress=unusedFunction
            --error-exitcode=1
            --inline-suppr
            --quiet
            -I ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running cppcheck static analysis..."
    )
    
    add_custom_target(cppcheck-verbose
        COMMAND ${CPPCHECK_EXE}
            --enable=all
            --std=c++17
            --suppress=missingIncludeSystem
            --verbose
            --error-exitcode=1
            -I ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running cppcheck with verbose output..."
    )
endif()

# ============================================================
# PACKAGING
# ============================================================

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# ============================================================
# CONFIGURATION SUMMARY
# ============================================================
message(STATUS "")
message(STATUS "=== LogosLab Configuration ===")
message(STATUS "  Version:          ${PROJECT_VERSION}")
message(STATUS "  Build type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Compiler:     ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Property tests:   ${ENABLE_PROPERTY_TESTS}")
message(STATUS "  Benchmarks:       ${ENABLE_BENCHMARKS}")
message(STATUS "  clang-tidy:       ${ENABLE_CLANG_TIDY}")
message(STATUS "  cppcheck:         ${ENABLE_CPPCHECK}")
message(STATUS "  AddressSanitizer: ${ENABLE_ASAN}")
message(STATUS "  UBSanitizer:      ${ENABLE_UBSAN}")
message(STATUS "  ThreadSanitizer:  ${ENABLE_TSAN}")
message(STATUS "  MemorySanitizer:  ${ENABLE_MSAN}")
message(STATUS "==============================")
message(STATUS "")
