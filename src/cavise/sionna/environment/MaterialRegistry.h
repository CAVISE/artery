#pragma once

#include <cavise/sionna/bridge/SionnaBridge.h>
#include <cavise/sionna/environment/Material.h>

#include <inet/environment/contract/IMaterialRegistry.h>

#include <unordered_map>

namespace artery::sionna {

    class MaterialRegistry
        : public inet::physicalenvironment::IMaterialRegistry {
    public:

        explicit MaterialRegistry(py::SionnaScene scene)
            : scene_(std::move(scene)) {
        }

        // inet::physicalenvironment::IMaterialRegistry implementation.
        const inet::physicalenvironment::Material* getMaterial(const char* name) const override;

    private:
        py::SionnaScene scene_;
        // this class owns pointers to the materials, so they must persist.
        mutable std::unordered_map<std::string, RadioMaterial> materials_;
    };

} // namespace artery::sionna
