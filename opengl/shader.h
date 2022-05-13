#ifndef SDL_TEST_SHADER_H
#define SDL_TEST_SHADER_H

#include "sdl_ext.h"
#include "gl_ext.h"
#include "file_util.h"
#include "cglm_ext.h"
#include "scene_object.h"

typedef struct shader {
    unsigned int id;
    char *vertex_shader_name;
    char *fragment_shader_name;
    unsigned int owners;
} shader_t;

shader_t *shader_load(const char *vertex_shader_name, const char *fragment_shader_name);

void shader_attach(shader_t **target, shader_t *shader);

void shader_detach(shader_t **shader_pointer);

void shader_use(shader_t *shader);

void shader_set_mat4(shader_t *shader, const char *name, mat4 value);

void shader_set_mat3(shader_t *shader, const char *name, mat3 value);

void shader_set_vec3(shader_t *shader, const char *name, vec3 value);

void shader_set_float(shader_t *shader, const char *name, float value);

void shader_set_int(shader_t *shader, const char *name, int value);

#endif //SDL_TEST_SHADER_H
