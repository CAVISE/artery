#pragma once

#include <omnetpp.h>

#include <mitsuba/core/bbox.h>
#include <mitsuba/core/vector.h>

#include <inet/common/geometry/common/Coord.h>
#include <inet/common/geometry/common/EulerAngles.h>

#include <artery/sionna/bridge/Fwd.h>

/**
 * @file Helpers for converting Mitsuba datatypes into INET and STL friendly
 * representations.
 */

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

/**
 * @brief Collection of conversions from Mitsuba types into INET-/STL-friendly
 * counterparts.
 */
MI_VARIANT
struct Compat {
    SIONNA_IMPORT_CORE_TYPES(BoundingBox3f, Vector3f, Point3f)

    struct converter {

        template <typename OutValue, typename InValue, typename STUBBER = void>
        struct impl {
            static OutValue convert(InValue value) = delete;
        };

        template <typename STUBBER>
        struct impl<inet::Coord, Point3f, STUBBER> {
            static inet::Coord convert(Point3f value);
        };

        template <typename STUBBER>
        struct impl<inet::EulerAngles, Vector3f, STUBBER> {
            static inet::EulerAngles convert(Vector3f value);
        };

        template <typename STUBBER>
        struct impl<omnetpp::cFigure::Color, std::tuple<float, float, float>, STUBBER> {
            static omnetpp::cFigure::Color convert(std::tuple<float, float, float> value);
        };

        template <typename STUBBER>
        struct impl<inet::Coord, BoundingBox3f, STUBBER> {
            static inet::Coord convert(BoundingBox3f value);
        };

    };

    /**
     * @brief Generic Mitsuba -> OMNeT++ conversion.
     */
    template <typename OutValue, typename InValue>
    static OutValue convert(InValue value) {
        return converter::template impl<OutValue, InValue>::convert(value);
    }

    /**
     * @brief Convert Dr.Jit value to a host double, detaching AD and taking lane 0 for JIT packets.
     */
    template <typename Value>
    static double toScalar(const Value& value) {
        if constexpr (drjit::is_diff_v<Value> || drjit::is_jit_v<Value>) {
            return static_cast<double>(drjit::slice(drjit::detach(value), 0));
        } else {
            // Some Value types provide quiet_NaN only as unqualified (ADL), avoid qualified lookup issues.
            return static_cast<double>(value);
        }
    }

};

SIONNA_EXTERN_CLASS(Compat)

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
