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
    attach_shader(&scene->indexed_color_shader,
                  load_shader("shaders/selection_vertex.glsl", "shaders/indexed_color_fragment.glsl"));
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
    shader_set_float(shader, "time", (float) SDL_GetTicks());

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

    rendering_context_t render_context = {scene->selection_shader, false, false, false, false, false, {0, 0, 0}, 0};
    shader_use(render_context.shader);
    scene_object_list_item_t *current_object_item = scene->objects;
    while (current_object_item != NULL) {
        scene_object_t *current_object = current_object_item->item;
        if (current_object != NULL && current_object->selected) {
            render_scene_object(current_object, scene->camera->project_view_matrix, &render_context);
        }
        current_object_item = current_object_item->next;
    }
    glFlush();
}

static void
render_scene_fair(scene_t *scene) {
    glPolygonMode(GL_FRONT_AND_BACK, scene->camera->polygon_mode);
    GL_CHECK_ERROR;

    scene_object_list_item_t *current_object_item = scene->objects;
    rendering_context_t context = {NULL, true, true, true, true, false, {0, 0, 0}, 0};
    if (scene->skybox.cubemap != NULL) {
        context.skybox_texture = scene->skybox.cubemap->texture;
    }
    while (current_object_item != NULL) {
        scene_object_t *current_object = current_object_item->item;
        context.shader = current_object->shader;
        render_object(scene, current_object, &context);
        current_object_item = current_object_item->next;
    }
    glFlush();
}

static void
prepare_selection_screen(scene_t *scene) {
    update_scene_screen(&scene->selection_screen, scene->camera);
    glBindFramebuffer(GL_FRAMEBUFFER, scene->selection_screen.render_buffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void
prepare_scene_screen(scene_t *scene) {
    update_scene_screen(&scene->scene_screen, scene->camera);
    glBindFramebuffer(GL_FRAMEBUFFER, scene->scene_screen.render_buffer);
    set_up_scene_options(scene);
    update_camera_views(scene->camera);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void
render_skybox(scene_t *scene) {
    skybox_t *skybox = &scene->skybox;
    if (skybox->cubemap == NULL) {
        return;
    }

    glDepthFunc(GL_LEQUAL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    shader_use(skybox->shader);
    glBindVertexArray(skybox->vertex_array);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->cubemap->texture);
    mat4 skybox_project_view = {0};
    compute_skybox_project_view_matrix(scene->camera, skybox_project_view);
    shader_set_mat4(skybox->shader, "project_view", skybox_project_view);
    GL_CHECK_ERROR;
    glDrawArrays(GL_TRIANGLES, 0, 36);
    GL_CHECK_ERROR;
    glDepthFunc(GL_LESS);
}

void
render_scene(scene_t *scene) {
    render_pass++;

    // drawing to the scene screen
    prepare_scene_screen(scene);
    render_scene_fair(scene);
    render_skybox(scene);

    // drawing selected objects
    prepare_selection_screen(scene);
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

static void
destroy_skybox_data(scene_t *scene) {
    set_scene_skybox(scene, NULL);
    skybox_t *skybox = &scene->skybox;
    detach_shader(&skybox->shader);

    if (skybox->vertex_array >= 0) {
        glDeleteVertexArrays(1, &skybox->vertex_array);
        skybox->vertex_array = -1;
    }

    if (skybox->vertex_buffer >= 0) {
        glDeleteBuffers(1, &skybox->vertex_buffer);
        skybox->vertex_buffer = -1;
    }
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
    detach_shader(&scene->indexed_color_shader);

    destroy_skybox_data(scene);

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

static void
encode_unsigned_to_color(unsigned int number, vec3 color) {
    float step = 1.0f / 255;
    color[0] = step * (float) (number & 0xff);
    color[1] = step * (float) (number >> 8 & 0xff);
    color[2] = step * (float) (number >> 16 & 0xff);
}

static void
render_with_indexed_colors(scene_t *scene) {
    render_pass++;
    prepare_scene_screen(scene);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    rendering_context_t context = {scene->indexed_color_shader, false, false, false, false, true, {0, 0, 0}, 0};

    scene_object_list_item_t *current_object_item = scene->objects;
    unsigned int object_counter = 1;
    while (current_object_item != NULL) {
        scene_object_t *current_object = current_object_item->item;
        encode_unsigned_to_color(object_counter, context.index_color);
        object_counter++;
        render_object(scene, current_object, &context);
        current_object_item = current_object_item->next;
    }
    glFlush();
}

static scene_object_t *
find_object_by_index_color(scene_t *scene, unsigned int screen_x, unsigned int screen_y) {
    float color[3] = {0};
    glReadPixels((GLint) screen_x, (GLint) (scene->selection_screen.height - screen_y), 1, 1, GL_RGB, GL_FLOAT, color);
    unsigned int object_index = (unsigned int) (color[0] * 255) +
                                ((unsigned int) (color[1] * 255) << 8) +
                                ((unsigned int) (color[2] * 255) << 16);
    if (object_index == 0) {
        return NULL;
    }
    scene_object_list_item_t *current_item = scene->objects;
    while (current_item != NULL) {
        if (--object_index == 0) {
            return current_item->item;
        }
        current_item = current_item->next;
    }
}

static scene_object_t *
find_object_at(scene_t *scene, unsigned int screen_x, unsigned int screen_y) {
    render_with_indexed_colors(scene);
    return find_object_by_index_color(scene, screen_x, screen_y);
}

void
select_object(scene_t *scene, unsigned int screen_x, unsigned int screen_y) {
    scene_object_t *object_at_coords = find_object_at(scene, screen_x, screen_y);
    if (object_at_coords == NULL) {
        return;
    }
    scene_object_list_item_t *current_item = scene->objects;
    while (current_item != NULL) {
        current_item->item->selected = (current_item->item == object_at_coords);
        current_item = current_item->next;
    }
}

void
select_next_object(scene_t *scene) {
    scene_object_list_item_t *current_item = scene->objects;
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

static void
init_scene_skybox(skybox_t *skybox) {
    if (skybox->shader != NULL) {
        return;
    }
    attach_shader(&skybox->shader, load_shader("shaders/skybox_vertex.glsl", "shaders/skybox_fragment.glsl"));
    glGenVertexArrays(1, &skybox->vertex_array);
    glBindVertexArray(skybox->vertex_array);

    glGenBuffers(1, &skybox->vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, skybox->vertex_buffer);

    const float screen_data[] = {
            // positions
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(screen_data), screen_data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);

    glBindVertexArray(0);
}


void
set_scene_skybox(scene_t *scene, cubemap_t *cubemap) {
    if (scene->skybox.cubemap != NULL) {
        detach_cubemap(&scene->skybox.cubemap);
    }
    if (cubemap != NULL) {
        attach_cubemap(&scene->skybox.cubemap, cubemap);
        init_scene_skybox(&scene->skybox);
    }
}