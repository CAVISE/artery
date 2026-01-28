#include "Constants.h"

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)
NAMESPACE_BEGIN(py)

SIONNA_INSTANTIATE_CLASS(ConstantsModule)
SIONNA_INSTANTIATE_CLASS(IntersectionTypes)
SIONNA_INSTANTIATE_CLASS(Constants)

MI_VARIANT
ConstantsModule<Float, Spectrum>::ConstantsModule()
    : defaultThickness_(defaulted<Float64>("DEFAULT_THICKNESS")) {}

MI_VARIANT
const char* ConstantsModule<Float, Spectrum>::moduleName() const {
    return "sionna.rt.constants";
}

MI_VARIANT
const Defaulted<typename ConstantsModule<Float, Spectrum>::Float64>&
ConstantsModule<Float, Spectrum>::defaultThickness() const {
    return defaultThickness_;
}

MI_VARIANT
IntersectionTypes<Float, Spectrum>::IntersectionTypes()
    : intersectionNone_(defaulted<Int32>("NONE"))
    , intersectionSpecular_(defaulted<Int32>("SPECULAR"))
    , intersectionDiffuse_(defaulted<Int32>("DIFFUSE"))
    , intersectionRefraction_(defaulted<Int32>("REFRACTION"))
    , intersectionDiffraction_(defaulted<Int32>("DIFFRACTION")) {}

MI_VARIANT
const char* IntersectionTypes<Float, Spectrum>::className() const {
    return "IntersectionTypes";
}

MI_VARIANT
const Defaulted<typename IntersectionTypes<Float, Spectrum>::Int32>&
IntersectionTypes<Float, Spectrum>::none() const {
    return intersectionNone_;
}

MI_VARIANT
const Defaulted<typename IntersectionTypes<Float, Spectrum>::Int32>&
IntersectionTypes<Float, Spectrum>::specular() const {
    return intersectionSpecular_;
}

MI_VARIANT
const Defaulted<typename IntersectionTypes<Float, Spectrum>::Int32>&
IntersectionTypes<Float, Spectrum>::diffuse() const {
    return intersectionDiffuse_;
}

MI_VARIANT
const Defaulted<typename IntersectionTypes<Float, Spectrum>::Int32>&
IntersectionTypes<Float, Spectrum>::refraction() const {
    return intersectionRefraction_;
}

MI_VARIANT
const Defaulted<typename IntersectionTypes<Float, Spectrum>::Int32>&
IntersectionTypes<Float, Spectrum>::diffraction() const {
    return intersectionDiffraction_;
}

MI_VARIANT
const ConstantsModule<Float, Spectrum>& Constants<Float, Spectrum>::constants() {
    static ConstantsModule<Float, Spectrum> instance;
    return instance;
}

MI_VARIANT
const IntersectionTypes<Float, Spectrum>& Constants<Float, Spectrum>::intersectionTypes() {
    static IntersectionTypes<Float, Spectrum> instance;
    return instance;
}

NAMESPACE_END(py)
NAMESPACE_END(sionna)
NAMESPACE_END(artery)
