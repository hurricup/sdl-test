cmake_minimum_required(VERSION 3.22)
project(sdl_test C)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(cglm REQUIRED)
find_package(assimp REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${CGLM_INCLUDE_DIRS} ${ASSIMP_INCLUDE_DIRS})

set(CMAKE_C_STANDARD 99)

add_executable(sdl_test sdl_test.c)
add_executable(opengl_test opengl_test.c opengl/camera.c opengl/camera.h opengl/file_util.c opengl/file_util.h opengl/texture.c opengl/texture.h opengl/shader.c opengl/shader.h models/cube.c models/cube.h opengl/material.h opengl/light.h opengl/gl_ext.h opengl/sdl_ext.h opengl/model.h opengl/model.c opengl/sdl_ext.c opengl/gl_ext.c)
target_link_libraries(sdl_test ${SDL2_LIBRARIES})
target_link_libraries(opengl_test ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} ${ASSIMP_LIBRARIES} m cglm)