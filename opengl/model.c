#include "model.h"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "sdl_ext.h"

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

void draw_mesh(mesh_t *mesh) {
    for (int i = 0; i < mesh->textures_number; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, mesh->textures[i]->id);
    }
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->indices_number, GL_UNSIGNED_INT, 0);
    GL_CHECK_ERROR;

    glBindVertexArray(0);
}

void
destroy_mesh(mesh_t *mesh) {
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
    if (mesh->textures) {
        free(mesh->textures);
        mesh->textures_number = 0;
        mesh->textures = NULL;
    }
    free(mesh);
}

void
model_info(model_t *model, const char *path) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Model %s (%s):", path, model->directory);
    unsigned int mesh_counter = 0;
    mesh_list_item_t *current_item = model->meshes;
    while (current_item != NULL) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "\tid: %u; vertices: %u; indices: %u", mesh_counter,
                    current_item->mesh.vertices_number, current_item->mesh.indices_number);
        current_item = current_item->next;
        mesh_counter++;
    }
}

void
draw_model(model_t *model) {
    mesh_list_item_t *current_item = model->meshes;
    while (current_item != NULL) {
        draw_mesh(&current_item->mesh);
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

static void
import_mesh(mesh_t *mesh, struct aiMesh *assimp_mesh, const struct aiScene *scene) {
    import_mesh_vertices(mesh, assimp_mesh);
    import_mesh_indices(mesh, assimp_mesh);

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
            import_mesh(&mesh_list_item->mesh, assimp_mesh, scene);
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
create_model(unsigned int vertices_number, vertex_t *vertices, unsigned int indices_number, unsigned int *indices) {
    model_t *model = alloc_model();
    model->meshes = alloc_mesh_list_item();
    mesh_t *mesh = &model->meshes->mesh;
    mesh->vertices_number = vertices_number;
    size_t vertices_size = vertices_number * sizeof(vertex_t);
    mesh->vertices = malloc(vertices_size);
    memcpy(mesh->vertices, vertices, vertices_size);
    mesh->indices_number = indices_number;
    size_t indices_size = indices_number * sizeof(unsigned int);
    mesh->indices = malloc(indices_size);
    memcpy(mesh->indices, indices, indices_size);
    init_mesh_gl(mesh);
    return model;
}

model_t *
load_model(char *path) {
    const struct aiScene *assimp_scene = aiImportFile(path, aiProcess_Triangulate | aiProcess_FlipUVs |
                                                            aiProcess_GenSmoothNormals);
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
        strncpy(model->directory, path, dir_length);
        model->directory[dir_length] = '\0';
    }

    import_node(model, assimp_scene->mRootNode, assimp_scene);
    model_info(model, path);
    aiReleaseImport(assimp_scene);
    return model;
}

void
destroy_model(model_t *model) {
    mesh_list_item_t *current_item = model->meshes;
    while (current_item != NULL) {
        mesh_list_item_t *next_item = current_item->next;
        destroy_mesh(&current_item->mesh);
        free(current_item);
        current_item = next_item;
    }
    model->meshes = NULL;

    if (model->directory != NULL) {
        free(model->directory);
    }
    free(model);
}