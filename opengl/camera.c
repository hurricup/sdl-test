#include "camera.h"

static vec3 camera_pos_default = {0.0f, 0.0f, 20.0f};
static vec3 camera_up_default = {0.0f, 1.0f, 0.0f};
static vec3 camera_front_default = {0.0f, 0.0f, -1.0f};

void camera_init(camera_t *camera) {
    camera->speed_move = 0.2f;
    camera->speed_pitch = 0.2f;
    camera->speed_yaw = 0.2f;
    camera->speed_roll = 0.02f;
    camera->mouse_sensitivity = 0.001f;
    glm_vec3_copy(camera_pos_default, camera->pos);
    glm_vec3_copy(camera_up_default, camera->up);
    glm_vec3_copy(camera_front_default, camera->front);
}

void camera_view(camera_t *camera, mat4 view_m) {
    glm_look(camera->pos, camera->front, camera->up, view_m);
}

void pitch_camera(camera_t *camera, float sign) {
    vec3 up = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(camera->up, sign * camera->speed_pitch, up);
    glm_vec3_add(camera->pos, up, camera->pos);
}

void yaw_camera(camera_t *camera, float sign) {
    vec3 right = GLM_VEC3_ZERO_INIT;
    glm_vec3_cross(camera->front, camera->up, right);
    glm_normalize(right);
    glm_vec3_scale(right, sign * camera->speed_yaw, right);
    glm_vec3_add(camera->pos, right, camera->pos);
}

void move_camera(camera_t *camera, float sign) {
    vec3 delta_front = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(camera->front, camera->speed_move * (float) sign, delta_front);
    glm_vec3_add(camera->pos, delta_front, camera->pos);
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
