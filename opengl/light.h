#ifndef SDL_TEST_LIGHT_H
#define SDL_TEST_LIGHT_H

#include "cglm/cglm.h"

typedef struct light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
} light_t;


#endif //SDL_TEST_LIGHT_H
