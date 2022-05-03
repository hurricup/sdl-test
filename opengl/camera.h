#ifndef SDL_TEST_CAMERA_H
#define SDL_TEST_CAMERA_H

#include "cglm/cglm.h"

typedef struct camera {
    float speed;
    float speed_y;
    float speed_x;
    float mouse_sensitivity;
    vec3 pos;
    vec3 up;
    vec3 front;
} camera_t;

/**
 * Resets camera to initial state
 */
void camera_init(camera_t *camera);

/**
 * Fills view matrix for the camera
 */
void camera_view(camera_t *camera, mat4 view_m);

/**
 * Moves camera up or down along the up vector. Direction depends on sign
 */
void move_camera_vertically(camera_t *camera, float sign);

/**
 * Moves camera left or right along the right vector. Direction depends on sign
 */
void move_camera_horizontally(camera_t *camera, float sign);

/**
 * Moves camera forward or backward along the front vector. Direction depends on sign
 */
void move_camera_front(camera_t *camera, float sign);

/**
 *  Rotates the front vector based on x and y deltas (e.g. mouse move)
 */
void move_camera_sight(camera_t *camera, int x, int y);

#endif //SDL_TEST_CAMERA_H
