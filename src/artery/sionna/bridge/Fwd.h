#pragma once

#include <mitsuba/core/fwd.h>
#include <mitsuba/render/fwd.h>
#include <mitsuba/core/platform.h>
#include <nanobind/nanobind.h>

#if defined(__GNUC__) && !defined(_WIN32)
#  define SIONNA_BRIDGE_API __attribute__((visibility("hidden")))
#else
#  define SIONNA_BRIDGE_API
#endif

/**
 * @file Forward declarations for Sionna bridge classes.
 * We follow Mitsuba library style, which forward declares all classes
 * and uses special struct for convenient aliasing in this library and its consumers.
 */

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

/**
 * The following template helpers make MI_INSTANTIATE_CLASS and MI_INSTANTIATE_STRUCT
 * portable by using namespace inclusion - not the best option, but the one that works.
 */

// Instance class template for all mitsuba variants configured for current
// distribution.
#define SIONNA_INSTANTIATE_CLASS(x)                         \
    using namespace mitsuba;                                \
    MI_INSTANTIATE_CLASS(x)

// Instance struct template for all mitsuba variants configured for current
// distribution.
#define SIONNA_INSTANTIATE_STRUCT(x)                        \
    using namespace mitsuba;                                \
    MI_INSTANTIATE_STRUCT(x)

// Defines external instances of this class template for all mitsuba variants configured for current
// distribution.
#define SIONNA_EXTERN_CLASS(x)                        \
    using namespace mitsuba;                          \
    MI_EXTERN_CLASS(x)

// Defines external instances of this struct template for all mitsuba variants configured for current
// distribution.
#define SIONNA_EXTERN_STRUCT(x)                        \
    using namespace mitsuba;                           \
    MI_EXTERN_STRUCT(x)

#define SIONNA_IMPORT_TYPE(alias, x) using x = typename alias::x;

#define SIONNA_IMPORT_CORE_TYPES_MACRO(x) SIONNA_IMPORT_TYPE(CoreAliases, x)
#define SIONNA_IMPORT_RENDER_TYPES_MACRO(x) SIONNA_IMPORT_TYPE(RenderAliases, x)
#define SIONNA_IMPORT_BRIDGE_TYPES_MACRO(x) using x = typename SionnaBridgeAliases::x##_;

#define SIONNA_IMPORT_RENDER_TYPES(...)                                 \
    using RenderAliases = mitsuba::RenderAliases<Float, Spectrum>;      \
    DRJIT_MAP(SIONNA_IMPORT_RENDER_TYPES_MACRO, __VA_ARGS__)

#define SIONNA_IMPORT_CORE_TYPES(...)                                   \
    using CoreAliases = mitsuba::CoreAliases<Float>;                    \
    DRJIT_MAP(SIONNA_IMPORT_CORE_TYPES_MACRO, __VA_ARGS__)

#define SIONNA_IMPORT_BRIDGE_TYPES(...)                                                 \
    using SionnaBridgeAliases = artery::sionna::SionnaBridgeAliases<Float, Spectrum>;   \
    DRJIT_MAP(SIONNA_IMPORT_BRIDGE_TYPES_MACRO, __VA_ARGS__)

/* ***********************
 * Forward declarations. *
 *************************
 */

MI_VARIANT class SionnaScene;
MI_VARIANT class RadioMaterialBase;
MI_VARIANT class RadioMaterial;
MI_VARIANT class ConstantsBase;
MI_VARIANT class Constants;
MI_VARIANT class SceneObject;
MI_VARIANT class Compat;

NAMESPACE_BEGIN(py)

MI_VARIANT class SionnaScene;
MI_VARIANT class RadioMaterialBase;
MI_VARIANT class RadioMaterial;
MI_VARIANT class ConstantsBase;
MI_VARIANT class Constants;
MI_VARIANT class SceneObject;

NAMESPACE_END(py)

MI_VARIANT struct SionnaBridgeAliases {
    using SionnaScene_ = py::SionnaScene<Float, Spectrum>;
    using RadioMaterial_ = py::RadioMaterial<Float, Spectrum>;
    using RadioMaterialBase_ = py::RadioMaterialBase<Float, Spectrum>;
    using Constants_ = py::Constants<Float, Spectrum>;
    using ConstantsBase_ = py::ConstantsBase<Float, Spectrum>;
    using SceneObject_ = py::SceneObject<Float, Spectrum>;
    using Compat_ = Compat<Float, Spectrum>;
    // using UnevenTerrain_ = py::UnevenTerrain<Float, Spectrum>;
};

template <typename T>
T access(const nanobind::object obj, const std::string& attribute, bool convert = true);

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
