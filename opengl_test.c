#include "opengl/sdl_ext.h"
#include "opengl/gl_ext.h"
#include "opengl/cglm_ext.h"
#include <stdbool.h>
#include <time.h>
#include "cglm/cglm.h"
#include "opengl/camera.h"
#include "opengl/shader.h"
#include "models/cube.h"
#include "opengl/material.h"
#include "opengl/light.h"
#include "opengl/model.h"

#define LOC_PROJECT_VIEW "project_view"
#define LOC_MODEL "model"

static int window_width = 1280;
static int window_height = 1280 / 16 * 9;

static const Uint32 FPS = 30;
static const Uint32 FPS_SIZE_MS = 1000 / FPS;

static camera_t camera;
static bool camera_light_on = true;

static model_t *backpack_model;
static shader_t *backpack_shader = NULL;
static mat4 backpack_model_m = GLM_MAT4_IDENTITY;

static struct cube_object {
    vec3 angles;
} cube_object;
static model_t *cube_model;
static shader_t *cube_shader = NULL;
static mat4 cube_model1_m = GLM_MAT4_IDENTITY;
static mat4 cube_model2_m = GLM_MAT4_IDENTITY;
static mat4 cube_model3_m = GLM_MAT4_IDENTITY;
static mat4 cube_model4_m = GLM_MAT4_IDENTITY;

static shader_t *light_shader = NULL;
static mat4 light_m = GLM_MAT4_IDENTITY;
static vec3 light_scale = {0.2f, 0.2f, 0.2f};
static omni_light_t omni_light = {
        {0.0f,  -3.0f, 0.0f},
        {0.95f, 0.95f, 0.95f},
        {0.95f, 0.95f, 0.95f},
        {0.95f, 0.95f, 0.95f}
};

static direct_light_t direct_light = {
        {1.0f,  0.0f,  1.0f},
        {0.95f, 0.95f, 0.95f},
        {0.95f, 0.95f, 0.95f},
        {0.95f, 0.95f, 0.95f}
};

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


static mat4 view_m = GLM_MAT4_IDENTITY;
static mat4 project_m = GLM_MAT4_IDENTITY;

static SDL_Window *window = NULL;
static SDL_GLContext context = NULL;

static bool initialize_app();

static void update_screen();

static void event_loop();

static void set_cube_material(shader_t *shader, material_t *mat);

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
    shader_use(light_shader);

    mat4 project_view = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_mul(project_m, view_m, project_view);
    shader_set_vec3(light_shader, "passed_color", omni_light.specular);
    shader_set_mat4(light_shader, LOC_PROJECT_VIEW, project_view);
    shader_set_mat4(light_shader, LOC_MODEL, light_m);

    draw_model(cube_model, light_shader);
}

static void set_up_light_and_camera(shader_t *shader) {
    // project and view
    mat4 project_view = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_mul(project_m, view_m, project_view);

    shader_set_mat4(shader, LOC_PROJECT_VIEW, project_view);

    // omni-light
    shader_set_vec3(shader, "omni_light.position", omni_light.position);
    shader_set_vec3(shader, "omni_light.light_prop.ambient", omni_light.ambient);
    shader_set_vec3(shader, "omni_light.light_prop.diffuse", omni_light.diffuse);
    shader_set_vec3(shader, "omni_light.light_prop.specular", omni_light.specular);

    // direct light
    shader_set_vec3(shader, "direct_light.front", direct_light.front);
    shader_set_vec3(shader, "direct_light.light_prop.ambient", direct_light.ambient);
    shader_set_vec3(shader, "direct_light.light_prop.diffuse", direct_light.diffuse);
    shader_set_vec3(shader, "direct_light.light_prop.specular", direct_light.specular);

    // spot light
    if (camera_light_on) {
        shader_set_vec3(shader, "spot_light.light_prop.ambient", spot_light.light.ambient);
        shader_set_vec3(shader, "spot_light.light_prop.diffuse", spot_light.light.diffuse);
        shader_set_vec3(shader, "spot_light.light_prop.specular", spot_light.light.specular);
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

static void set_up_model_and_normals(shader_t *shader, mat4 model) {
    // model matrix
    shader_set_mat4(shader, LOC_MODEL, model);

    // normals
    mat4 normals_model4;
    mat3 normals_model3;

    glm_mat4_inv(model, normals_model4);
    glm_mat4_transpose(normals_model4);
    glm_mat4_pick3(normals_model4, normals_model3);
    shader_set_mat3(shader, "normals_model", normals_model3);
}

static void
draw_backpack() {
    shader_t *shader = backpack_shader;
    shader_use(shader);

    set_up_model_and_normals(shader, backpack_model_m);
    set_up_light_and_camera(shader);

    // Bag material
    shader_set_vec3(shader, "material.light_prop.ambient", (vec3) {1, 1, 1});
    shader_set_vec3(shader, "material.light_prop.diffuse", (vec3) {1, 1, 1});
    shader_set_vec3(shader, "material.light_prop.specular", (vec3) {1, 1, 1});
    shader_set_float(shader, "material.shininess", 1);

    draw_model(backpack_model, shader);
}

static void
draw_cube(shader_t *shader, mat4 model, material_t *material) {
    set_cube_material(shader, material);
    set_up_model_and_normals(shader, model);
    draw_model(cube_model, shader);
}

static void
set_cube_material(shader_t *shader, material_t *mat) {
    shader_set_vec3(shader, "material.light_prop.ambient", mat->ambient);
    shader_set_vec3(shader, "material.light_prop.diffuse", mat->diffuse);
    shader_set_vec3(shader, "material.light_prop.specular", mat->specular);
    shader_set_float(shader, "material.shininess", mat->shininess);
}

static void
draw_cubes() {
    shader_t *shader = cube_shader;
    shader_use(shader);

    set_up_light_and_camera(shader);

    // drawing cube
    draw_cube(shader, cube_model1_m, (material_t *) &MATERIAL_IDEAL);
    draw_cube(shader, cube_model2_m, (material_t *) &MATERIAL_IDEAL);
    draw_cube(shader, cube_model3_m, (material_t *) &MATERIAL_IDEAL);
    draw_cube(shader, cube_model4_m, (material_t *) &MATERIAL_IDEAL);
}

static void
draw_scene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_cubes();
    draw_light();
    draw_backpack();

    glFlush();
}

static void
update_light() {
    vec3_set(omni_light.ambient, 0.025f, 0.025f, 0.025f);
    vec3_set(omni_light.diffuse, 0.5f, 0.5f, 0.5f);
    vec3_set(omni_light.specular, 0.5f, 0.5f, 0.5f);

    vec3_set(direct_light.ambient, 0.025f, 0.025f, 0.025f);
    vec3_set(direct_light.diffuse, 0.5f, 0.5f, 0.5f);
    vec3_set(direct_light.specular, 0.5f, 0.5f, 0.5f);

    glm_mat4_identity(light_m);
    glm_scale(light_m, light_scale);
    glm_translate(light_m, omni_light.position);
}

static void update_cube(float size, const float *trans, const float *angles, vec4 (*model)) {
    vec3 scale4 = {size, size, size};
    glm_mat4_identity(model);
    glm_translate_x(model, trans[0]);
    glm_translate_y(model, trans[1]);
    glm_translate_z(model, trans[2]);
    glm_scale(model, scale4);
    glm_rotate_x(model, angles[0], model);
    glm_rotate_y(model, angles[1], model);
    glm_rotate_z(model, angles[2], model);
}

static void
update_cubes() {
    // rotating
    cube_object.angles[0] += 0.01f;
    cube_object.angles[1] += 0.012f;
    cube_object.angles[2] += 0.013f;

    float size;
    float *angles = cube_object.angles;

    size = 2.0f;
    update_cube(size, (vec3) {-size, -size, -size}, (vec3) {angles[0], angles[1], 0}, cube_model1_m);
    size = 4.0f;
    update_cube(size, (vec3) {-size, size, -size}, (vec3) {0, angles[1], angles[2]}, cube_model2_m);
    size = 8.0f;
    update_cube(size, (vec3) {size, size, -size}, (vec3) {angles[0], 0, angles[2]}, cube_model3_m);
    size = 16.0f;
    update_cube(size, (vec3) {size, -size, -size}, (vec3) {angles[0], 0, 0}, cube_model4_m);
}

static void
update_backpack() {
    glm_mat4_identity(backpack_model_m);
    glm_translate_x(backpack_model_m, 4);
    glm_translate_y(backpack_model_m, -4);
    glm_translate_z(backpack_model_m, 4);
//    glm_scale(backpack_model_m, (vec3) {0.1f, 0.1f, 0.1f});
    glm_rotate_x(backpack_model_m, 0, backpack_model_m);
    glm_rotate_y(backpack_model_m, 0, backpack_model_m);
    glm_rotate_z(backpack_model_m, 0, backpack_model_m);
}

static void
update_camera_light() {
    vec3_set(spot_light.light.ambient, 0.05f, 0.05f, 0.05f);
    vec3_set(spot_light.light.diffuse, 1.0f, 1.0f, 1.0f);
    vec3_set(spot_light.light.specular, 1.0f, 1.0f, 1.0f);

    glm_vec3_copy(camera.pos, spot_light.light.position);
    glm_vec3_copy(camera.front, spot_light.front);
}

static void
update_scene() {
    update_cubes();
    update_light();
    update_camera_light();
    update_backpack();

    // View
    camera_view(&camera, view_m);

    // projection
    glm_perspective(M_PI_4, (float) window_width / (float) window_height, 0.1f, 100.0f, project_m);
}

static void
update_screen() {
    update_scene();
    draw_scene();
    SDL_GL_SwapWindow(window);
    SDL_CHECK_ERROR;
}

static void
initialize_cube() {
    vec3_set(cube_object.angles, 0, 0, 0);
    cube_model = cube_model_create();
    cube_shader = shader_load("shaders/cube_vertex.glsl", "shaders/cube_fragment.glsl");
}

static void
initialize_models() {
    backpack_shader = shader_load("shaders/bag_vertex.glsl", "shaders/bag_fragment.glsl");
//    backpack_model = load_model("assets/models/sirenhead/source/sirenhead.obj");
//    backpack_model = load_model("assets/models/hot_wheels1/Base Mesh.fbx");
    backpack_model = load_model("assets/models/backpack/backpack.obj");
//    backpack_model = load_model("assets/models/spider_obj/Only_Spider_with_Animations_Export.obj");
//    backpack_model = load_model("assets/models/handgun/Handgun_Packed.blend");
//    backpack_model = load_model("assets/models/cadnav.com_model/Models_G0901A079/T-rex.obj");
//    backpack_model = load_model("assets/models/lego_man/lego obj.obj");
//    backpack_model = load_model("assets/models/Lotus_Hot_Wheels_3DS/Lotus_HW_3DS.3DS");
//    backpack_model = load_model("assets/models/Subaru Impreza/subaru_impreza.fbx");
//    backpack_model = load_model("assets/models/male/FinalBaseMesh.obj");
//    backpack_model = load_model("assets/models/teapot.obj");
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

    initialize_cube();

    camera_init(&camera);

    light_shader = shader_load("shaders/light_vertex.glsl", "shaders/light_fragment.glsl");

    initialize_models();
    GL_CHECK_ERROR;

    return true;
}

static void
shutdown_app() {
    if (cube_model != NULL) {
        destroy_model(cube_model);
    }

    if (backpack_model != NULL) {
        destroy_model(backpack_model);
    }

    if (light_shader != NULL) {
        shader_destroy(light_shader);
    }

    if (cube_shader != NULL) {
        shader_destroy(cube_shader);
    }

    if (backpack_shader != NULL) {
        shader_destroy(backpack_shader);
    }

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