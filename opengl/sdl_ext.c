#include "sdl_ext.h"

void
SDL_CheckError(const char *file, int line) {
    const char *error = SDL_GetError();
    if (error != NULL && *error != '\0') {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL Error: %s in %s at %d\n", error, file, line);
        SDL_Quit();
        exit(1);
    }
}
