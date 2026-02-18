#pragma once

#include <artery/sionna/bridge/Capabilities.h>

namespace artery::sionna::py {

    class SionnaRtModule
        : public virtual IPythonModuleIdentityCapability {
    public:
        const char* moduleName() const override {
            return "sionna.rt";
        }
    };

    class SionnaRtConstantsModule
        : public virtual IPythonModuleIdentityCapability {
    public:
        const char* moduleName() const override {
            return "sionna.rt.constants";
        }
    };

    class SionnaRtModuleBase
        : public SionnaRtModule
        , public CachedImportCapability
        , public DefaultedModuleProviderCapability {};

    class SionnaRtConstantsModuleBase
        : public SionnaRtConstantsModule
        , public CachedImportCapability
        , public DefaultedModuleProviderCapability {};

}
