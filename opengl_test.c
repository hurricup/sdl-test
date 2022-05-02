#include <SDL2/SDL.h>
#include <stdbool.h>
#include <time.h>
#include <GL/gl.h>
#include <GLES3/gl3.h>
#include "data.h"
#include <errno.h>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

static const int WIDTH = 640;
static const int HEIGHT = 640;

static const Uint32 FPS = 30;
static const Uint32 FPS_SIZE_MS = 1000 / FPS;

static unsigned int cube_vao;
static unsigned int cube_vbo;
static unsigned int cube_ebo;
uivec4_t cube_sides[] = {
        {0, 1, 2, 3},
        {4, 5, 6, 7},
        {0, 4, 5, 1},
        {1, 5, 6, 2},
        {2, 6, 7, 3},
        {3, 7, 4, 0}
};
static int shader_color_location;
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
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(cube_vao);
    color_t cube_color = cube_data.color;
    glUniform3f(shader_color_location, cube_color.red, cube_color.green, cube_color.blue);
    glDrawElements(GL_QUADS, 3 * 4, GL_UNSIGNED_INT, 0);
    glFlush();
}

static void
update_scene() {
    cube_data.angles.x -= 1;
    cube_data.angles.y -= 1;
    cube_data.angles.z -= 1;

    double base_value = ((double) (SDL_GetTicks() % 5000)) * 2 * M_PI / 5000;
    cube_data.color.red = (float) sin(base_value) / 2 + 0.5f;
    cube_data.color.green = (float) sin(base_value + 2 * M_PI / 3) / 2 + 0.5f;
    cube_data.color.blue = (float) sin(base_value + M_PI / 3) / 2 + 0.5f;
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
               0.9f, 0.9f, 0.9f,
               0.9f, -0.3f, 0.9f,
               -0.3f, -0.3f, 0.9f,
               -0.3f, 0.9f, 0.9f);

    set_square(&cube_data.cube.side_b,
               0.3f, 0.3f, -0.9f,
               0.3f, -0.9f, -0.9f,
               -0.9f, -0.9f, -0.9f,
               -0.9f, 0.3f, -0.9f);

    set_square(&cube_data.cube_texture.side_a,
               1.0f, 1.0f, 0.0f,
               1.0f, 0.0f, 0.0f,
               0.0f, 0.0f, 0.0f,
               0.0f, 1.0f, 0.0f);

    // this is most likely be wrong, because of index buffer
    set_square(&cube_data.cube_texture.side_b,
               0.0f, 0.0f, 0.0f,
               0.0f, 1.0f, 0.0f,
               1.0f, 1.0f, 0.0f,
               1.0f, 0.0f, 0.0f);
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
    if (src == NULL) {
        return 0;
    }
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