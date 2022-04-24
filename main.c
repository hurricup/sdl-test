#include <SDL2/SDL.h>


int main() {
    if(SDL_Init(SDL_INIT_VIDEO) != 0){
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_Quit();
}
