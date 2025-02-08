#pragma once
#include "GLFW/glfw3.h"
#include "OpenVRInterface.h"
#include "Universe.h"

struct R_Context
{
	GLFWwindow* window;
	int left_eye_viewport, right_eye_viewport;

	int left_eye_color, right_eye_color;
	int left_eye_depth, right_eye_depth;

	int eye_width, eye_height;
};

static class Riptide
{
public:
	static R_Context* Context;
	static OpenVRInterface* VRInterface;
	static Universe* Universe;
};