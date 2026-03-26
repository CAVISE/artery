#pragma once

#include <cavise/sionna/bridge/SionnaBridge.h>

#include <mitsuba/core/fwd.h>
#include <inet/common/geometry/common/Coord.h>
#include <inet/common/geometry/common/EulerAngles.h>

namespace artery::sionna {

    template <typename Value>
    struct impl<inet::Coord, mitsuba::Point<Value, 3>, void> {
        static inet::Coord convert(const mitsuba::Point<Value, 3>& value) {
            return inet::Coord(
                toScalar(value[0]),
                toScalar(value[1]),
                toScalar(value[2]));
        }
    };

    template <typename Value>
    struct impl<inet::Coord, mitsuba::Vector<Value, 3>, void> {
        static inet::Coord convert(const mitsuba::Vector<Value, 3>& value) {
            return inet::Coord(
                toScalar(value[0]),
                toScalar(value[1]),
                toScalar(value[2]));
        }
    };

    template <typename Value>
    struct impl<inet::Coord, mitsuba::Normal<Value, 3>, void> {
        static inet::Coord convert(const mitsuba::Normal<Value, 3>& value) {
            return inet::Coord(
                toScalar(value[0]),
                toScalar(value[1]),
                toScalar(value[2]));
        }
    };

    template <typename Value>
    struct impl<inet::Coord, mitsuba::BoundingBox<mitsuba::Point<Value, 3>>, void> {
        static inet::Coord convert(const mitsuba::BoundingBox<mitsuba::Point<Value, 3>>& value) {
            return artery::sionna::convert<inet::Coord>(value.max - value.min);
        }
    };

    template <typename Value>
    struct impl<inet::EulerAngles, mitsuba::Point<Value, 3>, void> {
        static inet::EulerAngles convert(const mitsuba::Point<Value, 3>& value) {
            const inet::Coord coord = artery::sionna::convert<inet::Coord>(value);
            return inet::EulerAngles(coord.x, coord.y, coord.z);
        }
    };

} // namespace artery::sionna
