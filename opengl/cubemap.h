#ifndef SDL_TEST_CUBEMAP_H
#define SDL_TEST_CUBEMAP_H

#include "shader.h"

typedef struct cubemap {
    char *file_pattern;
    unsigned int texture;
    unsigned int owners;
} cubemap_t;

/**
 * Creating cubemap texture from the files specified by the file pattern.
 * @param file_pattern path pattern with %s placeholder for top/bottom/font/back/left/right
 */
cubemap_t *create_cubemap(char *file_pattern);

/**
 * Attaching cubemap to the owners field.
 */
void attach_cubemap(cubemap_t **target, cubemap_t *cubemap);

/**
 * Detaching cubemap from the owners field. If this is the last owner - destroys cubemap.
 */
void detach_cubemap(cubemap_t **p_cubemap);

#endif
