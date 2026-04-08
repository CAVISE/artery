#include "SionnaAssetMeshRegistry.h"

#include <cavise/sionna/bridge/Fwd.h>
#include <cavise/sionna/bridge/bindings/Modules.h>
#include <cavise/sionna/environment/config/meshes/IMeshRegistry.h>

#include <nanobind/nanobind.h>

#include <omnetpp/cexception.h>

#include <mitsuba/core/plugin.h>
#include <mitsuba/render/mesh.h>
#include <mitsuba/core/properties.h>

using namespace artery::sionna;

Define_Module(SionnaDirectAssetMeshRegistry);

namespace {

    using path = mitsuba::fs::path;

    // I don't think there is a better way to do that.
    // Paths are relative to scenes/ module.
    const std::unordered_map<MeshAsset, mitsuba::fs::path> meshes = {
        {MeshAsset::LowPolyCar, path("low_poly_car.ply")}};

} // namespace

void SionnaDirectAssetMeshRegistry::initialize() {
    root_ = ScenesFileview::scenes();
    EV_INFO << "Using meshes root: " << root_ << "\n";

    if (root_.empty()) {
        throw omnetpp::cRuntimeError("could not find Sionna's scene root: path is empty");
    }
}

SionnaMeshAsset SionnaDirectAssetMeshRegistry::getAsset(MeshAsset asset) const {
    if (auto mesh = meshes.find(asset); mesh == meshes.end()) {
        throw omnetpp::cRuntimeError("could not access asset: path for this asset is not defined");
    } else {
        const auto& [_, suffix] = *mesh;

        mitsuba::Properties props("ply");
        props.set("filename", (root_ / suffix).string());

        return {
            mitsuba::PluginManager::instance()->create_object<mitsuba::Resolve::Mesh>(props),
            py::RadioMaterial("mat-low_poly_car")
        };
    }
}
