#include "shader.h"

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

unsigned int
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
