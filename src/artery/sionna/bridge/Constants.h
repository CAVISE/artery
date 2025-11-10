#pragma once

#include <variant>

#include <mitsuba/core/fwd.h>
#include <nanobind/nanobind.h>

#include "Helpers.h"


namespace artery {

    namespace sionna {

        class BaseConstant {
        public:
            // Initialize constant with specified name, constant will be taken from module, from class if provided.
            BaseConstant(const std::string& module, const std::string& name, const std::string& cls = "")
                : module_(module)
                , name_(name)
                , cls_(cls)
            {}

            // Access name for this constant.
            const std::string& name() const {
                return name_;
            }

            // Access module name, where this constant is defined.
            const std::string& module() const {
                return module_;
            }

        protected:
            // Access module, optionally fetching class.
            nanobind::object source() {
                namespace nb = nanobind;

                nb::object module = nb::module_::import_(module_.c_str());
                if (cls_ != "") {
                    return module;
                }
                return nb::getattr(module, cls_.c_str());
            }

        protected:
            std::string cls_;
            std::string name_;
            std::string module_;
        };

        template<typename T, bool convert = true>
        class SimpleConstant
            : public BaseConstant {
        public:
            // Alias for convenient usage within arguments.
            using ActualOrConstant = std::variant<T, SimpleConstant<T, convert>>;

            // Resolve value: if user provided their own - use it, otherwise pull from Sionna presets.
            static T resolve(ActualOrConstant variadic) {
                struct Visitor {
                    void operator()(T value) const { return value; }
                    void operator()(SimpleConstant<T, convert> constant) const { return constant.value(); }
                };

                return std::visit(Visitor(), variadic);
            }

            SimpleConstant(const std::string& module, const std::string& name, const std::string& cls = "");

            // Access constant, which may be casted simply.
            T value() {
                return access<T>(source(), name_, convert);
            }

        };

        namespace constants {

            // Module, that defines constants.
            static constexpr const char* constantsModule = "sionna.rt.constants";

            // Intersection types, defined in
            // https://github.com/NVlabs/sionna-rt/blob/7c750b33f85df44a42bf0acd655087608e0d6095/src/sionna/rt/constants.py#L12C7-L12C22
            namespace intersection_type {

                // IntersectionType class with constants.
                static constexpr const char* interactionTypeClass = "InteractionType";

                // No intersection.
                static auto NONE = SimpleConstant<int, false>(constantsModule, "NONE", interactionTypeClass);

                // Specular reflection.
                static auto SPECULAR = SimpleConstant<int, false>(constantsModule, "SPECULAR", interactionTypeClass);

                // Diffuse reflection
                static auto DIFFUSE = SimpleConstant<int, false>(constantsModule, "DIFFUSE", interactionTypeClass);

                // Refraction.
                static auto REFRACTION = SimpleConstant<int, false>(constantsModule, "REFRACTION", interactionTypeClass);

                // Diffraction.
                static auto DIFFRACTION = SimpleConstant<int, false>(constantsModule, "DIFFRACTION", interactionTypeClass);
            }

            // Constant representing the default thickness of radio materials (in meters).
            static auto DEFAULT_THICKNESS = SimpleConstant<double, false>(constantsModule, "DEFAULT_THICKNESS");

        }


    }

}