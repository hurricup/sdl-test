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
    unsigned long vertices_number;
    vertex_t *vertices;
    GLsizei indices_number;
    unsigned int *indices;
    unsigned long textures_number;
    texture_t *textures;

    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
} mesh_t;


#endif //SDL_TEST_MODEL_H
