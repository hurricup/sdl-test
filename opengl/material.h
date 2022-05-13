#ifndef SDL_TEST_MATERIAL_H
#define SDL_TEST_MATERIAL_H
#define DEFAULT_SHININESS 32

#include "cglm/cglm.h"

typedef struct material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} material_t;

// http://devernay.free.fr/cours/opengl/materials.html
const material_t MATERIAL_IDEAL = {
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        DEFAULT_SHININESS
};

const material_t MATERIAL_BRONZE = {
        0.2125f, 0.1275f, 0.054f,
        0.714f, 0.4284f, 0.18144f,
        0.393548f, 0.271906f, 0.166721f,
        0.2f * 128
};

const material_t MATERIAL_GREEN_RUBBER = {
        0.0f, 0.05f, 0.0f,
        0.4f, 0.5f, 0.4f,
        0.04f, 0.7f, 0.04f,
        .078125f * 128
};

const material_t MATERIAL_CYAN_PLASTIC = {
        0.0f, 0.1f, 0.06f,
        0.0f, 0.50980392f, 0.50980392f,
        0.50196078f, 0.50196078f, 0.50196078f,
        .25f * 128
};

const material_t MATERIAL_OBSIDIAN = {
        0.05375f, 0.05f, 0.06625f,
        0.18275f, 0.17f, 0.22525f,
        0.332741f, 0.328634f, 0.346435f,
        0.3f * 128
};

#endif //SDL_TEST_MATERIAL_H
