cmake_minimum_required(VERSION 3.30)
project(raydiator
        VERSION 0.1
        DESCRIPTION "Header-only game engine"
        LANGUAGES CXX)

# Set global C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Also build examples
add_subdirectory(examples)

# Define engine library
add_library(raydiator INTERFACE)

target_include_directories(raydiator INTERFACE ${PROJECT_SOURCE_DIR}/include/)
target_include_directories(raydiator INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

find_package(raylib CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(raydiator INTERFACE raylib glfw)


