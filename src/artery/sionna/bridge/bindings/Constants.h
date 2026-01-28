#pragma once

#include <artery/sionna/bridge/Defaulted.h>
#include <artery/sionna/bridge/Capabilities.h>
#include <artery/sionna/bridge/bindings/Modules.h>

#include <mitsuba/core/fwd.h>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

NAMESPACE_BEGIN(py)

MI_VARIANT
class ConstantsModule
    : public DefaultedModuleProviderCapability
    , public SionnaRtModule {
public:
    SIONNA_IMPORT_CORE_TYPES(Float64)

    ConstantsModule();

    // IPythonModuleIdentityCapability implementation.
    const char* moduleName() const override;

    const Defaulted<Float64>& defaultThickness() const;

private:
    Defaulted<Float64> defaultThickness_;
};

MI_VARIANT
class IntersectionTypes
    : public DefaultedClassProviderCapability
    , public ConstantsModule<Float, Spectrum> {
public:
    SIONNA_IMPORT_CORE_TYPES(Int32)

    IntersectionTypes();

    // IPythonClassIdentityCapability implementation.
    const char* className() const override;

    const Defaulted<Int32>& none() const;
    const Defaulted<Int32>& specular() const;
    const Defaulted<Int32>& diffuse() const;
    const Defaulted<Int32>& refraction() const;
    const Defaulted<Int32>& diffraction() const;

private:
    Defaulted<Int32> intersectionNone_;
    Defaulted<Int32> intersectionSpecular_;
    Defaulted<Int32> intersectionDiffuse_;
    Defaulted<Int32> intersectionRefraction_;
    Defaulted<Int32> intersectionDiffraction_;
};

MI_VARIANT
class Constants {
public:
    static const ConstantsModule<Float, Spectrum>& constants();
    static const IntersectionTypes<Float, Spectrum>& intersectionTypes();
};

SIONNA_EXTERN_CLASS(ConstantsModule)
SIONNA_EXTERN_CLASS(IntersectionTypes)
SIONNA_EXTERN_CLASS(Constants)

NAMESPACE_END(py)

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
