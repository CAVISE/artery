#pragma once

#include <inet/common/geometry/base/ShapeBase.h>
#include <inet/common/geometry/shape/Cuboid.h>
#include <artery/sionna/bridge/Fwd.h>
#include <mitsuba/core/bbox.h>
#include <mitsuba/render/scene.h>
#include <mitsuba/core/ray.h>
#include <drjit/array.h>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

/**
 * @brief ShapeBase adapter that uses a Mitsuba scene to intersect a mesh; bbox
 * size is reported for metadata only.
 *
 * Intersection is performed via Mitsuba's acceleration structure
 * (ray_intersect_preliminary); if no scene is available, intersection fails.
 */
MI_VARIANT
class MitsubaBBoxShape
    : public inet::ShapeBase {
public:
    SIONNA_IMPORT_CORE_TYPES(Point3f, Vector3f)
    SIONNA_IMPORT_RENDER_TYPES(BBox, Scene, Ray3f)

    MitsubaBBoxShape(const BBox& bbox, mitsuba::ref<Scene> scene)
        : size_(bbox.extents().x(), bbox.extents().y(), bbox.extents().z())
        , scene_(std::move(scene))
    {}

    inet::Coord computeBoundingBoxSize() const override {
        return size_;
    }

    bool computeIntersection(
        const inet::LineSegment& lineSegment,
        inet::Coord& intersection1,
        inet::Coord& intersection2,
        inet::Coord& normal1,
        inet::Coord& normal2
    ) const override {
        // Build ray from segment
        Ray3f ray;
        const auto& p1 = lineSegment.getPoint1();
        const auto& p2 = lineSegment.getPoint2();

        ray.o = Point3f(p1.x, p1.y, p1.z);
        ray.d = Vector3f(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);

        const float len = drjit::norm(ray.d);
        if (len <= 0.0f) {
            intersection1 = intersection2 = normal1 = normal2 = inet::Coord::NIL;
            return false;
        }

        ray.d /= len;
        ray.mint = 0.0f;
        ray.maxt = len;

        if (scene_) {
            auto pi = scene_->ray_intersect_preliminary(ray);
            if (!pi.is_valid()) {
                intersection1 = intersection2 = normal1 = normal2 = inet::Coord::NIL;
                return false;
            }

            const auto si = pi.compute_surface_interaction(ray);
            intersection1 = inet::Coord(si.p.x(), si.p.y(), si.p.z());
            normal1 = inet::Coord(si.n.x(), si.n.y(), si.n.z());

            // Advance slightly beyond first hit to find an exit (if any)
            constexpr float eps = 1e-4f;
            Ray3f ray2 = ray;
            ray2.mint = si.t + eps;
            auto pi2 = scene_->ray_intersect_preliminary(ray2);

            if (pi2.is_valid()) {
                const auto si2 = pi2.compute_surface_interaction(ray2);
                intersection2 = inet::Coord(si2.p.x(), si2.p.y(), si2.p.z());
                normal2 = inet::Coord(si2.n.x(), si2.n.y(), si2.n.z());
            } else {
                intersection2 = intersection1;
                normal2 = normal1;
            }
            return true;
        }

        // No scene available: no intersection information
        intersection1 = intersection2 = normal1 = normal2 = inet::Coord::NIL;
        return false;
    }

private:
    inet::Coord size_;
    mitsuba::ref<Scene> scene_;
};

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
