#include "opengl/sdl_ext.h"
#include "opengl/gl_ext.h"
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

static int window_width = 1280;
static int window_height = 1280 / 16 * 9;

static const Uint32 FPS = 30;
static const Uint32 FPS_SIZE_MS = 1000 / FPS;

static camera_t camera;
static bool camera_light_on = true;

static scene_object_t *backpack;
static scene_object_t *cubes[4];
static scene_object_t *sirenhead;
static scene_object_t *light;
static scene_object_t *male_figure;
static scene_object_t *spider_obj;
static scene_object_t *lego_man;
static scene_object_t *t_rex1;

static omni_light_t omni_light = {
        {0.0f,  -3.0f, 0.0f},
        {0.95f, 0.95f, 0.95f},
        {0.95f, 0.95f, 0.95f},
        {0.95f, 0.95f, 0.95f}
};
static bool omni_light_on = true;

static direct_light_t direct_light = {
        {1.0f,  -3.0f, 1.0f},
        {0.95f, 0.95f, 0.95f},
        {0.95f, 0.95f, 0.95f},
        {0.95f, 0.95f, 0.95f}
};
static bool direct_light_on = true;

static spot_light_t spot_light = {
        {
                {0.0f, 0.0f, 0.0f},
                {0.95f, 0.95f, 0.95f},
                {0.95f, 0.95f, 0.95f},
                {0.95f, 0.95f, 0.95f}
        },
        {0.0f, 0.0f, 0.0f},
        10.5f * (float) M_PI / 180.0f,
        2.0f * (float) M_PI / 180.0f,
};

static int polygon_mode = GL_FILL;

static mat4 view_m = GLM_MAT4_IDENTITY;
static mat4 project_m = GLM_MAT4_IDENTITY;
static mat4 project_view = GLM_MAT4_IDENTITY;

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
            } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                SDL_GetWindowSize(window, &window_width, &window_height);
                glViewport(0, 0, window_width, window_height);
            } else if (event.type == SDL_MOUSEMOTION && event.motion.state & SDL_BUTTON_RMASK) {
                move_camera_front(&camera, event.motion.xrel, event.motion.yrel);
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_l: // toggle camera light
                        camera_light_on = !camera_light_on;
                        break;
                    case SDLK_a: // move camera right according to right vector
                        yaw_camera(&camera, -1);
                        break;
                    case SDLK_d: // move camera left according to up vector
                        yaw_camera(&camera, 1);
                        break;
                    case SDLK_r: // move camera up according to up vector
                        pitch_camera(&camera, 1);
                        break;
                    case SDLK_f: // move camera down according to up vector
                        pitch_camera(&camera, -1);
                        break;
                    case SDLK_w: // move camera forward
                        move_camera(&camera, 1);
                        break;
                    case SDLK_s: // move camera backward
                        move_camera(&camera, -1);
                        break;
                    case SDLK_q: // rotate left around sight
                        roll_camera(&camera, -1);
                        break;
                    case SDLK_e:  // rotate right around sight
                        roll_camera(&camera, 1);
                        break;
                    case SDLK_z: // reset camera position
                        camera_init(&camera);
                        break;
                    case SDLK_p: // change polygon mode
                        polygon_mode = polygon_mode == GL_FILL ? GL_LINE : GL_FILL;
                        break;
                    case SDLK_o: // toggle omni light
                        omni_light_on = !omni_light_on;
                        break;
                    case SDLK_t: // toggle direct light
                        direct_light_on = !direct_light_on;
                        break;
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
draw_light() {
    shader_use(light->shader);
    shader_set_vec4(light->shader, "vertex_color", omni_light.specular);
    draw_scene_object(light, project_view);
}

static void set_up_light_and_camera(shader_t *shader) {
    // omni-light
    shader_set_int(shader, "omni_light_on", omni_light_on);
    if (omni_light_on) {
        shader_set_vec3(shader, "omni_light.position", omni_light.position);
        shader_set_vec4(shader, "omni_light.light_prop.ambient", omni_light.ambient);
        shader_set_vec4(shader, "omni_light.light_prop.diffuse", omni_light.diffuse);
        shader_set_vec4(shader, "omni_light.light_prop.specular", omni_light.specular);
    }

    // direct light
    shader_set_int(shader, "direct_light_on", direct_light_on);
    if (direct_light_on) {
        shader_set_vec3(shader, "direct_light.front", direct_light.front);
        shader_set_vec4(shader, "direct_light.light_prop.ambient", direct_light.ambient);
        shader_set_vec4(shader, "direct_light.light_prop.diffuse", direct_light.diffuse);
        shader_set_vec4(shader, "direct_light.light_prop.specular", direct_light.specular);
    }

    // spot light
    if (camera_light_on) {
        shader_set_vec4(shader, "spot_light.light_prop.ambient", spot_light.light.ambient);
        shader_set_vec4(shader, "spot_light.light_prop.diffuse", spot_light.light.diffuse);
        shader_set_vec4(shader, "spot_light.light_prop.specular", spot_light.light.specular);
        shader_set_vec3(shader, "spot_light.position", spot_light.light.position);
        shader_set_vec3(shader, "spot_light.front", spot_light.front);
        shader_set_float(shader, "spot_light.angle_cos", (float) cos((double) spot_light.angle));
        shader_set_float(shader, "spot_light.smooth_angle_cos",
                         (float) cos((double) spot_light.angle + spot_light.smooth_angle));
    } else {
        shader_set_float(shader, "spot_light.angle_cos", 0.0f);
    }

    // camera position
    shader_set_vec3(shader, "camera_position", camera.pos);
}

static void
draw_backpack() {
    shader_t *shader = backpack->shader;
    shader_use(shader);
    set_up_light_and_camera(shader);
    draw_scene_object(backpack, project_view);
}

static void
draw_sirenhead() {
    shader_t *shader = sirenhead->shader;
    shader_use(shader);
    set_up_light_and_camera(shader);
    draw_scene_object(sirenhead, project_view);
}

static void
draw_male() {
    shader_t *shader = male_figure->shader;
    shader_use(shader);
    set_up_light_and_camera(shader);
    draw_scene_object(male_figure, project_view);
}

static void
draw_spider() {
    shader_t *shader = spider_obj->shader;
    shader_use(shader);
    set_up_light_and_camera(shader);
    draw_scene_object(spider_obj, project_view);
}

static void
draw_lego_man() {
    shader_t *shader = lego_man->shader;
    shader_use(shader);
    set_up_light_and_camera(shader);
    draw_scene_object(lego_man, project_view);
}

static void
draw_t_rex1() {
    shader_t *shader = t_rex1->shader;
    shader_use(shader);
    set_up_light_and_camera(shader);
    draw_scene_object(t_rex1, project_view);
}

static void
draw_cubes() {
    shader_t *shader = cubes[0]->shader;
    shader_use(shader);
    set_up_light_and_camera(shader);

    // drawing cube
    draw_scene_object(cubes[0], project_view);
    draw_scene_object(cubes[1], project_view);
    draw_scene_object(cubes[2], project_view);
    draw_scene_object(cubes[3], project_view);
}

static void
draw_scene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);
    GL_CHECK_ERROR;

    draw_light();
    draw_cubes();
    draw_backpack();
    draw_sirenhead();
    draw_male();
    draw_spider();
    draw_lego_man();
    draw_t_rex1();

    glFlush();
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
    glm_vec3_copy(camera.pos, spot_light.light.position);
    glm_vec3_copy(camera.front, spot_light.front);
}

static void
update_scene() {
    // View
    camera_view(&camera, view_m);

    // projection
    glm_perspective(M_PI_4, (float) window_width / (float) window_height, 0.1f, 100.0f, project_m);

    // project * view
    glm_mat4_identity(project_view);
    glm_mat4_mul(project_m, view_m, project_view);

    update_cubes();
    update_camera_light();

}

static void
update_screen() {
    update_scene();
    draw_scene();
    SDL_GL_SwapWindow(window);
    SDL_CHECK_ERROR;
}


static void
initialize_scene() {
    // omni light
    vec4_set(omni_light.ambient, 0.025f, 0.025f, 0.025f, 1.0f);
    vec4_set(omni_light.diffuse, 0.8f, 0.8f, 0.8f, 1.0f);
    vec4_set(omni_light.specular, 0.8f, 0.8f, 0.8f, 1.0f);

    // direct light
    vec4_set(direct_light.ambient, 0.025f, 0.025f, 0.025f, 1.0f);
    vec4_set(direct_light.diffuse, 0.8f, 0.8f, 0.8f, 1.0f);
    vec4_set(direct_light.specular, 0.3f, 0.3f, 0.3f, 1.0f);

    // spot light
    vec4_set(spot_light.light.ambient, 0.05f, 0.05f, 0.05f, 1.0f);
    vec4_set(spot_light.light.diffuse, 1.0f, 1.0f, 1.0f, 1.0f);
    vec4_set(spot_light.light.specular, 1.0f, 1.0f, 1.0f, 1.0f);

    model_t *cube_model = cube_model_create();
    shader_t *model_shader = load_shader("shaders/model_vertex.glsl", "shaders/model_fragment.glsl");

    // cubes
    float scale = 2.0f;
    for (int i = 0; i < 4; i++) {
        cubes[i] = create_scene_object();
        attach_model_to_scene_object(cubes[i], cube_model);
        attach_shader_to_scene_object(cubes[i], model_shader);
        scale_scene_object(cubes[i], scale);
        scale *= 2;
    }
    move_scene_object_to(cubes[0], -2, -2, -2);
    move_scene_object_to(cubes[1], -4, 4, -4);
    move_scene_object_to(cubes[2], 8, 8, -8);
    move_scene_object_to(cubes[3], 16, -16, -16);

    // light object
    light = create_scene_object();
    attach_model_to_scene_object(light, cube_model);
    attach_shader_to_scene_object(light, load_shader("shaders/light_vertex.glsl", "shaders/light_fragment.glsl"));
    scale_scene_object(light, 0.2f);
    move_scene_object_to_vec(light, omni_light.position);

    // backpack
    backpack = create_scene_object();
    attach_shader_to_scene_object(backpack, model_shader);
    attach_model_to_scene_object(backpack, load_model("assets/models/backpack/backpack.obj"));
    move_scene_object_to(backpack, 4, -2, 4);

    // sirenhead
    sirenhead = create_scene_object();
    attach_shader_to_scene_object(sirenhead, model_shader);
    attach_model_to_scene_object(sirenhead, load_model("assets/models/sirenhead/source/sirenhead.obj"));
    move_scene_object_to(sirenhead, -4, -4, 4);
    scale_scene_object(sirenhead, 3.0f);

    // male figure
    male_figure = create_scene_object();
    attach_shader_to_scene_object(male_figure, model_shader);
    attach_model_to_scene_object(male_figure, load_model("assets/models/male/FinalBaseMesh.obj"));
    move_scene_object_to(male_figure, 12, -4, 2);
    scale_scene_object(male_figure, 0.3f);

    // spider
    spider_obj = create_scene_object();
    attach_shader_to_scene_object(spider_obj, model_shader);
    attach_model_to_scene_object(spider_obj,
                                 load_model("assets/models/spider_obj/Only_Spider_with_Animations_Export.obj"));
    move_scene_object_to(spider_obj, -12, -4, 2);
    scale_scene_object(spider_obj, 0.06f);

    // lego man
    lego_man = create_scene_object();
    attach_shader_to_scene_object(lego_man, model_shader);
    attach_model_to_scene_object(lego_man,
                                 load_model("assets/models/lego_man/lego obj.obj"));
    move_scene_object_to(lego_man, 18, -4, 2);
    scale_scene_object(lego_man, 0.1f);

    // t-rex1
    t_rex1 = create_scene_object();
    attach_shader_to_scene_object(t_rex1, model_shader);
    attach_model_to_scene_object(t_rex1, load_model("assets/models/cadnav.com_model/Models_G0901A079/T-rex.obj"));
    move_scene_object_to(t_rex1, 28, -4, 2);
    scale_scene_object(t_rex1, 0.8f);
    rotate_scene_object_by_vec(t_rex1, (vec3) {M_PI_2, M_PI, M_PI_4});

//    backpack_model = load_model("assets/models/hot_wheels1/Base Mesh.fbx");
//    backpack_model = load_model("assets/models/handgun/Handgun_Packed.blend");
//    backpack_model = load_model("assets/models/Lotus_Hot_Wheels_3DS/Lotus_HW_3DS.3DS");
//    backpack_model = load_model("assets/models/Subaru Impreza/subaru_impreza.fbx");
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
    glEnable(GL_DEPTH_TEST); // enables z-buffering
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // background color

    camera_init(&camera);

    initialize_scene();
    GL_CHECK_ERROR;

    return true;
}

static void
shutdown_app() {
    destroy_scene_object(light);
    destroy_scene_object(male_figure);
    destroy_scene_object(spider_obj);
    destroy_scene_object(lego_man);
    destroy_scene_object(t_rex1);
    destroy_scene_object(sirenhead);
    destroy_scene_object(backpack);
    destroy_scene_object(cubes[0]);
    destroy_scene_object(cubes[1]);
    destroy_scene_object(cubes[2]);
    destroy_scene_object(cubes[3]);

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