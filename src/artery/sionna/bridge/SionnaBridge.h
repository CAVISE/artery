#pragma once

#if !defined(SIONNA_BRIDGE_COMPILATION_MODE_BASIC) && !defined(SIONNA_BRIDGE_COMPILATION_MODE_INET)
#    error "Compilation mode is missing. Cannot continue"
#endif

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Helpers.h>
#include <artery/sionna/bridge/Defaulted.h>
#include <artery/sionna/bridge/Compat.h>
#include <artery/sionna/bridge/Capabilities.h>

#include <artery/sionna/bridge/bindings/Modules.h>
#include <artery/sionna/bridge/bindings/Constants.h>
#include <artery/sionna/bridge/bindings/Material.h>
#include <artery/sionna/bridge/bindings/SceneObject.h>
#include <artery/sionna/bridge/bindings/Scene.h>
