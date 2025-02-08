#pragma once
#include "GLFW/glfw3.h"

struct R_Context
{
	GLFWwindow* window;
};

static class Riptide
{
public:
	static R_Context *Context;
};