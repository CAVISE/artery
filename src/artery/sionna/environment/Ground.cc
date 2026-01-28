#include "Ground.h"

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

SIONNA_INSTANTIATE_CLASS(UnevenTerrain)

MI_VARIANT
UnevenTerrain<Float, Spectrum>::UnevenTerrain(std::shared_ptr<SionnaScene> scene)
    : scene_(std::move(scene)) {}

MI_VARIANT
double UnevenTerrain<Float, Spectrum>::getElevation(const inet::Coord& position) const {
    mitsuba::Ray<Point3f, Spectrum> norm;
    norm.o = Point3f(position.x, position.y, std::numeric_limits<float>::max());
    norm.d = Vector3f(0, 0, -1);

    Scene& scene = scene_.scene();

    if (SurfaceInteraction3f surface = scene.ray_intersect(norm); surface.is_valid()) {
        return std::numeric_limits<double>::max();
    } else {
        return Compat::toScalar(surface.p.z());
    }
}

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
