#pragma once
#include "Instance.h"
#include "types.h"
#include "glm/matrix.hpp"
#include "TransformInstance.h"
#include <string>

class RenderInstance : public TransformInstance
{
public:
	bool Networked = true;
	virtual void Draw(float dt, glm::mat4 view, glm::mat4 proj);

	std::string GetClassName() const override { return "RenderInstance"; }
};