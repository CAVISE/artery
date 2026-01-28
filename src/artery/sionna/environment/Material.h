#pragma once

#include <nanobind/nanobind.h>

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Defaulted.h>
#include <artery/sionna/bridge/bindings/Constants.h>
#include <artery/sionna/bridge/bindings/Material.h>

#include <inet/common/INETDefs.h>
#include <inet/environment/contract/IMaterial.h>

#include <string>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

MI_VARIANT class RadioMaterial
    : public inet::physicalenvironment::IMaterial {
public:
    SIONNA_IMPORT_CORE_TYPES(Float64)
    SIONNA_IMPORT_BRIDGE_TYPES(Constants, Compat)

    /**
     * @brief Sionna radio material constructor. Current implementation assumes non-magnetic materials, so relative permeability that
     * is different to 1 will raise error.
     */
    RadioMaterial(
        const std::string& name,
        Float64 conductivity = 0.0,
        Float64 relativePermittivity = 1.0,
        typename Defaulted<Float64>::Argument thickness = Constants::constants().defaultThickness()
    );

    explicit RadioMaterial(nanobind::object obj);

    // inet::physicalenvironment::IMaterial implementation.
    inet::physicalenvironment::Ohmm getResistivity() const override;
    double getRelativePermittivity() const override;
    double getRelativePermeability() const override;

    double getDielectricLossTangent(inet::physicalenvironment::Hz frequency) const override;
    double getRefractiveIndex() const override;
    inet::physicalenvironment::mps getPropagationSpeed() const override;

    py::RadioMaterial<Float, Spectrum>& object();
    const py::RadioMaterial<Float, Spectrum>& object() const;

private:
    py::RadioMaterial<Float, Spectrum> py_;
};

SIONNA_EXTERN_CLASS(RadioMaterial)

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
