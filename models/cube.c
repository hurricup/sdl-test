#include "cube.h"

model_t *
cube_model_create() {
    vertex_t cube_vertices[] = {
            // front
            {{0.5f,  0.5f,  0.5f},  {0,  0,  1},  {1.0f, 1.0f}},
            {{-0.5f, 0.5f,  0.5f},  {0,  0,  1},  {0.0f, 1.0f}},
            {{-0.5f, -0.5f, 0.5f},  {0,  0,  1},  {0.0f, 0.0f}},
            {{0.5f,  -0.5f, 0.5f},  {0,  0,  1},  {1.0f, 0.0f}},
            // back
            {{0.5f,  0.5f,  -0.5f}, {0,  0,  -1}, {1.0f, 1.0f}},
            {{0.5f,  -0.5f, -0.5f}, {0,  0,  -1}, {1.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0,  0,  -1}, {0.0f, 0.0f}},
            {{-0.5f, 0.5f,  -0.5f}, {0,  0,  -1}, {0.0f, 1.0f}},
            // right
            {{0.5f,  0.5f,  0.5f},  {1,  0,  0},  {1.0f, 1.0f}},
            {{0.5f,  -0.5f, 0.5f},  {1,  0,  0},  {1.0f, 0.0f}},
            {{0.5f,  -0.5f, -0.5f}, {1,  0,  0},  {0.0f, 0.0f}},
            {{0.5f,  0.5f,  -0.5f}, {1,  0,  0},  {0.0f, 1.0f}},
            // left
            {{-0.5f, 0.5f,  0.5f},  {-1, 0,  0},  {1.0f, 1.0f}},
            {{-0.5f, 0.5f,  -0.5f}, {-1, 0,  0},  {0.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {-1, 0,  0},  {0.0f, 0.0f}},
            {{-0.5f, -0.5f, 0.5f},  {-1, 0,  0},  {1.0f, 0.0f}},
            // top
            {{0.5f,  0.5f,  0.5f},  {0,  1,  0},  {1.0f, 1.0f}},
            {{0.5f,  0.5f,  -0.5f}, {0,  1,  0},  {1.0f, 0.0f}},
            {{-0.5f, 0.5f,  -0.5f}, {0,  1,  0},  {0.0f, 0.0f}},
            {{-0.5f, 0.5f,  0.5f},  {0,  1,  0},  {0.0f, 1.0f}},
            // bottom
            {{0.5f,  -0.5f, 0.5f},  {0,  -1, 0},  {1.0f, 1.0f}},
            {{-0.5f, -0.5f, 0.5f},  {0,  -1, 0},  {0.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0,  -1, 0},  {0.0f, 0.0f}},
            {{0.5f,  -0.5f, -0.5f}, {0,  -1, 0},  {1.0f, 0.0f}}
    };

    unsigned int cube_indices[] = {
            0, 1, 2,
            2, 3, 0,
            4, 5, 6,
            6, 7, 4,
            8, 9, 10,
            10, 11, 8,
            12, 13, 14,
            14, 15, 12,
            16, 17, 18,
            18, 19, 16,
            20, 21, 22,
            22, 23, 20
    };

    model_t *cube_model = create_model(6 * 4, cube_vertices, 6 * 2 * 3, cube_indices, "assets/models/cube");

    mesh_t *mesh = &cube_model->meshes->mesh;
    mesh->textures = malloc(4 * sizeof(texture_t *));
    SDL_ALLOC_CHECK(mesh->textures);
    load_texture(cube_model, mesh, aiTextureType_DIFFUSE, "texture2.png");
    load_texture(cube_model, mesh, aiTextureType_SPECULAR, "specular_map.png");
    load_texture(cube_model, mesh, aiTextureType_REFLECTION, "texture_reflection.png");

    material_t *material = &mesh->material;
    glm_vec4_fill(material->ambient, 1.0f);
    glm_vec4_fill(material->diffuse, 1.0f);
    glm_vec4_fill(material->specular, 1.0f);
    glm_vec4_fill(material->emissive, 0.0f);
    material->shininess = DEFAULT_SHININESS;
    material->opacity = DEFAULT_OPACITY;

    return cube_model;
}
