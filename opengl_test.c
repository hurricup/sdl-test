#include <SDL2/SDL.h>
#include <stdbool.h>
#include <time.h>
#include <GL/gl.h>
#include <GLES3/gl3.h>
#include "data.h"
#include "cglm/cglm.h"
#include "opengl/camera.h"
#include "opengl/file_util.h"
#include "opengl/texture.h"

static const int WIDTH = 1280;
static const int HEIGHT = WIDTH / 16 * 9;

static const Uint32 FPS = 30;
static const Uint32 FPS_SIZE_MS = 1000 / FPS;

static unsigned int cube_vao;
static unsigned int cube_vbo;
static unsigned int cube_ebo;
uivec4_t cube_sides[] = {
        {0, 1, 2, 3},
        {2, 6, 7, 3},
        {0, 4, 5, 1},
        {4, 5, 6, 7},
        {1, 5, 6, 2},
        {3, 7, 4, 0}
};
static int model_location;
static int project_view_location;
static int shader_color_location;
static camera_t camera;
static mat4 model_m = GLM_MAT4_IDENTITY;
static mat4 model2_m = GLM_MAT4_IDENTITY;
static mat4 model3_m = GLM_MAT4_IDENTITY;
static mat4 model4_m = GLM_MAT4_IDENTITY;
static mat4 view_m = GLM_MAT4_IDENTITY;
static mat4 project_m = GLM_MAT4_IDENTITY;
static struct cube_object {
    vec3_t angles;
    color_t color;
} cube_object;

#define ANGLES_OFFSET 0
#define COLOR_OFFSET (ANGLES_OFFSET + VEC3_SIZE)

static struct {
    cube_t cube;
    cube_t cube_texture;
} cube_model;
#define CUBE_OFFSET 0
#define CUBE_TEXTURE_OFFSET (CUBE_OFFSET + CUBE_SIZE)
#define CUBE_VERTEX_ATTRIBUTE_ID 0
#define CUBE_TEXTURE_VERTEX_ATTRIBUTE_ID 1

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static unsigned int create_shader(const char *vertex_shader_name, const char *fragment_shader_name);

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

static void draw_scene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(cube_vao);
    color_t cube_color = cube_object.color;
    glUniform3f(shader_color_location, cube_color.red, cube_color.green, cube_color.blue);
    mat4 project_view = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_mul(project_m, view_m, project_view);
    glUniformMatrix4fv(project_view_location, 1, GL_FALSE, (GLfloat *) project_view);
    glUniformMatrix4fv(model_location, 1, GL_FALSE, (GLfloat *) model_m);
    glDrawElements(GL_QUADS, 6 * 4, GL_UNSIGNED_INT, 0);

    glUniformMatrix4fv(model_location, 1, GL_FALSE, (GLfloat *) model2_m);
    glDrawElements(GL_QUADS, 6 * 4, GL_UNSIGNED_INT, 0);

    glUniformMatrix4fv(model_location, 1, GL_FALSE, (GLfloat *) model3_m);
    glDrawElements(GL_QUADS, 6 * 4, GL_UNSIGNED_INT, 0);

    glUniformMatrix4fv(model_location, 1, GL_FALSE, (GLfloat *) model4_m);
    glDrawElements(GL_QUADS, 6 * 4, GL_UNSIGNED_INT, 0);
    glFlush();
}

static void
update_scene() {

    double base_value = ((double) (SDL_GetTicks() % 5000)) * 2 * M_PI / 5000;
    cube_object.color.red = (float) sin(base_value) / 2 + 0.5f;
    cube_object.color.green = (float) sin(base_value + 2 * M_PI / 3) / 2 + 0.5f;
    cube_object.color.blue = (float) sin(base_value + M_PI / 3) / 2 + 0.5f;

    // rotating
    cube_object.angles.x += 0.01f;
    cube_object.angles.y += 0.012f;
    cube_object.angles.z += 0.013f;

    // creating identity matrix
    glm_mat4_identity(model_m);

    glm_mat4_identity(model2_m);
    glm_translate_x(model2_m, 3.0f);
    glm_rotate_y(model2_m, cube_object.angles.x, model2_m);

    glm_mat4_identity(model3_m);
    glm_translate_y(model3_m, 3.0f);
    glm_rotate_z(model3_m, cube_object.angles.y, model3_m);

    glm_mat4_identity(model4_m);
    glm_translate_z(model4_m, 3.0f);
    glm_rotate_x(model4_m, cube_object.angles.z, model4_m);

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

static void
initialize_gl() {
    glEnable(GL_DEPTH_TEST); // enables z-buffering

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // background color

    glGenVertexArrays(1, &cube_vao); // creating vertex arrays, pretty useless now, but still
    glBindVertexArray(cube_vao);

    glGenBuffers(1, &cube_vbo); // creating VBO
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo); // selecting buffer of particular type
    glBufferData(GL_ARRAY_BUFFER, sizeof cube_model, &cube_model, GL_STATIC_DRAW); // copying data

    glGenBuffers(1, &cube_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_sides), cube_sides, GL_STATIC_DRAW);

//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glEnableVertexAttribArray(CUBE_VERTEX_ATTRIBUTE_ID);
    glVertexAttribPointer(CUBE_VERTEX_ATTRIBUTE_ID, 3, GL_FLOAT, GL_FALSE, 0, (void *) CUBE_OFFSET);

    glEnableVertexAttribArray(CUBE_TEXTURE_VERTEX_ATTRIBUTE_ID);
    glVertexAttribPointer(CUBE_TEXTURE_VERTEX_ATTRIBUTE_ID, 2, GL_FLOAT, GL_FALSE, POINT3_SIZE,
                          (void *) CUBE_TEXTURE_OFFSET);

    unsigned int shader = create_shader("shaders/vertex.glsl", "shaders/fragment.glsl");
    glUseProgram(shader);
    shader_color_location = glGetUniformLocation(shader, "passed_color");
    model_location = glGetUniformLocation(shader, "model_m");
    project_view_location = glGetUniformLocation(shader, "project_view_m");

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "OpenGL:\nVendor: %s\nRenderer: %s\nVersion: %s\nExtensions: %s",
                glGetString(GL_VENDOR),
                glGetString(GL_RENDER),
                glGetString(GL_VERSION),
                glGetString(GL_EXTENSIONS)
    );

    // generating texture
    load_texture(GL_TEXTURE0, "texture1.png");
    load_texture(GL_TEXTURE1, "texture2.png");
}

static void initialize_data() {
    camera_init(&camera);
    set_point3(&cube_object.angles, 0, 0, 0);
    set_color(&cube_object.color, 0, 0, 0);
    set_square(&cube_model.cube.side_a,
               0.5f, 0.5f, 0.5f,
               0.5f, -0.5f, 0.5f,
               -0.5f, -0.5f, 0.5f,
               -0.5f, 0.5f, 0.5f);

    set_square(&cube_model.cube.side_b,
               0.5f, 0.5f, -0.5f,
               0.5f, -0.5f, -0.5f,
               -0.5f, -0.5f, -0.5f,
               -0.5f, 0.5f, -0.5f);

    set_square(&cube_model.cube_texture.side_a,
               1.0f, 1.0f, 0.0f,
               1.0f, 0.0f, 0.0f,
               0.0f, 0.0f, 0.0f,
               0.0f, 1.0f, 0.0f);

    // this is most likely be wrong, because of index buffer
    set_square(&cube_model.cube_texture.side_b,
               0.0f, 1.0f, 0.0f,
               0.0f, 0.0f, 0.0f,
               1.0f, 0.0f, 0.0f,
               1.0f, 1.0f, 0.0f);
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


static unsigned int
load_shader(unsigned int shader_type, const char *shader_name) {
    unsigned int id = glCreateShader(shader_type);
    const char *src = load_text_file(shader_name);
    glShaderSource(id, 1, &src, NULL);
    glCompileShader(id);
    free((void *) src);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char *msg = alloca((length + 1) * sizeof(char));
        glGetShaderInfoLog(id, length, &length, msg);
        msg[length] = '\0';
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to compile shader %s: %s", shader_name, msg);
        glDeleteShader(id);
        exit(1);
    }
    return id;
}

static unsigned int
create_shader(const char *vertex_shader_name, const char *fragment_shader_name) {
    unsigned int program = glCreateProgram();
    unsigned vertex_shader = load_shader(GL_VERTEX_SHADER, vertex_shader_name);
    unsigned fragment_shader = load_shader(GL_FRAGMENT_SHADER, fragment_shader_name);

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint program_linked;
    glGetProgramiv(program, GL_LINK_STATUS, &program_linked);
    if (program_linked != GL_TRUE) {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &log_length, message);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error linking program: %s", message);
        glDeleteProgram(program);
        exit(1);
    }
    glValidateProgram(program);
    GLint program_validated;
    glGetProgramiv(program, GL_VALIDATE_STATUS, &program_validated);
    if (program_validated != GL_TRUE) {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &log_length, message);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error validating program: %s", message);
        glDeleteProgram(program);
        exit(1);
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
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