#pragma once

#include <nanobind/nanobind.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/string.h>

#include <cavise/sionna/bridge/Fwd.h>
#include <cavise/sionna/bridge/Capabilities.h>

namespace nanobind::detail {

    /**
     * @brief Generic caster for wrappers opted-in via nb_wrap_caster_enabled trait.
     */
    template <typename T>
    struct type_caster<T, artery::sionna::py::enable_if_wrap_caster<T>> {
        NB_TYPE_CASTER(T, const_name<T>())

        bool from_python(handle src, uint8_t, cleanup_list*) noexcept {
            value = Value();
            value.init(borrow<object>(src));
            return true;
        }

        static handle from_cpp(const Value& src, rv_policy /* policy */, cleanup_list* /* cleanup */) noexcept {
            return src.bound_.inc_ref();
        }
    };

} // namespace nanobind::detail
