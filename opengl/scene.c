#include "scene.h"
#include "sdl_ext.h"

static unsigned int draw_pass = 0;

scene_t *
create_scene() {
    scene_t *scene = calloc(1, sizeof(scene_t));
    return scene;
}

static void
set_up_omni_lights(scene_t *scene, shader_t *shader) {
    unsigned int lights_number = 0;
    omni_light_list_item_t *current_light = scene->omni_lights;
    while (current_light != NULL) {
        if (current_light->enabled) {
            omni_light_t *omni_light = current_light->item;
            shader_set_vec3_array_item(shader, "omni_lights[%d].position", lights_number, omni_light->position);
            shader_set_vec4_array_item(shader, "omni_lights[%d].light_prop.ambient", lights_number,
                                       omni_light->light_prop.ambient);
            shader_set_vec4_array_item(shader, "omni_lights[%d].light_prop.diffuse", lights_number,
                                       omni_light->light_prop.diffuse);
            shader_set_vec4_array_item(shader, "omni_lights[%d].light_prop.specular", lights_number,
                                       omni_light->light_prop.specular);
            lights_number++;
        }
        current_light = current_light->next;
    }
    shader_set_int(shader, "omni_lights_number", (int) lights_number);
}

static void
set_up_direct_lights(scene_t *scene, shader_t *shader) {
    unsigned int lights_number = 0;
    direct_light_list_item_t *current_light = scene->direct_lights;
    while (current_light != NULL) {
        if (current_light->enabled) {
            direct_light_t *direct_light = current_light->item;
            shader_set_vec3_array_item(shader, "direct_lights[%d].front", lights_number, direct_light->front);
            shader_set_vec4_array_item(shader, "direct_lights[%d].light_prop.ambient", lights_number,
                                       direct_light->light_prop.ambient);
            shader_set_vec4_array_item(shader, "direct_lights[%d].light_prop.diffuse", lights_number,
                                       direct_light->light_prop.diffuse);
            shader_set_vec4_array_item(shader, "direct_lights[%d].light_prop.specular", lights_number,
                                       direct_light->light_prop.specular);

            lights_number++;
        }
        current_light = current_light->next;
    }
    shader_set_int(shader, "direct_lights_number", (int) lights_number);
}

static void
set_up_spot_lights(scene_t *scene, shader_t *shader) {
    unsigned int lights_number = 0;
    spot_light_list_item_t *current_light = scene->spot_lights;
    while (current_light != NULL) {
        if (current_light->enabled) {
            spot_light_t *spot_light = current_light->item;
            shader_set_vec4_array_item(shader, "spot_lights[%d].light_prop.ambient", lights_number,
                                       spot_light->light_prop.ambient);
            shader_set_vec4_array_item(shader, "spot_lights[%d].light_prop.diffuse", lights_number,
                                       spot_light->light_prop.diffuse);
            shader_set_vec4_array_item(shader, "spot_lights[%d].light_prop.specular", lights_number,
                                       spot_light->light_prop.specular);
            shader_set_vec3_array_item(shader, "spot_lights[%d].position", lights_number, spot_light->position);
            shader_set_vec3_array_item(shader, "spot_lights[%d].front", lights_number, spot_light->front);
            shader_set_float_array_item(shader, "spot_lights[%d].angle_cos", lights_number,
                                        (float) cos((double) spot_light->angle));
            shader_set_float_array_item(shader, "spot_lights[%d].smooth_angle_cos", lights_number,
                                        (float) cos((double) spot_light->angle + spot_light->smooth_angle));
            lights_number++;
        }
        current_light = current_light->next;
    }
    shader_set_int(shader, "spot_lights_number", (int) lights_number);
}

static void
set_up_light_and_camera(scene_t *scene, shader_t *shader) {
    set_up_omni_lights(scene, shader);
    set_up_direct_lights(scene, shader);
    set_up_spot_lights(scene, shader);

    // camera position
    shader_set_vec3(shader, "camera_position", scene->camera->position);
}

static void
draw_object(scene_t *scene, scene_object_t *object) {
    shader_t *shader = object->shader;
    shader_use(shader);
    if (shader->draw_pass != draw_pass) {
        set_up_light_and_camera(scene, shader);
        shader->draw_pass = draw_pass;
    }
    draw_scene_object(object, scene->camera->project_view_matrix);
}

void draw_scene(scene_t *scene) {
    draw_pass++;
    update_camera_views(scene->camera);

    scene_object_list_item_t *current_object_item = scene->objects;
    while (current_object_item != NULL) {
        draw_object(scene, current_object_item->item);
        current_object_item = current_object_item->next;
    }
}

static void
destroy_scene_object_list_item(scene_object_list_item_t **pp_scene_object_list_item) {
    scene_object_list_item_t *scene_object_list_item = *pp_scene_object_list_item;
    scene_object_list_item_t *next_item = scene_object_list_item->next;
    destroy_scene_object(&scene_object_list_item->item);
    free(scene_object_list_item);
    *pp_scene_object_list_item = next_item;
}

static void
destroy_omni_light_list_item(omni_light_list_item_t **pp_omni_light_list_item) {
    omni_light_list_item_t *omni_light_list_item = *pp_omni_light_list_item;
    omni_light_list_item_t *next_item = omni_light_list_item->next;
    destroy_omni_light(&omni_light_list_item->item);
    free(omni_light_list_item);
    *pp_omni_light_list_item = next_item;
}

static void
destroy_spot_light_list_item(spot_light_list_item_t **pp_spot_light_list_item) {
    spot_light_list_item_t *spot_light_list_item = *pp_spot_light_list_item;
    spot_light_list_item_t *next_item = spot_light_list_item->next;
    destroy_spot_light(&spot_light_list_item->item);
    free(spot_light_list_item);
    *pp_spot_light_list_item = next_item;
}

static void
destroy_direct_light_list_item(direct_light_list_item_t **pp_direct_light_list_item) {
    direct_light_list_item_t *direct_light_list_item = *pp_direct_light_list_item;
    direct_light_list_item_t *next_item = direct_light_list_item->next;
    destroy_direct_light(&direct_light_list_item->item);
    free(direct_light_list_item);
    *pp_direct_light_list_item = next_item;
}


void destroy_scene(scene_t **pp_scene) {
    scene_t *scene = *pp_scene;
    if (scene == NULL) {
        return;
    }

    destroy_camera(&scene->camera);

    while (scene->objects != NULL) {
        destroy_scene_object_list_item(&scene->objects);
    }

    while (scene->omni_lights != NULL) {
        destroy_omni_light_list_item(&scene->omni_lights);
    }

    while (scene->direct_lights != NULL) {
        destroy_direct_light_list_item(&scene->direct_lights);
    }

    while (scene->spot_lights != NULL) {
        destroy_spot_light_list_item(&scene->spot_lights);
    }

    free(scene);
    *pp_scene = NULL;
}

void attach_camera_to_scene(scene_t *scene, camera_t *camera) {
    scene->camera = camera;
}

void attach_object_to_scene(scene_t *scene, scene_object_t *scene_object) {
    scene_object_list_item_t *new_item = calloc(1, sizeof(scene_object_list_item_t));
    SDL_ALLOC_CHECK(new_item)
    new_item->enabled = true;
    new_item->item = scene_object;
    new_item->next = scene->objects;
    scene->objects = new_item;
}

void attach_omni_light_to_scene(scene_t *scene, omni_light_t *omni_light) {
    omni_light_list_item_t *new_item = calloc(1, sizeof(omni_light_list_item_t));
    SDL_ALLOC_CHECK(new_item)
    new_item->enabled = true;
    new_item->item = omni_light;
    new_item->next = scene->omni_lights;
    scene->omni_lights = new_item;
}

void attach_direct_light_to_scene(scene_t *scene, direct_light_t *direct_light) {
    direct_light_list_item_t *new_item = calloc(1, sizeof(direct_light_list_item_t));
    SDL_ALLOC_CHECK(new_item)
    new_item->enabled = true;
    new_item->item = direct_light;
    new_item->next = scene->direct_lights;
    scene->direct_lights = new_item;
}

void attach_spot_light_to_scene(scene_t *scene, spot_light_t *spot_light) {
    spot_light_list_item_t *new_item = calloc(1, sizeof(spot_light_list_item_t));
    SDL_ALLOC_CHECK(new_item)
    new_item->enabled = true;
    new_item->item = spot_light;
    new_item->next = scene->spot_lights;
    scene->spot_lights = new_item;
}

void enable_scene_object(scene_t *scene, scene_object_t *scene_object, bool enabled) {
    scene_object_list_item_t *current_item = scene->objects;
    while (current_item != NULL) {
        if (scene_object == current_item->item) {
            current_item->enabled = enabled;
            return;
        }
        current_item = current_item->next;
    }
    SDL_Die("Unable to find the object item in the scene");
}

void enable_omni_light(scene_t *scene, omni_light_t *omni_light, bool enabled) {
    omni_light_list_item_t *current_item = scene->omni_lights;
    while (current_item != NULL) {
        if (omni_light == current_item->item) {
            current_item->enabled = enabled;
            return;
        }
        current_item = current_item->next;
    }
    SDL_Die("Unable to find the omni light item in the scene");
}

void enable_direct_light(scene_t *scene, direct_light_t *direct_light, bool enabled) {
    direct_light_list_item_t *current_item = scene->direct_lights;
    while (current_item != NULL) {
        if (direct_light == current_item->item) {
            current_item->enabled = enabled;
            return;
        }
        current_item = current_item->next;
    }
    SDL_Die("Unable to find the direct light item in the scene");
}

void enable_spot_light(scene_t *scene, spot_light_t *spot_light, bool enabled) {
    spot_light_list_item_t *current_item = scene->spot_lights;
    while (current_item != NULL) {
        if (spot_light == current_item->item) {
            current_item->enabled = enabled;
            return;
        }
        current_item = current_item->next;
    }
    SDL_Die("Unable to find the spot light item in the scene");
}