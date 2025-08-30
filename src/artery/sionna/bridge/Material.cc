#include "Material.h"

#include "artery/sionna/bridge/Helpers.h"
#include "omnetpp/cexception.h"

#include <pybind11/cast.h>
#include <pybind11/pytypes.h>

using namespace artery;

namespace py = pybind11;

sionna::RadioMaterialBase::RadioMaterialBase(pybind11::object object) : py::object(std::move(object)) {
}

void sionna::RadioMaterialBase::setColor(Color color) {
    sionna::set(this, "color", color);
}

std::string sionna::RadioMaterialBase::name() const {
    return sionna::access<std::string>(this, "name");
}

sionna::Color sionna::RadioMaterialBase::color() const {
    return sionna::access<Color>(this, "color");
}

sionna::RadioMaterial::RadioMaterial(CtorArguments args) : RadioMaterialBase(initialize(std::move(args))) {
}

pybind11::object sionna::RadioMaterial::initialize(CtorArguments args) {
    if (args.relativePermeability != 1.0) {
        throw omnetpp::cRuntimeError("sionna::RadioMaterial is non-magnetic, relativePermeability should be 1.0, but is %f", args.relativePermeability);
    }

    py::object cb = py::none();
    if (args.callback.has_value()) {
        cb = makeFrequencyUpdateCallback(args.callback.value());
    }

    py::object radioMaterial = sionna::sionnaRt().attr("RadioMaterial");
    return radioMaterial(
        args.name,
        args.thickness,
        args.relativePermittivity,
        args.conductivity,
        args.scatteringCoefficient,
        args.xpdCoefficient,
        args.relativePermeability,
        args.pattern,
        cb);
}

void sionna::RadioMaterial::setRelativePermittivity(double relativePermittivity) {
    sionna::set(this, "relative_permittivity", relativePermittivity);
}

void sionna::RadioMaterial::setConductivity(double conductivity) {
    sionna::set(this, "conductivity", conductivity);
}

void sionna::RadioMaterial::setThickness(double thickness) {
    sionna::set(this, "thickness", thickness);
}

void sionna::RadioMaterial::setScatteringCoefficient(double scatteringCoefficient) {
    sionna::set(this, "scattering_coefficient", scatteringCoefficient);
}

void sionna::RadioMaterial::setXpdCoefficient(double xpdCoefficient) {
    sionna::set(this, "xpd_coefficient", xpdCoefficient);
}

void sionna::RadioMaterial::setScatteringPattern(const std::string& pattern) {
    sionna::set(this, "scattering_pattern", pattern);
}

void sionna::RadioMaterial::setFrequencyUpdateCallback(const FrequencyUpdateCallbackType& callback) {
    sionna::set(this, "frequency_update_callback", makeFrequencyUpdateCallback(callback));
}

double sionna::RadioMaterial::relativePermittivity() const {
    return sionna::access<double>(this, "relative_permittivity");
}

double sionna::RadioMaterial::conductivity() const {
    return sionna::access<double>(this, "conductivity");
}

double sionna::RadioMaterial::thickness() const {
    return sionna::access<double>(this, "thickness");
}

double sionna::RadioMaterial::scatteringCoefficient() const {
    return sionna::access<double>(this, "scattering_coefficient");
}

double sionna::RadioMaterial::xpdCoefficient() const {
    return sionna::access<double>(this, "xpd_coefficient");
}

std::string sionna::RadioMaterial::scatteringPattern() const {
    return sionna::access<std::string>(this, "scattering_pattern");
}
