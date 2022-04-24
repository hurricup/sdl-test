#include <SDL2/SDL.h>
#include <stdbool.h>
#include <time.h>

static const int WIDTH = 640;
static const int HEIGHT = 640;

static const Uint32 FPS = 30;
static const Uint32 FPS_SIZE_MS = 1000 / FPS;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static Uint8 *buffer_red = NULL;
static Uint8 *buffer_green = NULL;
static Uint8 *buffer_blue = NULL;

static bool initialize_app();

static void initialize_buffers();

static void update_screen();

static void event_loop();

static void shutdown_app();

int main() {
    atexit(shutdown_app);
    if (!initialize_app()) {
        exit(1);
    }
    event_loop();
}

static void
event_loop() {
    SDL_Event event;
    Uint32 currentTicks = 0;
    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return;
            }
        }
        if (SDL_GetTicks() - currentTicks > FPS_SIZE_MS) {
            update_screen();
            currentTicks = SDL_GetTicks();
        }
    }
}

static void
update_screen() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            SDL_SetRenderDrawColor(renderer, buffer_red[x * HEIGHT + y], buffer_green[x * HEIGHT + y],
                                   buffer_blue[x * HEIGHT + y], 255);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }

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
    if (!renderer) {
        return false;
    }
    initialize_buffers();
    return true;
}

static void
initialize_buffers() {
    srand(SDL_GetTicks());
    buffer_red = calloc(WIDTH * HEIGHT, sizeof(Uint8));
    buffer_green = calloc(WIDTH * HEIGHT, sizeof(Uint8));
    buffer_blue = calloc(WIDTH * HEIGHT, sizeof(Uint8));
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            buffer_red[x * HEIGHT + y] = rand() % 255;
            buffer_green[x * HEIGHT + y] = rand() % 255;
            buffer_blue[x * HEIGHT + y] = rand() % 255;
        }
    }
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
    if (buffer_red) {
        free(buffer_red);
        buffer_red = NULL;
    }
    if (buffer_green) {
        free(buffer_green);
        buffer_green = NULL;
    }
    if (buffer_blue) {
        free(buffer_blue);
        buffer_blue = NULL;
    }

    SDL_Quit();
}