#include "Compat.h"

#include <drjit/array.h>
#include <drjit/array_traits.h>
#include <mitsuba/core/fwd.h>

#include <algorithm>
#include <cmath>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

SIONNA_INSTANTIATE_CLASS(Compat)

using Color = std::tuple<float, float, float>;

MI_VARIANT
template <typename STUBBER>
inet::Coord Compat<Float, Spectrum>::converter::impl<
    inet::Coord,
    typename mitsuba::CoreAliases<Float>::BoundingBox3f,
    STUBBER
>::convert(BoundingBox3f value) {
    if (!drjit::all_nested(value.valid())) {
        return inet::Coord::NIL;
    }

    return Compat<Float, Spectrum>::convert<inet::Coord>(value.extents());
}

MI_VARIANT
template <typename STUBBER>
inet::Coord Compat<Float, Spectrum>::converter::impl<
    inet::Coord,
    typename mitsuba::CoreAliases<Float>::Point3f,
    STUBBER
>::convert(Point3f value) {
    if (!drjit::all_nested(drjit::isfinite(value))) {
        return inet::Coord::NIL;
    }

    return inet::Coord(
        Compat<Float, Spectrum>::toScalar(value.x()),
        Compat<Float, Spectrum>::toScalar(value.y()),
        Compat<Float, Spectrum>::toScalar(value.z())
    );
}

template <typename Float, typename Spectrum>
template <typename STUBBER>
inet::EulerAngles Compat<Float, Spectrum>::converter::impl<
    inet::EulerAngles,
    typename mitsuba::CoreAliases<Float>::Vector3f,
    STUBBER
>::convert(Vector3f value) {
    return inet::EulerAngles(
        toScalar(value.x()),
        toScalar(value.y()),
        toScalar(value.z())
    );
}

template <typename Float, typename Spectrum>
template <typename STUBBER>
omnetpp::cFigure::Color Compat<Float, Spectrum>::converter::impl<
    omnetpp::cFigure::Color,
    Color,
    STUBBER
>::convert(Color value) {
    auto toChannel = [](float v) -> unsigned char {
        Float clamped = std::clamp(v, 0.0f, 1.0f);
        return static_cast<unsigned char>(std::lround(clamped * 255.0));
    };

    return omnetpp::cFigure::Color(
        toChannel(std::get<0>(value)),
        toChannel(std::get<1>(value)),
        toChannel(std::get<2>(value))
    );
}

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
