#ifndef SDL_TEST_MODEL_H
#define SDL_TEST_MODEL_H

#include "cglm_ext.h"
#include "gl_ext.h"

typedef struct vertex {
    vec3 position;
    vec3 normal;
    vec2 texture_position;
} vertex_t;

typedef struct texture {
    unsigned int id;
    char *shader_name;
} texture_t;

typedef struct mesh {
    unsigned int vertices_number;
    vertex_t *vertices;
    unsigned int indices_number;
    unsigned int *indices;
    unsigned int textures_number;
    texture_t *textures;

    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
} mesh_t;

typedef struct mesh_list_item {
    mesh_t mesh;
    struct mesh_list_item *next;
} mesh_list_item_t;

typedef struct model {
    mesh_list_item_t *meshes;
    char *directory;
} model_t;

model_t *
create_model(unsigned int vertices_number, vertex_t *vertices, unsigned int indices_number, unsigned int *indices);

void draw_model(model_t *model);

#endif //SDL_TEST_MODEL_H
