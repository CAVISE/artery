#pragma once

#include <traci/Boundary.h>

#include <cavise/sionna/bridge/Fwd.h>
#include <cavise/sionna/bridge/bindings/Scene.h>
#include <cavise/sionna/environment/api/SionnaAPI.h>
#include <cavise/sionna/bridge/bindings/SceneObject.h>

#include <optional>
#include <vector>

namespace artery::sionna {

    class MeshInteractionResolver {
    public:

        // Adds mesh to resolution order.
        MeshInteractionResolver& addMesh(const std::string& id);

        // Resolves surface shapes by casting ray down first. Shapes are resolved
        // as they were added, which means that shapes added last are accounted first. Terrain
        // should be added first as the last resort to place object, roads first.
        std::vector<mi::SurfaceInteraction3f> resolve(const mi::TensorXf& plane, mi::Scene& scene) const;

    private:
        std::vector<std::string> ids_;

    };

    // Rotation/translation/remap based coordinate transform used between SUMO,
    // canonical scene coordinates, and Sionna's local scene axis order.
    class AffineCoordinateTransform
        : public ICoordinateTransformProxy {
    public:
        // Initialize transformer with remapping matrices, and
        // Sionna API.
        AffineCoordinateTransform(
            const mi::Matrix3f& remap,
            const mi::Matrix2f& rotation,
            const mi::Vector3f& translation,
            const traci::Boundary& boundary,
            ISionnaAPI* api
        );

        MeshInteractionResolver& meshResolver();

        // ICoordinateTransformProxy implementation.
        mi::Vector3f convertCoordinates(CoordinateSystem from, CoordinateSystem to, const mi::Vector3f& v) override;
        void adjustVerticalComponent(py::SceneObject& object) override;

    private:
        // Dispatches for transforms.
        mi::Vector3f dispatchLocalScene(CoordinateSystem to, const mi::Vector3f& v);
        mi::Vector3f dispatchSionnaScene(CoordinateSystem to, const mi::Vector3f& v);
        mi::Vector3f dispatchSumo(CoordinateSystem to, const mi::Vector3f& v);
        mi::Vector3f dispatchInet(CoordinateSystem to, const mi::Vector3f& v);

        std::optional<mi::Point3f> orientationForPlane(const mi::TensorXf& plane) const;

    private:
        // Matrices used for Sionna transforms.
        mi::Matrix3f remap_;
        mi::Matrix3f inverseRemap_;
        mi::Matrix2f rotation_;
        mi::Matrix2f inverseRotation_;
        mi::Vector3f translation_;

        // Boundary is used for SUMO coordinate casts.
        traci::Boundary boundary_;
        ISionnaAPI* api_ = nullptr;

        MeshInteractionResolver resolver_;
    };

} // namespace artery::sionna
