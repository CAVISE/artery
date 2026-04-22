#pragma once

#include <cavise/sionna/bridge/SionnaBridge.h>

#include <mitsuba/core/fwd.h>
#include <inet/common/geometry/common/Coord.h>
#include <inet/common/geometry/common/EulerAngles.h>
#include <traci/Angle.h>
#include <traci/Position.h>

#include <numbers>

namespace artery::sionna {

    struct TraCIVelocity {
        double speed;
        traci::TraCIAngle heading;
    };

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

    template <>
    struct impl<mitsuba::Resolve::Point3f, traci::TraCIPosition, void> {
        static mitsuba::Resolve::Point3f convert(const traci::TraCIPosition& value) {
            return mitsuba::Resolve::Point3f(
                fromScalar<mitsuba::Resolve::Float>(value.x),
                fromScalar<mitsuba::Resolve::Float>(value.y),
                fromScalar<mitsuba::Resolve::Float>(value.z));
        }
    };

    template <>
    struct impl<mitsuba::Resolve::Vector3f, traci::TraCIPosition, void> {
        static mitsuba::Resolve::Vector3f convert(const traci::TraCIPosition& value) {
            return mitsuba::Resolve::Vector3f(
                fromScalar<mitsuba::Resolve::Float>(value.x),
                fromScalar<mitsuba::Resolve::Float>(value.y),
                fromScalar<mitsuba::Resolve::Float>(value.z));
        }
    };

    template <>
    struct impl<mitsuba::Resolve::Point3f, traci::TraCIAngle, void> {
        static mitsuba::Resolve::Point3f convert(const traci::TraCIAngle& value) {
            const double yaw = (90.0 - value.degree) * std::numbers::pi_v<double> / 180.0;
            return mitsuba::Resolve::Point3f(
                fromScalar<mitsuba::Resolve::Float>(0.0),
                fromScalar<mitsuba::Resolve::Float>(0.0),
                fromScalar<mitsuba::Resolve::Float>(yaw));
        }
    };

    template <>
    struct impl<mitsuba::Resolve::Vector3f, TraCIVelocity, void> {
        static mitsuba::Resolve::Vector3f convert(const TraCIVelocity& value) {
            const double yaw = (90.0 - value.heading.degree) * std::numbers::pi_v<double> / 180.0;
            return mitsuba::Resolve::Vector3f(
                fromScalar<mitsuba::Resolve::Float>(value.speed * std::cos(yaw)),
                fromScalar<mitsuba::Resolve::Float>(value.speed * -std::sin(yaw)),
                fromScalar<mitsuba::Resolve::Float>(0.0));
        }
    };

} // namespace artery::sionna
