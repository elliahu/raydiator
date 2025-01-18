cmake_minimum_required(VERSION 3.30)
project(raydiator)

# Set global C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src/raydiator)

# Define engine library
add_library(raydiator STATIC ${SOURCES})

target_include_directories(raydiator PUBLIC ${PROJECT_SOURCE_DIR}/include/)
target_include_directories(raydiator PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

find_package(raylib CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(raydiator PUBLIC raylib glfw)

# Also build examples
add_subdirectory(examples)
