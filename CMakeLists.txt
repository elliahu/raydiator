cmake_minimum_required(VERSION 3.30)
project(toaster)

set(CMAKE_CXX_STANDARD 23)

add_executable(toaster main.cpp)

find_package(raylib CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(toaster PRIVATE raylib glfw)
