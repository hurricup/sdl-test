#include <SDL2/SDL.h>
#include <stdbool.h>

static const int WIDTH = 640;
static const int HEIGHT = 640;
static const int BALL_SIZE = 10;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static bool initialize_app();

static void update_screen();

static void event_loop();

static void shutdown_app();

int main() {
    atexit(shutdown_app);
    if (!initialize_app()) {
        exit(1);
    }

    update_screen();
    event_loop();
}

static void
event_loop() {
    SDL_Event event;
    while (SDL_WaitEvent(&event)) {
        if (event.type == SDL_QUIT) {
            break;
        }
    }
}

static void
update_screen() {
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
}

static bool
initialize_app() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return false;
    }
    window = SDL_CreateWindow("program", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT,
                              SDL_WINDOW_SHOWN);
    if (!window) {
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    return renderer != NULL;
}

static void
shutdown_app() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
    SDL_Quit();
}