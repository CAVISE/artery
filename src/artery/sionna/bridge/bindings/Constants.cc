#include "Constants.h"

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)
NAMESPACE_BEGIN(py)

SIONNA_INSTANTIATE_CLASS(ConstModule_)
SIONNA_INSTANTIATE_CLASS(ConstIntersectionTypes_)
SIONNA_INSTANTIATE_CLASS(Const)

MI_VARIANT
ConstModule_<Float, Spectrum>::ConstModule_()
    : defaultThickness_(defaulted<Float64>("DEFAULT_THICKNESS"))
{}

const char* Const_::moduleName() const {
    return "sionna.rt.constants";
}

MI_VARIANT
const Defaulted<typename ConstModule_<Float, Spectrum>::Float64>&
ConstModule_<Float, Spectrum>::defaultThickness() const {
    return defaultThickness_;
}

MI_VARIANT
ConstIntersectionTypes_<Float, Spectrum>::ConstIntersectionTypes_()
    : intersectionNone_(DefaultedClassProviderCapability::defaulted<Int32>("NONE"))
    , intersectionSpecular_(DefaultedClassProviderCapability::defaulted<Int32>("SPECULAR"))
    , intersectionDiffuse_(DefaultedClassProviderCapability::defaulted<Int32>("DIFFUSE"))
    , intersectionRefraction_(DefaultedClassProviderCapability::defaulted<Int32>("REFRACTION"))
    , intersectionDiffraction_(DefaultedClassProviderCapability::defaulted<Int32>("DIFFRACTION")) {}

MI_VARIANT
const char* ConstIntersectionTypes_<Float, Spectrum>::className() const {
    return "ConstIntersectionTypes";
}

MI_VARIANT
const Defaulted<typename ConstIntersectionTypes_<Float, Spectrum>::Int32>&
ConstIntersectionTypes_<Float, Spectrum>::none() const {
    return intersectionNone_;
}

MI_VARIANT
const Defaulted<typename ConstIntersectionTypes_<Float, Spectrum>::Int32>&
ConstIntersectionTypes_<Float, Spectrum>::specular() const {
    return intersectionSpecular_;
}

MI_VARIANT
const Defaulted<typename ConstIntersectionTypes_<Float, Spectrum>::Int32>&
ConstIntersectionTypes_<Float, Spectrum>::diffuse() const {
    return intersectionDiffuse_;
}

MI_VARIANT
const Defaulted<typename ConstIntersectionTypes_<Float, Spectrum>::Int32>&
ConstIntersectionTypes_<Float, Spectrum>::refraction() const {
    return intersectionRefraction_;
}

MI_VARIANT
const Defaulted<typename ConstIntersectionTypes_<Float, Spectrum>::Int32>&
ConstIntersectionTypes_<Float, Spectrum>::diffraction() const {
    return intersectionDiffraction_;
}

MI_VARIANT
const ConstModule_<Float, Spectrum>& Const<Float, Spectrum>::constModule() {
    static ConstModule_<Float, Spectrum> instance;
    return instance;
}

MI_VARIANT
const ConstIntersectionTypes_<Float, Spectrum>& Const<Float, Spectrum>::constIntersectionTypes() {
    static ConstIntersectionTypes_<Float, Spectrum> instance;
    return instance;
}

MI_VARIANT
const Defaulted<typename Const<Float, Spectrum>::Float64>& Const<Float, Spectrum>::defaultThickness() {
    return constModule().defaultThickness();
}

MI_VARIANT
const Defaulted<typename Const<Float, Spectrum>::Int32>& Const<Float, Spectrum>::IntersectionTypes::none() {
    return Const::constIntersectionTypes().none();
}

MI_VARIANT
const Defaulted<typename Const<Float, Spectrum>::Int32>& Const<Float, Spectrum>::IntersectionTypes::specular() {
    return Const::constIntersectionTypes().specular();
}

MI_VARIANT
const Defaulted<typename Const<Float, Spectrum>::Int32>& Const<Float, Spectrum>::IntersectionTypes::diffuse() {
    return Const::constIntersectionTypes().diffuse();
}

MI_VARIANT
const Defaulted<typename Const<Float, Spectrum>::Int32>& Const<Float, Spectrum>::IntersectionTypes::refraction() {
    return Const::constIntersectionTypes().refraction();
}

MI_VARIANT
const Defaulted<typename Const<Float, Spectrum>::Int32>& Const<Float, Spectrum>::IntersectionTypes::diffraction() {
    return Const::constIntersectionTypes().diffraction();
}

NAMESPACE_END(py)
NAMESPACE_END(sionna)
NAMESPACE_END(artery)
