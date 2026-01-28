#include "Material.h"

#include <artery/sionna/bridge/Compat.h>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

MI_VARIANT
RadioMaterial<Float, Spectrum>::RadioMaterial(
    const std::string& name,
    Float64 conductivity,
    Float64 relativePermittivity,
    typename Defaulted<Float64>::Argument thickness
)
    : py_(name, conductivity, relativePermittivity, thickness)
{}

MI_VARIANT
RadioMaterial<Float, Spectrum>::RadioMaterial(nanobind::object obj)
    : py_(std::move(obj))
{}

MI_VARIANT
inet::physicalenvironment::Ohmm RadioMaterial<Float, Spectrum>::getResistivity() const {
    const double sigma = Compat::toScalar(py_.conductivity());
    if (sigma <= 0.0) {
        return inet::physicalenvironment::Ohmm(std::numeric_limits<double>::infinity());
    }
    return inet::physicalenvironment::Ohmm(1.0 / sigma);
}

MI_VARIANT
double RadioMaterial<Float, Spectrum>::getRelativePermittivity() const {
    return Compat::toScalar(py_.relativePermittivity());
}

MI_VARIANT
double RadioMaterial<Float, Spectrum>::getRelativePermeability() const {
    return 1.0;
}

MI_VARIANT
double RadioMaterial<Float, Spectrum>::getDielectricLossTangent(inet::physicalenvironment::Hz frequency) const {
    const double f = frequency.get();

    const double sigma = Compat::toScalar(py_.conductivity());
    const double epsR = Compat::toScalar(py_.relativePermittivity());

    const double omega = 2.0 * M_PI * f;
    const double e0 = inet::units::constants::e0.get();

    if (const double denom = omega * epsR * e0; denom <= 0.0 || epsR <= 0) {
        return 0.0;
    } else {
        return sigma / denom;
    }
}

MI_VARIANT
double RadioMaterial<Float, Spectrum>::getRefractiveIndex() const {
    const double epsR = Compat::toScalar(py_.relativePermittivity());
    return std::sqrt(std::max(0.0, epsR));
}

MI_VARIANT
inet::physicalenvironment::mps RadioMaterial<Float, Spectrum>::getPropagationSpeed() const {
    if (const double epsR = Compat::toScalar(py_.relativePermittivity()); epsR <= 0.0) {
        return inet::units::constants::c;
    } else if (const double n = std::sqrt(epsR); n <= 0.0) {
        return inet::units::constants::c;
    } else {
        return inet::units::constants::c / n;
    }
}

MI_VARIANT
py::RadioMaterial<Float, Spectrum>& RadioMaterial<Float, Spectrum>::object() {
    return py_;
}

MI_VARIANT
const py::RadioMaterial<Float, Spectrum>& RadioMaterial<Float, Spectrum>::object() const {
    return py_;
}

SIONNA_INSTANTIATE_CLASS(RadioMaterial)

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
