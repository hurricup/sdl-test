#include "cubemap.h"
#include "model.h"

cubemap_t *
create_cubemap(char *file_pattern) {
    cubemap_t *cubemap = calloc(1, sizeof(cubemap_t));
    SDL_ALLOC_CHECK(cubemap)

    cubemap->file_pattern = malloc(strlen(file_pattern) + 1);
    SDL_ALLOC_CHECK(cubemap->file_pattern)

    strcpy(cubemap->file_pattern, file_pattern);

    glGenTextures(1, &cubemap->texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->texture);

    char *name_buffer = malloc(strlen(file_pattern) + 1 + 4);
    SDL_ALLOC_CHECK(name_buffer)

    sprintf(name_buffer, file_pattern, "right");
    load_current_texture_file(name_buffer, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
    sprintf(name_buffer, file_pattern, "left");
    load_current_texture_file(name_buffer, GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
    sprintf(name_buffer, file_pattern, "top");
    load_current_texture_file(name_buffer, GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
    sprintf(name_buffer, file_pattern, "bottom");
    load_current_texture_file(name_buffer, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
    sprintf(name_buffer, file_pattern, "front");
    load_current_texture_file(name_buffer, GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
    sprintf(name_buffer, file_pattern, "back");
    load_current_texture_file(name_buffer, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);

    free(name_buffer);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return cubemap;
}

void
attach_cubemap(cubemap_t **target, cubemap_t *cubemap) {
    *target = cubemap;
    cubemap->owners++;
}

static void
destroy_cubemap(cubemap_t **p_cubemap) {
    cubemap_t *cubemap = *p_cubemap;
    if (cubemap == NULL) {
        return;
    }

    if (cubemap->texture >= 0) {
        glDeleteTextures(1, &cubemap->texture);
        cubemap->texture = -1;
    }

    if (cubemap->file_pattern != NULL) {
        free(cubemap->file_pattern);
        cubemap->file_pattern = NULL;
    }

    free(cubemap);
    *p_cubemap = NULL;
}

void
detach_cubemap(cubemap_t **p_cubemap) {
    cubemap_t *cubemap = *p_cubemap;
    if (cubemap == NULL) {
        return;
    }
    if (0 == cubemap->owners) {
        SDL_Die("Attempt to detach cubemap without owners");
    }
    cubemap->owners--;
    if (0 == cubemap->owners) {
        destroy_cubemap(p_cubemap);
    } else {
        *p_cubemap = NULL;
    }
}

