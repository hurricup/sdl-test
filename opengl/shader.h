#ifndef SDL_TEST_SHADER_H
#define SDL_TEST_SHADER_H

#define LOC_PROJECT_VIEW "project_view"
#define LOC_MODEL "model"
#define LOC_NORMALS_MODEL "normals_model"

#include "scene_types.h"
#include "sdl_ext.h"
#include "gl_ext.h"
#include "file_util.h"
#include "cglm_ext.h"
#include "scene_object.h"

shader_t *load_shader(const char *vertex_shader_name, const char *fragment_shader_name);

void attach_shader(shader_t **target, shader_t *shader);

void detach_shader(shader_t **shader_pointer);

void shader_use(shader_t *shader);

void shader_set_mat4(shader_t *shader, const char *name, mat4 value);

void shader_set_mat3(shader_t *shader, const char *name, mat3 value);

void shader_set_vec3(shader_t *shader, const char *name, vec3 value);

void shader_set_vec3_array_item(shader_t *shader, const char *name_template, unsigned int index, vec3 value);

void shader_set_vec4(shader_t *shader, const char *name, vec4 value);

void shader_set_vec4_array_item(shader_t *shader, const char *name_template, unsigned int index, vec4 value);

void shader_set_float(shader_t *shader, const char *name, float value);

void shader_set_float_array_item(shader_t *shader, const char *name_template, unsigned int index, float value);

void shader_set_int(shader_t *shader, const char *name, int value);

void shader_set_int_array_item(shader_t *shader, const char *name_template, unsigned int index, int value);

#endif //SDL_TEST_SHADER_H
