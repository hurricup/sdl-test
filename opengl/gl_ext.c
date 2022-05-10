#include "gl_ext.h"

void
glUniform3vf(GLint location, vec3 vec) {
    glUniform3f(location, vec[0], vec[1], vec[2]);
}

void
glCheckError(const char *file, int line) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        SDL_Die("OpenGl error %x in %s at %d\n", error, file, line);
    }
}
