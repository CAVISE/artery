#pragma once

#include <cavise/sionna/bridge/Fwd.h>
#include <cavise/sionna/bridge/Helpers.h>
#include <cavise/sionna/bridge/capabilities/Core.h>
#include <cavise/sionna/bridge/bindings/Modules.h>
#include <cavise/sionna/bridge/bindings/Material.h>

#include <nanobind/stl/string.h>

#include <mitsuba/render/mesh.h>
#include <mitsuba/core/filesystem.h>

#include <cstdint>

namespace artery::sionna {

    // Common entry point for Sionna-provided meshes and other scene stuff.
    class ScenesFileview {
    public:

        // Returns path to scenes root. All assets should be relative to this path.
        static mitsuba::fs::path scenes() {
            class Importer
                : public py::SionnaRtScenesModule
                , public py::CachedImportCapability {
            } importer;

            auto module = importer.module();
            auto path = sionna::access<std::string>(module, "__file__");

            return mitsuba::fs::path(path).parent_path();
        }
    };

    // Logical keys for mesh assets used by the simulation.
    enum class MeshAsset : std::uint8_t {
        LowPolyCar
    };

    struct SionnaMeshAsset {
        mitsuba::ref<mitsuba::Resolve::Mesh> mesh;
        py::RadioMaterial material;
    };

    // Registry contract for retrieving Mitsuba meshes and defaults by logical asset id.
    class IMeshRegistry {
    public:
        virtual ~IMeshRegistry() = default;

        // Return asset payload for a given logical asset.
        virtual SionnaMeshAsset getAsset(MeshAsset asset) const = 0;
    };

} // namespace artery::sionna
