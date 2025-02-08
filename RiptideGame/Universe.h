#pragma once
#include "Instance.h"

class Universe : public Instance
{
public:
	Instance* Workspace;
	Instance* Lighting;
	std::string GetClassName() const override { return "Universe"; }
};