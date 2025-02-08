#include "Instance.h"

std::vector<Instance*>& Instance::Everything = *new std::vector<Instance*>();

Instance::Instance()
{
	Everything.push_back(this);
}

bool Instance::GetAllocated()
{
	return allocated;
}

int Instance::GetID()
{
	return id;
}

Instance* Instance::FindFirstChild(std::string name)
{
	for (Instance* inst : Everything)
	{
		if (inst->Parent == this && inst->Name == name) {
			return inst;
		}
	}
	return nullptr;
}

void Instance::Dispose()
{
	if (!allocated) return;
	Everything.erase(std::remove(Everything.begin(), Everything.end(), this), Everything.end());
	allocated = false;
}

void Instance::Update(float dt)
{
}
