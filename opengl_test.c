#include <SDL2/SDL.h>
#include <stdbool.h>
#include <time.h>
#include <GL/gl.h>
#include <GLES3/gl3.h>
#include "data.h"
#include <errno.h>
#include "cglm/cglm.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

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
static const float camera_speed = 0.2f;
static const float camera_speed_y = 0.2f;
static const float camera_speed_x = 0.2f;
static const float mouse_sensitivity = 0.001f;
static vec3 camera_pos_default = {0.0f, 0.0f, 10.0f};
static vec3 camera_pos = {0.0f, 0.0f, 10.0f};
static vec3 camera_up_default = {0.0f, 1.0f, 0.0f};
static vec3 camera_up = {0.0f, 1.0f, 0.0f};
static vec3 camera_front_default = {0.0f, 0.0f, -1.0f};
static vec3 camera_front = {0.0f, 0.0f, -1.0f};
static mat4 model_m = GLM_MAT4_IDENTITY;
static mat4 model2_m = GLM_MAT4_IDENTITY;
static mat4 model3_m = GLM_MAT4_IDENTITY;
static mat4 model4_m = GLM_MAT4_IDENTITY;
static mat4 view_m = GLM_MAT4_IDENTITY;
static mat4 project_m = GLM_MAT4_IDENTITY;
static struct {
    vec3_t angles;
    vec3_t depth;
    axes_t axes;
    color_t color;
    cube_t cube;
    cube_t cube_texture;
} cube_data;
#define ANGLES_OFFSET 0
#define DEPTH_OFFSET (ANGLES_OFFSET + VEC3_SIZE)
#define AXES_OFFSET (DEPTH_OFFSET + VEC3_SIZE)
#define COLOR_OFFSET (AXES_OFFSET + AXES_SIZE)
#define CUBE_OFFSET (COLOR_OFFSET + COLOR_SIZE)
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
reset_camera() {
    glm_vec3_copy(camera_front_default, camera_front);
    glm_vec3_copy(camera_pos_default, camera_pos);
    glm_vec3_copy(camera_up_default, camera_up);
};

static void
move_camera_vertically(float sign) {
    vec3 up = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(camera_up, sign * camera_speed_y, up);
    glm_vec3_add(camera_pos, up, camera_pos);
}

static void
move_camera_horizontally(float sign) {
    vec3 right = GLM_VEC3_ZERO_INIT;
    glm_vec3_cross(camera_front, camera_up, right);
    glm_normalize(right);
    glm_vec3_scale(right, sign * camera_speed_x, right);
    glm_vec3_add(camera_pos, right, camera_pos);
}

static void move_camera_front(float sign) {
    vec3 delta_front = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(camera_front, camera_speed * (float) sign, delta_front);
    glm_vec3_add(camera_pos, delta_front, camera_pos);
}

static void
move_camera_sight(int x, int y) {
    if (x == 0 && y == 0) {
        return;
    }
    vec3 delta_up = GLM_VEC3_ZERO_INIT;
    vec3 right = GLM_VEC3_ZERO_INIT;
    vec3 delta_right = GLM_VEC3_ZERO_INIT;

    glm_vec3_cross(camera_front, camera_up, right);
    glm_normalize(right);

    glm_vec3_scale(camera_up, -(float) y * mouse_sensitivity, delta_up);
    glm_vec3_scale(right, (float) x * mouse_sensitivity, delta_right);

    glm_vec3_add(camera_front, delta_right, camera_front);
    glm_vec3_cross(camera_front, camera_up, right);
    glm_vec3_add(camera_front, delta_up, camera_front);
    glm_vec3_cross(right, camera_front, camera_up);

    glm_normalize(camera_front);
    glm_normalize(camera_up);
}

static void
event_loop() {
    SDL_Event event;
    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return;
            } else if (event.type == SDL_MOUSEMOTION && event.motion.state & SDL_BUTTON_RMASK) {
                move_camera_sight(event.motion.xrel, event.motion.yrel);
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_a: // move camera right according to right vector
                        move_camera_horizontally(-1);
                        break;
                    case SDLK_d: // move camera left according to up vector
                        move_camera_horizontally(1);
                        break;
                    case SDLK_r: // move camera up according to up vector
                        move_camera_vertically(1);
                        break;
                    case SDLK_f: // move camera down according to up vector
                        move_camera_vertically(-1);
                        break;
                    case SDLK_w: // move camera forward
                        move_camera_front(1);
                        break;
                    case SDLK_s: // move camera backward
                        move_camera_front(-1);
                        break;
                    case SDLK_z: // reset camera position
                        reset_camera();
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
    color_t cube_color = cube_data.color;
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
    cube_data.color.red = (float) sin(base_value) / 2 + 0.5f;
    cube_data.color.green = (float) sin(base_value + 2 * M_PI / 3) / 2 + 0.5f;
    cube_data.color.blue = (float) sin(base_value + M_PI / 3) / 2 + 0.5f;

    // rotating
    cube_data.angles.x += 0.01f;
    cube_data.angles.y += 0.012f;
    cube_data.angles.z += 0.013f;

    // creating identity matrix
    glm_mat4_identity(model_m);

    glm_mat4_identity(model2_m);
    glm_translate_x(model2_m, 3.0f);
    glm_rotate_y(model2_m, cube_data.angles.x, model2_m);

    glm_mat4_identity(model3_m);
    glm_translate_y(model3_m, 3.0f);
    glm_rotate_z(model3_m, cube_data.angles.y, model3_m);

    glm_mat4_identity(model4_m);
    glm_translate_z(model4_m, 3.0f);
    glm_rotate_x(model4_m, cube_data.angles.z, model4_m);

    // View
    glm_look(camera_pos, camera_front, camera_up, view_m);

    // projection
    glm_perspective(M_PI_4, (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f, project_m);
}

static void
update_screen() {
    update_scene();
    draw_scene();
    SDL_GL_SwapWindow(window);
}

static void load_texture(GLenum texture_unit, const char *filename) {
    int width, height, channels_number;

    unsigned int texture;
    glGenTextures(1, &texture);
    glActiveTexture(texture_unit);
    glBindTexture(GL_TEXTURE_2D, texture);

    // configure texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // creating from external file
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filename, &width, &height, &channels_number, 0);
    if (data == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error loading texture");
    } else {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                    "Loaded %s: %u x %u, channels %u", filename, width, height, channels_number);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);
}

static void
initialize_gl() {
    glEnable(GL_DEPTH_TEST); // enables z-buffering

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // background color

    glGenVertexArrays(1, &cube_vao); // creating vertex arrays, pretty useless now, but still
    glBindVertexArray(cube_vao);

    glGenBuffers(1, &cube_vbo); // creating VBO
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo); // selecting buffer of particular type
    glBufferData(GL_ARRAY_BUFFER, sizeof cube_data, &cube_data, GL_STATIC_DRAW); // copying data

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
    set_point3(&cube_data.angles, 0, 0, 0);
    set_point3(&cube_data.depth, 0, 0, -7);
    set_axes(&cube_data.axes,
             1, 0, 0,
             0, 1, 0,
             0, 0, 1);
    set_color(&cube_data.color, 0, 0, 0);
    set_square(&cube_data.cube.side_a,
               0.5f, 0.5f, 0.5f,
               0.5f, -0.5f, 0.5f,
               -0.5f, -0.5f, 0.5f,
               -0.5f, 0.5f, 0.5f);

    set_square(&cube_data.cube.side_b,
               0.5f, 0.5f, -0.5f,
               0.5f, -0.5f, -0.5f,
               -0.5f, -0.5f, -0.5f,
               -0.5f, 0.5f, -0.5f);

    set_square(&cube_data.cube_texture.side_a,
               1.0f, 1.0f, 0.0f,
               1.0f, 0.0f, 0.0f,
               0.0f, 0.0f, 0.0f,
               0.0f, 1.0f, 0.0f);

    // this is most likely be wrong, because of index buffer
    set_square(&cube_data.cube_texture.side_b,
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


static char *
load_text_file(const char *shader_name) {
    char *buffer;
    long length;
    FILE *file = fopen(shader_name, "rb");

    if (file) {
        fseek(file, 0, SEEK_END);
        length = ftell(file);
        fseek(file, 0, SEEK_SET);
        buffer = malloc(length + 1);
        if (buffer) {
            fread(buffer, 1, length, file);
        }
        fclose(file);
        buffer[length] = '\0';
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error reading file: %s, errno: %d", shader_name, errno);
        exit(1);
    }
    return buffer;
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