#include <SDL2/SDL.h>
#include <stdbool.h>
#include <time.h>
#include <GL/gl.h>
#include <GLES3/gl3.h>
#include "opengl/cglm_ext.h"
#include "cglm/cglm.h"
#include "opengl/camera.h"
#include "opengl/texture.h"
#include "opengl/shader.h"
#include "models/cube.h"

static const int WIDTH = 1280;
static const int HEIGHT = WIDTH / 16 * 9;

static const Uint32 FPS = 30;
static const Uint32 FPS_SIZE_MS = 1000 / FPS;

static unsigned int cube_shader;
static unsigned int cube_vao;
static unsigned int cube_vbo;
static int cube_model_location;
static int cube_normals_model_location;
static int cube_project_location;
static int cube_light_color_location;
static int cube_light_pos_location;
static camera_t camera;
static mat4 model1_m = GLM_MAT4_IDENTITY;
static mat4 model2_m = GLM_MAT4_IDENTITY;
static mat4 model3_m = GLM_MAT4_IDENTITY;
static mat4 model4_m = GLM_MAT4_IDENTITY;

static mat4 light_m = GLM_MAT4_IDENTITY;
static vec3 light_scale = {0.2f, 0.2f, 0.2f};
static vec3 light_pos = {0.0f, -3.0f, 0.0f};
static vec3 light_color = {0.95f, 0.95f, 0.95f};
static int light_model_location;
static int light_project_location;
static int light_color_location;
static unsigned int light_shader;
static unsigned int light_vao;

static mat4 view_m = GLM_MAT4_IDENTITY;
static mat4 project_m = GLM_MAT4_IDENTITY;
static struct cube_object {
    vec3 angles;
} cube_object;

#define ANGLES_OFFSET 0
#define COLOR_OFFSET (ANGLES_OFFSET + VEC3_SIZE)

static struct textured_cube {
    cube_t model;
    cube_textures_t textures;
    cube_t normals; // this is expensive, we should have a normal vector for each vertex from model
} cube_model;

#define CUBE_OFFSET 0
#define CUBE_TEXTURE_OFFSET (CUBE_OFFSET + CUBE_SIZE)
#define CUBE_NORMALS_OFFSET (CUBE_TEXTURE_OFFSET + CUBE_TEXTURES_SIZE)

#define CUBE_VERTEX_ATTRIBUTE_ID 0
#define CUBE_TEXTURE_VERTEX_ATTRIBUTE_ID 1
#define CUBE_NORMALS_ATTRIBUTE_ID 2

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
    event_loop();
}


static void
event_loop() {
    SDL_Event event;
    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return;
            } else if (event.type == SDL_MOUSEMOTION && event.motion.state & SDL_BUTTON_RMASK) {
                move_camera_sight(&camera, event.motion.xrel, event.motion.yrel);
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_a: // move camera right according to right vector
                        move_camera_horizontally(&camera, -1);
                        break;
                    case SDLK_d: // move camera left according to up vector
                        move_camera_horizontally(&camera, 1);
                        break;
                    case SDLK_r: // move camera up according to up vector
                        move_camera_vertically(&camera, 1);
                        break;
                    case SDLK_f: // move camera down according to up vector
                        move_camera_vertically(&camera, -1);
                        break;
                    case SDLK_w: // move camera forward
                        move_camera_front(&camera, 1);
                        break;
                    case SDLK_s: // move camera backward
                        move_camera_front(&camera, -1);
                        break;
                    case SDLK_z: // reset camera position
                        camera_init(&camera);
                        break;
                    default:
                        break;
                }
            }
        }
        update_screen();
        SDL_Delay(FPS_SIZE_MS);
    }
}

static void
draw_light() {
    glBindVertexArray(light_vao);
    glUseProgram(light_shader);

    mat4 project_view = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_mul(project_m, view_m, project_view);
    glUniform3f(light_color_location, light_color[0], light_color[1], light_color[2]);
    glUniformMatrix4fv(light_project_location, 1, GL_FALSE, (GLfloat *) project_view);
    glUniformMatrix4fv(light_model_location, 1, GL_FALSE, (GLfloat *) light_m);

    glDrawArrays(GL_QUADS, 0, 6 * 4);
}

static void
draw_cube(mat4 model) {
    mat4 normals_model4;
    mat3 normals_model3;

    glm_mat4_inv(model, normals_model4);
    glm_mat4_transpose(normals_model4);
    glm_mat4_pick3(normals_model4, normals_model3);

    glUniformMatrix3fv(cube_normals_model_location, 1, GL_FALSE, (GLfloat *) normals_model3);
    glUniformMatrix4fv(cube_model_location, 1, GL_FALSE, (GLfloat *) model);
    glDrawArrays(GL_QUADS, 0, 6 * 4);
}

static void
draw_cubes() {
    glBindVertexArray(cube_vao);
    glUseProgram(cube_shader);

    glUniform3f(cube_light_color_location, light_color[0], light_color[1], light_color[2]);
    glUniform3f(cube_light_pos_location, light_pos[0], light_pos[1], light_pos[2]);

    mat4 project_view = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_mul(project_m, view_m, project_view);
    glUniformMatrix4fv(cube_project_location, 1, GL_FALSE, (GLfloat *) project_view);

    draw_cube(model1_m);
    draw_cube(model2_m);
    draw_cube(model3_m);
    draw_cube(model4_m);
}

static void draw_scene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_cubes();
    draw_light();

    glFlush();
}

static void
update_light() {
    double base_value = ((double) (SDL_GetTicks() % 5000)) * 2 * M_PI / 5000;

/*
    light_color[0] = (float) sin(base_value) / 2 + 0.5f;;
    light_color[1] = (float) sin(base_value + 2 * M_PI / 3) / 2 + 0.5f;
    light_color[2] = (float) sin(base_value + M_PI / 3) / 2 + 0.5f;
*/

    light_color[0] = 1;
    light_color[1] = 1;
    light_color[2] = 1;

    glm_mat4_identity(light_m);
    glm_scale(light_m, light_scale);
    glm_translate(light_m, light_pos);
}

static void update_cube(float size, const float *trans, const float *angles, vec4 (*model)) {
    vec3 scale4 = {size, size, size};
    glm_mat4_identity(model);
    glm_translate_x(model, trans[0]);
    glm_translate_y(model, trans[1]);
    glm_translate_z(model, trans[2]);
    glm_scale(model, scale4);
    glm_rotate_x(model, angles[0], model);
    glm_rotate_y(model, angles[1], model);
    glm_rotate_z(model, angles[2], model);
}

static void
update_cubes() {

    // rotating
    cube_object.angles[0] += 0.01f;
    cube_object.angles[1] += 0.012f;
    cube_object.angles[2] += 0.013f;

    // creating identity matrix
    glm_mat4_identity(model1_m);
    glm_translate_x(model1_m, -1.0f);
    glm_translate_y(model1_m, -1.0f);
    glm_translate_z(model1_m, -1.0f);
    glm_rotate_x(model1_m, -cube_object.angles[0], model1_m);
    glm_rotate_y(model1_m, -cube_object.angles[1], model1_m);

    float size;
    float *angles = cube_object.angles;

    size = 2.0f;
    update_cube(size, (vec3) {-size, -size, -size}, (vec3) {angles[0], angles[1], 0}, model1_m);
    size = 4.0f;
    update_cube(size, (vec3) {-size, size, -size}, (vec3) {0, angles[1], angles[2]}, model2_m);
    size = 8.0f;
    update_cube(size, (vec3) {size, size, -size}, (vec3) {angles[0], 0, angles[2]}, model3_m);
    size = 16.0f;
    update_cube(size, (vec3) {size, -size, -size}, angles, model4_m);
}

static void
update_scene() {

    update_cubes();
    update_light();

    // View
    camera_view(&camera, view_m);

    // projection
    glm_perspective(M_PI_4, (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f, project_m);
}

static void
update_screen() {
    update_scene();
    draw_scene();
    SDL_GL_SwapWindow(window);
}

static
void initialize_gl_light() {
    glGenVertexArrays(1, &light_vao);
    glBindVertexArray(light_vao);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);

    glEnableVertexAttribArray(CUBE_VERTEX_ATTRIBUTE_ID);
    glVertexAttribPointer(CUBE_VERTEX_ATTRIBUTE_ID, 3, GL_FLOAT, GL_FALSE, 0, (void *) CUBE_OFFSET);

    light_shader = create_shader("shaders/light_vertex.glsl", "shaders/light_fragment.glsl");
    light_color_location = glGetUniformLocation(light_shader, "passed_color");
    light_model_location = glGetUniformLocation(light_shader, "model");
    light_project_location = glGetUniformLocation(light_shader, "project_view");
}

static
void initialize_gl_cube() {
    glGenVertexArrays(1, &cube_vao);
    glBindVertexArray(cube_vao);

    glGenBuffers(1, &cube_vbo); // creating VBO
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo); // selecting buffer of particular type
    glBufferData(GL_ARRAY_BUFFER, sizeof cube_model, &cube_model, GL_STATIC_DRAW); // copying data

    glEnableVertexAttribArray(CUBE_VERTEX_ATTRIBUTE_ID);
    glVertexAttribPointer(CUBE_VERTEX_ATTRIBUTE_ID, 3, GL_FLOAT, GL_FALSE, 0, (void *) CUBE_OFFSET);

    glEnableVertexAttribArray(CUBE_TEXTURE_VERTEX_ATTRIBUTE_ID);
    glVertexAttribPointer(CUBE_TEXTURE_VERTEX_ATTRIBUTE_ID, 2, GL_FLOAT, GL_FALSE, 0,
                          (void *) CUBE_TEXTURE_OFFSET);

    glEnableVertexAttribArray(CUBE_NORMALS_ATTRIBUTE_ID);
    glVertexAttribPointer(CUBE_NORMALS_ATTRIBUTE_ID, 3, GL_FLOAT, GL_FALSE, 0, (void *) CUBE_NORMALS_OFFSET);

    cube_shader = create_shader("shaders/vertex.glsl", "shaders/fragment.glsl");

    cube_light_pos_location = glGetUniformLocation(cube_shader, "light_pos");
    cube_light_color_location = glGetUniformLocation(cube_shader, "light_color");
    cube_model_location = glGetUniformLocation(cube_shader, "model");
    cube_normals_model_location = glGetUniformLocation(cube_shader, "normals_model");
    cube_project_location = glGetUniformLocation(cube_shader, "project_view");

    // generating texture
    load_texture(GL_TEXTURE0, "texture1.png");
    load_texture(GL_TEXTURE1, "texture2.png");
}

static void
initialize_gl() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "OpenGL:\nVendor: %s\nRenderer: %s\nVersion: %s\nExtensions: %s",
                glGetString(GL_VENDOR),
                glGetString(GL_RENDER),
                glGetString(GL_VERSION),
                glGetString(GL_EXTENSIONS)
    );
    glEnable(GL_DEPTH_TEST); // enables z-buffering
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // background color
//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // cube
    initialize_gl_cube();
    initialize_gl_light();
}


static void initialize_data() {
    camera_init(&camera);
    vec3_set(cube_object.angles, 0, 0, 0);
    cube_model_init(&cube_model.model);
    cube_textures_init(&cube_model.textures);
    cube_normals_init(&cube_model.normals);
}

static bool
initialize_app() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize SDL: %s\n", SDL_GetError());
        exit(1);
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