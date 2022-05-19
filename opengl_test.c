#include "opengl/sdl_ext.h"
#include "opengl/cglm_ext.h"
#include <stdbool.h>
#include <GL/gl.h>
#include "cglm/cglm.h"
#include "opengl/camera.h"
#include "opengl/shader.h"
#include "models/cube.h"
#include "opengl/material.h"
#include "opengl/light.h"
#include "opengl/model.h"
#include "opengl/scene_object.h"
#include "opengl/scene.h"
#include "opengl/cubemap.h"

static int window_width = 1280;
static int window_height = 1280 / 16 * 9;

static const Uint32 FPS = 30;
static const Uint32 FPS_SIZE_MS = 1000 / FPS;

static scene_t *scene;

static spot_light_t *flying_spot_light;
static scene_object_t *flying_spot_lighter;
static omni_light_t *flying_omni_light;
static scene_object_t *flying_omni_lighter;
static direct_light_t *flying_direct_light;

static spot_light_t *camera_light;
static direct_light_t *direct_light;
static omni_light_t *omni_light;
static scene_object_t *cubes[4];

static SDL_Window *window = NULL;
static SDL_GLContext context = NULL;

static bool initialize_app();

static void update_screen();

static void event_loop();

static void shutdown_app();

int main() {
    atexit(shutdown_app);
    if (!initialize_app()) {
        exit(1);
    }
    event_loop();
}


static void
event_loop() {
    SDL_Event event;
    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return;
            } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == 1) {
                select_object(scene, event.button.x, event.button.y);
            } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                SDL_GetWindowSize(window, &window_width, &window_height);
                glViewport(0, 0, window_width, window_height);
                set_aspect_ratio(scene->camera, window_width, window_height);
            } else if (event.type == SDL_MOUSEMOTION && event.motion.state & SDL_BUTTON_RMASK) {
                move_camera_front(scene->camera, event.motion.xrel, event.motion.yrel);
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_l: {
                        // toggle camera light
                        camera_light->enabled = !camera_light->enabled;
                        flying_spot_light->enabled = !flying_spot_light->enabled;
                        break;
                    }
                    case SDLK_TAB:
                        select_next_object(scene);
                        break;
                    case SDLK_a: // move camera right according to right vector
                        yaw_camera(scene->camera, -1);
                        break;
                    case SDLK_d: // move camera left according to up vector
                        yaw_camera(scene->camera, 1);
                        break;
                    case SDLK_r: // move camera up according to up vector
                        pitch_camera(scene->camera, 1);
                        break;
                    case SDLK_f: // move camera down according to up vector
                        pitch_camera(scene->camera, -1);
                        break;
                    case SDLK_w: // move camera forward
                        move_camera(scene->camera, 1);
                        break;
                    case SDLK_s: // move camera backward
                        move_camera(scene->camera, -1);
                        break;
                    case SDLK_q: // rotate left around sight
                        roll_camera(scene->camera, -1);
                        break;
                    case SDLK_e:  // rotate right around sight
                        roll_camera(scene->camera, 1);
                        break;
                    case SDLK_z: // reset camera position
                        camera_init(scene->camera, window_width, window_height);
                        break;
                    case SDLK_p: // change polygon mode
                        scene->camera->polygon_mode = scene->camera->polygon_mode == GL_FILL ? GL_LINE : GL_FILL;
                        break;
                    case SDLK_o: {
                        // toggle omni light
                        omni_light->enabled = !omni_light->enabled;
                        flying_omni_light->enabled = !flying_omni_light->enabled;
                        break;
                    }
                    case SDLK_t: {
                        // toggle direct light
                        direct_light->enabled = !direct_light->enabled;
                        flying_direct_light->enabled = !flying_direct_light->enabled;
                        break;
                    }
                    case SDLK_y: {
                        scene->effect_type++;
                        if (scene->effect_type >= EFFECT_LAST_TYPE) {
                            scene->effect_type = EFFECT_NONE;
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
        update_screen();
        SDL_Delay(FPS_SIZE_MS);
        SDL_CHECK_ERROR;
    }
}

static void
update_cubes() {
    float x = 0.01f;
    float y = 0.012f;
    float z = 0.013f;

    rotate_scene_object_by(cubes[0], x, 0, 0);
    rotate_scene_object_by(cubes[1], 0, y, 0);
    rotate_scene_object_by(cubes[2], 0, 0, z);
    rotate_scene_object_by(cubes[3], x, y, z);
}

static void
update_camera_light() {
    glm_vec3_copy(scene->camera->position, camera_light->position);
    glm_vec3_copy(scene->camera->front, camera_light->front);
}

static void
update_flying_lights() {
    int period_ms = 30000;
    Uint32 int_value = SDL_GetTicks() % period_ms;
    double angle = M_PI * 2 * int_value / 30000;
    double phase = M_PI * 2 / 3;
    float range = 20.0f;

    // spot light
    float x = range * (float) cos(angle);
    float y = range * (float) sin(angle);
    float z = 0.0f;
    vec3_set(flying_spot_light->position, x, y, z);
    vec3_set(flying_spot_light->front, -x, -y, -z);
    move_scene_object_to_vec(flying_spot_lighter, flying_spot_light->position);

    // direct light
    angle += phase;
    y = range * (float) cos(angle);
    z = range * (float) sin(angle);
    x = 0.0f;
    vec3_set(flying_direct_light->front, -x, -y, -z);

    // omni light
    angle += phase;
    z = range * (float) cos(angle);
    x = range * (float) sin(angle);
    y = 0.0f;
    vec3_set(flying_omni_light->position, x, y, z);
    move_scene_object_to_vec(flying_omni_lighter, flying_omni_light->position);
}

static void
update_scene() {
    update_cubes();
    update_camera_light();
    update_flying_lights();
}

static void
update_screen() {
    update_scene();
    render_scene(scene);
    SDL_GL_SwapWindow(window);
    SDL_CHECK_ERROR;
}


static scene_object_t *create_lighter(model_t *cube_model, shader_t *model_shader) {
    scene_object_t *lighter = create_scene_object();
    attach_object_to_scene(scene, lighter);
    attach_model_to_scene_object(lighter, cube_model);
    attach_shader_to_scene_object(lighter, model_shader);
    scale_scene_object(lighter, 0.2f);
    return lighter;
}

static void
initialize_scene() {
    scene = create_scene();

    set_scene_skybox(scene, create_cubemap("assets/textures/skybox/%s.jpg"));

    camera_t *camera = create_camera();
    attach_camera_to_scene(scene, camera);
    camera_init(camera, window_width, window_height);
    camera->far_z = 250.0f;

    omni_light = create_omni_light();
    attach_omni_light_to_scene(scene, omni_light);
    vec3_set(omni_light->position, 0.0f, -3.0f, 0.0f);
    vec4_set(omni_light->light_prop.ambient, 0.025f, 0.025f, 0.025f, 1.0f);
    vec4_set(omni_light->light_prop.diffuse, 0.8f, 0.8f, 0.8f, 1.0f);
    vec4_set(omni_light->light_prop.specular, 0.8f, 0.8f, 0.8f, 1.0f);

    flying_omni_light = create_omni_light();
    attach_omni_light_to_scene(scene, flying_omni_light);
    memcpy(flying_omni_light, omni_light, sizeof(omni_light_t));

    direct_light = create_direct_light();
    attach_direct_light_to_scene(scene, direct_light);
    vec3_set(direct_light->front, 1.0f, -3.0f, 1.0f);
    vec4_set(direct_light->light_prop.ambient, 0.025f, 0.025f, 0.025f, 1.0f);
    vec4_set(direct_light->light_prop.diffuse, 0.8f, 0.8f, 0.8f, 1.0f);
    vec4_set(direct_light->light_prop.specular, 0.8f, 0.8f, 0.8f, 1.0f);

    flying_direct_light = create_direct_light();
    attach_direct_light_to_scene(scene, flying_direct_light);
    memcpy(flying_direct_light, direct_light, sizeof(direct_light_t));

    camera_light = create_spot_light();
    attach_spot_light_to_scene(scene, camera_light);
    vec4_set(camera_light->light_prop.ambient, 0.01f, 0.01f, 0.01f, 1.0f);
    vec4_set(camera_light->light_prop.diffuse, 0.8f, 0.8f, 0.8f, 1.0f);
    vec4_set(camera_light->light_prop.specular, 0.8f, 0.8f, 0.8f, 1.0f);
    camera_light->angle = 10.5f * (float) M_PI / 180.0f;
    camera_light->smooth_angle = 2.0f * (float) M_PI / 180.0f;

    flying_spot_light = create_spot_light();
    attach_spot_light_to_scene(scene, flying_spot_light);
    memcpy(flying_spot_light, camera_light, sizeof(spot_light_t));

    model_t *cube_model = cube_model_create();
    shader_t *model_shader = load_shader("shaders/model_vertex.glsl", "shaders/model_fragment.glsl");

    // cubes
    float scale = 2.0f;
    for (int i = 0; i < 4; i++) {
        cubes[i] = create_scene_object();
        attach_object_to_scene(scene, cubes[i]);
        attach_model_to_scene_object(cubes[i], cube_model);
        attach_shader_to_scene_object(cubes[i], model_shader);
        scale_scene_object(cubes[i], scale);
        scale *= 2;
    }
    move_scene_object_to(cubes[0], -2, -2, -2);
    move_scene_object_to(cubes[1], -4, 4, -4);
    move_scene_object_to(cubes[2], 8, 8, -8);
    move_scene_object_to(cubes[3], 16, -16, -16);

    // subaru
    scene_object_t *subaru = create_scene_object();
    attach_object_to_scene(scene, subaru);
    attach_shader_to_scene_object(subaru, model_shader);
    attach_model_to_scene_object(subaru, load_model("assets/models/Subaru Impreza/subaru2.obj", aiProcess_FlipUVs));
    move_scene_object_to(subaru, -30, -2, 2);

    // handgun
    scene_object_t *handgun = create_scene_object();
    attach_object_to_scene(scene, handgun);
    attach_shader_to_scene_object(handgun, model_shader);
    attach_model_to_scene_object(handgun, load_model("assets/models/handgun/handgun.obj", aiProcess_FlipUVs));
    move_scene_object_to(handgun, -22, -2, 2);
    scale_scene_object(handgun, 3.0f);


    // spider
    scene_object_t *spider_obj = create_scene_object();
    attach_object_to_scene(scene, spider_obj);
    attach_shader_to_scene_object(spider_obj, model_shader);
    attach_model_to_scene_object(spider_obj,
                                 load_model("assets/models/spider/spider.obj",
                                            aiProcess_FlipUVs));
    move_scene_object_to(spider_obj, -12, -4, 2);
    scale_scene_object(spider_obj, 0.06f);

    // sirenhead
    scene_object_t *sirenhead = create_scene_object();
    attach_object_to_scene(scene, sirenhead);
    attach_shader_to_scene_object(sirenhead, model_shader);
    attach_model_to_scene_object(sirenhead,
                                 load_model("assets/models/sirenhead/source/sirenhead.obj", aiProcess_FlipUVs));
    move_scene_object_to(sirenhead, -4, -4, 4);
    scale_scene_object(sirenhead, 3.0f);

    // backpack
    scene_object_t *backpack = create_scene_object();
    attach_object_to_scene(scene, backpack);
    attach_shader_to_scene_object(backpack, model_shader);
    attach_model_to_scene_object(backpack, load_model("assets/models/backpack/backpack.obj", 0));
    move_scene_object_to(backpack, 4, -2, 4);

    // male figure
    scene_object_t *male_figure = create_scene_object();
    attach_object_to_scene(scene, male_figure);
    attach_shader_to_scene_object(male_figure, model_shader);
    attach_model_to_scene_object(male_figure, load_model("assets/models/male/FinalBaseMesh.obj", 0));
    move_scene_object_to(male_figure, 12, -4, 2);
    scale_scene_object(male_figure, 0.3f);

    // lego man
    scene_object_t *lego_man = create_scene_object();
    attach_object_to_scene(scene, lego_man);
    attach_shader_to_scene_object(lego_man, model_shader);
    attach_model_to_scene_object(lego_man,
                                 load_model("assets/models/lego_man/lego obj.obj", 0));
    move_scene_object_to(lego_man, 18, -4, 2);
    scale_scene_object(lego_man, 0.1f);

    // t-rex1
    scene_object_t *t_rex1 = create_scene_object();
    attach_object_to_scene(scene, t_rex1);
    attach_shader_to_scene_object(t_rex1, model_shader);
    attach_model_to_scene_object(t_rex1,
                                 load_model("assets/models/TrexModelByJoel3d_FBX/TrexByJoel3d.fbx.obj",
                                            aiProcess_FlipUVs));
    move_scene_object_to(t_rex1, 28, -4, 2);
    scale_scene_object(t_rex1, 40.0f);

    // lighter object
    scene_object_t *lighter = create_lighter(cube_model, model_shader);
    move_scene_object_to_vec(lighter, omni_light->position);

    flying_omni_lighter = create_lighter(cube_model, model_shader);
    flying_spot_lighter = create_lighter(cube_model, model_shader);
}


static bool
initialize_app() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Die("Failed to initialize SDL: %s\n", SDL_GetError());
    }
    window = SDL_CreateWindow("program", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_CHECK_ERROR;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_CHECK_ERROR;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_CHECK_ERROR;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_CHECK_ERROR;

    context = SDL_GL_CreateContext(window);
    SDL_CHECK_ERROR;
    SDL_GL_SetSwapInterval(1);
    SDL_CHECK_ERROR;

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "OpenGL information:\n\tVendor: %s\n\tRenderer: %s\n\tVersion: %s\n\tShading language: %s",
                glGetString(GL_VENDOR),
                glGetString(GL_RENDERER),
                glGetString(GL_VERSION),
                glGetString(GL_SHADING_LANGUAGE_VERSION)
    );

    initialize_scene();

    return true;
}

static void
shutdown_app() {
    destroy_scene(&scene);

    if (context) {
        SDL_GL_DeleteContext(context);
        context = NULL;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }

    SDL_Quit();
}