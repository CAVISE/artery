#pragma once

#include <type_traits>

#include <nanobind/nanobind.h>
#include <omnetpp/cexception.h>

namespace artery {

    namespace sionna {

        // Some of sionna's objects use non-standard integer, floating point etc. types.
        // Propagating them back to Artery may be done with this method - it also casts
        // property (attribute) value to a standard python type before returning it.
        template <typename T>
        T access(const nanobind::object* obj, const std::string& attribute, bool convert = true) {
            namespace nb = nanobind;

            try {
                return nb::cast<T>(obj->attr(attribute.c_str()), convert);
            } catch (const nb::python_error& error) {
                throw omnetpp::cRuntimeError(
                    "sionna: failed to convert property %s for object at %p",
                    attribute.c_str(), obj->ptr()
                );
            }
        }

        // Set an attribute on a Sionna Python object, with automatic type wrapping.
        template <typename T>
        void set(nanobind::object* obj, const std::string& attribute, T value) {
            namespace nb = nanobind;

            try {
                if constexpr (std::is_base_of_v<nb::handle, T>) {
                    nb::setattr(*obj, attribute.c_str(), value);
                } else {
                    nb::setattr(*obj, attribute.c_str(), nb::cast(std::move(value)));
                }
            } catch (const nb::python_error& error) {
                throw omnetpp::cRuntimeError(
                    "sionna: failed to set property %s of object at %p",
                    attribute.c_str(), obj->ptr()
                );
            }
        }

        // Access Sionna python module.
        inline nanobind::module_ sionna() {
            return nanobind::module_::import_("sionna");
        }

        // Access Sionna RT python submodule.
        inline nanobind::module_ sionnaRt() {
            return nanobind::module_::import_("sionna.rt");
        }

    }  // namespace sionna

}  // namespace artery