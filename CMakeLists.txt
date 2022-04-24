cmake_minimum_required(VERSION 3.22)
project(sdl_test C)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

set(CMAKE_C_STANDARD 99)

add_executable(sdl_test main.c)
target_link_libraries(sdl_test ${SDL2_LIBRARIES})
