#include "AssetSystem.h"

Asset* AssetSystem::GetLocalAsset(std::string relpath)
{
    // sanitise the path.
    // only these are allowed
    // content://x
    // cache://x
    // backslashes should be removed
    // eg cache://../../ will be rejected and a nullptr is returned
    return nullptr;
}

void AssetSystem::FreeAsset(Asset* ref)
{
    if (ref == nullptr) return;
    free(ref->content);
    delete ref;
}
