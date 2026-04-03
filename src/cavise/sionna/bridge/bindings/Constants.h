#pragma once

#include <cavise/sionna/bridge/Defaulted.h>
#include <cavise/sionna/bridge/Capabilities.h>
#include <cavise/sionna/bridge/bindings/Modules.h>

#include <mitsuba/core/fwd.h>

namespace artery::sionna::py {

    class SIONNA_BRIDGE_API Constants
        : public SionnaRtModule
        , public DefaultedModuleProviderCapability {
    public:
        // IPythonModuleIdentityCapability implementation.
        const char* moduleName() const override;

        // Module Constants.
        static const DefaultedWithDeferredResolution<mitsuba::Resolve::Float64, ModuleResolver<Constants>> defaultThickness;
    };

    class SIONNA_BRIDGE_API IntersectionTypes
        : public Constants
        , public DefaultedClassProviderCapability {
    public:
        template <typename T>
        using TIntersection = DefaultedWithDeferredResolution<T, ModuleAndClassResolver<IntersectionTypes>>;

        // IPythonClassIdentityCapability implementation.
        const char* className() const override;

        static const TIntersection<mitsuba::Resolve::Int32> none;
        static const TIntersection<mitsuba::Resolve::Int32> specular;
        static const TIntersection<mitsuba::Resolve::Int32> diffuse;
        static const TIntersection<mitsuba::Resolve::Int32> refraction;
        static const TIntersection<mitsuba::Resolve::Int32> diffraction;
    };

} // namespace artery::sionna::py
