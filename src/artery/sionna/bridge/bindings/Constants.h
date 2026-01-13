#pragma once

#include <artery/sionna/bridge/Defaulted.h>
#include <artery/sionna/bridge/Capabilities.h>
#include <artery/sionna/bridge/bindings/Modules.h>
#include <mitsuba/core/fwd.h>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

NAMESPACE_BEGIN(py)

MI_VARIANT class ConstantsModule
    : public DefaultedModuleProviderCapability
    , public SionnaRtModule {
public:
    SIONNA_IMPORT_CORE_TYPES(Float64)

    ConstantsModule()
        : defaultThickness_(defaulted<Float64>("DEFAULT_THICKNESS"))
    {}

    virtual const char* moduleName() const override {
        return SionnaRtModule::moduleName() + ".constants";
    }

    const Defaulted<Float64>& defaultThickness() const {
        return defaultThickness_;
    }

private:
    Defaulted<Float64> defaultThickness_;
};

MI_VARIANT class IntersectionTypes
    : public DefaultedClassProviderCapability
    , public ConstantsModule<Float, Spectrum> {
public:
    SIONNA_IMPORT_CORE_TYPES(Int32)

    IntersectionTypes()
        : intersectionNone_(defaulted<Int32>("NONE"))
        , intersectionSpecular_(defaulted<Int32>("SPECULAR"))
        , intersectionDiffuse_(defaulted<Int32>("DIFFUSE"))
        , intersectionRefraction_(defaulted<Int32>("REFRACTION"))
        , intersectionDiffraction_(defaulted<Int32>("DIFFRACTION")) {
    }

    virtual const char* className() const override {
        return "IntersectionTypes";
    }

    const Defaulted<Int32>& none() const {
        return intersectionNone_;
    }

    const Defaulted<Int32>& specular() const {
        return intersectionSpecular_;
    }

    const Defaulted<Int32>& diffuse() const {
        return intersectionDiffuse_;
    }

    const Defaulted<Int32>& refraction() const {
        return intersectionRefraction_;
    }

    const Defaulted<Int32>& diffraction() const {
        return intersectionDiffraction_;
    }

private:
    Defaulted<Int32> intersectionNone_;
    Defaulted<Int32> intersectionSpecular_;
    Defaulted<Int32> intersectionDiffuse_;
    Defaulted<Int32> intersectionRefraction_;
    Defaulted<Int32> intersectionDiffraction_;
};

MI_VARIANT
class Constants {
    static const ConstantsModule<Float, Spectrum>& constants() {
        static ConstantsModule<Float, Spectrum> instance;
        return instance;
    }
    
    static const IntersectionTypes<Float, Spectrum>& intersectionTypes() {
        static IntersectionTypes<Float, Spectrum> instance;
        return instance;
    }
};

NAMESPACE_END(py)

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
