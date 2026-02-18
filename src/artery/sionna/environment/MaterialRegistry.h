#pragma once

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/SionnaBridge.h>
#include <artery/sionna/environment/Material.h>

#include <inet/environment/contract/IMaterialRegistry.h>

#include <unordered_map>

namespace artery {

    namespace sionna {

        MI_VARIANT
        class InetMaterialRegistry
            : public inet::physicalenvironment::IMaterialRegistry {
        public:
            SIONNA_BRIDGE_IMPORT_BRIDGE_TYPES()

            explicit InetMaterialRegistry(SionnaScene scene)
                : scene_(std::move(scene))
            {}

            // inet::physicalenvironment::IMaterialRegistry implementation.
            const inet::physicalenvironment::Material *getMaterial(const char *name) const override;

            virtual ~InetMaterialRegistry() = default;

        private:
            SionnaScene scene_;
            // this class owns pointers to the materials, so they must persist.
            mutable std::unordered_map<std::string, InetRadioMaterial<Float, Spectrum>> materials_;
        };

    }

}

SIONNA_BRIDGE_EXTERN_CLASS(artery::sionna::InetMaterialRegistry)
