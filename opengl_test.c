#include <SDL2/SDL.h>
#include <stdbool.h>
#include <time.h>
#include <GL/gl.h>
#include <GL/glu.h>

static const int WIDTH = 640;
static const int HEIGHT = 640;

static const Uint32 FPS = 30;
static const Uint32 FPS_SIZE_MS = 1000 / FPS;

static float xrf = 0, yrf = 0, zrf = 0;

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

static void draw_gl() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -7.0f);    // Сдвинуть вглубь экрана

    glRotatef(xrf, 1.0f, 0.0f, 0.0f);    // Вращение куба по X, Y, Z
    glRotatef(yrf, 0.0f, 1.0f, 0.0f);    // Вращение куба по X, Y, Z
    glRotatef(zrf, 0.0f, 0.0f, 1.0f);    // Вращение куба по X, Y, Z

    glBegin(GL_QUADS);        // Рисуем куб

    glColor3f(0.0f, 1.0f, 0.0f);        // Синяя сторона (Верхняя)
    glVertex3f(1.0f, 1.0f, -1.0f);        // Верхний правый угол квадрата
    glVertex3f(-1.0f, 1.0f, -1.0f);        // Верхний левый
    glVertex3f(-1.0f, 1.0f, 1.0f);        // Нижний левый
    glVertex3f(1.0f, 1.0f, 1.0f);        // Нижний правый

    glColor3f(1.0f, 0.5f, 0.0f);        // Оранжевая сторона (Нижняя)
    glVertex3f(1.0f, -1.0f, 1.0f);    // Верхний правый угол квадрата
    glVertex3f(-1.0f, -1.0f, 1.0f);    // Верхний левый
    glVertex3f(-1.0f, -1.0f, -1.0f);    // Нижний левый
    glVertex3f(1.0f, -1.0f, -1.0f);    // Нижний правый

    glColor3f(1.0f, 0.0f, 0.0f);        // Красная сторона (Передняя)
    glVertex3f(1.0f, 1.0f, 1.0f);        // Верхний правый угол квадрата
    glVertex3f(-1.0f, 1.0f, 1.0f);        // Верхний левый
    glVertex3f(-1.0f, -1.0f, 1.0f);        // Нижний левый
    glVertex3f(1.0f, -1.0f, 1.0f);        // Нижний правый

    glColor3f(1.0f, 1.0f, 0.0f);            // Желтая сторона (Задняя)
    glVertex3f(1.0f, -1.0f, -1.0f);    // Верхний правый угол квадрата
    glVertex3f(-1.0f, -1.0f, -1.0f);    // Верхний левый
    glVertex3f(-1.0f, 1.0f, -1.0f);    // Нижний левый
    glVertex3f(1.0f, 1.0f, -1.0f);    // Нижний правый

    glColor3f(0.0f, 0.0f, 1.0f);            // Синяя сторона (Левая)
    glVertex3f(-1.0f, 1.0f, 1.0f);    // Верхний правый угол квадрата
    glVertex3f(-1.0f, 1.0f, -1.0f);    // Верхний левый
    glVertex3f(-1.0f, -1.0f, -1.0f);    // Нижний левый
    glVertex3f(-1.0f, -1.0f, 1.0f);    // Нижний правый

    glColor3f(1.0f, 0.0f, 1.0f);            // Фиолетовая сторона (Правая)
    glVertex3f(1.0f, 1.0f, -1.0f);    // Верхний правый угол квадрата
    glVertex3f(1.0f, 1.0f, 1.0f);    // Верхний левый
    glVertex3f(1.0f, -1.0f, 1.0f);    // Нижний левый
    glVertex3f(1.0f, -1.0f, -1.0f);    // Нижний правый

    glEnd();    // Закончили квадраты
}

static void
update_screen() {
    xrf -= 1;
    yrf -= 1;
    zrf -= 1;

    draw_gl();
    glFlush();
    SDL_GL_SwapWindow(window);
}

static void
initialize_gl() {
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // устанавливаем фоновый цвет на черный
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST); // включаем тест глубины
    glShadeModel(GL_SMOOTH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f); // настраиваем трехмерную перспективу
    glMatrixMode(GL_MODELVIEW); // переходим в трехмерный режим
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