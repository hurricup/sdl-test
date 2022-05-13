#ifndef SDL_TEST_MODEL_H
#define SDL_TEST_MODEL_H

#include "scene_types.h"
#include "gl_ext.h"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "sdl_ext.h"
#include "stb_image.h"
#include "shader.h"

model_t *
create_model(unsigned int vertices_number, vertex_t *vertices, unsigned int indices_number, unsigned int *indices,
             const char *directory_name);

void draw_model(model_t *model, shader_t *shader);

model_t *load_model(char *path);

void load_texture(model_t *model, mesh_t *mesh, enum aiTextureType type, const char *filename);

void attach_model(model_t **target, model_t *model);

void detach_model(model_t **model_pointer);

#endif //SDL_TEST_MODEL_H
