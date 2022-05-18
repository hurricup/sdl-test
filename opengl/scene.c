#include <GL/gl.h>
#include "scene.h"
#include "sdl_ext.h"
#include "assert.h"

static unsigned int render_pass = 0;

/**
 * Destroys the object we are drawing on the scene screen (two triangles) and shader
 */
static void
destroy_scene_screen_object_content(scene_screen_object_t *scene_screen_object) {
    if (scene_screen_object->vertex_array >= 0) {
        glDeleteVertexArrays(1, &scene_screen_object->vertex_array);
        scene_screen_object->vertex_array = -1;
    }
    if (scene_screen_object->vertex_buffer >= 0) {
        glDeleteBuffers(1, &scene_screen_object->vertex_buffer);
        scene_screen_object->vertex_buffer = -1;
    }
    if (scene_screen_object->shader != NULL) {
        detach_shader(&scene_screen_object->shader);
    }
}

/**
 * Initialize persistent data like vertex arrays and shaders. This data persists through the scene lifetime
 */
static void
init_scene_screen_object(scene_screen_object_t *scene_screen_object) {
    glGenVertexArrays(1, &scene_screen_object->vertex_array);
    glBindVertexArray(scene_screen_object->vertex_array);

    glGenBuffers(1, &scene_screen_object->vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, scene_screen_object->vertex_buffer);

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

    glBindVertexArray(0);

    shader_t *shader = load_shader("shaders/scene_screen_vertex.glsl", "shaders/scene_screen_fragment.glsl");
    attach_shader(&scene_screen_object->shader, shader);
}

scene_t *
create_scene() {
    scene_t *scene = calloc(1, sizeof(scene_t));
    SDL_ALLOC_CHECK(scene);
    init_scene_screen_object(&scene->scene_screen_object);
    attach_shader(&scene->selection_shader,
                  load_shader("shaders/selection_vertex.glsl", "shaders/selection_fragment.glsl"));
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
set_up_light_and_camera(scene_t *scene, rendering_context_t *context) {
    shader_t *shader = context->shader;
    if (context->add_lights) {
        set_up_omni_lights(scene, shader);
        set_up_direct_lights(scene, shader);
        set_up_spot_lights(scene, shader);
    }

    if (context->add_camera_position) {
        shader_set_vec3(shader, "camera_position", scene->camera->position);
    }
}

static void
render_object(scene_t *scene, scene_object_t *object, rendering_context_t *context) {
    shader_t *shader = context->shader;
    shader_use(shader);
    if (shader->render_pass != render_pass) {
        set_up_light_and_camera(scene, context);
        shader->render_pass = render_pass;
    }
    render_scene_object(object, scene->camera->project_view_matrix, context);
}

static void
set_up_scene_options(scene_t *scene) {
    glEnable(GL_DEPTH_TEST); // enables z-buffering
    GL_CHECK_ERROR;

    glEnable(GL_BLEND);
    GL_CHECK_ERROR;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GL_CHECK_ERROR;

    glEnable(GL_CULL_FACE);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    GL_CHECK_ERROR;

    glPolygonMode(GL_FRONT_AND_BACK, scene->camera->polygon_mode);
    GL_CHECK_ERROR;
}

static void
render_scene_screen(scene_t *scene) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    scene_screen_t *scene_screen = &scene->scene_screen;
    scene_screen_object_t *scene_screen_object = &scene->scene_screen_object;
    shader_t *shader = scene_screen_object->shader;
    shader_use(shader);
    glBindVertexArray(scene_screen_object->vertex_array);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene_screen->texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, scene->selection_screen.texture);
    shader_set_int(shader, "effect_type", scene->effect_type);
    shader_set_float(shader, "step_x", 1.0f / (float) scene_screen->width);
    shader_set_float(shader, "step_y", 1.0f / (float) scene_screen->height);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    GL_CHECK_ERROR;

    glFlush();
}

/**
 * Drawing selected object with specific shader
 * This method MUST be invoked AFTER render_scene_fair, because it does not do some common stuff
 */
static void
render_selected_objects(scene_t *scene) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    rendering_context_t drawing_context = {0, scene->selection_shader, false, false, false, false, false};
    shader_use(drawing_context.shader);
    scene_object_list_item_t *current_object_item = scene->objects;
    while (current_object_item != NULL) {
        scene_object_t *current_object = current_object_item->item;
        if (current_object != NULL && current_object->selected) {
            render_scene_object(current_object, scene->camera->project_view_matrix, &drawing_context);
        }
        current_object_item = current_object_item->next;
    }
    glFlush();
}

static void
render_scene_fair(scene_t *scene) {
    scene_object_list_item_t *current_object_item = scene->objects;
    rendering_context_t context = {0, NULL, true, true, true, true, false};
    while (current_object_item != NULL) {
        scene_object_t *current_object = current_object_item->item;
        context.shader = current_object->shader;
        render_object(scene, current_object, &context);
        context.object_counter++;
        current_object_item = current_object_item->next;
    }
    glFlush();
}

void render_scene(scene_t *scene) {
    render_pass++;

    // drawing to the scene screen
    update_scene_screen(&scene->scene_screen, scene->camera);
    glBindFramebuffer(GL_FRAMEBUFFER, scene->scene_screen.render_buffer);
    set_up_scene_options(scene);
    update_camera_views(scene->camera);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    render_scene_fair(scene);

    // drawing selected objects
    update_scene_screen(&scene->selection_screen, scene->camera);
    glBindFramebuffer(GL_FRAMEBUFFER, scene->selection_screen.render_buffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    render_selected_objects(scene);

    // drawing results
    render_scene_screen(scene);
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
    destroy_scene_screen_contents(&scene->scene_screen);
    destroy_scene_screen_contents(&scene->selection_screen);
    destroy_scene_screen_object_content(&scene->scene_screen_object);

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

    detach_shader(&scene->selection_shader);

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

void
select_next_object(scene_t *scene) {
    scene_object_list_item_t *current_item = scene->objects;
    if (current_item == NULL) {
        return;
    }
    while (current_item != NULL) {
        if (current_item->item != NULL && current_item->item->selected) {
            current_item->item->selected = false;
            if (current_item->next == NULL) {
                break;
            }
            if (current_item->next->item != NULL) {
                current_item->next->item->selected = true;
                return;
            }
        }
        current_item = current_item->next;
    }
    if (scene->objects != NULL && scene->objects->item != NULL) {
        scene->objects->item->selected = true;
    }
}