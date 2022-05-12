#include "shader.h"

static unsigned int
load_shader_file(unsigned int shader_type, const char *file_name) {
    unsigned int id = glCreateShader(shader_type);
    const char *src = load_text_file(file_name);
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
        glDeleteShader(id);
        SDL_Die("Failed to compile shader %s: %s", file_name, msg);
        exit(1);
    }
    return id;
}

shader_t *
shader_load(const char *vertex_shader_name, const char *fragment_shader_name) {
    unsigned int program = glCreateProgram();
    unsigned vertex_shader = load_shader_file(GL_VERTEX_SHADER, vertex_shader_name);
    unsigned fragment_shader = load_shader_file(GL_FRAGMENT_SHADER, fragment_shader_name);

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint program_linked;
    glGetProgramiv(program, GL_LINK_STATUS, &program_linked);
    if (program_linked != GL_TRUE) {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &log_length, message);
        glDeleteProgram(program);
        SDL_Die("Error linking program: %s", message);
        exit(1);
    }
    glValidateProgram(program);
    GLint program_validated;
    glGetProgramiv(program, GL_VALIDATE_STATUS, &program_validated);
    if (program_validated != GL_TRUE) {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &log_length, message);
        glDeleteProgram(program);
        SDL_Die("Error validating program: %s", message);
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Created shader from: %s and %s", vertex_shader_name,
                fragment_shader_name);

    shader_t *shader = calloc(1, sizeof(shader_t));
    SDL_ALLOC_CHECK(shader)
    shader->id = program;

    shader->vertex_shader_name = malloc(strlen(vertex_shader_name) + 1);
    SDL_ALLOC_CHECK(shader->vertex_shader_name)
    strcpy(shader->vertex_shader_name, vertex_shader_name);

    shader->fragment_shader_name = malloc(strlen(fragment_shader_name) + 1);
    SDL_ALLOC_CHECK(shader->fragment_shader_name)
    strcpy(shader->fragment_shader_name, fragment_shader_name);

    return shader;
}

void
shader_destroy(shader_t *shader) {
    if (shader->vertex_shader_name) {
        free(shader->vertex_shader_name);
        shader->vertex_shader_name = NULL;
    }
    if (shader->fragment_shader_name) {
        free(shader->fragment_shader_name);
        shader->fragment_shader_name = NULL;
    }
    free(shader);
}

void
shader_use(shader_t *shader) {
    glUseProgram(shader->id);
}

static GLint
uniform_name(shader_t *shader, const char *name) {
    GLint uniformLocation = glGetUniformLocation(shader->id, name);
    if (uniformLocation < 0 && SDL_DEBUG_ENABLED) {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Could not find the uniform %s in shader built from %s and %s",
                     name,
                     shader->vertex_shader_name,
                     shader->fragment_shader_name);
    }
    GL_CHECK_ERROR;
    return uniformLocation;
}

void
shader_set_mat4(shader_t *shader, const char *name, mat4 value) {
    glUniformMatrix4fv(uniform_name(shader, name), 1, GL_FALSE, (GLfloat *) value);
    GL_CHECK_ERROR;
}

void
shader_set_mat3(shader_t *shader, const char *name, mat3 value) {
    glUniformMatrix3fv(uniform_name(shader, name), 1, GL_FALSE, (GLfloat *) value);
    GL_CHECK_ERROR;
}

void
shader_set_vec3(shader_t *shader, const char *name, vec3 value) {
    glUniform3f(uniform_name(shader, name), value[0], value[1], value[2]);
    GL_CHECK_ERROR;
}

void
shader_set_float(shader_t *shader, const char *name, float value) {
    glUniform1f(uniform_name(shader, name), value);
    GL_CHECK_ERROR;
}

void
shader_set_int(shader_t *shader, const char *name, int value) {
    glUniform1i(uniform_name(shader, name), value);
    GL_CHECK_ERROR;
}

