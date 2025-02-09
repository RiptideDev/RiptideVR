#pragma once
#include "Instance.h"
#include "types.h"
#include "glm/matrix.hpp"

struct TransformInstanceNetworkPacket
{
	glm::mat4 ModelMatrix;
};

class TransformInstance : public Instance
{
public:
	glm::mat4 ModelMatrix;
	// on draw we should update our position and rotation based on our parent
	void Update(float dt) override;

	void ResetMatrix();
	void SetPosition(Vector3 position);
	void SetRotation(Vector3 rotation);

	glm::mat4 GetGlobalMatrix();

	char* GetNetworkPacket(float dt) override;
	void ApplyNetworkPacket(float dt, const char* packet) override;

	std::string GetClassName() const override { return "TransformInstance"; }
private:
	glm::mat4 RenderModelMatrix;
};