#pragma once
#include "glm/matrix.hpp"
#include "Instance.h"


class Shader : public Instance
{
private:
	int Native;
public:
	std::string VertexCode;
	std::string FragmentCode;
	bool Compile();
	int GetNative();
	void Destroy() override;

	void SetMat4(const std::string& name, const glm::mat4& value) const;
	void SetFloat(const std::string& name, float value) const;

	std::string GetClassName() const override { return "Shader"; }
};