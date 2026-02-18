#pragma once

#include <mitsuba/core/fwd.h>
#include <mitsuba/render/fwd.h>
#include <mitsuba/core/platform.h>

#include <nanobind/nanobind.h>

// SB API partially uses nanobind objects. Because of this, where applicable,
// we should comply with nanobind symbols visibility.

#if defined(__GNUC__) && !defined(_WIN32)
#  define SIONNA_BRIDGE_API __attribute__((visibility("default")))
#  define SIONNA_BRIDGE_INTERNAL __attribute__((visibility("hidden")))
#else
#  define SIONNA_BRIDGE_API
#  define SIONNA_BRIDGE_INTERNAL
#endif

/**
 * @file Forward declarations for Sionna bridge classes.
 * We follow Mitsuba library style, which forward declares all classes
 * and uses special struct for convenient aliasing in this library and its consumers.
 */

 // Isolate and expand macro inside mitsuba namespace. Used if maintainers
 // did not use quilified names for identifiers.
 #define SIONNA_BRIDGE_MI_ENCLOSE(macro, ...) \
    using namespace mitsuba; \
    macro(__VA_ARGS__)

// Forward declare bridge class for usage with all variants. Pass
// fully quilified name (e.g. my_namespace::class).
#define SIONNA_BRIDGE_INSTANTIATE_CLASS(qualified_name) \
    SIONNA_BRIDGE_MI_ENCLOSE(MI_INSTANTIATE_CLASS, ::qualified_name)

// Same as  SIONNA_BRIDGE_INSTANTIATE_CLASS, but for structs.
#define SIONNA_BRIDGE_INSTANTIATE_STRUCT(qualified_name) \
    SIONNA_BRIDGE_MI_ENCLOSE(MI_INSTANTIATE_STRUCT, ::qualified_name)

// Defines external instances of this class template for all mitsuba variants configured for current
// distribution.
#define SIONNA_BRIDGE_EXTERN_CLASS(qualified_name) \
    SIONNA_BRIDGE_MI_ENCLOSE(MI_EXTERN_CLASS, ::qualified_name)

// Defines external instances of this struct template for all mitsuba variants configured for current
// distribution.
#define SIONNA_BRIDGE_EXTERN_STRUCT(qualified_name) \
    SIONNA_BRIDGE_MI_ENCLOSE(MI_EXTERN_STRUCT ::qualified_name)

namespace artery {

    namespace sionna {

        namespace dr = drjit;

        /* ***********************
        * Forward declarations. *
        *************************
        */

        template <typename T>
        T access(const nanobind::object& obj, const std::string& attribute, bool convert = false);

        namespace py SIONNA_BRIDGE_INTERNAL {

            MI_VARIANT class Compat;
            MI_VARIANT class Constants;
            MI_VARIANT class IntersectionTypes;
            MI_VARIANT class RadioMaterialBase;
            MI_VARIANT class RadioMaterial;
            MI_VARIANT class SionnaScene;
            MI_VARIANT class SceneObject;

        }

        MI_VARIANT struct SionnaBridgeAliases {
            using Compat = py::Compat<Float, Spectrum>;
            using Constants = py::Constants<Float, Spectrum>;
            using IntersectionTypes = py::IntersectionTypes<Float, Spectrum>;
            using RadioMaterialBase = py::RadioMaterialBase<Float, Spectrum>;
            using RadioMaterial = py::RadioMaterial<Float, Spectrum>;
            using SionnaScene = py::SionnaScene<Float, Spectrum>;
            using SceneObject = py::SceneObject<Float, Spectrum>;
        };

    }

}

#define SIONNA_BRIDGE_IMPORT_RENDER_TYPES() \
    MI_IMPORT_RENDER_BASIC_TYPES() \
    MI_IMPORT_OBJECT_TYPES()

#define SIONNA_BRIDGE_IMPORT_CORE_TYPES() \
    MI_IMPORT_CORE_TYPES()

// Import all available bridge types, adding mitsuba core and render types.
#define SIONNA_BRIDGE_IMPORT_BRIDGE_TYPES() \
    SIONNA_BRIDGE_IMPORT_RENDER_TYPES() \
    using SionnaBridgeAliases = artery::sionna::SionnaBridgeAliases<Float, Spectrum>; \
    using Compat = typename SionnaBridgeAliases::Compat; \
    using Constants = typename SionnaBridgeAliases::Constants; \
    using IntersectionTypes = typename SionnaBridgeAliases::IntersectionTypes; \
    using RadioMaterialBase = typename SionnaBridgeAliases::RadioMaterialBase; \
    using RadioMaterial = typename SionnaBridgeAliases::RadioMaterial; \
    using SionnaScene = typename SionnaBridgeAliases::SionnaScene; \
    using SceneObject = typename SionnaBridgeAliases::SceneObject;
