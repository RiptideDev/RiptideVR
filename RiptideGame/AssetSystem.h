#pragma once
#include <string>
#include "types.h"

class AssetSystem
{
public:
	Asset* GetLocalAsset(std::string relpath);
	void FreeAsset(Asset* ref);
private:
	std::string ContentPath = "Content";
	std::string CachePath = "Cache";
};