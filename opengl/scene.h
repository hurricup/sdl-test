#ifndef SDL_TEST_SCENE_H
#define SDL_TEST_SCENE_H

#include "camera.h"
#include "scene_object.h"
#include "light.h"
#include "scene_screen.h"
#include "cubemap.h"

typedef enum {
    EFFECT_NONE = 0,
    EFFECT_INVERT,
    EFFECT_GRAYSCALE,
    EFFECT_SHARP,
    EFFECT_BLUR,
    EFFECT_EDGE,
    EFFECT_LAST_TYPE
} effect_type_t;

typedef struct scene_object_list_item {
    scene_object_t *item;
    struct scene_object_list_item *next;
} scene_object_list_item_t;

typedef struct omni_light_list_item {
    omni_light_t *item;
    struct omni_light_list_item *next;
} omni_light_list_item_t;

typedef struct direct_light_list_item {
    direct_light_t *item;
    struct direct_light_list_item *next;
} direct_light_list_item_t;

typedef struct spot_light_list_item {
    spot_light_t *item;
    struct spot_light_list_item *next;
} spot_light_list_item_t;

typedef struct scene_screen_object {
    unsigned int vertex_array;
    unsigned int vertex_buffer;
    shader_t *shader;
} scene_screen_object_t;

typedef struct skybox {
    unsigned int vertex_array;
    unsigned int vertex_buffer;
    cubemap_t *cubemap;
    shader_t *shader;
} skybox_t;

typedef struct scene {
    camera_t *camera;
    scene_object_list_item_t *objects;
    omni_light_list_item_t *omni_lights;
    direct_light_list_item_t *direct_lights;
    spot_light_list_item_t *spot_lights;
    scene_screen_t scene_screen;
    scene_screen_t selection_screen;
    shader_t *selection_shader;
    shader_t *indexed_color_shader;
    scene_screen_object_t scene_screen_object;
    effect_type_t effect_type;
    skybox_t skybox;
} scene_t;

scene_t *create_scene();

void render_scene(scene_t *scene);

/**
 * Destroying scene and all attached objects, lights and so on
 */
void destroy_scene(scene_t **pp_scene);

void attach_camera_to_scene(scene_t *scene, camera_t *camera);

void attach_object_to_scene(scene_t *scene, scene_object_t *scene_object);

void attach_omni_light_to_scene(scene_t *scene, omni_light_t *omni_light);

void attach_direct_light_to_scene(scene_t *scene, direct_light_t *direct_light);

void attach_spot_light_to_scene(scene_t *scene, spot_light_t *spot_light);

/**
 * If no object is currently selected, selects first
 * If any object selected - removes the selection from it and selects the next one in the list (or first)
 */
void select_next_object(scene_t *scene);

/**
 * Attempts to find an object at screen_x and screen_y and mark it as selected. If other object was selected at the
 * moment it is deselected.
 */
void select_object(scene_t *scene, unsigned int screen_x, unsigned int screen_y);

/**
 * Attaches cubemap cubemap to the scene, detaching previous one if we did have one
 */
void set_scene_skybox(scene_t *scene, cubemap_t *cubemap);

#endif //SDL_TEST_SCENE_H
