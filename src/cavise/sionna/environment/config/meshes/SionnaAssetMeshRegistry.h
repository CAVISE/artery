#pragma once

#include <omnetpp/csimplemodule.h>

#include <cavise/sionna/bridge/bindings/Modules.h>
#include <cavise/sionna/environment/config/meshes/IMeshRegistry.h>

#include <mitsuba/core/filesystem.h>

namespace artery::sionna {

    // Simple mesh registry that returns meshes from static mapping.
    class SionnaDirectAssetMeshRegistry
        : public IMeshRegistry
        , public omnetpp::cSimpleModule {
    public:
        SionnaDirectAssetMeshRegistry() = default;

        // omnetpp::cSimpleModule implementation.
        void initialize() override;

        // IMeshRegistry implementation.
        mitsuba::ref<mitsuba::Resolve::Mesh> getMesh(MeshAsset asset) const override;

    private:
        mitsuba::fs::path root_;
    };

} // namespace artery::sionna
