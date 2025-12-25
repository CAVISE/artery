#pragma once

#include "artery/sionna/bridge/Layout.h"

#include <artery/sionna/bridge/Defaulted.h>
#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Material.h>
#include <inet/environment/contract/IMaterial.h>
#include <nanobind/nanobind.h>

#include <string>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

/**
 * @brief Maps RadioMaterialBase, serves to ease later implementations of other materials.
 * This class is abstract - sionna does not allow creation of instances.
 */
MI_VARIANT class RadioMaterialBase : public nanobind::object
{
public:
    SIONNA_IMPORT_CORE_TYPES(Vector3d)

    /**
     * @brief Access outer color of the material.
     */
    Vector3d color() const;

    /**
     * @brief Access name, given to the material.
     */
    std::string name() const;

    /**
     * @brief Set outer color of the material.
     */
    void setColor(Vector3d color);
};

MI_VARIANT class RadioMaterial : public inet::physicalenvironment::IMaterial, public RadioMaterialBase<Float, Spectrum>
{
public:
    SIONNA_IMPORT_CORE_TYPES(Float64)

    using Constants = Constants<Float, Spectrum>;

    /**
     * @brief Sionna radio material constructor. Current implementation assumes non-magnetic materials, so relative permeability that
     * is different to 1 will raise error.
     */
    RadioMaterial(
        const std::string& name, Float conductivity = 0.0, Float relativePermittivity = 1.0, Float relativePermeability = 1.0,
        typename Defaulted<Float64>::Argument thickness = Constants::DEFAULT_THICKNESS);

    // inet::physicalenvironment::IMaterial implementation.
    inet::physicalenvironment::Ohmm getResistivity() const override;
    double getRelativePermittivity() const override;
    double getRelativePermeability() const override;

    double getDielectricLossTangent(inet::physicalenvironment::Hz frequency) const override;
    double getRefractiveIndex() const override;
    inet::physicalenvironment::mps getPropagationSpeed() const override;

private:
    nanobind::object handle_;
};

NAMESPACE_END(sionna)
NAMESPACE_END(artery)

MI_VARIANT
artery::sionna::RadioMaterial<Float, Spectrum>::RadioMaterial(
    const std::string& name, Float conductivity, Float relativePermittivity, Float relativePermeability,
    typename Defaulted<Float64>::Argument thickness)
{
    namespace nb = nanobind;
    using namespace sionna::literals;

    if (relativePermeability != 1.0) {
        throw wrapRuntimeError("sionna::RadioMaterial is non-magnetic, relativePermeability should be 1.0, but is %f", relativePermeability);
    }

    nb::object radioMaterial = Access::getClass(Access::sionnaRt(), ModuleLayout::Classes::radioMaterial);
    auto kw = kwargs("name"_a = name, "thickness"_a = thickness, "relative_permittivity"_a = relativePermittivity, "conductivity"_a = conductivity);
    return radioMaterial(**kw);
}

MI_VARIANT
inet::physicalenvironment::Ohmm artery::sionna::RadioMaterial<Float, Spectrum>::getResistivity() const
{
}

