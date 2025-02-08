#pragma once
#include <string>
#include <vector>

class Instance
{
private:
	bool allocated = false;
	int id;
public:
	Instance();
	static std::vector<Instance*>& Everything;

	bool GetAllocated();
	int GetID();

	Instance *FindFirstChild(std::string name);

	std::string Name;
	Instance* Parent;

	virtual void Dispose();
	// update/draw
	virtual void Update(float dt);
};