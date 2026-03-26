#include <drjit/array.h>

#include <cavise/sionna/environment/Compat.h>
#include <inet/common/geometry/common/Coord.h>

#include "MitsubaShape.h"

using namespace artery::sionna;

namespace {

    constexpr float advancementEpsilon = 1e-4F;

    void nullIntersection(inet::Coord& intersection, inet::Coord& normal) {
        intersection = normal = inet::Coord::NIL;
    }

    void nullPassthrough(inet::Coord& outerIntersecion, inet::Coord& innerIntersection, inet::Coord& outerNormal, inet::Coord& innerNormal) {
        nullIntersection(outerIntersecion, outerNormal);
        nullIntersection(innerIntersection, innerNormal);
    }

} // namespace

MitsubaShape::MitsubaShape(mitsuba::ref<mitsuba::Resolve::Mesh> mesh)
    : size_(convert<inet::Coord>(mesh->bbox()))
    , mesh_(mesh) {
}

inet::Coord MitsubaShape::computeBoundingBoxSize() const {
    return size_;
}

bool MitsubaShape::computeIntersection(
    const inet::LineSegment& lineSegment,
    inet::Coord& intersection1,
    inet::Coord& intersection2,
    inet::Coord& normal1,
    inet::Coord& normal2) const {
    const inet::Coord& p2 = lineSegment.getPoint2();
    const inet::Coord& p1 = lineSegment.getPoint1();

    mitsuba::Resolve::Ray3f ray;
    ray.o = mitsuba::Resolve::Point3f(p1.x, p1.y, p1.z);
    ray.d = mitsuba::Resolve::Vector3f(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);

    const auto len = drjit::norm(ray.d);
    const double lenScalar = toScalar(len);

    nullPassthrough(intersection1, intersection2, normal1, normal2);
    if (lenScalar <= 0.0) {
        return false;
    }

    ray.d /= len;
    ray.maxt = fromScalar<mitsuba::Resolve::Float>(lenScalar);

    mitsuba::Resolve::PreliminaryIntersection3f pi = mesh_->ray_intersect_preliminary(ray);
    if (!drjit::all_nested(pi.is_valid())) {
        return false;
    }

    const auto si = pi.compute_surface_interaction(ray);
    intersection1 = convert<inet::Coord>(si.p);
    normal1 = convert<inet::Coord>(si.n);

    mitsuba::Resolve::Ray3f ray2 = ray;
    ray2.o = si.p + ray.d * advancementEpsilon;

    ray2.maxt = static_cast<mitsuba::Resolve::Float>(lenScalar - toScalar(si.t) - advancementEpsilon);
    auto pi2 = mesh_->ray_intersect_preliminary(ray2);

    if (drjit::all_nested(pi2.is_valid())) {
        const auto si2 = pi2.compute_surface_interaction(ray2);
        intersection2 = convert<inet::Coord>(si2.p);
        normal2 = convert<inet::Coord>(si2.n);
    } else {
        intersection2 = intersection1;
        normal2 = normal1;
    }
    return true;
}
