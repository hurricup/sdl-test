#include "light.h"
#include "sdl_ext.h"

direct_light_t *create_direct_light() {
    direct_light_t *direct_light = calloc(1, sizeof(direct_light_t));
    SDL_ALLOC_CHECK(direct_light)
    return direct_light;
}

omni_light_t *create_omni_light() {
    omni_light_t *omni_light = calloc(1, sizeof(omni_light_t));
    SDL_ALLOC_CHECK(omni_light)
    return omni_light;
}

spot_light_t *create_spot_light() {
    spot_light_t *spot_light = calloc(1, sizeof(spot_light_t));
    SDL_ALLOC_CHECK(spot_light)
    return spot_light;
}

void destroy_direct_light(direct_light_t **pp_direct_light) {
    direct_light_t *direct_light = *pp_direct_light;
    if (direct_light == NULL) {
        return;
    }
    free(direct_light);
    *pp_direct_light = NULL;
}

void destroy_omni_light(omni_light_t **pp_omni_light) {
    omni_light_t *omni_light = *pp_omni_light;
    if (omni_light == NULL) {
        return;
    }
    free(omni_light);
    *pp_omni_light = NULL;
}

void destroy_spot_light(spot_light_t **pp_spot_light) {
    spot_light_t *spot_light = *pp_spot_light;
    if (spot_light == NULL) {
        return;
    }
    free(spot_light);
    *pp_spot_light = NULL;
}
