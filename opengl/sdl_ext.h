#ifndef SDL_TEST_SDL_EXT_H
#define SDL_TEST_SDL_EXT_H

#include <SDL2/SDL.h>

#define SDL_CHECK_ERROR SDL_CheckError(__FILE__, __LINE__)

void SDL_CheckError(const char *file, int line);

void SDL_Die(const char *format, ...);

#endif //SDL_TEST_SDL_EXT_H
