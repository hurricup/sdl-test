#include "sdl_ext.h"

void
SDL_Die(const char *format, ...) {
    va_list argptr;
    va_start(argptr, format);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL, format, argptr);
    va_end(argptr);
    SDL_Quit();
    exit(1);
}


void
SDL_CheckError(const char *file, int line) {
    const char *error = SDL_GetError();
    if (error != NULL && *error != '\0') {
        SDL_Die("SDL Error: %s in %s at %d\n", error, file, line);
    }
}
