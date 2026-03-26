#include <limits>

#include "Material.h"

using namespace artery::sionna;

namespace {

    constexpr double relativePermeability = 1.0;

    inet::physicalenvironment::Ohmm computeResistivityFromConductivity(double conductivity) {
        if (conductivity <= 0.0) {
            return inet::physicalenvironment::Ohmm(std::numeric_limits<double>::infinity());
        }
        return inet::physicalenvironment::Ohmm(1.0 / conductivity);
    }

} // namespace

RadioMaterial::RadioMaterial(
    const std::string& name,
    mitsuba::Resolve::Float64 conductivity,
    mitsuba::Resolve::Float64 relativePermittivity,
    typename Defaulted<mitsuba::Resolve::Float64>::Argument thickness)
    : inet::physicalenvironment::Material(
          name.c_str(),
          computeResistivityFromConductivity(toScalar(conductivity)),
          toScalar(relativePermittivity),
          ::relativePermeability)
    , py_(name, conductivity, relativePermittivity, std::move(thickness)) {
}

RadioMaterial::RadioMaterial(py::RadioMaterial material)
    : inet::physicalenvironment::Material(
          material.materialName().c_str(),
          computeResistivityFromConductivity(toScalar(material.conductivity())),
          toScalar(material.relativePermittivity()),
          ::relativePermeability)
    , py_(std::move(material)) {
}

RadioMaterial::RadioMaterial(nanobind::object obj)
    : RadioMaterial(py::RadioMaterial(std::move(obj))) {
}

inet::physicalenvironment::Ohmm RadioMaterial::getResistivity() const {
    const double sigma = toScalar(py_.conductivity());
    return computeResistivityFromConductivity(sigma);
}

double RadioMaterial::getRelativePermittivity() const {
    return toScalar(py_.relativePermittivity());
}

double RadioMaterial::getRelativePermeability() const {
    return ::relativePermeability;
}

double RadioMaterial::getDielectricLossTangent(inet::physicalenvironment::Hz frequency) const {
    const double f = frequency.get();

    const double sigma = toScalar(py_.conductivity());
    const double epsR = toScalar(py_.relativePermittivity());

    const double omega = 2.0 * M_PI * f;
    const double e0 = inet::units::constants::e0.get();

    const double denom = omega * epsR * e0;
    if (denom <= 0.0 || epsR <= 0) {
        return 0.0;
    }

    return sigma / denom;
}

double RadioMaterial::getRefractiveIndex() const {
    const double epsR = toScalar(py_.relativePermittivity());
    return std::sqrt(std::max(0.0, epsR));
}

inet::physicalenvironment::mps RadioMaterial::getPropagationSpeed() const {
    const double epsR = toScalar(py_.relativePermittivity());
    if (epsR <= 0.0) {
        return inet::units::constants::c;
    }

    const double n = std::sqrt(epsR);
    if (n <= 0.0) {
        return inet::units::constants::c;
    }

    return inet::units::constants::c / n;
}

py::RadioMaterial& RadioMaterial::object() {
    return py_;
}

const py::RadioMaterial& RadioMaterial::object() const {
    return py_;
}
