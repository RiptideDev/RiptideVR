#include "TransformInstance.h"
#include <glm/ext/matrix_transform.hpp>

void TransformInstance::Update(float dt)
{
	auto parent = GetParent();
	RenderModelMatrix = ModelMatrix;
	if (parent != nullptr)
	{
		if (dynamic_cast<TransformInstance*>(parent))
		{
			auto rparent = dynamic_cast<TransformInstance*>(parent);
			RenderModelMatrix *= rparent->RenderModelMatrix;
		}
	}
}

void TransformInstance::ResetMatrix()
{
	ModelMatrix = {};
}

void TransformInstance::SetPosition(Vector3 position)
{
	glm::translate(ModelMatrix, glm::vec3{ position.x, position.y, position.z });
}

void TransformInstance::SetRotation(Vector3 rotation)
{
	glm::rotate(ModelMatrix, rotation.x, glm::vec3{ 1, 0, 0 });
	glm::rotate(ModelMatrix, rotation.y, glm::vec3{ 0, 1, 0 });
	glm::rotate(ModelMatrix, rotation.z, glm::vec3{ 0, 0, 1 });
}

glm::mat4 TransformInstance::GetGlobalMatrix()
{
	return RenderModelMatrix;
}


char* TransformInstance::GetNetworkPacket(float dt)
{
	auto inst = TransformInstanceNetworkPacket{ ModelMatrix };
	return (char*)(&inst);
}

void TransformInstance::ApplyNetworkPacket(float dt, char *packet)
{
	auto Packet = (TransformInstanceNetworkPacket*)packet;
	ModelMatrix = Packet->ModelMatrix;
	free(Packet);
}
