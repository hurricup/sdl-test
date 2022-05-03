#ifndef SDL_TEST_SHADER_H
#define SDL_TEST_SHADER_H

#include <SDL2/SDL.h>
#include <GLES3/gl3.h>
#include "file_util.h"

unsigned int create_shader(const char *vertex_shader_name, const char *fragment_shader_name);

#endif //SDL_TEST_SHADER_H
