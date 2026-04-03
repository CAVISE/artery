#include "Ground.h"

#include "cavise/sionna/bridge/Fwd.h"

using namespace artery::sionna;

Terrain::Terrain(const py::SionnaScene& scene)
    : scene_(scene) {
}

double Terrain::getElevation(const inet::Coord& position) const {
    mitsuba::Resolve::Ray3f norm;

    // Avoid implicit narrowing.
    if constexpr (std::is_same_v<mitsuba::Resolve::Float, float>) {
        auto x = static_cast<mitsuba::Resolve::Float>(position.x);
        auto y = static_cast<mitsuba::Resolve::Float>(position.y);
        norm.o = mitsuba::Resolve::Point3f(x, y, std::numeric_limits<float>::max());
    } else {
        norm.o = mitsuba::Resolve::Point3f(position.x, position.y, std::numeric_limits<float>::max());
    }

    norm.d = mitsuba::Resolve::Vector3f(0.0, 0.0, -1.0);

    auto scene = scene_.miScene();
    mitsuba::Resolve::SurfaceInteraction3f surface;
    if (surface = scene->ray_intersect(norm); !drjit::all_nested(surface.is_valid())) {
        return std::numeric_limits<double>::max();
    }

    return artery::sionna::toScalar(surface.p.z());
}
