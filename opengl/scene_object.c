#include "scene_object.h"

scene_object_t *
create_scene_object() {
    scene_object_t *scene_object = calloc(1, sizeof(scene_object_t));
    SDL_ALLOC_CHECK(scene_object)
    scale_scene_object(scene_object, 1.0f);
    return scene_object;
}

void
destroy_scene_object(scene_object_t **pp_scene_object) {
    scene_object_t *scene_object = *pp_scene_object;
    if (scene_object == NULL) {
        return;
    }
    detach_model(&scene_object->model);
    detach_shader(&scene_object->shader);
    free(scene_object);
    *pp_scene_object = NULL;
}

void
attach_shader_to_scene_object(scene_object_t *scene_object, shader_t *shader) {
    attach_shader(&scene_object->shader, shader);
}

void
attach_model_to_scene_object(scene_object_t *scene_object, model_t *model) {
    attach_model(&scene_object->model, model);
}

void
scale_scene_object(scene_object_t *scene_object, float scale) {
    vec3_set(scene_object->scale, scale, scale, scale);
}

void
rotate_scene_object_to(scene_object_t *scene_object, float x, float y, float z) {
    vec3_set(scene_object->angles, x, y, z);
}

void
rotate_scene_object_by(scene_object_t *scene_object, float x, float y, float z) {
    scene_object->angles[0] += x;
    scene_object->angles[1] += y;
    scene_object->angles[2] += z;
}

void
rotate_scene_object_by_vec(scene_object_t *scene_object, vec3 angles) {
    rotate_scene_object_by(scene_object, angles[0], angles[1], angles[2]);
}

void
move_scene_object_to(scene_object_t *scene_object, float x, float y, float z) {
    vec3_set(scene_object->position, x, y, z);
}

void
move_scene_object_to_vec(scene_object_t *scene_object, vec3 position) {
    move_scene_object_to(scene_object, position[0], position[1], position[2]);
}

void
render_scene_object(scene_object_t *scene_object, mat4 project_view, rendering_context_t *context) {
    // model matrix
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    glm_translate(model, scene_object->position);
    glm_scale(model, scene_object->scale);
    glm_rotate_x(model, scene_object->angles[0], model);
    glm_rotate_y(model, scene_object->angles[1], model);
    glm_rotate_z(model, scene_object->angles[2], model);

    // normals matrix
    mat4 normals_model4;
    mat3 normals_model3;

    glm_mat4_inv(model, normals_model4);
    glm_mat4_transpose(normals_model4);
    glm_mat4_pick3(normals_model4, normals_model3);

    shader_t *shader = context->shader;
    shader_set_mat4(shader, LOC_MODEL, model);
    shader_set_mat4(shader, LOC_PROJECT_VIEW, project_view);
    shader_set_mat3(shader, LOC_NORMALS_MODEL, normals_model3);

    render_model(scene_object->model, context);
}

