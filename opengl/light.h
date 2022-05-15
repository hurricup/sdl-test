#ifndef SDL_TEST_LIGHT_H
#define SDL_TEST_LIGHT_H

#include "cglm/cglm.h"

typedef struct light_prop {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
} light_prop_t;

typedef struct direct_light {
    light_prop_t light_prop;
    vec3 front;
} direct_light_t;

typedef struct omni_light {
    light_prop_t light_prop;
    vec3 position;
} omni_light_t;

typedef struct spot_light {
    light_prop_t light_prop;
    vec3 position;
    vec3 front;
    float angle;
    float smooth_angle; // additional angle for smooth border
} spot_light_t;

direct_light_t *create_direct_light();

omni_light_t *create_omni_light();

spot_light_t *create_spot_light();

void destroy_direct_light(direct_light_t **pp_direct_light);

void destroy_omni_light(omni_light_t **pp_omni_light);

void destroy_spot_light(spot_light_t **pp_spot_light);

#endif //SDL_TEST_LIGHT_H
