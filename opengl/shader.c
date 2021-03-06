#include "shader.h"
#define UNIFORM_CACHE_CAPACITY_STEP 10
#define NAME_BUFFER_SIZE 80
#define array_item_name(template, index) ({ \
        char name[NAME_BUFFER_SIZE]; \
        sprintf(name, name_template, index); \
        name; \
    })


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
load_shader(const char *vertex_shader_name, const char *fragment_shader_name) {
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

static void
destroy_shader(shader_t *shader) {
    if (shader->vertex_shader_name) {
        free(shader->vertex_shader_name);
        shader->vertex_shader_name = NULL;
    }
    if (shader->fragment_shader_name) {
        free(shader->fragment_shader_name);
        shader->fragment_shader_name = NULL;
    }
    if (shader->id >= 0) {
        glDeleteProgram(shader->id);
    }

    for (int i = 0; i < shader->uniform_cache_items; i++) {
        free(shader->uniforms_cache[i].uniform_name);
        shader->uniforms_cache[i].uniform_name = NULL;
    }
    if (shader->uniform_cache_items_allocated > 0) {
        free(shader->uniforms_cache);
        shader->uniforms_cache = NULL;
    }

    free(shader);
}

void
shader_use(shader_t *shader) {
    glUseProgram(shader->id);
}

static GLint
get_cached_uniform_name(shader_t *shader, const char *name) {
    for (int i = 0; i < shader->uniform_cache_items; i++) {
        if (strcmp(shader->uniforms_cache[i].uniform_name, name) == 0) {
            return shader->uniforms_cache[i].uniform_id;
        }
    }
    return -1;
}

static GLint
cache_uniform_name(shader_t *shader, const char *name, GLint uniform_id) {
    if (0 == shader->uniform_cache_items_allocated) {
        shader->uniform_cache_items_allocated = UNIFORM_CACHE_CAPACITY_STEP;
        shader->uniforms_cache = calloc(shader->uniform_cache_items_allocated, sizeof(uniform_cache_item_t));
    } else if (shader->uniform_cache_items == shader->uniform_cache_items_allocated) {
        shader->uniform_cache_items_allocated += UNIFORM_CACHE_CAPACITY_STEP;
        shader->uniforms_cache = reallocarray(shader->uniforms_cache, shader->uniform_cache_items_allocated,
                                              sizeof(uniform_cache_item_t));
        SDL_ALLOC_CHECK(shader->uniforms_cache)
    }

    shader->uniforms_cache[shader->uniform_cache_items].uniform_name = malloc(strlen(name) + 1);
    strcpy(shader->uniforms_cache[shader->uniform_cache_items].uniform_name, name);
    shader->uniforms_cache[shader->uniform_cache_items].uniform_id = uniform_id;

    shader->uniform_cache_items++;

    return uniform_id;
}

static GLint
uniform_name(shader_t *shader, const char *name) {
    GLint cached_result = get_cached_uniform_name(shader, name);
    if (cached_result >= 0) {
        return cached_result;
    }
    GLint unform_id = glGetUniformLocation(shader->id, name);
    if (unform_id < 0 && SDL_DEBUG_ENABLED) {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                     "Could not find the uniform %s in shader built from %s and %s",
                     name,
                     shader->vertex_shader_name,
                     shader->fragment_shader_name);
    }
    GL_CHECK_ERROR;

    return cache_uniform_name(shader, name, unform_id);
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
shader_set_vec3_array_item(shader_t *shader, const char *name_template, unsigned int index, vec3 value) {
    shader_set_vec3(shader, array_item_name(name_template, index), value);
}

void
shader_set_vec4(shader_t *shader, const char *name, vec4 value) {
    glUniform4f(uniform_name(shader, name), value[0], value[1], value[2], value[3]);
    GL_CHECK_ERROR;
}

void
shader_set_vec4_array_item(shader_t *shader, const char *name_template, unsigned int index, vec4 value) {
    shader_set_vec4(shader, array_item_name(name_template, index), value);
}

void
shader_set_float(shader_t *shader, const char *name, float value) {
    glUniform1f(uniform_name(shader, name), value);
    GL_CHECK_ERROR;
}

void
shader_set_float_array_item(shader_t *shader, const char *name_template, unsigned int index, float value) {
    shader_set_float(shader, array_item_name(name_template, index), value);
}

void
shader_set_int(shader_t *shader, const char *name, int value) {
    glUniform1i(uniform_name(shader, name), value);
    GL_CHECK_ERROR;
}

void
shader_set_int_array_item(shader_t *shader, const char *name_template, unsigned int index, int value) {
    shader_set_int(shader, array_item_name(name_template, index), value);
}


void
attach_shader(shader_t **target, shader_t *shader) {
    *target = shader;
    shader->owners++;
}

void
detach_shader(shader_t **shader_pointer) {
    if (*shader_pointer == NULL) {
        return;
    }
    shader_t *shader = *shader_pointer;
    if (shader->owners == 0) {
        SDL_Die("All owners already removed from this shader");
    }
    shader->owners--;
    if (shader->owners == 0) {
        destroy_shader(shader);
    }
    *shader_pointer = NULL;
}