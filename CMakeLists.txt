cmake_minimum_required(VERSION 3.5.0)
project(LogosLab VERSION 0.1.0)

# Set the C++ standard to C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the build type to Debug (if not specified)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

include(CTest)
enable_testing()

# Include the header files
include_directories(${CMAKE_SOURCE_DIR}/include/)

# List core source files explicitly to avoid including test files
set(TARGET_SRC
    ${CMAKE_SOURCE_DIR}/src/Proposition.cpp
    ${CMAKE_SOURCE_DIR}/src/Expression.cpp
    ${CMAKE_SOURCE_DIR}/src/Parser.cpp
    ${CMAKE_SOURCE_DIR}/src/InferenceEngine.cpp
    ${CMAKE_SOURCE_DIR}/src/Ratiocinator.cpp
)

# Create a static library from core project files
add_library(LogosLabLib STATIC ${TARGET_SRC})

# Create the main executable and link to the core library
add_executable(main ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_link_libraries(main LogosLabLib)

# Create the test executable for Propositions and link to the core library
add_executable(testProposition ${CMAKE_SOURCE_DIR}/tests/testProposition.cpp)
target_link_libraries(testProposition LogosLabLib)

# Create the test executable for Expressions and link to the core library
add_executable(testExpression ${CMAKE_SOURCE_DIR}/tests/testExpression.cpp )
target_link_libraries(testExpression LogosLabLib)

# Create the test executable for Ratiocinator and link to the core library
add_executable(testRatiocinator ${CMAKE_SOURCE_DIR}/tests/testRatiocinator.cpp)
target_link_libraries(testRatiocinator LogosLabLib)
target_compile_definitions(testRatiocinator PRIVATE TEST_DATA_DIR="${CMAKE_SOURCE_DIR}")

# REgister the test executables
add_test(NAME testProposition COMMAND testProposition)
add_test(NAME testExpression COMMAND testExpression)
add_test(NAME testRatiocinator COMMAND testRatiocinator)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
