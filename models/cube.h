#ifndef SDL_TEST_CUBE_H
#define SDL_TEST_CUBE_H

#include "../opengl/cglm_ext.h"

typedef struct square2 {
    vec2 a;
    vec2 b;
    vec2 c;
    vec2 d;
} square2_t;

typedef struct square3 {
    vec3 a;
    vec3 b;
    vec3 c;
    vec3 d;
} square3_t;

#define SQUARE_SIZE sizeof(square_t)

typedef struct cube {
    square3_t side_a;
    square3_t side_b;
    square3_t side_c;
    square3_t side_d;
    square3_t side_e;
    square3_t side_f;
} cube_t;

#define CUBE_SIZE sizeof(cube_t)

typedef struct cube_textures {
    square2_t side_a;
    square2_t side_b;
    square2_t side_c;
    square2_t side_d;
    square2_t side_e;
    square2_t side_f;
} cube_textures_t;

#define CUBE_TEXTURES_SIZE sizeof(cube_textures_t)

void cube_model_init(cube_t *cube);

void cube_textures_init(cube_textures_t *cube_textures);

void cube_normals_init(cube_t *cube_normals);

#endif //SDL_TEST_CUBE_H
