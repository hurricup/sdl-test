#ifndef SDL_TEST_SCENE_SCREEN_H
#define SDL_TEST_SCENE_SCREEN_H

#include "gl_ext.h"
#include "camera.h"

typedef struct scene_screen {
    unsigned int frame_buffer;
    unsigned int render_buffer;
    unsigned int texture;
    unsigned int width;
    unsigned int height;
} scene_screen_t;

/**
 * Re-allocating buffers necessary for the scene_screen if camera dimensions changed
 */
void update_scene_screen(scene_screen_t *scene_screen, camera_t *camera);

/**
 * Releasing all buffers allocated for the scene screen
 */
void destroy_scene_screen_contents(scene_screen_t *scene_screen);

#endif //SDL_TEST_SCENE_SCREEN_H
