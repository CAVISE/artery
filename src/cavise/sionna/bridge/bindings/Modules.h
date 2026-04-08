#pragma once

#include <cavise/sionna/bridge/capabilities/Core.h>

namespace artery::sionna::py {

    class Scene;

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

    class SionnaRtScenesModule
        : public virtual IPythonModuleIdentityCapability {
    public:
        const char* moduleName() const override {
            return "sionna.rt.scenes";
        }
    };

    class Scene;

} // namespace artery::sionna::py
