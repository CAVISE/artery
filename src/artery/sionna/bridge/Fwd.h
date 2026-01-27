#pragma once

#include <mitsuba/core/fwd.h>
#include <mitsuba/render/fwd.h>
#include <mitsuba/core/platform.h>
#include <nanobind/nanobind.h>

/**
 * @file Forward declarations for Sionna bridge classes.
 * We follow Mitsuba library style, which forward declares all classes
 * and uses special struct for convenient aliasing in this library and its consumers.
 */

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

#define SIONNA_IMPORT_TYPE(alias, x) using x = typename alias::x;

#define SIONNA_IMPORT_CORE_TYPES_MACRO(x) SIONNA_IMPORT_TYPE(CoreAliases, x)
#define SIONNA_IMPORT_RENDER_TYPES_MACRO(x) SIONNA_IMPORT_TYPE(RenderAliases, x)
#define SIONNA_IMPORT_BRIDGE_TYPES_MACRO(x) SIONNA_IMPORT_TYPE(SionnaBridgeAliases, x)

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

MI_VARIANT struct SionnaBridgeAliases {
    using SionnaBridge = SionnaScene<Float, Spectrum>;
    using RadioMaterial = RadioMaterial<Float, Spectrum>;
    using RadioMaterialBase = RadioMaterialBase<Float, Spectrum>;
    using Constants = Constants<Float, Spectrum>;
    using ConstantsBase = ConstantsBase<Float, Spectrum>;
    using SceneObject = SceneObject<Float, Spectrum>;
    // using UnevenTerrain = UnevenTerrain<Float, Spectrum>;
};

template <typename T>
T access(const nanobind::object obj, const std::string& attribute, bool convert = true);

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
