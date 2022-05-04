#ifndef SDL_TEST_CGLM_EXT_H
#define SDL_TEST_CGLM_EXT_H

#include "cglm/cglm.h"

static inline void
vec2_set(vec2 dst, float a, float b) {
    dst[0] = a;
    dst[1] = b;
}

static inline void
vec3_set(vec3 dst, float a, float b, float c) {
    dst[0] = a;
    dst[1] = b;
    dst[2] = c;
}

#endif