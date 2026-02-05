#pragma once

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/SionnaBridge.h>
#include <artery/sionna/environment/Material.h>

#include <inet/environment/contract/IMaterialRegistry.h>

#include <unordered_map>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

MI_VARIANT
class SionnaSceneMaterialRegistry : public inet::physicalenvironment::IMaterialRegistry {
public:
    SIONNA_IMPORT_BRIDGE_TYPES(SionnaScene)

    explicit SionnaSceneMaterialRegistry(py::SionnaScene<Float, Spectrum> scene)
        : scene_(std::move(scene))
    {}

    // inet::physicalenvironment::IMaterialRegistry implementation.
    const inet::physicalenvironment::Material *getMaterial(const char *name) const override;

private:
    // this class owns pointers to the materials, so they must persist.
    mutable std::unordered_map<std::string, RadioMaterial<Float, Spectrum>> materials_;
    py::SionnaScene<Float, Spectrum> scene_;
};

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
