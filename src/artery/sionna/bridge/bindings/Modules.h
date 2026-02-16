#pragma once

#include <artery/sionna/bridge/Capabilities.h>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)
NAMESPACE_BEGIN(py)

class SIONNA_BRIDGE_API SionnaRtModule
    : public virtual IPythonModuleIdentityCapability {
public:
    const char* moduleName() const override {
        return "sionna.rt";
    }
};

class SIONNA_BRIDGE_API SionnaRtConstantsModule
    : public virtual IPythonModuleIdentityCapability {
public:
    const char* moduleName() const override {
        return "sionna.rt.constants";
    }
};

class SIONNA_BRIDGE_API SionnaRtModuleBase
    : public SionnaRtModule
    , public CachedImportCapability
    , public DefaultedModuleProviderCapability {};

class SIONNA_BRIDGE_API SionnaRtConstantsModuleBase
    : public SionnaRtConstantsModule
    , public CachedImportCapability
    , public DefaultedModuleProviderCapability {};

NAMESPACE_END(py)
NAMESPACE_END(sionna)
NAMESPACE_END(artery)
