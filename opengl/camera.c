#include <GL/gl.h>
#include "camera.h"
#include "sdl_ext.h"

static vec3 camera_pos_default = {0.0f, 0.0f, 20.0f};
static vec3 camera_up_default = {0.0f, 1.0f, 0.0f};
static vec3 camera_front_default = {0.0f, 0.0f, -1.0f};

camera_t *
create_camera() {
    camera_t *camera = calloc(1, sizeof(camera_t));
    SDL_ALLOC_CHECK(camera);
    camera->polygon_mode = GL_FILL;

    return camera;
}

void
destroy_camera(camera_t **camera) {
    if (*camera == NULL) {
        return;
    }
    free(*camera);
    *camera = NULL;
}

void camera_init(camera_t *camera, unsigned int window_width, unsigned int window_height) {
    camera->speed_move = 0.2f;
    camera->speed_pitch = 0.2f;
    camera->speed_yaw = 0.2f;
    camera->speed_roll = 0.02f;
    camera->mouse_sensitivity = 0.001f;
    camera->fov = M_PI_4;
    set_aspect_ratio(camera, window_width, window_height);
    camera->near_z = 0.1f;
    camera->far_z = 100.0f;
    glm_vec3_copy(camera_pos_default, camera->position);
    glm_vec3_copy(camera_up_default, camera->up);
    glm_vec3_copy(camera_front_default, camera->front);
}

void
set_aspect_ratio(camera_t *camera, unsigned int window_width, unsigned int window_height) {
    camera->viewport_width = window_width;
    camera->viewport_height = window_height;
}

void
update_camera_views(camera_t *camera) {
    // projection
    glm_perspective(camera->fov, (float) camera->viewport_width / (float) camera->viewport_height,
                    camera->near_z, camera->far_z, camera->projection_matrix);

    // View
    glm_look(camera->position, camera->front, camera->up, camera->view_matrix);

    // project * view
    glm_mat4_identity(camera->project_view_matrix);
    glm_mat4_mul(camera->projection_matrix, camera->view_matrix, camera->project_view_matrix);
}

void pitch_camera(camera_t *camera, float sign) {
    vec3 up = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(camera->up, sign * camera->speed_pitch, up);
    glm_vec3_add(camera->position, up, camera->position);
}

void yaw_camera(camera_t *camera, float sign) {
    vec3 right = GLM_VEC3_ZERO_INIT;
    glm_vec3_cross(camera->front, camera->up, right);
    glm_normalize(right);
    glm_vec3_scale(right, sign * camera->speed_yaw, right);
    glm_vec3_add(camera->position, right, camera->position);
}

void move_camera(camera_t *camera, float sign) {
    vec3 delta_front = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(camera->front, camera->speed_move * (float) sign, delta_front);
    glm_vec3_add(camera->position, delta_front, camera->position);
}

void roll_camera(camera_t *camera, float sign) {
    glm_vec3_rotate(camera->up, sign * camera->speed_roll, camera->front);
}

void move_camera_front(camera_t *camera, int x, int y) {
    if (x == 0 && y == 0) {
        return;
    }
    vec3 delta_up = GLM_VEC3_ZERO_INIT;
    vec3 right = GLM_VEC3_ZERO_INIT;
    vec3 delta_right = GLM_VEC3_ZERO_INIT;

    glm_vec3_cross(camera->front, camera->up, right);
    glm_normalize(right);

    glm_vec3_scale(camera->up, -(float) y * camera->mouse_sensitivity, delta_up);
    glm_vec3_scale(right, (float) x * camera->mouse_sensitivity, delta_right);

    glm_vec3_add(camera->front, delta_right, camera->front);
    glm_vec3_cross(camera->front, camera->up, right);
    glm_vec3_add(camera->front, delta_up, camera->front);
    glm_vec3_cross(right, camera->front, camera->up);

    glm_normalize(camera->front);
    glm_normalize(camera->up);
}
