#ifndef SDL_TEST_GL_EXT_H
#define SDL_TEST_GL_EXT_H

#include <GLES3/gl32.h>
#include "cglm_ext.h"

#define GL_CHECK_ERROR glCheckError(__FILE__, __LINE__)

static inline
void glUniform3vf(GLint location, vec3 vec) {
    glUniform3f(location, vec[0], vec[1], vec[2]);
}

void
glCheckError(const char *file, int line) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "OpenGl error %x in %s at %d\n", error, file, line);
        exit(1);
    }
}

#endif //SDL_TEST_GL_EXT_H
