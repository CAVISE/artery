#pragma once

#include <mitsuba/core/ray.h>
#include <mitsuba/render/scene.h>
#include <mitsuba/render/interaction.h>

#include <inet/environment/contract/IGround.h>

#include <cavise/sionna/bridge/Fwd.h>
#include <cavise/sionna/bridge/bindings/Scene.h>

namespace artery::sionna {

    class Terrain
        : public omnetpp::cSimpleModule
        , public inet::physicalenvironment::IGround {
    public:

        Terrain(const py::SionnaScene& scene);

        // inet::physicalenvironment::IGround implementation
        double getElevation(const inet::Coord& position) const override;

    private:
        py::SionnaScene scene_;
    };

} // namespace artery::sionna
