#ifndef SDL_TEST_SCENE_OBJECT_H
#define SDL_TEST_SCENE_OBJECT_H

#include "scene_types.h"
#include "model.h"
#include "shader.h"
#include "cglm_ext.h"

scene_object_t *create_scene_object();

void destroy_scene_object(scene_object_t **scene_object);

void attach_shader_to_scene_object(scene_object_t *scene_object, shader_t *shader);

void attach_model_to_scene_object(scene_object_t *scene_object, model_t *model);

void scale_scene_object(scene_object_t *scene_object, float scale);

void rotate_scene_object_to(scene_object_t *scene_object, float x, float y, float z);

void rotate_scene_object_by(scene_object_t *scene_object, float x, float y, float z);

void rotate_scene_object_by_vec(scene_object_t *scene_object, vec3 angles);

void move_scene_object_to(scene_object_t *scene_object, float x, float y, float z);

void move_scene_object_to_vec(scene_object_t *scene_object, vec3 position);

void draw_scene_object(scene_object_t *scene_object, mat4 project_view, drawing_context_t *context);

#endif //SDL_TEST_SCENE_OBJECT_H
