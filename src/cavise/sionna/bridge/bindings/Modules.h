#pragma once

#include <cavise/sionna/bridge/Capabilities.h>

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

} // namespace artery::sionna::py
