#include "scene.h"
#include "sdl_ext.h"
#include "assert.h"

static unsigned int draw_pass = 0;

static void
destroy_scene_screen_contents(scene_screen_t *scene_screen) {
    if (scene_screen == NULL) {
        return;
    }
    if (scene_screen->frame_buffer >= 0) {
        glDeleteFramebuffers(1, &scene_screen->frame_buffer);
        scene_screen->frame_buffer = -1;
    }
    if (scene_screen->texture >= 0) {
        glDeleteTextures(1, &scene_screen->texture);
        scene_screen->texture = -1;
    }
    if (scene_screen->render_buffer >= 0) {
        glDeleteRenderbuffers(1, &scene_screen->render_buffer);
        scene_screen->render_buffer = -1;
    }
    if (scene_screen->vertex_array >= 0) {
        glDeleteVertexArrays(1, &scene_screen->vertex_array);
        scene_screen->vertex_array = -1;
    }
    if (scene_screen->vertex_buffer >= 0) {
        glDeleteBuffers(1, &scene_screen->vertex_buffer);
        scene_screen->vertex_buffer = -1;
    }
}

static void
destroy_scene_screen(scene_screen_t **pp_scene_screen) {
    scene_screen_t *scene_screen = *pp_scene_screen;
    if (scene_screen == NULL) {
        return;
    }
    destroy_scene_screen_contents(scene_screen);
    free(scene_screen);
    *pp_scene_screen = NULL;
}

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
        if (current_light->item->enabled) {
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
        if (current_light->item->enabled) {
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
        if (current_light->item->enabled) {
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

static void
init_scene_screen(scene_t *scene) {
    camera_t *camera = scene->camera;
    if (camera == NULL) {
        SDL_Die("Attempt to draw without a camera");
        assert(camera != NULL);
    }

    scene_screen_t *scene_screen = scene->scene_screen;
    if (scene_screen != NULL && scene_screen->width == camera->viewport_width &&
        scene_screen->height == camera->viewport_height) {
        return;
    }

    destroy_scene_screen_contents(scene->scene_screen);
    if (scene_screen == NULL) {
        scene_screen = scene->scene_screen = calloc(1, sizeof(scene_screen_t));
        SDL_ALLOC_CHECK(scene_screen)
        assert(scene_screen != NULL);
    }

    scene_screen->width = camera->viewport_width;
    scene_screen->height = camera->viewport_height;

    glGenFramebuffers(1, &scene_screen->frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, scene_screen->frame_buffer);
    GL_CHECK_ERROR;

    glGenTextures(1, &scene_screen->texture);
    glBindTexture(GL_TEXTURE_2D, scene_screen->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (int) scene_screen->width, (int) scene_screen->height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scene_screen->texture, 0);
    GL_CHECK_ERROR;

    glGenRenderbuffers(1, &scene_screen->render_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, scene_screen->render_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (int) scene_screen->width, (int) scene_screen->height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              scene_screen->render_buffer);
    if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
        SDL_Die("Frame buffer incomplete");
    }
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    GL_CHECK_ERROR;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // feels like this data is pretty static and may be created only once
    glGenVertexArrays(1, &scene_screen->vertex_array);
    glBindVertexArray(scene_screen->vertex_array);

    glGenBuffers(1, &scene_screen->vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, scene_screen->vertex_buffer);

    const float screen_data[] = {
            // positions   // texture coords
            -1.0f, 1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,

            -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(screen_data), screen_data, GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);

    // texture coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));

    GL_CHECK_ERROR;

    glBindVertexArray(0);
}

void draw_scene(scene_t *scene) {
    init_scene_screen(scene);
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

void
destroy_scene(scene_t **pp_scene) {
    scene_t *scene = *pp_scene;
    if (scene == NULL) {
        return;
    }

    destroy_camera(&scene->camera);
    destroy_scene_screen(&scene->scene_screen);

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
    new_item->item = scene_object;
    new_item->next = scene->objects;
    scene->objects = new_item;
}

void attach_omni_light_to_scene(scene_t *scene, omni_light_t *omni_light) {
    omni_light_list_item_t *new_item = calloc(1, sizeof(omni_light_list_item_t));
    SDL_ALLOC_CHECK(new_item)
    omni_light->enabled = true;
    new_item->item = omni_light;
    new_item->next = scene->omni_lights;
    scene->omni_lights = new_item;
}

void attach_direct_light_to_scene(scene_t *scene, direct_light_t *direct_light) {
    direct_light_list_item_t *new_item = calloc(1, sizeof(direct_light_list_item_t));
    SDL_ALLOC_CHECK(new_item)
    direct_light->enabled = true;
    new_item->item = direct_light;
    new_item->next = scene->direct_lights;
    scene->direct_lights = new_item;
}

void attach_spot_light_to_scene(scene_t *scene, spot_light_t *spot_light) {
    spot_light_list_item_t *new_item = calloc(1, sizeof(spot_light_list_item_t));
    SDL_ALLOC_CHECK(new_item)
    spot_light->enabled = true;
    new_item->item = spot_light;
    new_item->next = scene->spot_lights;
    scene->spot_lights = new_item;
}
