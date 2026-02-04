#include "MaterialRegistry.h"

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

SIONNA_INSTANTIATE_CLASS(SionnaSceneMaterialRegistry)

MI_VARIANT
const inet::physicalenvironment::Material *
SionnaSceneMaterialRegistry<Float, Spectrum>::getMaterial(const char *name) const {
    auto mats = scene_.radioMaterials();
    if (auto it = mats.find(name); it != mats.end()) {
        auto inserted = materials_.try_emplace(
            it->first, it->second.object());
        return &inserted.first->second;
    }
    return nullptr;
}

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
