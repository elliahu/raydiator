cmake_minimum_required(VERSION 3.30)
project(raydiator)

set(CMAKE_CXX_STANDARD 23)

add_executable(raydiator main.cpp)

find_package(raylib CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(raydiator PRIVATE raylib glfw)
