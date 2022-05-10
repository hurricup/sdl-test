#include "model.h"

mesh_t *
create_mesh(unsigned long vertices_number, GLsizei indices_number) {
    mesh_t *mesh = malloc(sizeof(mesh_t));
    mesh->vertices_number = vertices_number;
    mesh->vertices = malloc(vertices_number * sizeof(vertex_t));
    mesh->indices_number = indices_number;
    mesh->indices = malloc(indices_number * sizeof(unsigned long));
    mesh->textures_number = 0;
    mesh->textures = NULL;

    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);

    glBindVertexArray(mesh->vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices_number * sizeof(vertex_t), mesh->vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_number * sizeof(unsigned int), mesh->indices, GL_STATIC_DRAW);

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

    return mesh;
}

void draw_mesh(mesh_t *mesh) {
    for (int i = 0; i < mesh->textures_number; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, mesh->textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->indices_number, GL_UNSIGNED_INT, 0);
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

void draw_model(model_t *model) {
    for (int i = 0; i < model->meshes_number; i++) {
        draw_mesh(&model->meshes[i]);
    }
}

static model_t *
alloc_model() {
    model_t *model = malloc(sizeof(model_t));
    model->meshes = NULL;
    model->meshes_number = 0;
    model->directory = NULL;
    return model;
}

model_t *
load_model(char *path) {
    return alloc_model();
}

void
destroy_model(model_t *model) {
    for (int i = 0; i < model->meshes_number; i++) {
        destroy_mesh(&model->meshes[i]);
    }
    model->meshes_number = 0;
    free(model->meshes);
    model->meshes = NULL;
    free(model);
};