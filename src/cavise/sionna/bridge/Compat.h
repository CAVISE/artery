#pragma once

#include <mitsuba/core/bbox.h>
#include <mitsuba/core/vector.h>

#include <drjit/jit.h>
#include <drjit-core/traits.h>
#include <drjit/array_traits.h>

#include <cavise/sionna/bridge/Fwd.h>

#include <type_traits>

/**
 * @file Helpers for converting Mitsuba datatypes into various user-end frinedly
 * types.
 */

namespace artery::sionna {

    // Resolves to false boolean value for any template.
    template <typename...>
    constexpr static bool always_false_v = false;

    // Converter implementation.
    template <typename OutValue, typename InValue, typename STUBBER = void>
    struct impl;

    // Converter resolution - will resolve to failed cast if suitable caster is not visible.
    template <typename OutValue, typename InValue>
    static OutValue convert(InValue value) {
        return impl<OutValue, InValue>::convert(value);
    }

    template <typename OutValue, typename InValue, typename STUBBER = void>
    struct CompatImpl {
        static OutValue convert(InValue /* value */) {
            static_assert(
                always_false_v<OutValue, InValue>,
                "convert: no conversion available for requested input/output types");
        }
    };

    template <typename OutValue, typename InValue, typename STUBBER>
    struct impl : CompatImpl<OutValue, InValue, STUBBER> {};

    // Ensure passed value is scalar. If passing scalar type, it gets returned back to caller.
    // Also performs static cast if scalar types mismatch.
    template <typename ReturnT = double, typename ValueT>
    static ReturnT toScalar(const ValueT& value) {
        if constexpr (drjit::is_array_v<ValueT>) {
            return static_cast<ReturnT>(drjit::slice(drjit::detach(value), 0));
        } else {
            return static_cast<ReturnT>(value);
        }
    }

    // Ensure returned value is compatible with Float template parameter. Generally, you shall call just call Float(),
    // but the thing is - it may not always work, so I implemented this generic converter.
    template <typename Float, typename ScalarT>
    static Float fromScalar(const ScalarT& value) {
        if constexpr (std::is_floating_point_v<Float>) {
            if constexpr (std::is_same_v<Float, std::decay_t<ScalarT>>) {
                return value;
            }
            return static_cast<Float>(value);
        } else if constexpr (drjit::is_llvm_v<Float> || drjit::is_cuda_v<Float>) {
            return Float(value);
        }
    }

    // For JIT non-AD types, select their AD counterpart. Otherwise return T. Used when
    // underlying API tracks gradient and SB module wishes to maintain that.
    template <typename T>
    using maybe_diff_t =
        std::conditional_t<drjit::is_array_v<T> && !drjit::is_diff_v<T>,
                           drjit::diff_array_t<T>,
                           T>;

} // namespace artery::sionna
