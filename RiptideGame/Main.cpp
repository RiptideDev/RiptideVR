#include "glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <iostream>
#include "Riptide.h"
#include "RenderInstance.h"
#include "Instance.h"

float deltaTime;

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

    Riptide::VRInterface->Shutdown();
    glfwDestroyWindow(Riptide::Context->window);
    glfwTerminate();
    
    delete Riptide::VRInterface;
    delete Riptide::Context;
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
    glClearColor(0, 0, 0, 1);
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

    for (Instance* inst : Instance::Everything)
    {
        if (dynamic_cast<RenderInstance*>(inst))
        {
            ((RenderInstance*)inst)->Draw(deltaTime, view * pose, proj);
        }
        // inst->Update(deltaTime);
    }

    //glPushMatrix();

    //glMatrixMode(GL_PROJECTION);
    //glLoadMatrixf(glm::value_ptr(Riptide::VRInterface->GetProjectionMatrix(vreye, 0.1f, 1000.0f)));

    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();

    //auto viewMatrix = Riptide::VRInterface->GetEyeViewMatrix(vreye);
    //glMultMatrixf(glm::value_ptr(viewMatrix));

    //auto pose = glm::inverse(Riptide::VRInterface->GetHMDPoseMatrix());
    //glMultMatrixf(glm::value_ptr(pose));

    //auto right_ctrl = Riptide::VRInterface->GetControllerPoseMatrix(vr::TrackedControllerRole_RightHand);
    //auto left_ctrl = Riptide::VRInterface->GetControllerPoseMatrix(vr::TrackedControllerRole_LeftHand);
    //glPushMatrix();
    //glMultMatrixf(glm::value_ptr(right_ctrl));

    //// Draw a triangle
    //glBegin(GL_TRIANGLES);
    //glColor3f(  1,  0,      0);
    //glVertex3f(-0.25f, -0.25f,  0);
    //glColor3f(  0,      1,  0);
    //glVertex3f( 0.25f, -0.25f,  0);
    //glColor3f(  0,      0,      1);
    //glVertex3f( 0,      0.25f,  0);
    //glEnd();

    //glPopMatrix();
    //glPushMatrix();
    //glMultMatrixf(glm::value_ptr(left_ctrl));

    //// Draw a triangle
    //glBegin(GL_TRIANGLES);
    //glColor3f(1, 0, 0);
    //glVertex3f(-0.25f, -0.25f, 0);
    //glColor3f(0, 1, 0);
    //glVertex3f(0.25f, -0.25f, 0);
    //glColor3f(0, 0, 1);
    //glVertex3f(0, 0.25f, 0);
    //glEnd();
    //glPopMatrix();
    //glPopMatrix();
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

    CreateEyeFramebuffer(vr::Eye_Left);
    CreateEyeFramebuffer(vr::Eye_Right);
    float currentFrame;
    float lastFrame;
    while (!glfwWindowShouldClose(ctx->window))
    {
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        Riptide::VRInterface->UpdateTracking();

        for (Instance* inst : Instance::Everything)
        {
            inst->Update(deltaTime);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, Riptide::Context->left_eye_viewport);
        glViewport(0, 0, Riptide::Context->eye_width, Riptide::Context->eye_height);
        glEnable(GL_DEPTH_TEST);
        DrawEye(1);
        

        glBindFramebuffer(GL_FRAMEBUFFER, Riptide::Context->right_eye_viewport);
        glViewport(0, 0, Riptide::Context->eye_width, Riptide::Context->eye_height);
        DrawEye(2);
        glDisable(GL_DEPTH_TEST);


        Riptide::VRInterface->SubmitOpenGLTextures(Riptide::Context->left_eye_color, Riptide::Context->right_eye_color);
        glfwSwapBuffers(ctx->window);
        glfwPollEvents();
    }
    ApplicationExit();
}