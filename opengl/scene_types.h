#ifndef SDL_TEST_SCENE_TYPES_H
#define SDL_TEST_SCENE_TYPES_H

#include "cglm_ext.h"
#include "assimp/scene.h"

typedef struct material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 emissive;
    float shininess;
    float opacity;
} material_t;

typedef struct uniform_cache_item {
    char *uniform_name;
    int uniform_id;
} uniform_cache_item_t;

typedef struct shader {
    unsigned int id;
    char *vertex_shader_name;
    char *fragment_shader_name;
    unsigned int owners;
    /**
     * used for shader re-use with multiple scene model, to avoid useless re-configuring of global stuff, like lightning.
     */
    unsigned int render_pass;
    unsigned int uniform_cache_items;
    unsigned int uniform_cache_items_allocated;
    uniform_cache_item_t *uniforms_cache;
} shader_t;

typedef struct rendering_context {
    shader_t *shader;
    bool add_lights;
    bool add_camera_position;
    bool add_textures;
    bool add_material_properties;
    bool add_index_color;
    vec3 index_color;
    unsigned int skybox_texture;
} rendering_context_t;

typedef struct vertex {
    vec3 position;
    vec3 normal;
    vec2 texture_position;
} vertex_t;

typedef struct texture {
    unsigned int id;
    char *filename;
    enum aiTextureType type;
} texture_t;

typedef struct mesh {
    unsigned int vertices_number;
    vertex_t *vertices;
    unsigned int indices_number;
    unsigned int *indices;
    unsigned int textures_number;
    texture_t **textures;
    material_t material;

    unsigned int vertex_array;
    unsigned int vertex_buffer;
    unsigned int element_buffer;
} mesh_t;

typedef struct mesh_list_item {
    mesh_t mesh;
    struct mesh_list_item *next;
} mesh_list_item_t;

typedef struct texture_list_item {
    texture_t texture;
    struct texture_list_item *next;
} texture_list_item_t;

typedef struct model {
    mesh_list_item_t *meshes;
    texture_list_item_t *textures;
    char *directory;
    unsigned int owners;
} model_t;

typedef struct scene_object {
    model_t *model;
    shader_t *shader;
    vec3 position;
    vec3 angles;
    vec3 scale;
    bool selected;
} scene_object_t;


#endif //SDL_TEST_SCENE_TYPES_H
