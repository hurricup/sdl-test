#include <SDL2/SDL.h>
#include <stdbool.h>
#include <time.h>

#define ALLOC_BUFFER calloc(WIDTH * HEIGHT, sizeof(Uint8))
#define OFFSET(x, y) y * WIDTH + x

static const int WIDTH = 640;
static const int HEIGHT = 640;

static const Uint32 FPS = 30;
static const Uint32 FPS_SIZE_MS = 1000 / FPS;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static Uint8 *buffer_red = NULL;
static Uint8 *buffer_green = NULL;
static Uint8 *buffer_blue = NULL;

static void recalc_buffer(Uint8 **old_buffer);

static bool initialize_app();

static void add_disturbance();

static void initialize_buffers();

static void update_screen();

static void recalc_buffers();

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
    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return;
            }
        }
        update_screen();
        SDL_Delay(FPS_SIZE_MS);
    }
}

static void
update_screen() {
    recalc_buffers();
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            SDL_SetRenderDrawColor(renderer, buffer_red[x * HEIGHT + y], buffer_green[x * HEIGHT + y],
                                   buffer_blue[x * HEIGHT + y], SDL_ALPHA_OPAQUE);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }

    SDL_RenderPresent(renderer);
}

static void
recalc_buffers() {
    add_disturbance();
    recalc_buffer(&buffer_red);
    recalc_buffer(&buffer_green);
    recalc_buffer(&buffer_blue);
}

static void
add_disturbance() {
    for (int i = 0; i < 10; i++) {
        int dist_x = rand() % WIDTH;
        int dist_y = rand() % HEIGHT;
        int offset = OFFSET(dist_x, dist_y);

        buffer_red[offset] = rand() % 256;
        buffer_green[offset] = rand() % 256;
        buffer_blue[offset] = rand() % 256;
    }
}

static void
recalc_buffer(Uint8 **old_buffer_ptr) {
    Uint8 *new_buffer = ALLOC_BUFFER;
    Uint8 *old_buffer = *old_buffer_ptr;

    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            int offset = OFFSET(x, y);

            Uint16 new_value = old_buffer[offset];
            Uint16 count = 1;
            if (x > 0) {
                new_value += old_buffer[offset - 1];
                count++;
                if (y > 0) {
                    new_value += old_buffer[offset - WIDTH - 1];
                    count++;
                }
                if (y < HEIGHT - 1) {
                    new_value += old_buffer[offset + WIDTH - 1];
                    count++;
                }
            }
            if (x < WIDTH - 1) {
                new_value += old_buffer[offset + 1];
                count++;
                if (y > 0) {
                    new_value += old_buffer[offset - WIDTH + 1];
                    count++;
                }
                if (y < HEIGHT - 1) {
                    new_value += old_buffer[offset + WIDTH + 1];
                    count++;
                }
            }
            if (y > 0) {
                new_value += old_buffer[offset - WIDTH];
                count++;
            }
            if (y < HEIGHT - 1) {
                new_value += old_buffer[offset + WIDTH];
                count++;
            }
            new_buffer[offset] = new_value / 8;
        }
    }

    free(*old_buffer_ptr);
    *old_buffer_ptr = new_buffer;
}

static bool
initialize_app() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize SDL: %s\n", SDL_GetError());
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
    buffer_red = ALLOC_BUFFER;
    buffer_green = ALLOC_BUFFER;
    buffer_blue = ALLOC_BUFFER;
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            int offset = OFFSET(x, y);
            buffer_red[offset] = rand() % 256;
            buffer_green[offset] = rand() % 256;
            buffer_blue[offset] = rand() % 256;
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