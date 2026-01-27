#pragma once

#include <artery/sionna/bridge/Defaulted.h>
#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Helpers.h>
#include <artery/sionna/bridge/Capabilities.h>
#include <artery/sionna/bridge/bindings/Constants.h>
#include <artery/sionna/bridge/bindings/Modules.h>

#include <nanobind/nanobind.h>

#include <string>
#include <tuple>
#include <utility>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

NAMESPACE_BEGIN(py)

using literals::operator""_a;

MI_VARIANT
class RadioMaterialBase
    : public SionnaRtModuleBase
    , public CachedFetchCapability
    , public InitPythonClassCapability {
public:
    using ColorType = std::tuple<Float, Float, Float>;

    const char* className() const override {
        return "RadioMaterialBase";
    }

    std::string materialName() const {
        return sionna::access<std::string>(*bound_, "name");
    }

    ColorType color() const {
        return sionna::access<ColorType>(*bound_, "color");
    }

    void setColor(ColorType newColor) {
        sionna::set<ColorType>(*bound_, "color", std::move(newColor));
    }
};

MI_VARIANT
class RadioMaterial
    : public SionnaRtModuleBase
    , public CachedFetchCapability
    , public DefaultedClassProviderCapability
    , public WrapPythonClassCapability
    , public RadioMaterialBase<Float, Spectrum> {
public:
    SIONNA_IMPORT_CORE_TYPES(Float64)

    using Constants = Constants<Float, Spectrum>;

    const char* className() const override {
        return "RadioMaterial";
    }

    RadioMaterial() = default;

    RadioMaterial(nb::object obj) {
        WrapPythonClassCapability::init(std::move(obj));
    }

    RadioMaterial(
        const std::string& name,
        Float64 conductivity = 0.0,
        Float64 relativePermittivity = 1.0,
        typename Defaulted<Float64>::Argument thickness =
            Constants::constants().defaultThickness()) {
        InitPythonClassCapability::init(
            "name"_a = name,
            "thickness"_a = thickness,
            "relative_permittivity"_a = relativePermittivity,
            "conductivity"_a = conductivity);
    }

    Float64 relativePermittivity() const {
        return sionna::access<Float64>(*bound_, "relative_permittivity");
    }

    void setRelativePermittivity(Float64 relativePermittivity) {
        sionna::set(*bound_, "relative_permittivity", relativePermittivity);
    }

    Float64 conductivity() const {
        return sionna::access<Float64>(*bound_, "conductivity");
    }

    void setConductivity(Float64 conductivity) {
        sionna::set(*bound_, "conductivity", conductivity);
    }

    Float64 thickness() const {
        return sionna::access<Float64>(*bound_, "thickness");
    }

    void setThickness(Float64 thickness) {
        sionna::set(*bound_, "thickness", thickness);
    }
};

NAMESPACE_END(py)

NAMESPACE_END(sionna)
NAMESPACE_END(artery)

NAMESPACE_BEGIN(nanobind)
NAMESPACE_BEGIN(detail)

MI_VARIANT
struct sionna_wrap_caster_enabled<artery::sionna::py::RadioMaterial<Float, Spectrum>>
    : std::true_type {};

NAMESPACE_END(detail)
NAMESPACE_END(nanobind)
