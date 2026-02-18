#include "artery/sionna/bridge/Fwd.h"
#include "artery/sionna/environment/Material.h"

#include "MaterialRegistry.h"

using namespace artery::sionna;

SIONNA_BRIDGE_INSTANTIATE_CLASS(artery::sionna::InetMaterialRegistry)

MI_VARIANT
const inet::physicalenvironment::Material *
InetMaterialRegistry<Float, Spectrum>::getMaterial(const char *name) const {
    auto mats = scene_.radioMaterials();
    if (auto it = mats.find(name); it != mats.end()) {
        auto inserted = materials_.try_emplace(it->first, InetRadioMaterial<Float, Spectrum>(it->second));
        return &inserted.first->second;
    }
    return nullptr;
}
