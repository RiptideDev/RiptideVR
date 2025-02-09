#include "Cloud.h"
#include <thread>

std::thread *prev_async_thread = nullptr;

void download_async(std::string assetid)
{
	// todo
}

void CloudManager::DownloadAssetAsync(std::string asset)
{
	auto thread = std::thread(download_async, asset);
	prev_async_thread = &thread;
}

Asset* CloudManager::GetAsset(std::string asset)
{
	return new Asset();
}

bool CloudManager::IsPrevAsyncDone()
{
	if (prev_async_thread == nullptr) return true;
	if (prev_async_thread->joinable()) return false;
	return true;
}

void CloudManager::ClearCache()
{
}
