cmake_minimum_required(VERSION 3.14)
project(LogosLab VERSION 0.1.0 LANGUAGES CXX)

# C++ Standard Configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type defaults to Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Testing support
include(CTest)
enable_testing()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Core library source files
set(LOGOSLAB_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Proposition.cpp
    ${CMAKE_SOURCE_DIR}/src/Expression.cpp
    ${CMAKE_SOURCE_DIR}/src/Lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/Parser.cpp
    ${CMAKE_SOURCE_DIR}/src/InferenceEngine.cpp
    ${CMAKE_SOURCE_DIR}/src/Ratiocinator.cpp
)

# Static library
add_library(LogosLabLib STATIC ${LOGOSLAB_SOURCES})

# Main executable
add_executable(main ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_link_libraries(main PRIVATE LogosLabLib)

# Test executables
add_executable(testProposition ${CMAKE_SOURCE_DIR}/tests/testProposition.cpp)
target_link_libraries(testProposition PRIVATE LogosLabLib)

add_executable(testExpression ${CMAKE_SOURCE_DIR}/tests/testExpression.cpp)
target_link_libraries(testExpression PRIVATE LogosLabLib)

add_executable(testRatiocinator ${CMAKE_SOURCE_DIR}/tests/testRatiocinator.cpp)
target_link_libraries(testRatiocinator PRIVATE LogosLabLib)
target_compile_definitions(testRatiocinator PRIVATE TEST_DATA_DIR="${CMAKE_SOURCE_DIR}")

add_executable(testLexer ${CMAKE_SOURCE_DIR}/tests/testLexer.cpp)
target_link_libraries(testLexer PRIVATE LogosLabLib)

# Register tests
add_test(NAME testProposition COMMAND testProposition)
add_test(NAME testExpression COMMAND testExpression)
add_test(NAME testRatiocinator COMMAND testRatiocinator)
add_test(NAME testLexer COMMAND testLexer)

# CPack configuration
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
