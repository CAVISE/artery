#pragma once

#include <limits>
#include <memory>

#include <mitsuba/core/ray.h>
#include <mitsuba/render/scene.h>
#include <mitsuba/render/interaction.h>
#include <inet/environment/contract/IGround.h>

#include "artery/sionna/bridge/Fwd.h"

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

MI_VARIANT class UnevenTerrain
    : public omnetpp::cSimpleModule
    , public inet::physicalenvironment::IGround {
public:
    SIONNA_IMPORT_CORE_TYPES(Point3f, Vector3f)
    SIONNA_IMPORT_RENDER_TYPES(SurfaceInteraction, Scene)
    SIONNA_IMPORT_BRIDGE_TYPES(SionnaScene)

    UnevenTerrain(std::shared_ptr<SionnaScene> scene);

    // inet::physicalenvironment::IGround implementation
    virtual double getElevation(const inet::Coord& position) const;

private:
    std::shared_ptr<SionnaScene> scene_; 
};

NAMESPACE_END(sionna)
NAMESPACE_END(artery)

MI_VARIANT
artery::sionna::UnevenTerrain<Float, Spectrum>::UnevenTerrain(std::shared_ptr<SionnaScene> scene)
    : scene_(std::move(scene))
{
    // NOOP.
}

MI_VARIANT
double artery::sionna::UnevenTerrain<Float, Spectrum>::getElevation(const inet::Coord& position) const {
    mitsuba::Ray<Point3f, Spectrum> norm;
    norm.o = Point3f(position.x, position.y, std::numeric_limits<float>::max());
    norm.d = Vector3f(0, 0, -1);

    Scene& scene = scene_->scene();

    if (SurfaceInteraction surface = scene.ray_intersect(norm); surface.is_valid()) {
        return std::numeric_limits<double>::max();
    } else {
        return static_cast<double>(surface.p.z());
    }
}
