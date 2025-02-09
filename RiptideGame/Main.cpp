#include "glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <iostream>
#include "Riptide.h"


#include "RenderInstance.h"
#include "Instance.h"
#include "Shader.h"
#include "Universe.h"
#include "Mesh.h"
#include "TrackedTransform.h"

float deltaTime = 0;

void GLFW_error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

bool Closed = false;

void ApplicationExit()
{
    if (Closed) return;

    glDeleteTextures(1, (const GLuint*) & (Riptide::Context->left_eye_color));
    glDeleteTextures(1, (const GLuint*) & (Riptide::Context->left_eye_depth));
    glDeleteFramebuffers(1, (const GLuint*)&(Riptide::Context->left_eye_viewport));

    glDeleteTextures(1, (const GLuint*)&(Riptide::Context->right_eye_color));
    glDeleteTextures(1, (const GLuint*)&(Riptide::Context->right_eye_depth));
    glDeleteFramebuffers(1, (const GLuint*)&(Riptide::Context->right_eye_viewport));

    Riptide::Universe->Destroy();

    Riptide::VRInterface->Shutdown();
    glfwDestroyWindow(Riptide::Context->window);
    glfwTerminate();
    
    delete Riptide::VRInterface;
    delete Riptide::Context;
    delete Riptide::Cloud;
    Closed = true;
}

void CreateEyeFramebuffer(vr::Hmd_Eye eye)
{
    int width, height;

    auto vec = Riptide::VRInterface->GetEyeRenderTargetSize();
    width = vec.x;
    height = vec.y;

    Riptide::Context->eye_width = width;
    Riptide::Context->eye_height = height;

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color texture
    GLuint colorTexture;
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

    // Create a depth texture
    GLuint depthTexture;
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    // Check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
        return;
    }

    // Unbind the FBO to render to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    switch (eye)
    {
    case vr::Eye_Left:
        Riptide::Context->left_eye_viewport = fbo;
        Riptide::Context->left_eye_color = colorTexture;
        Riptide::Context->left_eye_depth = depthTexture;
        break;
    case vr::Eye_Right:
        Riptide::Context->right_eye_viewport = fbo;
        Riptide::Context->right_eye_color = colorTexture;
        Riptide::Context->right_eye_depth = depthTexture;
        break;
    }
}

// 0=window 1=left 2=right
void DrawEye(int eye) {
    glClearColor(1, 0, 0, 1);
    vr::Hmd_Eye vreye = vr::Eye_Left;
    switch (eye) {
    case 1:
        vreye = vr::Eye_Left;
        break;
    case 2:
        vreye = vr::Eye_Right;
        break;
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    auto pose = glm::inverse(Riptide::VRInterface->GetHMDPoseMatrix());
    auto view = Riptide::VRInterface->GetEyeViewMatrix(vreye);
    auto proj = Riptide::VRInterface->GetProjectionMatrix(vreye, 0.1f, 1000.0f);
    auto vp = view * pose;

    for (Instance* inst : Instance::GetAllInstances())
    {
        if (dynamic_cast<RenderInstance*>(inst))
        {
            ((RenderInstance*)inst)->Draw(deltaTime, vp, proj);
        }
        // inst->Update(deltaTime);
    }
}

int main()
{
    if (!glfwInit())
    {
        Closed = true;
        throw new std::runtime_error("Failed to init GLFW");
    }

    glfwSetErrorCallback(GLFW_error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    R_Context* ctx = new R_Context();
    ctx->window = glfwCreateWindow(640, 480, "RiptideGame", NULL, NULL);
    if (!ctx->window)
    {
        Closed = true;
        glfwTerminate();
        throw new std::runtime_error("Failed to create window");
    }

    glfwMakeContextCurrent(ctx->window);
    gladLoadGL();
    // glfwSwapInterval(1);
    auto VRInterface = new OpenVRInterface();
    VRInterface->Initialize();
    
    Riptide::VRInterface = VRInterface;
    Riptide::Context = ctx;
    Riptide::Cloud = new CloudManager();

    CreateEyeFramebuffer(vr::Eye_Left);
    CreateEyeFramebuffer(vr::Eye_Right);
    float currentFrame = 0;
    float lastFrame = 0;

    auto univ = new Universe();
    univ->SetName("Universe");

    auto workspace = new Instance();
    workspace->SetName("Workspace");
    workspace->SetParent(univ);
    univ->Workspace = workspace;

    auto lighting = new Instance();
    lighting->SetName("Lighting");
    lighting->SetParent(univ);
    univ->Lighting = lighting;

    auto genericShader = new Shader();
    genericShader->SetName("test");
    genericShader->FragmentCode = R"(
#version 440 core
out vec4 FragColor;

in vec3 vertexColor;

void main()
{
    FragColor = vec4(1, 1, 1, 1.0f);
}
    )";
    genericShader->VertexCode = R"(
#version 440 core
layout (location = 0) in vec3 aPos; 
layout (location = 1) in vec3 aCol; 

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vertexColor;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vertexColor = aCol;
}
)";
    genericShader->Compile();
    genericShader->SetParent(lighting);

    Riptide::Universe = univ;

    auto leftctrl = new TrackedTransform();
    leftctrl->SetName("LeftController");
    leftctrl->PoseRole = TRACKEDPOSEROLE_LEFT_CONTROLLER;

    auto rightctrl = new TrackedTransform();
    rightctrl->SetName("RightController");
    rightctrl->PoseRole = TRACKEDPOSEROLE_RIGHT_CONTROLLER;

    rightctrl->SetParent(workspace);
    leftctrl->SetParent(workspace);

    float cube[] = {
        -1.f,     -1.f,     0.f,      1.f, 0.f, 0.f,
         1.f,     -1.f,     0.f,      1.f, 0.f, 0.f,
         0.f,     0.f,      0.f,      1.f, 0.f, 0.f,
    };

    int idxs[] = {
        0, 1, 2
    };

    auto cubee = new Mesh();
    cubee->Shader = genericShader;
    cubee->Upload(cube, 18, idxs, 3);
    cubee->SetName("SigmaCube");

    cubee->SetParent(workspace);

    //auto left = new Mesh();
    //auto right = new Mesh();
    //left->Upload(cube, 48, idxs, 36);
    //left->SetName("LeftControllerMesh");
    //left->Shader = genericShader;

    //right->Upload(cube, 48, idxs, 36);
    //right->SetName("RightControllerMesh");
    //right->Shader = genericShader;

    //left->SetParent(leftctrl);
    //right->SetParent(rightctrl);

    Riptide::Universe->PrintTree();

    glDisable(GL_CULL_FACE);

    while (!glfwWindowShouldClose(ctx->window))
    {
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        Riptide::VRInterface->UpdateTracking();

        for (Instance* inst : Instance::GetAllInstances())
        {
            inst->Update(deltaTime);
        }
        glEnable(GL_DEPTH_TEST);

        glBindFramebuffer(GL_FRAMEBUFFER, Riptide::Context->left_eye_viewport);
        glViewport(0, 0, Riptide::Context->eye_width, Riptide::Context->eye_height);
        DrawEye(1);
        
        glBindFramebuffer(GL_FRAMEBUFFER, Riptide::Context->right_eye_viewport);
        glViewport(0, 0, Riptide::Context->eye_width, Riptide::Context->eye_height);
        DrawEye(2);

        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //int width, height;
        //glfwGetFramebufferSize(Riptide::Context->window, &width, &height);
        //glViewport(0, 0, width, height);
        //DrawEye(0);

        glDisable(GL_DEPTH_TEST);


        Riptide::VRInterface->SubmitOpenGLTextures(Riptide::Context->left_eye_color, Riptide::Context->right_eye_color);
        glfwSwapBuffers(ctx->window);
        glfwPollEvents();
    }
    ApplicationExit();
}