#include "glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <iostream>
#include "Riptide.h"

void GLFW_error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void ApplicationExit()
{
    glfwDestroyWindow(Riptide::Context->window);
    delete Riptide::Context;
    glfwTerminate();
}

int main()
{
    if (!glfwInit())
    {
        throw new std::runtime_error("Failed to init GLFW");
    }

    glfwSetErrorCallback(GLFW_error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    R_Context* ctx = new R_Context();
    ctx->window = glfwCreateWindow(640, 480, "RiptideGame", NULL, NULL);
    if (!ctx->window)
    {
        glfwTerminate();
        throw new std::runtime_error("Failed to create window");
    }

    glfwMakeContextCurrent(ctx->window);
    gladLoadGL();
    glfwSwapInterval(1);

    Riptide::Context = ctx;
    while (!glfwWindowShouldClose(ctx->window))
    {
        // g
        int width, height;

        glfwGetFramebufferSize(ctx->window, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwSwapBuffers(ctx->window);
        glfwPollEvents();
    }
    ApplicationExit();
}