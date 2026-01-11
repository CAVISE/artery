#pragma once

#include <artery/sionna/bridge/Bindings.h>
#include <artery/sionna/bridge/Defaulted.h>
#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/bindings/Constants.h>
#include <artery/sionna/bridge/bindings/Material.h>
#include <inet/common/INETDefs.h>
#include <inet/environment/contract/IMaterial.h>

#include <string>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

MI_VARIANT class RadioMaterial : public inet::physicalenvironment::IMaterial
{
public:
    SIONNA_IMPORT_CORE_TYPES(Float64)

    using Constants = py::Constants<Float, Spectrum>;

    /**
     * @brief Sionna radio material constructor. Current implementation assumes non-magnetic materials, so relative permeability that
     * is different to 1 will raise error.
     */
    RadioMaterial(
        const std::string& name, Float64 conductivity = 0.0, Float64 relativePermittivity = 1.0,
        typename Defaulted<Float64>::Argument thickness = Constants::DEFAULT_THICKNESS);

    // inet::physicalenvironment::IMaterial implementation.
    inet::physicalenvironment::Ohmm getResistivity() const override;
    double getRelativePermittivity() const override;
    double getRelativePermeability() const override;

    double getDielectricLossTangent(inet::physicalenvironment::Hz frequency) const override;
    double getRefractiveIndex() const override;
    inet::physicalenvironment::mps getPropagationSpeed() const override;

private:
    py::RadioMaterial<Float, Spectrum> py_;
};

NAMESPACE_END(sionna)
NAMESPACE_END(artery)

MI_VARIANT
artery::sionna::RadioMaterial<Float, Spectrum>::RadioMaterial(
    const std::string& name, Float64 conductivity, Float64 relativePermittivity, typename Defaulted<Float64>::Argument thickness) :
    py_(name, conductivity, relativePermittivity, thickness)
{
}

MI_VARIANT inet::physicalenvironment::Ohmm artery::sionna::RadioMaterial<Float, Spectrum>::getResistivity() const
{
    const double sigma = static_cast<double>(py_->conductivity());
    if (sigma <= 0.0) {
        return inet::physicalenvironment::Ohmm(std::numeric_limits<double>::infinity());
    }
    return inet::physicalenvironment::Ohmm(1.0 / sigma);
}

MI_VARIANT double artery::sionna::RadioMaterial<Float, Spectrum>::getRelativePermittivity() const
{
    return static_cast<double>(py_->relativePermittivity());
}

MI_VARIANT double artery::sionna::RadioMaterial<Float, Spectrum>::getRelativePermeability() const
{
    return 1.0;
}

MI_VARIANT double artery::sionna::RadioMaterial<Float, Spectrum>::getDielectricLossTangent(inet::physicalenvironment::Hz frequency) const
{
    const double f = frequency.get();

    const double sigma = static_cast<double>(py_->conductivity());
    const double epsR = static_cast<double>(py_->relativePermittivity());
    
    const double omega = 2.0 * M_PI * f;
    const double e0 = inet::units::constants::e0.get();

    if (const double denom = omega * epsR * e0; denom <= 0.0 || epsR <= 0) {
        return 0.0;
    } else {
        return sigma / denom;
    }
}

MI_VARIANT double artery::sionna::RadioMaterial<Float, Spectrum>::getRefractiveIndex() const
{
    const double epsR = static_cast<double>(py_->relativePermittivity());
    return std::sqrt(std::max(0.0, epsR));
}

MI_VARIANT inet::physicalenvironment::mps artery::sionna::RadioMaterial<Float, Spectrum>::getPropagationSpeed() const
{
    if (const double epsR = static_cast<double>(py_->relativePermittivity()); epsR <= 0.0) {
        return inet::units::constants::c;
    } else if (const double n = std::sqrt(epsR); n <= 0.0) {
        return inet::units::constants::c;
    } else {
        return inet::units::constants::c / n;
    }
}
