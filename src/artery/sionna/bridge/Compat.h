#pragma once

#include <mitsuba/core/bbox.h>
#include <mitsuba/core/vector.h>

#include <drjit-core/traits.h>
#include <drjit/array_traits.h>
#include <drjit/jit.h>

#include <artery/sionna/bridge/Fwd.h>

#include <type_traits>

/**
 * @file Helpers for converting Mitsuba datatypes into INET and STL friendly
 * representations.
 */

namespace artery {

    namespace sionna {

        namespace py {

            /**
            * @brief Collection of conversions from Mitsuba types into INET-/STL-friendly
            * counterparts.
            */
            MI_VARIANT
            struct Compat {
                SIONNA_BRIDGE_IMPORT_CORE_TYPES()

                template <typename...>
                constexpr static bool always_false_v = false;

                template <typename OutValue, typename InValue, typename STUBBER = void>
                struct impl;

                template <typename OutValue, typename InValue>
                static OutValue convert(InValue value) {
                    return impl<OutValue, InValue>::convert(value);
                }

                template <typename T>
                struct is_mitsuba_variant : std::bool_constant<std::is_floating_point_v<T> || drjit::is_llvm_v<T> || drjit::is_cuda_v<T>> {};

                template <typename T>
                constexpr static bool is_mitsuba_variant_v = is_mitsuba_variant<T>::value;

                /**
                * @brief Convert Dr.Jit value to a host double, detaching AD and taking lane 0 for JIT packets.
                */
                template <typename ReturnT = double, typename ValueT>
                static ReturnT toScalar(const ValueT& value)
                {
                    if constexpr (drjit::is_array_v<ValueT>) {
                        return static_cast<ReturnT>(drjit::slice(drjit::detach(value), 0));
                    } else {
                        return static_cast<ReturnT>(value);
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

            template <typename Float, typename Spectrum, typename OutValue, typename InValue, typename STUBBER = void>
            struct CompatImpl {
                static OutValue convert(InValue) {
                    static_assert(
                        Compat<Float, Spectrum>::template always_false_v<OutValue, InValue>,
                        "Compat::convert: no conversion available for requested input/output types"
                    );
                }
            };

            template <typename Float, typename Spectrum>
            template <typename OutValue, typename InValue, typename STUBBER>
            struct Compat<Float, Spectrum>::impl : CompatImpl<Float, Spectrum, OutValue, InValue, STUBBER> {};

        }
    }
}

SIONNA_BRIDGE_EXTERN_CLASS(artery::sionna::py::Compat)
