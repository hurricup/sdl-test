#ifndef SDL_TEST_TEXTURE_H
#define SDL_TEST_TEXTURE_H

#include <stdbool.h>
#include "sdl_ext.h"
#include <GLES3/gl3.h>
#include "stb_image.h"

void load_texture(GLenum texture_unit, const char *filename);

#endif //SDL_TEST_TEXTURE_H
