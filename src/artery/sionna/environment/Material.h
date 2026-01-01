#pragma once

#include <artery/sionna/bridge/Bindings.h>
#include <artery/sionna/bridge/bindings/Material.h>
#include <artery/sionna/bridge/bindings/Constants.h>

#include <artery/sionna/bridge/Defaulted.h>
#include <artery/sionna/bridge/Fwd.h>
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
    py::RadioMaterial<Float, Spectrum> py_;
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
    const double f = frequency;
    if (!(f > 0.0)) {
        return 0.0;
    }

    const double sigma = static_cast<double>(py_->conductivity());
    const double eps_r = static_cast<double>(py_->relativePermittivity());

    const double omega = 2.0 * M_PI * f;
    const double denom = omega * EPS0 * eps_r;
    if (!(denom > 0.0)) {
        return 0.0;
    }
    return sigma / denom;
}

MI_VARIANT double RadioMaterial::getRefractiveIndex() const
{
    const double eps_r = static_cast<double>(py_->relativePermittivity());
    return std::sqrt(std::max(0.0, eps_r)); // mu_r=1
}

MI_VARIANT inet::physicalenvironment::mps RadioMaterial::getPropagationSpeed() const
{
    const double n = getRefractiveIndex();
    if (!(n > 0.0)) {
        return inet::physicalenvironment::mps(C0);
    }
    return inet::physicalenvironment::mps(C0 / n);
}

