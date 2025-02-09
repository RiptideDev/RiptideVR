#pragma once

#include "types.h"
#include <string>

class CloudManager
{
public:
	void DownloadAssetAsync(std::string asset);

	Asset* GetAsset(std::string asset); // downloads the asset (if it isnt cached) and loads it

	bool IsPrevAsyncDone();

	void ClearCache();
private:
};