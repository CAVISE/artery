#include "MaterialRegistry.h"

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

SIONNA_INSTANTIATE_CLASS(SionnaSceneMaterialRegistry)

MI_VARIANT
const inet::physicalenvironment::Material *
SionnaSceneMaterialRegistry<Float, Spectrum>::getMaterial(const char *name) const {
    if (!scene_) {
        return nullptr;
    }
    auto mats = scene_->radioMaterials();
    auto it = mats.find(name);
    if (it == mats.end()) {
        return nullptr;
    }
    return &it->second;
}

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
