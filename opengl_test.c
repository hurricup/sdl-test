#include "opengl/sdl_ext.h"
#include "opengl/gl_ext.h"
#include "opengl/cglm_ext.h"
#include <stdbool.h>
#include <time.h>
#include "cglm/cglm.h"
#include "opengl/camera.h"
#include "opengl/texture.h"
#include "opengl/shader.h"
#include "models/cube.h"
#include "opengl/material.h"
#include "opengl/light.h"
#include "opengl/model.h"

static int window_width = 1280;
static int window_height = 1280 / 16 * 9;

static const Uint32 FPS = 30;
static const Uint32 FPS_SIZE_MS = 1000 / FPS;

static unsigned int cube_shader;
static int cube_oscillation_location;
static int cube_model_location;
static int cube_normals_model_location;
static int cube_project_location;

static int direct_light_front_location;
static int direct_light_ambient_location;
static int direct_light_diffuse_location;
static int direct_light_specular_location;

static int spot_light_pos_location;
static int spot_light_ambient_location;
static int spot_light_diffuse_location;
static int spot_light_specular_location;
static int spot_light_front_location;
static int spot_light_angle_location;
static int spot_light_smooth_angle_location;

static int cube_light_pos_location;
static int cube_light_ambient_location;
static int cube_light_diffuse_location;
static int cube_light_specular_location;
static int cube_material_ambient_location;
static int cube_material_diffuse_location;
static int cube_material_specular_location;
static int cube_material_shininess_location;
static int camera_pos_location;

static model_t *car;
static mat4 model_car = GLM_MAT4_IDENTITY;

static camera_t camera;
static mat4 model1_m = GLM_MAT4_IDENTITY;
static mat4 model2_m = GLM_MAT4_IDENTITY;
static mat4 model3_m = GLM_MAT4_IDENTITY;
static mat4 model4_m = GLM_MAT4_IDENTITY;

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

static bool camera_light_on = true;
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

static int light_model_location;
static int light_project_location;
static int light_color_location;
static unsigned int light_shader;

static mat4 view_m = GLM_MAT4_IDENTITY;
static mat4 project_m = GLM_MAT4_IDENTITY;
static struct cube_object {
    vec3 angles;
} cube_object;

static model_t *cube_model;

static SDL_Window *window = NULL;
static SDL_GLContext context = NULL;

static bool initialize_app();

static void update_screen();

static void event_loop();

static void set_cube_material(const material_t *mat);

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
    glUseProgram(light_shader);

    mat4 project_view = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_mul(project_m, view_m, project_view);
    glUniform3vf(light_color_location, omni_light.specular);
    glUniformMatrix4fv(light_project_location, 1, GL_FALSE, (GLfloat *) project_view);
    glUniformMatrix4fv(light_model_location, 1, GL_FALSE, (GLfloat *) light_m);

    draw_model(cube_model);
}

static void
draw_car() {
    mat4 project_view = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_mul(project_m, view_m, project_view);
    glUseProgram(cube_shader);
    glUniformMatrix4fv(cube_project_location, 1, GL_FALSE, (GLfloat *) project_view);
    glUniformMatrix4fv(cube_model_location, 1, GL_FALSE, (GLfloat *) model_car);
    draw_model(car);
}

static void
draw_cube(mat4 model, const material_t *material) {
    mat4 normals_model4;
    mat3 normals_model3;

    set_cube_material(material);

    glm_mat4_inv(model, normals_model4);
    glm_mat4_transpose(normals_model4);
    glm_mat4_pick3(normals_model4, normals_model3);

    glUniformMatrix3fv(cube_normals_model_location, 1, GL_FALSE, (GLfloat *) normals_model3);
    glUniformMatrix4fv(cube_model_location, 1, GL_FALSE, (GLfloat *) model);
    draw_model(cube_model);
}

static void set_cube_material(const material_t *mat) {
    glUniform3vf(cube_material_ambient_location, mat->ambient);
    glUniform3vf(cube_material_diffuse_location, mat->diffuse);
    glUniform3vf(cube_material_specular_location, mat->specular);
    glUniform1f(cube_material_shininess_location, mat->shininess);
}

static void
draw_cubes() {
    glUseProgram(cube_shader);

    // spot light
    if (camera_light_on) {
        glUniform3vf(spot_light_pos_location, spot_light.light.position);
        glUniform3vf(spot_light_ambient_location, spot_light.light.ambient);
        glUniform3vf(spot_light_diffuse_location, spot_light.light.diffuse);
        glUniform3vf(spot_light_specular_location, spot_light.light.specular);
        glUniform3vf(spot_light_front_location, spot_light.front);
        glUniform1f(spot_light_angle_location, (float) cos((double) spot_light.angle));
        glUniform1f(spot_light_smooth_angle_location, (float) cos((double) spot_light.angle + spot_light.smooth_angle));
    } else {
        glUniform1f(spot_light_angle_location, 0.0f);
    }

    // direct light
    glUniform3vf(direct_light_front_location, direct_light.front);
    glUniform3vf(direct_light_ambient_location, direct_light.ambient);
    glUniform3vf(direct_light_diffuse_location, direct_light.diffuse);
    glUniform3vf(direct_light_specular_location, direct_light.specular);

    // light source
    glUniform3vf(cube_light_pos_location, omni_light.position);
    glUniform3vf(cube_light_ambient_location, omni_light.ambient);
    glUniform3vf(cube_light_diffuse_location, omni_light.diffuse);
    glUniform3vf(cube_light_specular_location, omni_light.specular);

    // project * view matrix
    mat4 project_view = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_mul(project_m, view_m, project_view);
    glUniformMatrix4fv(cube_project_location, 1, GL_FALSE, (GLfloat *) project_view);

    // camera position
    glUniform3vf(camera_pos_location, camera.pos);

    // oscillation for 2 textures
    double base_value = M_PI * SDL_GetTicks() / 180 / FPS_SIZE_MS / 0.5;
    double oscillation = sin(base_value) / 2 + 0.5;
    glUniform1f(cube_oscillation_location, (float) oscillation);

    // drawing cube
    draw_cube(model1_m, &MATERIAL_IDEAL);
    draw_cube(model2_m, &MATERIAL_IDEAL);
    draw_cube(model3_m, &MATERIAL_IDEAL);
    draw_cube(model4_m, &MATERIAL_IDEAL);
}

static void
draw_scene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_cubes();
    draw_light();
    draw_car();

    glFlush();
}

static void
update_light() {
    vec3_set(omni_light.ambient, 0.01f, 0.01f, 0.05f);
    vec3_set(omni_light.diffuse, 0.24f, 0.24f, 0.8f);
    vec3_set(omni_light.specular, 0.3f, 0.3f, 1);

    vec3_set(direct_light.ambient, 0.01f, 0.05f, 0.01f);
    vec3_set(direct_light.diffuse, 0.3f, 1, 0.3f);
    vec3_set(direct_light.specular, 0.3f, 1, 0.3f);

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
    update_cube(size, (vec3) {-size, -size, -size}, (vec3) {angles[0], angles[1], 0}, model1_m);
    size = 4.0f;
    update_cube(size, (vec3) {-size, size, -size}, (vec3) {0, angles[1], angles[2]}, model2_m);
    size = 8.0f;
    update_cube(size, (vec3) {size, size, -size}, (vec3) {angles[0], 0, angles[2]}, model3_m);
    size = 16.0f;
    update_cube(size, (vec3) {size, -size, -size}, (vec3) {angles[0], 0, 0}, model4_m);
}

static void
update_car() {
    glm_mat4_identity(model_car);
    glm_translate_x(model_car, 0);
    glm_translate_y(model_car, -4);
    glm_translate_z(model_car, 0);
//    glm_scale(model_car, (vec3) {0.1f, 0.1f, 0.1f});
    glm_rotate_x(model_car, 0, model_car);
    glm_rotate_y(model_car, 0, model_car);
    glm_rotate_z(model_car, 0, model_car);
}

static void
update_camera_light() {
    vec3_set(spot_light.light.ambient, 0.2f, 0.2f, 0.2f);
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
    update_car();

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
initialize_camera() {
    camera_init(&camera);
    camera_pos_location = glGetUniformLocation(cube_shader, "camera_pos");
}

static void
initialize_light() {
    light_shader = create_shader("shaders/light_vertex.glsl", "shaders/light_fragment.glsl");

    cube_light_pos_location = glGetUniformLocation(cube_shader, "light.position");
    cube_light_ambient_location = glGetUniformLocation(cube_shader, "light.ambient");
    cube_light_diffuse_location = glGetUniformLocation(cube_shader, "light.diffuse");
    cube_light_specular_location = glGetUniformLocation(cube_shader, "light.specular");

    light_color_location = glGetUniformLocation(light_shader, "passed_color");
    light_model_location = glGetUniformLocation(light_shader, "model");
    light_project_location = glGetUniformLocation(light_shader, "project_view");

}

static void
initialize_direct_light() {
    direct_light_front_location = glGetUniformLocation(cube_shader, "direct_light.front");
    direct_light_ambient_location = glGetUniformLocation(cube_shader, "direct_light.ambient");
    direct_light_diffuse_location = glGetUniformLocation(cube_shader, "direct_light.diffuse");
    direct_light_specular_location = glGetUniformLocation(cube_shader, "direct_light.specular");
}

static void
initialize_spot_light() {
    spot_light_pos_location = glGetUniformLocation(cube_shader, "spot_light.light.position");
    spot_light_ambient_location = glGetUniformLocation(cube_shader, "spot_light.light.ambient");
    spot_light_diffuse_location = glGetUniformLocation(cube_shader, "spot_light.light.diffuse");
    spot_light_specular_location = glGetUniformLocation(cube_shader, "spot_light.light.specular");
    spot_light_front_location = glGetUniformLocation(cube_shader, "spot_light.front");
    spot_light_angle_location = glGetUniformLocation(cube_shader, "spot_light.angle_cos");
    spot_light_smooth_angle_location = glGetUniformLocation(cube_shader, "spot_light.smooth_angle_cos");
}

static void
initialize_cube() {
    vec3_set(cube_object.angles, 0, 0, 0);
    cube_model = cube_model_create();

    cube_shader = create_shader("shaders/vertex.glsl", "shaders/fragment.glsl");

    cube_material_ambient_location = glGetUniformLocation(cube_shader, "material.ambient");
    cube_material_diffuse_location = glGetUniformLocation(cube_shader, "material.diffuse");
    cube_material_specular_location = glGetUniformLocation(cube_shader, "material.specular");
    cube_material_shininess_location = glGetUniformLocation(cube_shader, "material.shininess");

    cube_oscillation_location = glGetUniformLocation(cube_shader, "oscillation");

    cube_model_location = glGetUniformLocation(cube_shader, "model");
    cube_normals_model_location = glGetUniformLocation(cube_shader, "normals_model");
    cube_project_location = glGetUniformLocation(cube_shader, "project_view");

    // generating textures, should be moved to cube_model as well
    load_texture(GL_TEXTURE0, "texture1.png");
    load_texture(GL_TEXTURE1, "texture2.png");
    load_texture(GL_TEXTURE2, "diffuse_map.png");
    load_texture(GL_TEXTURE3, "specular_map.png");
}

static void
initialize_car() {
    car = load_model("assets/models/cadnav.com_model/Models_G0901A079/T-rex.obj");
//    car = load_model("assets/models/lego_man/lego obj.obj");
//    car = load_model("assets/models/Lotus_Hot_Wheels_3DS/Lotus_HW_3DS.3DS");
//    car = load_model("assets/models/Subaru Impreza/subaru_impreza.fbx");
//    car = load_model("assets/models/male/FinalBaseMesh.obj");
//    car = load_model("assets/models/teapot.obj");
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
    initialize_camera();
    initialize_light();
    initialize_direct_light();
    initialize_spot_light();
    initialize_car();
    GL_CHECK_ERROR;

    return true;
}

static void
shutdown_app() {
    if (cube_model != NULL) {
        destroy_model(cube_model);
    }

    if (car != NULL) {
        destroy_model(car);
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