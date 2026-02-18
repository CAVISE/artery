#include <drjit/array.h>

#include "artery/sionna/bridge/Fwd.h"
#include "inet/common/geometry/common/Coord.h"

#include "Compat.h"
#include "MitsubaShape.h"

using namespace artery::sionna;

SIONNA_BRIDGE_INSTANTIATE_CLASS(artery::sionna::MitsubaShape)

namespace {

    constexpr float advancementEpsilon = 1e-4F;

    void nullIntersection(inet::Coord& intersection, inet::Coord& normal) {
        intersection = normal = inet::Coord::NIL;
    }

    void nullPassthrough(inet::Coord& outerIntersecion, inet::Coord& innerIntersection, inet::Coord& outerNormal, inet::Coord& innerNormal) {
        nullIntersection(outerIntersecion, outerNormal);
        nullIntersection(innerIntersection, innerNormal);
    }

}

MI_VARIANT
MitsubaShape<Float, Spectrum>::MitsubaShape(mitsuba::ref<Mesh> mesh)
    : size_(Compat::template convert<inet::Coord>(mesh->bbox()))
    , mesh_(mesh)
{}

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
    const inet::Coord& p2 = lineSegment.getPoint2();
    const inet::Coord& p1 = lineSegment.getPoint1();

    Ray3f ray;
    ray.o = Point3f(p1.x, p1.y, p1.z);
    ray.d = Vector3f(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);

    const auto len = drjit::norm(ray.d);
    const double lenScalar = Compat::toScalar(len);

    nullPassthrough(intersection1, intersection2, normal1, normal2);
    if (lenScalar <= 0.0) {
        return false;
    }

    ray.d /= len;
    ray.maxt = Compat::fromScalar(lenScalar);

    typename RenderAliases::PreliminaryIntersection3f pi = mesh_->ray_intersect_preliminary(ray);
    if (!drjit::all_nested(pi.is_valid())) {
        return false;
    }

    const auto si = pi.compute_surface_interaction(ray);
    intersection1 = Compat::template convert<inet::Coord>(si.p);
    normal1 = Compat::template convert<inet::Coord>(si.n);

    Ray3f ray2 = ray;
    ray2.o = si.p + ray.d * advancementEpsilon;

    ray2.maxt = static_cast<Float>(lenScalar - Compat::toScalar(si.t) - advancementEpsilon);
    auto pi2 = mesh_->ray_intersect_preliminary(ray2);

    if (drjit::all_nested(pi2.is_valid())) {
        const auto si2 = pi2.compute_surface_interaction(ray2);
        intersection2 = Compat::template convert<inet::Coord>(si2.p);
        normal2 = Compat::template convert<inet::Coord>(si2.n);
    } else {
        intersection2 = intersection1;
        normal2 = normal1;
    }
    return true;

}
