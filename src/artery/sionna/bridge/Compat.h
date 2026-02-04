#pragma once

#include <mitsuba/core/bbox.h>
#include <mitsuba/core/vector.h>
#include <inet/common/geometry/common/Coord.h>
#include <inet/common/geometry/common/EulerAngles.h>

#include <artery/sionna/bridge/Fwd.h>

#include <drjit-core/traits.h>
#include <drjit/array_traits.h>
#include <drjit/jit.h>

#include <omnetpp.h>

#include <type_traits>

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
    SIONNA_IMPORT_CORE_TYPES(BoundingBox3f, Vector3f, Point3f, Normal3f)

    template <typename...>
    inline constexpr static bool always_false_v = false;

    struct converter {

        template <typename OutValue, typename InValue, typename STUBBER = void>
        struct impl {
            static OutValue convert(InValue) {
                static_assert(
                    always_false_v<OutValue, InValue>,
                    "Compat::convert: no conversion available for requested input/output types"
                );
            }
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
        struct impl<inet::Coord, Vector3f, STUBBER> {
            static inet::Coord convert(Vector3f value);
        };

        template <typename STUBBER>
        struct impl<inet::Coord, Normal3f, STUBBER> {
            static inet::Coord convert(Normal3f value);
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

    template <typename OutValue, typename InValue>
    static OutValue convert(InValue value) {
        return converter::template impl<OutValue, InValue>::convert(value);
    }

    static inet::Coord toCoord(Point3f value)           { return convert<inet::Coord>(value); }
    static inet::Coord toCoord(Vector3f value)          { return convert<inet::Coord>(value); }
    static inet::Coord toCoord(Normal3f value)          { return convert<inet::Coord>(value); }
    static inet::Coord toCoord(BoundingBox3f value)     { return convert<inet::Coord>(value); }
    static inet::EulerAngles toEuler(Vector3f value)    { return convert<inet::EulerAngles>(value); }

    static omnetpp::cFigure::Color toColor(std::tuple<float, float, float> value) {
        return convert<omnetpp::cFigure::Color>(value);
    }

    template <typename T = Float, typename = std::enable_if_t<!std::is_same_v<T, float>>>
    static omnetpp::cFigure::Color toColor(std::tuple<Float, Float, Float> value) {
        auto toChannel = [](Float v) -> unsigned char {
            const double scalar = Compat::toScalar<double>(v);
            const double clamped = std::clamp(scalar, 0.0, 1.0);
            return static_cast<unsigned char>(std::lround(clamped * 255.0));
        };
        return omnetpp::cFigure::Color(
            toChannel(std::get<0>(value)),
            toChannel(std::get<1>(value)),
            toChannel(std::get<2>(value))
        );
    }

    template <typename T>
    struct is_mitsuba_variant : std::bool_constant<std::is_floating_point_v<T> || drjit::is_llvm_v<T> || drjit::is_cuda_v<T>> {
    };

    template <typename T>
    constexpr static bool is_mitsuba_variant_v = is_mitsuba_variant<T>::value;

    /**
     * @brief Convert Dr.Jit value to a host double, detaching AD and taking lane 0 for JIT packets.
     */
    template <typename ReturnT = double, typename ValueT>
    static ReturnT toScalar(const ValueT& value)
    {
        static_assert(is_mitsuba_variant_v<Float>, "Float is not valid mitsuba variant");

        if constexpr (std::is_floating_point_v<Float>) {
            if constexpr (std::is_same_v<Float, std::decay_t<ValueT>>) {
                return value;
            }
            return static_cast<Float>(value);
        } else if constexpr (drjit::is_array_v<Float>) {
            return static_cast<ReturnT>(drjit::slice(drjit::detach(value), 0));
        }
    }

    template <typename ScalarT>
    static Float fromScalar(const ScalarT& value)
    {
        static_assert(is_mitsuba_variant_v<Float>, "Float is not valid mitsuba variant");

        if constexpr (std::is_floating_point_v<Float>) {
            if constexpr (std::is_same_v<Float, std::decay_t<ScalarT>>) {
                return value;
            }
            return static_cast<Float>(value);
        } else if constexpr (drjit::is_llvm_v<Float> || drjit::is_cuda_v<Float>) {
            return Float(value);
        }
    }
};

SIONNA_EXTERN_CLASS(Compat)

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
