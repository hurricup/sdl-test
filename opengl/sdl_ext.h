#ifndef SDL_TEST_SDL_EXT_H
#define SDL_TEST_SDL_EXT_H

#include <SDL2/SDL.h>

#define SDL_CHECK_ERROR SDL_CheckError(__FILE__, __LINE__)

void
SDL_CheckError(const char *file, int line) {
    const char *error = SDL_GetError();
    if (error != NULL && *error != '\0') {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL Error: %s in %s at %d\n", error, file, line);
        SDL_Quit();
        exit(1);
    }
}


#endif //SDL_TEST_SDL_EXT_H
