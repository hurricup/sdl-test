#include <SDL2/SDL.h>
#include <stdbool.h>
#include <time.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GLES2/gl2.h>
#include "data.h"

static const int WIDTH = 640;
static const int HEIGHT = 640;

static const Uint32 FPS = 30;
static const Uint32 FPS_SIZE_MS = 1000 / FPS;

static unsigned int cube_buffer;
static struct {
    vec3_t angles;
    vec3_t depth;
    axes_t axes;
    color_t color;
    cube_t cube;
} cube_data;
#define ANGLES_OFFSET 0
#define DEPTH_OFFSET (ANGLES_OFFSET + VEC3_SIZE)
#define AXES_OFFSET (DEPTH_OFFSET + VEC3_SIZE)
#define COLOR_OFFSET (AXES_OFFSET + AXES_SIZE)
#define CUBE_OFFSET (COLOR_OFFSET + COLOR_SIZE)
#define CUBE_VERTEX_ATTRIBUTE_ID 0

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static void recalc_buffer(Uint8 **old_buffer);

static bool initialize_app();

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

static void draw_scene() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glFlush();
}

static void
update_scene() {
    cube_data.angles.x -= 1;
    cube_data.angles.y -= 1;
    cube_data.angles.z -= 1;
}

static void
update_screen() {
    update_scene();
    draw_scene();
    SDL_GL_SwapWindow(window);
}

static void
initialize_gl() {
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // background color
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST); // enable depth test?
    glShadeModel(GL_SMOOTH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f); // set up perspective
    glMatrixMode(GL_MODELVIEW); // 3d mode

    glGenBuffers(1, &cube_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, cube_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof cube_data, &cube_data, GL_STATIC_DRAW);

    glVertexAttribPointer(CUBE_VERTEX_ATTRIBUTE_ID, 3, GL_FLOAT, GL_FALSE, POINT3_SIZE, (void *) CUBE_OFFSET);
    glEnableVertexAttribArray(CUBE_VERTEX_ATTRIBUTE_ID);

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "OpenGL:\nVendor: %s\nRenderer: %s\nVersion: %s\nExtensions: %s",
                glGetString(GL_VENDOR),
                glGetString(GL_RENDER),
                glGetString(GL_VERSION),
                glGetString(GL_EXTENSIONS)
    );
}

static void initialize_data() {
    set_point3(cube_data.angles, 0, 0, 0);
    set_point3(cube_data.depth, 0, 0, -7);
    set_axes(cube_data.axes,
             1, 0, 0,
             0, 1, 0,
             0, 0, 1);
    set_color(cube_data.color, 1, 0, 0);
    set_square(cube_data.cube.side_a,
               1.0f, 1.0f, 1.0f,
               -1.0f, 1.0f, 1.0f,
               -1.0f, -1.0f, 1.0f,
               1.0f, -1.0f, 1.0f);

    set_square(cube_data.cube.side_b,
               1.0f, -1.0f, -1.0f,
               -1.0f, -1.0f, -1.0f,
               -1.0f, 1.0f, -1.0f,
               1.0f, 1.0f, -1.0f);
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

    initialize_data();
    initialize_gl();

    return true;
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