cmake_minimum_required(VERSION 3.28)
project(VerletPhysicsSimulator)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(SOURCES src/main.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

# Default to Release if no build type specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type (Debug or Release)" FORCE)
endif()

# Compiler-specific flags
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "Configuring Debug build")
        target_compile_options(${PROJECT_NAME} PRIVATE -O0 -g -Wall -Wextra)
    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        message(STATUS "Configuring Release build")
        target_compile_options(${PROJECT_NAME} PRIVATE -O3 -march=native -ffast-math)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "Configuring Debug build")
        target_compile_options(${PROJECT_NAME} PRIVATE /Od /Zi /Wall)
    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        message(STATUS "Configuring Release build")
        target_compile_options(${PROJECT_NAME} PRIVATE /O2 /fp:fast /FAs)
    endif()
endif()

#Find and link SFML
find_package(SFML 2.5 COMPONENTS graphics window system QUIET)
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-graphics sfml-window sfml-system)
