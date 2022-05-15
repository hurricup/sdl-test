#define STB_IMAGE_IMPLEMENTATION

#include "model.h"

#define MIN_TEXTURE_TYPE aiTextureType_DIFFUSE
#define MAX_TEXTURE_TYPE aiTextureType_OPACITY
#define MAX_TEXTURES_PER_TYPE 2
#define TEXTURE_SLOT_NAME_SIZE 20

static bool texture_uniform_names_initialized = false;
static char texture_uniform_names_templates[MAX_TEXTURE_TYPE + 1][TEXTURE_SLOT_NAME_SIZE] = {
        "texture_none%u",
        "texture_diffuse%u",
        "texture_specular%u",
        "texture_ambient%u",
        "texture_emissive%u",
        "texture_height%u",
        "texture_normals%u",
        "texture_shininess%u",
        "texture_opacity%u"
};


static char texture_uniform_names[MAX_TEXTURE_TYPE + 1][MAX_TEXTURES_PER_TYPE][TEXTURE_SLOT_NAME_SIZE];
static char texture_flags_names[MAX_TEXTURE_TYPE + 1][TEXTURE_SLOT_NAME_SIZE] = {
        "textures_number[0]",
        "textures_number[1]",
        "textures_number[2]",
        "textures_number[3]",
        "textures_number[4]",
        "textures_number[5]",
        "textures_number[6]",
        "textures_number[7]",
        "textures_number[8]"
};

static void
init_mesh_gl(mesh_t *mesh) {
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);

    glBindVertexArray(mesh->vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertices_number * (long) sizeof(vertex_t), mesh->vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices_number * (long) sizeof(unsigned int), mesh->indices,
                 GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *) offsetof(vertex_t, position));
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *) offsetof(vertex_t, normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *) offsetof(vertex_t, texture_position));

    glBindVertexArray(0);
}

inline static void
init_texture_uniform_names() {
    if (texture_uniform_names_initialized) {
        return;
    }
    for (int type = MIN_TEXTURE_TYPE; type <= MAX_TEXTURE_TYPE; type++) {
        for (int i = 0; i < MAX_TEXTURES_PER_TYPE; i++) {
            sprintf(texture_uniform_names[type][i], texture_uniform_names_templates[type], i);
        }
    }
    texture_uniform_names_initialized = true;
}

static char *
get_texture_uniform_name(enum aiTextureType type, unsigned int index) {
    init_texture_uniform_names();
    if (type > MAX_TEXTURE_TYPE) {
        SDL_Die("Texture type: %u is not supported, max supported type is %u", type, MAX_TEXTURE_TYPE);
    }
    if (index >= MAX_TEXTURES_PER_TYPE) {
        SDL_Die("You can't have more than %u textures of a single type, requested %u", MAX_TEXTURES_PER_TYPE, index);
    }
    return texture_uniform_names[type][index];
}

static void
draw_mesh(mesh_t *mesh, shader_t *shader) {
    // textures
    unsigned int type_index[MAX_TEXTURE_TYPE + 1] = {0};
    if (mesh->textures_number) {
        for (int i = 0; i < mesh->textures_number; i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            texture_t *texture = mesh->textures[i];
            char *texture_uniform_name = get_texture_uniform_name(texture->type, type_index[texture->type]++);
            shader_set_int(shader, texture_uniform_name, i);
            glBindTexture(GL_TEXTURE_2D, texture->id);
        }
        glActiveTexture(GL_TEXTURE0);
    }
    for (int i = 0; i <= MAX_TEXTURE_TYPE; i++) {
        shader_set_int(shader, texture_flags_names[i], (int) type_index[i]);
    }

    // material properties
    shader_set_vec4(shader, "material.ambient", mesh->material.ambient);
    shader_set_vec4(shader, "material.diffuse", mesh->material.diffuse);
    shader_set_vec4(shader, "material.specular", mesh->material.specular);
    shader_set_vec4(shader, "material.emissive", mesh->material.emissive);
    shader_set_float(shader, "material.shininess", mesh->material.shininess);
    shader_set_float(shader, "material.opacity", mesh->material.opacity);

    // draw
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->indices_number, GL_UNSIGNED_INT, 0);
    GL_CHECK_ERROR;

    glBindVertexArray(0);
}

static void
destroy_texture(texture_t *texture) {
    free(texture->filename);
    texture->filename = NULL;
}

static void
destroy_mesh_content(mesh_t *mesh) {
    if (mesh->vertices) {
        free(mesh->vertices);
        mesh->vertices_number = 0;
        mesh->vertices = NULL;
    }
    if (mesh->indices) {
        free(mesh->indices);
        mesh->indices_number = 0;
        mesh->indices = NULL;
    }
    if (mesh->textures_number > 0) {
        free(mesh->textures);
        mesh->textures_number = 0;
        mesh->textures = NULL;
    }
}

static void
destroy_mesh(mesh_t *mesh) {
    destroy_mesh_content(mesh);
    free(mesh);
}

void
model_info(model_t *model, const char *path) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Model %s (%s):", path, model->directory);
    unsigned int mesh_counter = 0;
    mesh_list_item_t *current_item = model->meshes;
    while (current_item != NULL) {
        mesh_t *currentMesh = &current_item->mesh;
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "\tid: %u; vertices: %u; indices: %u; textures: %u", mesh_counter,
                    currentMesh->vertices_number, currentMesh->indices_number, currentMesh->textures_number);
        current_item = current_item->next;
        mesh_counter++;
    }
}

void
draw_model(model_t *model, shader_t *shader) {
    shader_use(shader);
    mesh_list_item_t *current_item = model->meshes;
    while (current_item != NULL) {
        draw_mesh(&current_item->mesh, shader);
        current_item = current_item->next;
    }
}

static model_t *
alloc_model() {
    model_t *model = calloc(1, sizeof(model_t));
    SDL_ALLOC_CHECK(model);
    return model;
}

static void
import_mesh_vertices(mesh_t *mesh, struct aiMesh *assimp_mesh) {
    if (!assimp_mesh->mNumVertices) {
        return;
    }
    mesh->vertices = calloc(assimp_mesh->mNumVertices, sizeof(vertex_t));
    SDL_ALLOC_CHECK(mesh->vertices);

    for (int i = 0; i < assimp_mesh->mNumVertices; i++) {
        vertex_t *vertex = &mesh->vertices[i];
        vec3_set(vertex->position, assimp_mesh->mVertices[i].x, assimp_mesh->mVertices[i].y,
                 assimp_mesh->mVertices[i].z);
        if (assimp_mesh->mNormals != NULL) {
            vec3_set(vertex->normal, assimp_mesh->mNormals[i].x, assimp_mesh->mNormals[i].y,
                     assimp_mesh->mNormals[i].z);
        }
        if (assimp_mesh->mTextureCoords[0] != NULL) {
            vec2_set(vertex->texture_position, assimp_mesh->mTextureCoords[0][i].x,
                     assimp_mesh->mTextureCoords[0][i].y);
        }
    }
    mesh->vertices_number = assimp_mesh->mNumVertices;
}

static void
import_mesh_indices(mesh_t *mesh, struct aiMesh *assimp_mesh) {
    unsigned int indices_number = 0;
    for (int i = 0; i < assimp_mesh->mNumFaces; i++) {
        indices_number += assimp_mesh->mFaces[i].mNumIndices;
    }

    if (!indices_number) {
        return;
    }

    mesh->indices = calloc(indices_number, sizeof(unsigned int));
    SDL_ALLOC_CHECK(mesh->indices);

    unsigned int index_index = 0;
    for (int i = 0; i < assimp_mesh->mNumFaces; i++) {
        struct aiFace assimp_face = assimp_mesh->mFaces[i];
        for (int j = 0; j < assimp_face.mNumIndices; j++) {
            mesh->indices[index_index] = assimp_face.mIndices[j];
            index_index++;
        }
    }

    mesh->indices_number = indices_number;
}

static unsigned int
load_texture_file(const char *dirname, const char *filename) {
    int width, height, channels_number;

    unsigned int dir_len = strlen(dirname);
    unsigned int filename_len = strlen(filename);
    unsigned int path_len = dir_len + filename_len + 1;
    char *path_name = alloca(path_len + 1);
    SDL_ALLOC_CHECK(path_name);
    strcpy(path_name, dirname);
    path_name[dir_len] = '/';
    strcpy(&path_name[dir_len + 1], filename);

    unsigned int texture_id;
    glGenTextures(1, &texture_id);

    // creating from external file
    unsigned char *data = stbi_load(path_name, &width, &height, &channels_number, 0);
    if (data == NULL) {
        SDL_Die("Error loading texture_id %s", path_name);
    } else {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                    "Loaded %s: %u x %u, channels %u", path_name, width, height, channels_number);

        GLenum format;
        if (channels_number == 1) {
            format = GL_RED;
        } else if (channels_number == 3) {
            format = GL_RGB;
        } else if (channels_number == 4) {
            format = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, texture_id);

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // configure texture_id options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    stbi_image_free(data);
    return texture_id;
}

void
load_texture(model_t *model, mesh_t *mesh, enum aiTextureType type, const char *filename) {
    texture_list_item_t *texture_list_item = model->textures;
    while (texture_list_item != NULL) {
        if (strcmp(filename, texture_list_item->texture.filename) == 0) {
            mesh->textures[mesh->textures_number++] = &texture_list_item->texture;
            return;
        }
        if (texture_list_item->next == NULL) {
            break;
        }
        texture_list_item = texture_list_item->next;
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Importing texture: %s; type: %u", filename, type);

    texture_list_item_t *new_list_item = calloc(1, sizeof(texture_list_item_t));
    SDL_ALLOC_CHECK(new_list_item);
    if (texture_list_item == NULL) {
        model->textures = new_list_item;
    } else {
        texture_list_item->next = new_list_item;
    }

    texture_t *texture = &new_list_item->texture;
    texture->id = load_texture_file(model->directory, filename);
    texture->type = type;
    texture->filename = malloc(strlen(filename) + 1);
    SDL_ALLOC_CHECK(texture->filename)
    strcpy(texture->filename, filename);
    mesh->textures[mesh->textures_number++] = texture;
}

static void
import_material_textures(mesh_t *mesh, struct aiMaterial *material, enum aiTextureType type, model_t *model) {
    unsigned int textures_count = aiGetMaterialTextureCount(material, type);
    for (int i = 0; i < textures_count; i++) {
        struct aiString path;
        aiGetMaterialTexture(material, type, i, &path, NULL, NULL, NULL, NULL, NULL, NULL);
        load_texture(model, mesh, type, path.data);
    }
}

static void
import_mesh_material(struct aiMaterial *assimp_material, mesh_t *mesh) {
    material_t *material = &mesh->material;
    struct aiColor4D color;
    if (AI_SUCCESS == aiGetMaterialColor(assimp_material, AI_MATKEY_COLOR_AMBIENT, &color)) {
        vec4_set(material->ambient, color.r, color.g, color.b, color.a);
    } else {
        glm_vec4_fill(material->ambient, 0.2f);
    }

    if (AI_SUCCESS == aiGetMaterialColor(assimp_material, AI_MATKEY_COLOR_DIFFUSE, &color)) {
        vec4_set(material->diffuse, color.r, color.g, color.b, color.a);
    } else {
        glm_vec4_fill(material->diffuse, 0.8f);
    }

    if (AI_SUCCESS == aiGetMaterialColor(assimp_material, AI_MATKEY_COLOR_SPECULAR, &color)) {
        vec4_set(material->specular, color.r, color.g, color.b, color.a);
    } else {
        glm_vec4_fill(material->specular, 0.0f);
    }

    if (AI_SUCCESS == aiGetMaterialColor(assimp_material, AI_MATKEY_COLOR_EMISSIVE, &color)) {
        vec4_set(material->emissive, color.r, color.g, color.b, color.a);
    } else {
        glm_vec4_fill(material->emissive, 0.0f);
    }

    unsigned int max = 1;
    float opacity;
    if (AI_SUCCESS == aiGetMaterialFloatArray(assimp_material, AI_MATKEY_OPACITY, &opacity, &max)) {
        material->opacity = opacity;
    } else {
        material->opacity = DEFAULT_OPACITY;
    }

    float shininess;
    max = 1;
    if (AI_SUCCESS == aiGetMaterialFloatArray(assimp_material, AI_MATKEY_SHININESS, &shininess, &max)) {
        material->shininess = shininess;
        max = 1;
        if (AI_SUCCESS == aiGetMaterialFloatArray(assimp_material, AI_MATKEY_SHININESS_STRENGTH, &shininess, &max)) {
            material->shininess *= shininess;
        }
    } else {
        material->shininess = DEFAULT_SHININESS;
    }
}

static void
import_mesh_textures(mesh_t *mesh, struct aiMesh *assimp_mesh, const struct aiScene *scene, model_t *model) {
    if (assimp_mesh->mMaterialIndex < 0) {
        return;
    }
    struct aiMaterial *material = scene->mMaterials[assimp_mesh->mMaterialIndex];

    import_mesh_material(material, mesh);

    unsigned int textures_number = 0;
    for (int texture_type = 0; texture_type < aiTextureType_UNKNOWN; texture_type++) {
        unsigned int textureCount = aiGetMaterialTextureCount(material, texture_type);
        if (textureCount > 0) {
            if (texture_type >= MIN_TEXTURE_TYPE && texture_type <= MAX_TEXTURE_TYPE) {
                textures_number += textureCount;
            } else {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Found %u textures of unhandled type %u", textureCount,
                            texture_type);
            }
        }
    }

    if (textures_number == 0) {
        return;
    }

    mesh->textures = calloc(textures_number, sizeof(texture_t *));
    SDL_ALLOC_CHECK(mesh->textures)
    for (int texture_type = MIN_TEXTURE_TYPE; texture_type <= MAX_TEXTURE_TYPE; texture_type++) {
        import_material_textures(mesh, material, texture_type, model);
    }
}

static void
import_mesh(mesh_t *mesh, struct aiMesh *assimp_mesh, const struct aiScene *scene, model_t *model) {
    import_mesh_vertices(mesh, assimp_mesh);
    import_mesh_indices(mesh, assimp_mesh);
    import_mesh_textures(mesh, assimp_mesh, scene, model);
    init_mesh_gl(mesh);
}

static mesh_list_item_t *
alloc_mesh_list_item() {
    mesh_list_item_t *mesh_list_item = calloc(1, sizeof(mesh_list_item_t));
    SDL_ALLOC_CHECK(mesh_list_item);
    return mesh_list_item;
}

static void
import_node(model_t *model, struct aiNode *node, const struct aiScene *scene) {
    // process all the node's meshes (if any)
    if (node->mNumMeshes > 0) {
        mesh_list_item_t *last_mesh_item = model->meshes;
        while (last_mesh_item != NULL && last_mesh_item->next != NULL) {
            last_mesh_item = last_mesh_item->next;
        }

        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            struct aiMesh *assimp_mesh = scene->mMeshes[node->mMeshes[i]];
            mesh_list_item_t *mesh_list_item = alloc_mesh_list_item();
            import_mesh(&mesh_list_item->mesh, assimp_mesh, scene, model);
            if (last_mesh_item == NULL) {
                model->meshes = mesh_list_item;
            } else {
                last_mesh_item->next = mesh_list_item;
            }
            last_mesh_item = mesh_list_item;
        }
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        import_node(model, node->mChildren[i], scene);
    }
}

model_t *
create_model(unsigned int vertices_number, vertex_t *vertices, unsigned int indices_number, unsigned int *indices,
             const char *directory_name) {
    model_t *model = alloc_model();
    model->meshes = alloc_mesh_list_item();
    mesh_t *mesh = &model->meshes->mesh;
    mesh->vertices_number = vertices_number;
    size_t vertices_size = vertices_number * sizeof(vertex_t);
    mesh->vertices = malloc(vertices_size);
    SDL_ALLOC_CHECK(mesh->vertices)
    memcpy(mesh->vertices, vertices, vertices_size);
    mesh->indices_number = indices_number;
    size_t indices_size = indices_number * sizeof(unsigned int);
    mesh->indices = malloc(indices_size);
    SDL_ALLOC_CHECK(mesh->indices);
    memcpy(mesh->indices, indices, indices_size);
    if (directory_name != NULL) {
        model->directory = malloc(strlen(directory_name) + 1);
        SDL_ALLOC_CHECK(model->directory);
        strcpy(model->directory, directory_name);
    }
    init_mesh_gl(mesh);
    return model;
}

model_t *
load_model(char *path, unsigned int additionalOptions) {
    const struct aiScene *assimp_scene = aiImportFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                                                            additionalOptions);
    if (assimp_scene == NULL) {
        SDL_Die("Error loading assimp_scene from %s, %s", path, aiGetErrorString());
    } else if (assimp_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        SDL_Die("Incomplete assimp_scene in %s, %s", path, aiGetErrorString());
    } else if (assimp_scene->mRootNode == NULL) {
        SDL_Die("No root node in assimp_scene %s", path);
    }

    model_t *model = alloc_model();

    char *last_index = strrchr(path, '/');
    if (last_index != NULL) {
        unsigned int dir_length = last_index - path;
        model->directory = malloc(dir_length + 1);
        SDL_ALLOC_CHECK(model->directory);
        strncpy(model->directory, path, dir_length);
        model->directory[dir_length] = '\0';
    }

    import_node(model, assimp_scene->mRootNode, assimp_scene);
    model_info(model, path);
    aiReleaseImport(assimp_scene);
    return model;
}

static void
destroy_model(model_t *model) {
    if (model == NULL) {
        return;
    }
    mesh_list_item_t *current_item = model->meshes;
    while (current_item != NULL) {
        mesh_list_item_t *next_item = current_item->next;
        destroy_mesh_content(&current_item->mesh);
        free(current_item);
        current_item = next_item;
    }
    model->meshes = NULL;

    texture_list_item_t *current_texture_item = model->textures;
    while (current_texture_item != NULL) {
        texture_list_item_t *next_item = current_texture_item->next;
        destroy_texture(&current_texture_item->texture);
        free(current_texture_item);
        current_texture_item = next_item;
    }
    model->textures = NULL;

    if (model->directory != NULL) {
        free(model->directory);
    }
    free(model);
}

void
attach_model(model_t **target, model_t *model) {
    *target = model;
    model->owners++;
}

void
detach_model(model_t **model_pointer) {
    if (*model_pointer == NULL) {
        return;
    }
    model_t *model = *model_pointer;
    if (model->owners == 0) {
        SDL_Die("All owners already removed from this model");
    }
    model->owners--;
    if (model->owners == 0) {
        destroy_model(model);
    }
    *model_pointer = NULL;
}