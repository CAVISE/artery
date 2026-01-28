#include "MitsubaShape.h"

#include <drjit/array.h>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

SIONNA_INSTANTIATE_CLASS(MitsubaShape)

MI_VARIANT
MitsubaShape<Float, Spectrum>::MitsubaShape(const BoundingBox3f& bbox, mitsuba::ref<Scene> scene)
    : size_(compat::Compat<Float, Spectrum>::bboxExtentsToCoord(bbox))
    , scene_(std::move(scene)) {}

MI_VARIANT
inet::Coord MitsubaShape<Float, Spectrum>::computeBoundingBoxSize() const {
    return size_;
}

MI_VARIANT
bool MitsubaShape<Float, Spectrum>::computeIntersection(
    const inet::LineSegment& lineSegment,
    inet::Coord& intersection1,
    inet::Coord& intersection2,
    inet::Coord& normal1,
    inet::Coord& normal2
) const {
    Ray3f ray;
    const auto& p1 = lineSegment.getPoint1();
    const auto& p2 = lineSegment.getPoint2();

    ray.o = Point3f(p1.x, p1.y, p1.z);
    ray.d = Vector3f(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);

    const auto len = drjit::norm(ray.d);
    const double lenScalar = compat::toScalar(len);
    if (lenScalar <= 0.0) {
        intersection1 = intersection2 = normal1 = normal2 = inet::Coord::NIL;
        return false;
    }

    ray.d /= len;
    ray.maxt = static_cast<Float>(lenScalar);

    if (scene_) {
        auto pi = scene_->ray_intersect_preliminary(ray);
        if (!drjit::all_nested(pi.is_valid())) {
            intersection1 = intersection2 = normal1 = normal2 = inet::Coord::NIL;
            return false;
        }

        const auto si = pi.compute_surface_interaction(ray);
        intersection1 = inet::Coord(
            compat::toScalar(si.p.x()),
            compat::toScalar(si.p.y()),
            compat::toScalar(si.p.z()));
        normal1 = inet::Coord(
            compat::toScalar(si.n.x()),
            compat::toScalar(si.n.y()),
            compat::toScalar(si.n.z()));

        // Advance slightly beyond first hit to find an exit (if any)
        constexpr float eps = 1e-4f;
        Ray3f ray2 = ray;
        ray2.o = si.p + ray.d * eps;

        // remaining distance along the original segment after first hit
        ray2.maxt = static_cast<Float>(lenScalar - compat::toScalar(si.t) - eps);
        auto pi2 = scene_->ray_intersect_preliminary(ray2);

        if (drjit::all_nested(pi2.is_valid())) {
            const auto si2 = pi2.compute_surface_interaction(ray2);
            intersection2 = inet::Coord(
                compat::toScalar(si2.p.x()),
                compat::toScalar(si2.p.y()),
                compat::toScalar(si2.p.z()));
            normal2 = inet::Coord(
                compat::toScalar(si2.n.x()),
                compat::toScalar(si2.n.y()),
                compat::toScalar(si2.n.z()));
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

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
