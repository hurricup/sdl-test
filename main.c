#include <SDL2/SDL.h>
#include <stdbool.h>
#include <time.h>

#define DEFAULT_WIDTH 640
#define DEFAULT_HEIGHT 640

static const int WIDTH = DEFAULT_WIDTH;
static const int HEIGHT = DEFAULT_HEIGHT;

static const Uint32 FPS = 30;
static const Uint32 FPS_SIZE_MS = 1000 / FPS;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static Uint8 buffer_red[DEFAULT_WIDTH][DEFAULT_HEIGHT];
static Uint8 buffer_green[DEFAULT_WIDTH][DEFAULT_HEIGHT];
static Uint8 buffer_blue[DEFAULT_WIDTH][DEFAULT_HEIGHT];

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
            SDL_SetRenderDrawColor(renderer, buffer_red[x][y], buffer_green[x][y], buffer_blue[x][y], 255);
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
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            buffer_red[x][y] = rand() % 255;
            buffer_green[x][y] = rand() % 255;
            buffer_blue[x][y] = rand() % 255;
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
    SDL_Quit();
}