cmake_minimum_required(VERSION 3.29.6)

project(raydiator
        VERSION 0.1
        DESCRIPTION "Game engine"
        LANGUAGES CXX
)

# Set global C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add hammock dependency
add_subdirectory(hammock)

# Collect sources
file(GLOB_RECURSE RAYDIATOR_SOURCES
        ${PROJECT_SOURCE_DIR}/engine/src/*.cpp
)

# Define engine library
add_library(raydiator
        raydiator.h
        ${RAYDIATOR_SOURCES}
        engine/include/Renderer.h
        engine/src/Renderer.cpp
        tools/test_game/main.cpp
        engine/src/Engine.cpp
        engine/include/Engine.h
)

# Public headers visible to consumers of the library
target_include_directories(raydiator PUBLIC
        ${PROJECT_SOURCE_DIR}/engine/include
        ${PROJECT_SOURCE_DIR}              # so that #include "raydiator.h" works
        ${CMAKE_CURRENT_BINARY_DIR}
)

# Private headers only needed inside the library
target_include_directories(raydiator PRIVATE
        ${PROJECT_SOURCE_DIR}/hammock/include
)

target_link_libraries(raydiator PRIVATE hammock)

# Also build tools
add_subdirectory(tools/level_editor)
