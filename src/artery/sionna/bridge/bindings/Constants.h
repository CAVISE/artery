#pragma once

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Defaulted.h>
#include <artery/sionna/bridge/Capabilities.h>
#include <artery/sionna/bridge/bindings/Modules.h>

#include <mitsuba/core/fwd.h>

namespace artery {
    namespace sionna {
        namespace py {

            MI_VARIANT
            class SIONNA_BRIDGE_API Constants
                : public SionnaRtModule
                , public DefaultedModuleProviderCapability {
            public:
                SIONNA_BRIDGE_IMPORT_CORE_TYPES()

                // IPythonModuleIdentityCapability implementation.
                const char* moduleName() const override;

                // Module Constants.
                static const DefaultedWithDeferredResolution<Float64, ModuleResolver<Constants>> defaultThickness;
            };

            MI_VARIANT
            class SIONNA_BRIDGE_API IntersectionTypes
                : public Constants<Float, Spectrum>
                , public DefaultedClassProviderCapability {
            public:
                SIONNA_BRIDGE_IMPORT_CORE_TYPES()

                template <typename T>
                using TIntersection = DefaultedWithDeferredResolution<T, ModuleAndClassResolver<IntersectionTypes>>;

                // IPythonClassIdentityCapability implementation.
                const char* className() const override;

                static const TIntersection<Int32> none;
                static const TIntersection<Int32> specular;
                static const TIntersection<Int32> diffuse;
                static const TIntersection<Int32> refraction;
                static const TIntersection<Int32> diffraction;
            };

        } // namespace py
    } // namespace sionna
} // namespace artery

SIONNA_BRIDGE_EXTERN_CLASS(artery::sionna::py::Constants);
SIONNA_BRIDGE_EXTERN_CLASS(artery::sionna::py::IntersectionTypes);
