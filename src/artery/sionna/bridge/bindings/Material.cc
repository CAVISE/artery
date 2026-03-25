#include "artery/sionna/bridge/Capabilities.h"
#include "artery/sionna/bridge/Helpers.h"

#include "Material.h"

SIONNA_BRIDGE_INSTANTIATE_CLASS(artery::sionna::py::RadioMaterialBase);
SIONNA_BRIDGE_INSTANTIATE_CLASS(artery::sionna::py::RadioMaterial);

using namespace artery::sionna;
using namespace artery::sionna::literals;

MI_VARIANT
const char* py::RadioMaterialBase<Float, Spectrum>::className() const {
    return "RadioMaterialBase";
}

MI_VARIANT
std::string py::RadioMaterialBase<Float, Spectrum>::materialName() const {
    return sionna::access<std::string>(this->bound_, "name");
}

MI_VARIANT
auto py::RadioMaterialBase<Float, Spectrum>::color() const -> TColor {
    return sionna::access<TColor>(this->bound_, "color");
}

MI_VARIANT
void py::RadioMaterialBase<Float, Spectrum>::setColor(TColor newColor) {
    sionna::set<TColor>(this->bound_, "color", std::move(newColor));
}

MI_VARIANT
const char* py::RadioMaterial<Float, Spectrum>::className() const {
    return "RadioMaterial";
}

MI_VARIANT
py::RadioMaterial<Float, Spectrum>::RadioMaterial() = default;

MI_VARIANT
py::RadioMaterial<Float, Spectrum>::RadioMaterial(nanobind::object obj) {
    this->init(std::move(obj));
}

MI_VARIANT
py::RadioMaterial<Float, Spectrum>::RadioMaterial(
    const std::string& name,
    Float64 conductivity,
    Float64 relativePermittivity,
    typename Defaulted<Float64>::Argument thickness) {
    this->InitPythonClassCapability::init(
        "name"_a = name,
        "thickness"_a = thickness,
        "relative_permittivity"_a = relativePermittivity,
        "conductivity"_a = conductivity);
}

MI_VARIANT
maybe_diff_t<Float> py::RadioMaterial<Float, Spectrum>::relativePermittivity() const {
    return sionna::access<maybe_diff_t<Float>>(this->bound_, "relative_permittivity");
}

MI_VARIANT
void py::RadioMaterial<Float, Spectrum>::setRelativePermittivity(maybe_diff_t<Float> relativePermittivity) {
    sionna::set(this->bound_, "relative_permittivity", relativePermittivity);
}

MI_VARIANT
maybe_diff_t<Float> py::RadioMaterial<Float, Spectrum>::conductivity() const {
    return sionna::access<maybe_diff_t<Float>>(this->bound_, "conductivity");
}

MI_VARIANT
void py::RadioMaterial<Float, Spectrum>::setConductivity(maybe_diff_t<Float> conductivity) {
    sionna::set(this->bound_, "conductivity", conductivity);
}

MI_VARIANT
maybe_diff_t<Float> py::RadioMaterial<Float, Spectrum>::thickness() const {
    return sionna::access<maybe_diff_t<Float>>(this->bound_, "thickness");
}

MI_VARIANT
void py::RadioMaterial<Float, Spectrum>::setThickness(maybe_diff_t<Float> thickness) {
    sionna::set(this->bound_, "thickness", thickness);
}
