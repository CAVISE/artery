#pragma once

#include <cavise/sionna/bridge/Compat.h>
#include <cavise/sionna/bridge/Capabilities.h>
#include <cavise/sionna/bridge/bindings/Constants.h>

#include <drjit/array_traits.h>
#include <nanobind/nanobind.h>

#include <string>
#include <tuple>

namespace artery::sionna::py {

    class SIONNA_BRIDGE_API RadioMaterialBase
        : public SionnaRtModule
        , public ExportBoundObjectCapability {
    public:
        using TColor = std::tuple<float, float, float>;

        // IPythonClassIdentityCapability implementation.
        const char* className() const override;

        // Returns material name.
        std::string materialName() const;

        TColor color() const;
        void setColor(TColor newColor);
    };

    class SIONNA_BRIDGE_API RadioMaterial
        : public DefaultedClassProviderCapability
        , public RadioMaterialBase {
    public:
        // IPythonClassIdentityCapability implementation.
        const char* className() const override;

        RadioMaterial();
        explicit RadioMaterial(nanobind::object obj);

        RadioMaterial(
            const std::string& name,
            mitsuba::Resolve::Float64 conductivity = 0.0,
            mitsuba::Resolve::Float64 relativePermittivity = 1.0,
            typename Defaulted<mitsuba::Resolve::Float64>::Argument thickness = Constants::defaultThickness);

        maybe_diff_t<mitsuba::Resolve::Float> relativePermittivity() const;
        void setRelativePermittivity(maybe_diff_t<mitsuba::Resolve::Float> relativePermittivity);

        maybe_diff_t<mitsuba::Resolve::Float> conductivity() const;
        void setConductivity(maybe_diff_t<mitsuba::Resolve::Float> conductivity);

        maybe_diff_t<mitsuba::Resolve::Float> thickness() const;
        void setThickness(maybe_diff_t<mitsuba::Resolve::Float> thickness);
    };

} // namespace artery::sionna::py
