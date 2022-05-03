cmake_minimum_required(VERSION 3.22)
project(sdl_test C)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(cglm REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories((${CGLM_INCLUDE_DIRS}))

set(CMAKE_C_STANDARD 99)

add_executable(sdl_test sdl_test.c)
add_executable(opengl_test opengl_test.c opengl/camera.c opengl/camera.h opengl/file_util.c opengl/file_util.h)
target_link_libraries(sdl_test ${SDL2_LIBRARIES})
target_link_libraries(opengl_test ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} m cglm)