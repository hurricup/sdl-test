#include "gl_ext.h"

void
glCheckError(const char *file, int line) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        SDL_Die("OpenGl error %x in %s at %d\n", error, file, line);
    }
}
