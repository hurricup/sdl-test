#ifndef SDL_TEST_GL_EXT_H
#define SDL_TEST_GL_EXT_H

#include <GLES3/gl32.h>
#include "cglm_ext.h"
#include "sdl_ext.h"

#define GL_CHECK_ERROR glCheckError(__FILE__, __LINE__)

void glUniform3vf(GLint location, vec3 vec);

void glCheckError(const char *file, int line);

#endif //SDL_TEST_GL_EXT_H
