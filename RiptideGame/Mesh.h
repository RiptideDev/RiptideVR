// Mesh.h
#pragma once
#include "glad.h"
#include "RenderInstance.h"
#include "Shader.h"
#include <memory>

class Mesh : public RenderInstance {
private:
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;
    int indexCount = 0;

public:
    Shader *Shader;

    void Upload(float* vertexData, int vertexCount, int* indexData, int indexCount);
    void Destroy() override;
    void Draw(float dt, glm::mat4 view, glm::mat4 proj) override;
    std::string GetClassName() const override { return "Mesh"; }

private:
    void SetupBuffers(float* vertexData, int vertexCount, int* indexData, int indexCount);
};