#pragma once

#include <cavise/sionna/bridge/bindings/Scene.h>
#include <cavise/sionna/bridge/bindings/SceneObject.h>
#include <cavise/sionna/environment/config/dynamic/TraciCoordinateTransformer.h>

#include <string>
#include <vector>
#include <optional>

namespace artery::sionna {

    // This coordinate transformer uses ray casts to dynamically adjust Z
    // coordinate if it's required by the scene's parameters.
    class TraciRelaxedCoordinateTransformer
        : public TraciCoordinateTransformer {
    public:
        // omnetpp::cSimpleModule implementation.
        int numInitStages() const override;
        void initialize(int stage) override;

        // Set relevant Sionna scene.
        void bindScene(py::SionnaScene scene);

        // Relaxed transformer finds better Z coordinate for objects themselves. This
        // assumes object is already present - ensure it was first created on top of a road.
        void adjust(py::SceneObject object) const;

    private:
        // Gets all hits for given plane.
        std::vector<mitsuba::Resolve::Point3f> discoverRoadHits(const mitsuba::Resolve::Matrix4f& upperPoints) const;

        std::optional<mitsuba::Resolve::Point3f> orientationForPlane(
            const mitsuba::Resolve::Matrix4f& plane,
            const mitsuba::Resolve::Vector3f& localVelocity) const;

    private:
        // NOTE: Sionna rebuilds scene for each change, so we keep
        // the wrapper here to fetch relevant ref to current mi.Scene.
        std::optional<py::SionnaScene> scene_;

        std::string roadMeshName_;
    };

} // namespace artery::sionna
