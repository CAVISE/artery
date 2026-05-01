#include "Ground.h"

#include "cavise/sionna/bridge/Fwd.h"
#include <mitsuba/core/spectrum.h>

using namespace artery::sionna;

Terrain::Terrain(const py::SionnaScene& scene)
    : scene_(scene) {
}

double Terrain::getElevation(const inet::Coord& position) const {
    using Ray = mitsuba::Resolve::Ray3f;
    typename Ray::Wavelength wavelengths = drjit::zeros<typename Ray::Wavelength>();
    if constexpr (mitsuba::is_spectral_v<mitsuba::Resolve::Spectrum>) {
        wavelengths = mitsuba::sample_wavelength<typename Ray::Float, mitsuba::Resolve::Spectrum>(typename Ray::Float(0.5f)).first;
    }

    // Avoid implicit narrowing.
    mitsuba::Resolve::Point3f origin;
    if constexpr (std::is_same_v<mitsuba::Resolve::Float, float>) {
        auto x = static_cast<mitsuba::Resolve::Float>(position.x);
        auto y = static_cast<mitsuba::Resolve::Float>(position.y);
        origin = mitsuba::Resolve::Point3f(x, y, std::numeric_limits<float>::max());
    } else {
        origin = mitsuba::Resolve::Point3f(position.x, position.y, std::numeric_limits<float>::max());
    }

    Ray norm(origin, mitsuba::Resolve::Vector3f(0.0, 0.0, -1.0), typename Ray::Float(0.f), wavelengths);

    auto scene = scene_.miScene();
    mitsuba::Resolve::SurfaceInteraction3f surface;
    if (surface = scene->ray_intersect(norm); !drjit::all_nested(surface.is_valid())) {
        return std::numeric_limits<double>::max();
    }

    return artery::sionna::toScalar(surface.p.z());
}
