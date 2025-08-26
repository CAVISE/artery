#include "Material.h"

#include "artery/sionna/bridge/Helpers.h"

#include <pybind11/cast.h>
#include <pybind11/pytypes.h>

using namespace artery;

namespace py = pybind11;

namespace pybind11 {

    namespace detail {

        template <>
        struct type_caster<sionna::Color> {
        public:
            PYBIND11_TYPE_CASTER(sionna::Color, _("Color"));

            bool load(handle src, bool /* status */) {
                if (!src) {
                    return false;
                }
                py::object obj = py::reinterpret_borrow<py::object>(src);

                if (!py::isinstance<py::tuple>(obj)) {
                    return false;
                }

                if (py::sequence seq = obj.cast<py::sequence>(); py::len(seq) != 3) {
                    return false;
                } else {
                    double r = py::float_(seq[0]).cast<double>();
                    double g = py::float_(seq[1]).cast<double>();
                    double b = py::float_(seq[2]).cast<double>();
                    value = sionna::Color(r, g, b);
                    return true;
                }
            }

            static handle cast(const sionna::Color& c, return_value_policy /*policy*/, handle /*parent*/) { return py::make_tuple(c.r, c.g, c.b).release(); }
        };

    }  // namespace detail

}  // namespace pybind11

sionna::Color::Color(double r, double g, double b) : r(r), g(g), b(b) {
}

void sionna::RadioMaterialBase::setColor(Color color) {
    sionna::set(handle_, "color", color);
}

std::string sionna::RadioMaterialBase::name() {
    return sionna::access<std::string>(handle_, "name");
}

sionna::Color sionna::RadioMaterialBase::color() const {
    return sionna::access<Color>(handle_, "color");
}

sionna::RadioMaterial::RadioMaterial(
    const std::string& name,
    double thickness,
    double relativePermittivity,
    double conductivity,
    double scatteringCoefficient,
    double xpdCoefficient,
    double relativePermeability,
    const std::string& pattern,
    std::optional<FrequencyUpdateCallbackType> callback) {
    py::object radioMaterial = sionna::sionnaRt().attr("RadioMaterial");

    py::object cb = py::none();
    if (callback.has_value()) {
        cb = makeFrequencyUpdateCallback(callback.value());
    }

    handle_ = radioMaterial(name, thickness, relativePermittivity, conductivity, scatteringCoefficient, xpdCoefficient, relativePermeability, pattern, cb);
}

void sionna::RadioMaterial::setRelativePermittivity(double relativePermittivity) {
    sionna::set(handle_, "relative_permittivity", relativePermittivity);
}

void sionna::RadioMaterial::setConductivity(double conductivity) {
    sionna::set(handle_, "conductivity", conductivity);
}

void sionna::RadioMaterial::setThickness(double thickness) {
    sionna::set(handle_, "thickness", thickness);
}

void sionna::RadioMaterial::setScatteringCoefficient(double scatteringCoefficient) {
    sionna::set(handle_, "scattering_coefficient", scatteringCoefficient);
}

void sionna::RadioMaterial::setXpdCoefficient(double xpdCoefficient) {
    sionna::set(handle_, "xpd_coefficient", xpdCoefficient);
}

void sionna::RadioMaterial::setScatteringPattern(const std::string& pattern) {
    sionna::set(handle_, "scattering_pattern", pattern);
}

void sionna::RadioMaterial::setFrequencyUpdateCallback(const FrequencyUpdateCallbackType& callback) {
    sionna::set(handle_, "frequency_update_callback", makeFrequencyUpdateCallback(callback));
}

double sionna::RadioMaterial::relativePermittivity() const {
    return sionna::access<double>(handle_, "relative_permittivity");
}

double sionna::RadioMaterial::conductivity() const {
    return sionna::access<double>(handle_, "conductivity");
}

double sionna::RadioMaterial::thickness() const {
    return sionna::access<double>(handle_, "thickness");
}

double sionna::RadioMaterial::scatteringCoefficient() const {
    return sionna::access<double>(handle_, "scattering_coefficient");
}

double sionna::RadioMaterial::xpdCoefficient() const {
    return sionna::access<double>(handle_, "xpd_coefficient");
}

std::string sionna::RadioMaterial::scatteringPattern() const {
    return sionna::access<std::string>(handle_, "scattering_pattern");
}
