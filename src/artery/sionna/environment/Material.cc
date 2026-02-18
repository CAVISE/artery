#include <limits>

#include "Material.h"

SIONNA_BRIDGE_INSTANTIATE_CLASS(artery::sionna::InetRadioMaterial)

using namespace artery::sionna;

namespace {

    inet::physicalenvironment::Ohmm computeResistivityFromConductivity(double conductivity) {
        if (conductivity <= 0.0) {
            return inet::physicalenvironment::Ohmm(std::numeric_limits<double>::infinity());
        }
        return inet::physicalenvironment::Ohmm(1.0 / conductivity);
    }

} // namespace

MI_VARIANT
InetRadioMaterial<Float, Spectrum>::InetRadioMaterial(
    const std::string& name,
    Float64 conductivity,
    Float64 relativePermittivity,
    typename DefaultedThicknessType::Argument thickness
)
    : inet::physicalenvironment::Material(
          name.c_str(),
          computeResistivityFromConductivity(Compat::toScalar(conductivity)),
          Compat::toScalar(relativePermittivity),
          1.0
    )
    , py_(name, conductivity, relativePermittivity, thickness)
{}

MI_VARIANT
InetRadioMaterial<Float, Spectrum>::InetRadioMaterial(py::RadioMaterial<Float, Spectrum> material)
    : inet::physicalenvironment::Material(
          material.materialName().c_str(),
          computeResistivityFromConductivity(Compat::toScalar(material.conductivity())),
          Compat::toScalar(material.relativePermittivity()),
          1.0
    )
    , py_(std::move(material))
{}

MI_VARIANT
InetRadioMaterial<Float, Spectrum>::InetRadioMaterial(nanobind::object obj)
    : InetRadioMaterial(py::RadioMaterial<Float, Spectrum>(std::move(obj)))
{}

MI_VARIANT
inet::physicalenvironment::Ohmm InetRadioMaterial<Float, Spectrum>::getResistivity() const {
    const double sigma = Compat::toScalar(py_.conductivity());
    return computeResistivityFromConductivity(sigma);
}

MI_VARIANT
double InetRadioMaterial<Float, Spectrum>::getRelativePermittivity() const {
    return Compat::toScalar(py_.relativePermittivity());
}

MI_VARIANT
double InetRadioMaterial<Float, Spectrum>::getRelativePermeability() const {
    return 1.0;
}

MI_VARIANT
double InetRadioMaterial<Float, Spectrum>::getDielectricLossTangent(inet::physicalenvironment::Hz frequency) const {
    const double f = frequency.get();

    const double sigma = Compat::toScalar(py_.conductivity());
    const double epsR = Compat::toScalar(py_.relativePermittivity());

    const double omega = 2.0 * M_PI * f;
    const double e0 = inet::units::constants::e0.get();

    const double denom = omega * epsR * e0;
    if (denom <= 0.0 || epsR <= 0) {
        return 0.0;
    }
    return sigma / denom;
}

MI_VARIANT
double InetRadioMaterial<Float, Spectrum>::getRefractiveIndex() const {
    const double epsR = Compat::toScalar(py_.relativePermittivity());
    return std::sqrt(std::max(0.0, epsR));
}

MI_VARIANT
inet::physicalenvironment::mps InetRadioMaterial<Float, Spectrum>::getPropagationSpeed() const {
    if (const double epsR = Compat::toScalar(py_.relativePermittivity()); epsR <= 0.0) {
        return inet::units::constants::c;
    } else if (const double n = std::sqrt(epsR); n <= 0.0) {
        return inet::units::constants::c;
    } else {
        return inet::units::constants::c / n;
    }
}

MI_VARIANT
py::RadioMaterial<Float, Spectrum>& InetRadioMaterial<Float, Spectrum>::object() {
    return py_;
}

MI_VARIANT
const py::RadioMaterial<Float, Spectrum>& InetRadioMaterial<Float, Spectrum>::object() const {
    return py_;
}
