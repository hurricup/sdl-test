#include "cglm/cglm.h"

static inline void
vec3_set(vec3 dst, float a, float b, float c) {
    dst[0] = a;
    dst[1] = b;
    dst[2] = c;
}

typedef struct square {
    vec3 a;
    vec3 b;
    vec3 c;
    vec3 d;
} square_t;

#define SQUARE_SIZE sizeof(square_t)

static inline void
set_square(square_t *square,
           float x1, float y1, float z1,
           float x2, float y2, float z2,
           float x3, float y3, float z3,
           float x4, float y4, float z4) {
    vec3_set(square->a, x1, y1, z1);
    vec3_set(square->b, x2, y2, z2);
    vec3_set(square->c, x3, y3, z3);
    vec3_set(square->d, x4, y4, z4);
}

typedef struct cube {
    square_t side_a;
    square_t side_b;
} cube_t;

#define CUBE_SIZE sizeof(cube_t)
