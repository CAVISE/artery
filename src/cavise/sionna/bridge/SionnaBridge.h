#pragma once

// clang-format off
#if !defined(SIONNA_BRIDGE_COMPILATION_MODE_BASIC) && !defined(SIONNA_BRIDGE_COMPILATION_MODE_INET)
    #error "Compilation mode is missing. Cannot continue"
#endif

#if !defined(SIONNA_BRIDGE_RESOLVE_FLOAT)
    #if defined(__clang__) || defined(__GNUC__)
        #warning "Cannot detect mitsuba Float parameter, bridge module falls back to scalar type"
    #elif defined(_MSC_VER)
        #pragma message("Cannot detect mitsuba Float parameter, bridge module falls back to scalar type")
    #endif
#endif

#if !defined(SIONNA_BRIDGE_RESOLVE_SPECTRUM)
    #if defined(__clang__) || defined(__GNUC__)
        #warning "Cannot detect mitsuba Spectrum parameter, bridge module falls back to scalar type"
    #elif defined(_MSC_VER)
        #pragma message("Cannot detect mitsuba Spectrum parameter, bridge module falls back to scalar type")
    #endif
#endif
// clang-format on

#include <cavise/sionna/bridge/Fwd.h>
#include <cavise/sionna/bridge/Casters.h>
#include <cavise/sionna/bridge/Helpers.h>
#include <cavise/sionna/bridge/Defaulted.h>
#include <cavise/sionna/bridge/Compat.h>

#include <cavise/sionna/bridge/capabilities/Core.h>
#include <cavise/sionna/bridge/capabilities/Calling.h>
#include <cavise/sionna/bridge/capabilities/Defaulted.h>

#include <cavise/sionna/bridge/bindings/Modules.h>
#include <cavise/sionna/bridge/bindings/Camera.h>
#include <cavise/sionna/bridge/bindings/Constants.h>
#include <cavise/sionna/bridge/bindings/Material.h>
#include <cavise/sionna/bridge/bindings/SceneObject.h>
#include <cavise/sionna/bridge/bindings/Scene.h>
