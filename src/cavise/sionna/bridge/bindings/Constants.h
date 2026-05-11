#pragma once

#include <cavise/sionna/bridge/Defaulted.h>
#include <cavise/sionna/bridge/bindings/Modules.h>
#include <cavise/sionna/bridge/capabilities/Defaulted.h>
#include <cavise/sionna/bridge/capabilities/Core.h>

#include <mitsuba/core/fwd.h>

#include <tuple>

namespace artery::sionna::py {

    // Accessor for constants exported by Sionna. Values are resolved lazily so
    // they remain valid after the Python runtime and Mitsuba variant are initialized.
    class SIONNA_BRIDGE_API Constants
        : public SionnaRtModule
        , public DefaultedModuleProviderCapability {
    public:
        // IPythonModuleIdentityCapability implementation.
        const char* moduleName() const override;

        // Sionna's default effective material thickness, used when constructing
        // radio materials without an explicit thickness.
        static const DefaultedWithDeferredResolution<mi::Float64, ModuleResolver<Constants>> defaultThickness;
    };

    // Wrapper for Sionna interaction type constants used by Paths.interactions.
    // These classify what happened at each path vertex.
    class SIONNA_BRIDGE_API InteractionTypes
        : public Constants
        , public DefaultedClassProviderCapability {
    public:
        // Deferred wrapper for one interaction-type value.
        using TInteraction = DefaultedWithDeferredResolution<int, ModuleAndClassResolver<InteractionTypes>>;

        // IPythonClassIdentityCapability implementation.
        const char* className() const override;

        // No interaction at this depth, usually terminates the path.
        static const TInteraction none;

        // Specular reflection interaction.
        static const TInteraction specular;

        // Diffuse reflection interaction.
        static const TInteraction diffuse;

        // Transmission/refraction through a material.
        static const TInteraction refraction;

        // Diffraction interaction around an edge.
        static const TInteraction diffraction;
    };

    // Accessor and mutator for path interaction colors used by Sionna's
    // visualization helpers.
    class SIONNA_BRIDGE_API InteractionTypeColors
        : public Constants
        , public BasePythonImportCapability {
    public:
        using TColor = std::tuple<float, float, float>;
        using TDeferredColor = DefaultedWithDeferredResolution<TColor, ModuleResolver<Constants>>;

        static const TDeferredColor los;
        static const TDeferredColor specular;
        static const TDeferredColor diffuse;
        static const TDeferredColor refraction;
        static const TDeferredColor diffraction;

        TColor color(const InteractionTypes::TInteraction& interaction);
        void setColor(const InteractionTypes::TInteraction& interaction, const TColor& color);
    };

} // namespace artery::sionna::py
