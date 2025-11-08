#pragma once

#include <omnetpp/cexception.h>
#include <pybind11/cast.h>
#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>

#include <type_traits>

namespace artery {

    namespace sionna {

        // Some of sionna's objects use non-standard integer, floating point etc. types.
        // Propagating them back to Artery may be done with this method - it also casts
        // property (attribute) value to a standard python type before returning it.
        template <typename T>
        T access(const pybind11::object* obj, const std::string attribute) {
            pybind11::object property = obj->attr(attribute.c_str());

            try {
                if constexpr (std::is_integral_v<T>) {
                    property = pybind11::int_(property);
                } else if constexpr (std::is_floating_point_v<T>) {
                    property = pybind11::float_(property);
                } else if constexpr (std::is_same_v<T, std::string>) {
                    property = pybind11::str(property);
                }
            } catch (const pybind11::error_already_set& error) {
                throw omnetpp::cRuntimeError("sionna: failed to convert property %s for object at %d", attribute, obj->ptr());
            }

            return property.cast<T>();
        }

        template <typename T>
        void set(pybind11::object* obj, const std::string& attribute, T value) {
            try {
                if constexpr (std::is_base_of<pybind11::handle, T>::value) {
                    pybind11::setattr(*obj, attribute.c_str(), value);    
                } else {
                    pybind11::setattr(*obj, attribute.c_str(), pybind11::cast(std::move(value)));
                }
            } catch (const pybind11::error_already_set& error) {
                throw omnetpp::cRuntimeError("sionna: failed set property %s of object at %d", attribute, obj->ptr());
            }
        }

        // Access sionna python module.
        inline pybind11::module_ sionna() {
            return pybind11::module_::import("sionna");
        }

        // Access sionna rt module.
        inline pybind11::module_ sionnaRt() {
            return pybind11::module_::import("sionna.rt");
        }

    }  // namespace sionna

}  // namespace artery