// Mesh.cpp
#include "Mesh.h"
#include <iostream>
#include <glm/ext/matrix_transform.hpp>

void Mesh::Upload(float* vertexData, int vertexCount, int* indexData, int indexCount) {
    if (!vertexData || !indexData || vertexCount <= 0 || indexCount <= 0) {
        std::cerr << "Mesh::Upload: Invalid data parameters!\n";
        return;
    }

    // Clear existing buffers
    Destroy();

    // Generate new buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    this->indexCount = indexCount; // Properly store index count
    SetupBuffers(vertexData, vertexCount, indexData, indexCount);
}

void Mesh::SetupBuffers(float* vertexData, int vertexCount, int* indexData, int indexCount) {
    glBindVertexArray(VAO);

    // Vertex Buffer (position + color)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertexData, GL_STATIC_DRAW);

    // Element Buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(int), indexData, GL_STATIC_DRAW);

    // Position attribute (3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute (3 floats after position)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Mesh::Destroy() {
    if (IsDestroyed()) return;
    RenderInstance::Destroy();

    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);

    VAO = VBO = EBO = 0;
    indexCount = 0;
}

void Mesh::Draw(float dt, glm::mat4 view, glm::mat4 proj) {
    if (!Shader || indexCount == 0) {
        return;
    }

    glUseProgram(Shader->GetNative());

    Shader->SetMat4("model", GetGlobalMatrix());
    Shader->SetMat4("view", view);
    Shader->SetMat4("projection", proj);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}