#include "Material.h"
#include "artery/sionna/bridge/Capabilities.h"

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)
NAMESPACE_BEGIN(py)

SIONNA_INSTANTIATE_CLASS(RadioMaterialBase)
SIONNA_INSTANTIATE_CLASS(RadioMaterial)

MI_VARIANT
const char* RadioMaterialBase<Float, Spectrum>::className() const {
    return "RadioMaterialBase";
}

MI_VARIANT
std::string RadioMaterialBase<Float, Spectrum>::materialName() const {
    return sionna::access<std::string>(*this->bound_, "name");
}

MI_VARIANT
auto RadioMaterialBase<Float, Spectrum>::color() const -> ColorType {
    return sionna::access<ColorType>(*this->bound_, "color");
}

MI_VARIANT
void RadioMaterialBase<Float, Spectrum>::setColor(ColorType newColor) {
    sionna::set<ColorType>(*this->bound_, "color", std::move(newColor));
}

MI_VARIANT
const char* RadioMaterial<Float, Spectrum>::className() const {
    return "RadioMaterial";
}

MI_VARIANT
RadioMaterial<Float, Spectrum>::RadioMaterial() = default;

MI_VARIANT
RadioMaterial<Float, Spectrum>::RadioMaterial(nb::object obj) {
    this->init(std::move(obj));
}

MI_VARIANT
RadioMaterial<Float, Spectrum>::RadioMaterial(
    const std::string& name,
    Float64 conductivity,
    Float64 relativePermittivity,
    typename Defaulted<Float64>::Argument thickness
) {
    WrapPythonClassCapability::init(
        "name"_a = name,
        "thickness"_a = thickness,
        "relative_permittivity"_a = relativePermittivity,
        "conductivity"_a = conductivity
    );
}

MI_VARIANT
auto RadioMaterial<Float, Spectrum>::relativePermittivity() const -> Float64 {
    return sionna::access<Float64>(*this->bound_, "relative_permittivity");
}

MI_VARIANT
void RadioMaterial<Float, Spectrum>::setRelativePermittivity(Float64 relativePermittivity) {
    sionna::set(*this->bound_, "relative_permittivity", relativePermittivity);
}

MI_VARIANT
auto RadioMaterial<Float, Spectrum>::conductivity() const -> Float64 {
    return sionna::access<Float64>(*this->bound_, "conductivity");
}

MI_VARIANT
void RadioMaterial<Float, Spectrum>::setConductivity(Float64 conductivity) {
    sionna::set(*this->bound_, "conductivity", conductivity);
}

MI_VARIANT
auto RadioMaterial<Float, Spectrum>::thickness() const -> Float64 {
    return sionna::access<Float64>(*this->bound_, "thickness");
}

MI_VARIANT
void RadioMaterial<Float, Spectrum>::setThickness(Float64 thickness) {
    sionna::set(*this->bound_, "thickness", thickness);
}

NAMESPACE_END(py)
NAMESPACE_END(sionna)
NAMESPACE_END(artery)
