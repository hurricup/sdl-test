#ifndef SDL_TEST_LIGHT_H
#define SDL_TEST_LIGHT_H

#include "cglm/cglm.h"

typedef struct direct_light {
    vec3 front;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
} direct_light_t;

typedef struct omni_light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
} omni_light_t;


typedef struct spot_light {
    omni_light_t light;
    vec3 front;
    float angle;
    float smooth_angle; // additional angle for smooth border
} spot_light_t;

#endif //SDL_TEST_LIGHT_H
