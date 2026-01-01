#pragma once

#include <artery/sionna/bridge/Helpers.h>
#include <artery/sionna/bridge/Bindings.h>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

NAMESPACE_BEGIN(py)

PY_IDENTITY_TAG(SionnaRt, sionna.rt);
PY_IDENTITY_TAG(RadioMaterial, RadioMaterial);
PY_IDENTITY_TAG(RadioMaterialBase, RadioMaterialBase);

MI_VARIANT PY_CLASS(RadioMaterialBase, RadioMaterialBaseTag, SionnaRtTag)
{
    // Color type used by material objects (RGB format).
    using ColorType = std::tuple<Float, Float, Float>;

    // Name for this material.
    std::string materialName() const
    {
        return sionna::access<std::string>(*bound_, "name");
    }

    // Color for this material (RGB tuple).
    ColorType color() const
    {
        return sionna::access<ColorType>(*bound_, "color");
    }

    // Set color on this material (RGB tuple).
    void setColor(ColorType newColor)
    {
        sionna::set<ColorType>(*bound_, "color", std::move(newColor));
    }
};

/* clang-format off */
MI_VARIANT PY_CLASS(RadioMaterial, RadioMaterialTag, SionnaRtTag, PY_BASE((RadioMaterialBase<Float, Spectrum>)))
{
    SIONNA_IMPORT_CORE_TYPES(Float64)

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
/* clang-format on */

NAMESPACE_END(py)

NAMESPACE_END(artery)
NAMESPACE_END(sionna)
