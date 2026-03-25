#pragma once

#include <artery/sionna/bridge/Compat.h>
#include <artery/sionna/bridge/Capabilities.h>
#include <artery/sionna/bridge/bindings/Constants.h>
#include <artery/sionna/bridge/bindings/Modules.h>

#include <drjit/array_traits.h>
#include <nanobind/nanobind.h>

#include <string>
#include <tuple>

namespace artery::sionna::py {

    MI_VARIANT
    class SIONNA_BRIDGE_API RadioMaterialBase
        : public SionnaRtModuleBase
        , public ExportBoundObjectCapability {
    public:
        using TColor = std::tuple<float, float, float>;

        // IPythonClassIdentityCapability implementation.
        const char* className() const override;

        // returns material name.
        std::string materialName() const;

        // color getter.
        TColor color() const;

        // color setter.
        void setColor(TColor newColor);
    };

    MI_VARIANT
    class SIONNA_BRIDGE_API RadioMaterial
        : public DefaultedClassProviderCapability
        , public RadioMaterialBase<Float, Spectrum> {
    public:
        SIONNA_BRIDGE_IMPORT_CORE_TYPES();

        using Constants = py::Constants<Float, Spectrum>;

        // IPythonClassIdentityCapability implementation.
        const char* className() const override;

        RadioMaterial();
        explicit RadioMaterial(nanobind::object obj);

        RadioMaterial(
            const std::string& name,
            Float64 conductivity = 0.0,
            Float64 relativePermittivity = 1.0,
            typename Defaulted<Float64>::Argument thickness = Constants::defaultThickness);

        maybe_diff_t<Float> relativePermittivity() const;
        void setRelativePermittivity(maybe_diff_t<Float> relativePermittivity);

        maybe_diff_t<Float> conductivity() const;
        void setConductivity(maybe_diff_t<Float> conductivity);

        maybe_diff_t<Float> thickness() const;
        void setThickness(maybe_diff_t<Float> thickness);
    };

} // namespace artery::sionna::py

SIONNA_BRIDGE_EXTERN_CLASS(artery::sionna::py::RadioMaterialBase)
SIONNA_BRIDGE_EXTERN_CLASS(artery::sionna::py::RadioMaterial)
