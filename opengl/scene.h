#ifndef SDL_TEST_SCENE_H
#define SDL_TEST_SCENE_H

#include "camera.h"
#include "scene_object.h"
#include "light.h"

typedef struct scene_object_list_item {
    scene_object_t *item;
    bool enabled;
    struct scene_object_list_item *next;
} scene_object_list_item_t;

typedef struct omni_light_list_item {
    omni_light_t *item;
    bool enabled;
    struct omni_light_list_item *next;
} omni_light_list_item_t;

typedef struct direct_light_list_item {
    direct_light_t *item;
    bool enabled;
    struct direct_light_list_item *next;
} direct_light_list_item_t;

typedef struct spot_light_list_item {
    spot_light_t *item;
    bool enabled;
    struct spot_light_list_item *next;
} spot_light_list_item_t;

typedef struct scene {
    camera_t *camera;
    scene_object_list_item_t *objects;
    omni_light_list_item_t *omni_lights;
    direct_light_list_item_t *direct_lights;
    spot_light_list_item_t *spot_lights;
} scene_t;

scene_t *create_scene();

void draw_scene(scene_t *scene);

/**
 * Destroying scene and all attached objects, lights and so on
 */
void destroy_scene(scene_t **pp_scene);

void attach_camera_to_scene(scene_t *scene, camera_t *camera);

void attach_object_to_scene(scene_t *scene, scene_object_t *scene_object);

void attach_omni_light_to_scene(scene_t *scene, omni_light_t *omni_light);

void attach_direct_light_to_scene(scene_t *scene, direct_light_t *direct_light);

void attach_spot_light_to_scene(scene_t *scene, spot_light_t *spot_light);

void enable_scene_object(scene_t *scene, scene_object_t *scene_object, bool enabled);

void enable_omni_light(scene_t *scene, omni_light_t *omni_light, bool enabled);

void enable_direct_light(scene_t *scene, direct_light_t *direct_light, bool enabled);

void enable_spot_light(scene_t *scene, spot_light_t *spot_light, bool enabled);

#endif //SDL_TEST_SCENE_H
