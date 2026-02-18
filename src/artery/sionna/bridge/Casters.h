#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

#include <artery/sionna/bridge/Fwd.h>

#include <unordered_map>


namespace nanobind {

    namespace detail {

        template <typename T>
        struct sionna_wrap_caster_enabled : std::false_type {};

        template <typename Float, typename Spectrum>
        struct sionna_wrap_caster_enabled<artery::sionna::py::SceneObject<Float, Spectrum>>
            : std::true_type {};

        template <typename Float, typename Spectrum>
        struct sionna_wrap_caster_enabled<artery::sionna::py::RadioMaterial<Float, Spectrum>>
            : std::true_type {};

        template <typename T>
        using enable_if_wrap_caster = std::enable_if_t<sionna_wrap_caster_enabled<T>::value &&
                                                    std::is_default_constructible_v<T>, int>;

        /**
        * @brief Generic caster for wrappers opted-in via nb_wrap_caster_enabled trait.
        */
        template <typename T>
        struct type_caster<T, enable_if_wrap_caster<T>> {
            NB_TYPE_CASTER(T, const_name<T>())

            bool from_python(handle src, uint8_t, cleanup_list *) noexcept {
                value = Value();
                value.init(borrow<object>(src));
                return true;
            }

            static handle from_cpp(const Value& src, rv_policy, cleanup_list *) noexcept {
                return src.bound_.inc_ref();
            }

            static handle from_cpp(Value&& src, rv_policy, cleanup_list *) noexcept {
                return src.bound_.release();
            }
        };

        template <typename T>
        struct type_caster<std::unordered_map<std::string, T>, enable_if_wrap_caster<T>> {
            using ObjectsDict = std::unordered_map<std::string, T>;
            NB_TYPE_CASTER(ObjectsDict, const_name<ObjectsDict>())

            bool from_python(handle src, uint8_t flags, cleanup_list *cleanup) noexcept {
                make_caster<dict> dict_caster;
                if (!dict_caster.from_python(src, flags_for_local_caster<dict>(flags), cleanup)) {
                    return false;
                }

                dict d = cast_t<dict>(dict_caster);
                for (auto item : d) {
                    make_caster<std::string> key_caster;
                    if (!key_caster.from_python(item.first, flags_for_local_caster<std::string>(flags), cleanup)) {
                        return false;
                    }

                    make_caster<T> value_caster;
                    if (!value_caster.from_python(item.second, flags_for_local_caster<T>(flags), cleanup)) {
                        return false;
                    }
                    value.emplace(cast_t<std::string>(key_caster), cast_t<T>(value_caster));
                }

                return true;
            }

            static handle from_cpp(const Value& src, rv_policy policy, cleanup_list *cleanup) noexcept {
                dict d;
                for (const auto& [k, v] : src) {
                    d[k.c_str()] = make_caster<T>::from_cpp(v, policy, cleanup);
                }
                return d.release();
            }
        };

    }
}
