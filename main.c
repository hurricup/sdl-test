#include <SDL2/SDL.h>

const int WIDTH = 640;
const int HEIGHT = 640;
const int BALL_SIZE = 10;

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_Window *window = SDL_CreateWindow("program", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT,
                                          SDL_WINDOW_SHOWN);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 0);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect ballRect = {
            .x = WIDTH / 2 - BALL_SIZE / 2,
            .y = HEIGHT / 2 - BALL_SIZE / 2,
            .w = BALL_SIZE,
            .h = BALL_SIZE
    };
    SDL_RenderFillRect(renderer, &ballRect);

    SDL_RenderPresent(renderer);

    SDL_Event event;
    while (SDL_WaitEvent(&event)) {
        if (event.type == SDL_QUIT) {
            break;
        }
    }
    SDL_Quit();
}
