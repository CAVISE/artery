#pragma once

#include <artery/sionna/bridge/Defaulted.h>
#include <artery/sionna/bridge/Capabilities.h>
#include <artery/sionna/bridge/bindings/Modules.h>

#include <mitsuba/core/fwd.h>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

NAMESPACE_BEGIN(py)

class SIONNA_BRIDGE_API Const_
    : public DefaultedModuleProviderCapability
    , public SionnaRtModule {
public:

    // IPythonModuleIdentityCapability implementation.
    const char* moduleName() const override;
};

MI_VARIANT
class SIONNA_BRIDGE_API ConstModule_
    : public Const_ {
public:
    SIONNA_IMPORT_CORE_TYPES(Float64)

    ConstModule_();

    const Defaulted<Float64>& defaultThickness() const;

private:
    Defaulted<Float64> defaultThickness_;
};

MI_VARIANT
class SIONNA_BRIDGE_API ConstIntersectionTypes_
    : public DefaultedClassProviderCapability
    , public Const_ {
public:
    SIONNA_IMPORT_CORE_TYPES(Int32)

    ConstIntersectionTypes_();

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
class SIONNA_BRIDGE_API Const {
public:
    SIONNA_IMPORT_CORE_TYPES(Int32, Float64)

    static const Defaulted<Float64>& defaultThickness();

    class IntersectionTypes {
    public:
        static const Defaulted<Int32>& none();
        static const Defaulted<Int32>& specular();
        static const Defaulted<Int32>& diffuse();
        static const Defaulted<Int32>& refraction();
        static const Defaulted<Int32>& diffraction();
    };

private:
    static const ConstModule_<Float, Spectrum>& constModule();
    static const ConstIntersectionTypes_<Float, Spectrum>& constIntersectionTypes();
};

SIONNA_EXTERN_CLASS(ConstModule_)
SIONNA_EXTERN_CLASS(ConstIntersectionTypes_)
SIONNA_EXTERN_CLASS(Const)

NAMESPACE_END(py)

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
