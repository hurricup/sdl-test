#include "cube.h"
#include <string.h>

static inline void
set_square2(square2_t *square,
            float x1, float y1,
            float x2, float y2,
            float x3, float y3,
            float x4, float y4) {
    vec2_set(square->a, x1, y1);
    vec2_set(square->b, x2, y2);
    vec2_set(square->c, x3, y3);
    vec2_set(square->d, x4, y4);
}

static inline void
set_square3(square3_t *square,
            float x1, float y1, float z1,
            float x2, float y2, float z2,
            float x3, float y3, float z3,
            float x4, float y4, float z4) {
    vec3_set(square->a, x1, y1, z1);
    vec3_set(square->b, x2, y2, z2);
    vec3_set(square->c, x3, y3, z3);
    vec3_set(square->d, x4, y4, z4);
}

void
cube_model_init(cube_t *cube) {
    // front
    set_square3(&cube->side_a,
                0.5f, 0.5f, 0.5f,
                0.5f, -0.5f, 0.5f,
                -0.5f, -0.5f, 0.5f,
                -0.5f, 0.5f, 0.5f
    );
    // back
    set_square3(&cube->side_b,
                0.5f, 0.5f, -0.5f,
                0.5f, -0.5f, -0.5f,
                -0.5f, -0.5f, -0.5f,
                -0.5f, 0.5f, -0.5f
    );
    // right
    set_square3(&cube->side_c,
                0.5f, 0.5f, 0.5f,
                0.5f, -0.5f, 0.5f,
                0.5f, -0.5f, -0.5f,
                0.5f, 0.5f, -0.5f
    );
    // left
    set_square3(&cube->side_d,
                -0.5f, 0.5f, 0.5f,
                -0.5f, -0.5f, 0.5f,
                -0.5f, -0.5f, -0.5f,
                -0.5f, 0.5f, -0.5f
    );
    // top
    set_square3(&cube->side_e,
                0.5f, 0.5f, 0.5f,
                0.5f, 0.5f, -0.5f,
                -0.5f, 0.5f, -0.5f,
                -0.5f, 0.5f, 0.5f
    );
    // bottom
    set_square3(&cube->side_f,
                0.5f, -0.5f, 0.5f,
                0.5f, -0.5f, -0.5f,
                -0.5f, -0.5f, -0.5f,
                -0.5f, -0.5f, 0.5f
    );
}

void
cube_textures_init(cube_textures_t *cube_textures) {
    set_square2(&cube_textures->side_a,
                1.0f, 1.0f,
                1.0f, 0.0f,
                0.0f, 0.0f,
                0.0f, 1.0f
    );
    memcpy(&cube_textures->side_b, &cube_textures->side_a, sizeof(square2_t));
    memcpy(&cube_textures->side_c, &cube_textures->side_a, sizeof(square2_t));
    memcpy(&cube_textures->side_d, &cube_textures->side_a, sizeof(square2_t));
    memcpy(&cube_textures->side_e, &cube_textures->side_a, sizeof(square2_t));
    memcpy(&cube_textures->side_f, &cube_textures->side_a, sizeof(square2_t));
}

static void
set_normal(square3_t *normal, float x, float y, float z) {
    set_square3(normal,
                x, y, z,
                x, y, z,
                x, y, z,
                x, y, z);

}

void
cube_normals_init(cube_t *cube_normals) {
    set_normal(&cube_normals->side_a, 0, 0, 1);
    set_normal(&cube_normals->side_b, 0, 0, -1);
    set_normal(&cube_normals->side_c, 1, 0, 0);
    set_normal(&cube_normals->side_d, -1, 0, 0);
    set_normal(&cube_normals->side_e, 0, 1, 0);
    set_normal(&cube_normals->side_f, 0, -1, 0);
}