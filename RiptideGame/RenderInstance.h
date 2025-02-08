#pragma once
#include "Instance.h"
#include "types.h"
#include "glm/matrix.hpp"

class RenderInstance : Instance
{
public:
	Vector3 Position{};
	Vector3 Rotation{};

	virtual void Draw(float dt, glm::mat4 view, glm::mat4 proj);
};