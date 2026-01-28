#pragma once

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/bindings/Scene.h>
#include <artery/sionna/environment/Material.h>

#include <inet/environment/contract/IMaterialRegistry.h>

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
    py::SionnaScene<Float, Spectrum> scene_;
};

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
