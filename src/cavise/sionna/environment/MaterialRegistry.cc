#include "MaterialRegistry.h"

using namespace artery::sionna;

const inet::physicalenvironment::Material*
MaterialRegistry::getMaterial(const char* name) const {
    auto mats = scene_.radioMaterials();
    if (auto it = mats.find(name); it != mats.end()) {
        auto inserted = materials_.try_emplace(it->first, RadioMaterial(it->second));
        return &inserted.first->second;
    }
    return nullptr;
}
