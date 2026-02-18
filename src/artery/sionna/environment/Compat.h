#pragma once

#include <artery/sionna/bridge/Compat.h>
#include <inet/common/geometry/common/Coord.h>
#include <mitsuba/core/fwd.h>

namespace artery::sionna::py {

    template <typename Float, typename Spectrum, typename Value>
    struct CompatImpl<Float, Spectrum, inet::Coord, mitsuba::Point<Value, 3>, void> {
        static inet::Coord convert(const mitsuba::Point<Value, 3>& value) {
            return inet::Coord(
                Compat<Float, Spectrum>::toScalar(value[0]),
                Compat<Float, Spectrum>::toScalar(value[1]),
                Compat<Float, Spectrum>::toScalar(value[2])
            );
        }
    };

    template <typename Float, typename Spectrum, typename Value>
    struct CompatImpl<Float, Spectrum, inet::Coord, mitsuba::Vector<Value, 3>, void> {
        static inet::Coord convert(const mitsuba::Vector<Value, 3>& value) {
            return inet::Coord(
                Compat<Float, Spectrum>::toScalar(value[0]),
                Compat<Float, Spectrum>::toScalar(value[1]),
                Compat<Float, Spectrum>::toScalar(value[2])
            );
        }
    };

    template <typename Float, typename Spectrum, typename Value>
    struct CompatImpl<Float, Spectrum, inet::Coord, mitsuba::Normal<Value, 3>, void> {
        static inet::Coord convert(const mitsuba::Normal<Value, 3>& value) {
            return inet::Coord(
                Compat<Float, Spectrum>::toScalar(value[0]),
                Compat<Float, Spectrum>::toScalar(value[1]),
                Compat<Float, Spectrum>::toScalar(value[2])
            );
        }
    };

    template <typename Float, typename Spectrum, typename Value>
    struct CompatImpl<Float, Spectrum, inet::Coord, mitsuba::BoundingBox<mitsuba::Point<Value, 3>>, void> {
        static inet::Coord convert(const mitsuba::BoundingBox<mitsuba::Point<Value, 3>>& value) {
            return Compat<Float, Spectrum>::template convert<inet::Coord>(value.max - value.min);
        }
    };

}
