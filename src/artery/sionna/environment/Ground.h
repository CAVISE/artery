#pragma once

#include <memory>

#include <mitsuba/core/ray.h>
#include <mitsuba/render/scene.h>
#include <mitsuba/render/interaction.h>

#include <inet/environment/contract/IGround.h>

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Compat.h>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

MI_VARIANT class UnevenTerrain
    : public omnetpp::cSimpleModule
    , public inet::physicalenvironment::IGround {
public:
    SIONNA_IMPORT_CORE_TYPES(Point3f, Vector3f)
    SIONNA_IMPORT_RENDER_TYPES(SurfaceInteraction3f, Scene)
    SIONNA_IMPORT_BRIDGE_TYPES(SionnaScene, Compat)

    UnevenTerrain(std::shared_ptr<SionnaScene> scene);

    // inet::physicalenvironment::IGround implementation
    virtual double getElevation(const inet::Coord& position) const;

private:
    std::shared_ptr<SionnaScene> scene_;
};

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
