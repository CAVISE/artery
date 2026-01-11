#pragma once

#include <artery/sionna/bridge/Bindings.h>
#include <artery/sionna/bridge/Helpers.h>
#include <artery/sionna/bridge/bindings/Constants.h>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

NAMESPACE_BEGIN(py)

PY_IDENTITY_TAG(SionnaRt, sionna.rt);
PY_IDENTITY_TAG(RadioMaterial, RadioMaterial);
PY_IDENTITY_TAG(RadioMaterialBase, RadioMaterialBase);

MI_VARIANT
class RadioMaterialBase : public Class<SionnaRtTag, RadioMaterialBaseTag>
{
    // Color type used by material objects (RGB format).
    using ColorType = std::tuple<Float, Float, Float>;

    // Name for this material.
    std::string materialName() const { return sionna::access<std::string>(*bound_, "name"); }

    // Color for this material (RGB tuple).
    ColorType color() const { return sionna::access<ColorType>(*bound_, "color"); }

    // Set color on this material (RGB tuple).
    void setColor(ColorType newColor) { sionna::set<ColorType>(*bound_, "color", std::move(newColor)); }
};

MI_VARIANT
class RadioMaterial : public Class<SionnaRtTag, RadioMaterialTag>
{
    SIONNA_IMPORT_CORE_TYPES(Float64)

    using Constants = py::Constants<Float, Spectrum>;

    RadioMaterial(
        const std::string& name, Float64 conductivity = 0.0, Float64 relativePermittivity = 1.0,
        typename Defaulted<Float64>::Argument thickness = Constants::DEFAULT_THICKNESS
    ) : Class{ctor(
        "name"_a = name,
        "thickness"_a = thickness,
        "relative_permittivity"_a = relativePermittivity,
        "conductivity"_a = conductivity
    )} {}

    // Relative permittivity for material.
    Float64 relativePermittivity() const
    {
        return sionna::access<Float64>(*bound_, "relative_permittivity");
    }

    // Sets relative permittivity for material.
    void setRelativePermittivity(Float64 relativePermittivity)
    {
        sionna::set(*bound_, "relative_permittivity", relativePermittivity);
    }

    // Conductivity for material.
    Float64 conductivity() const
    {
        return sionna::access<Float64>(*bound_, "conductivity");
    }

    // Sets conductivity for material.
    void setConductivity(Float64 conductivity)
    {
        sionna::set(*bound_, "conductivity", conductivity);
    }

    // Thickness for material.
    Float64 thickness() const
    {
        return sionna::access<Float64>(*bound_, "thickness");
    }
    
    // Sets thickness for material.
    void setThickness(Float64 thickness)
    {
        sionna::set(*bound_, "thickness", thickness);
    }
};

NAMESPACE_END(py)

NAMESPACE_END(artery)
NAMESPACE_END(sionna)
