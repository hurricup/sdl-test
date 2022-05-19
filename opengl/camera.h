#ifndef SDL_TEST_CAMERA_H
#define SDL_TEST_CAMERA_H

#include "cglm/cglm.h"

typedef struct camera {
    float mouse_sensitivity;
    float speed_move;
    float speed_pitch;
    float speed_yaw;
    float speed_roll;
    vec3 position;
    vec3 up;
    vec3 front;
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 project_view_matrix;
    float fov;
    float near_z;
    float far_z;
    unsigned int viewport_width;
    unsigned int viewport_height;
    int polygon_mode;
} camera_t;

camera_t *create_camera();

void destroy_camera(camera_t **camera);

/**
 * Resets camera to initial state
 */
void camera_init(camera_t *camera, unsigned int window_width, unsigned int window_height);

/**
 * Fills view matrix for the camera
 */
void update_camera_views(camera_t *camera);

/**
 * Fills the project_view with projection_matrix * default_view_matrix
 * default_view matrix is the look matrix from 0,0,0 position
 */
void compute_skybox_project_view_matrix(camera_t *camera, mat4 project_view);

/**
 * Moves camera up or down along the up vector. Direction depends on sign
 */
void pitch_camera(camera_t *camera, float sign);

/**
 * Moves camera left or right along the right vector. Direction depends on sign
 */
void yaw_camera(camera_t *camera, float sign);

/**
 * Moves camera forward or backward along the front vector. Direction depends on sign
 */
void move_camera(camera_t *camera, float sign);

/**
 *  Rotates the front vector based on x and y deltas (e.g. mouse move)
 */
void move_camera_front(camera_t *camera, int x, int y);

/**
 * Rolling camera up vector around the front vector
 */
void roll_camera(camera_t *camera, float sign);

/**
 * Re-compute projection matrix with new aspect aspect_ratio
 */
void set_aspect_ratio(camera_t *camera, unsigned int window_width, unsigned int window_height);

#endif //SDL_TEST_CAMERA_H
