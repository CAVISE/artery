#pragma once

#include <artery/sionna/bridge/Defaulted.h>
#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Helpers.h>
#include <artery/sionna/bridge/Capabilities.h>
#include <artery/sionna/bridge/bindings/Constants.h>
#include <artery/sionna/bridge/bindings/Modules.h>

#include <nanobind/nanobind.h>

#include <string>
#include <tuple>

namespace artery {
    namespace sionna {
        namespace py {

            MI_VARIANT
            class SIONNA_BRIDGE_API RadioMaterialBase
                : public SionnaRtModuleBase
                , public ExportBoundObjectCapability {
            public:
                using ColorType = std::tuple<Float, Float, Float>;

                // IPythonClassIdentityCapability implementation.
                const char* className() const override;

                std::string materialName() const;

                ColorType color() const;
                void setColor(ColorType newColor);
            };

            MI_VARIANT
            class SIONNA_BRIDGE_API RadioMaterial
                : public DefaultedClassProviderCapability
                , public RadioMaterialBase<Float, Spectrum> {
            public:
                SIONNA_BRIDGE_IMPORT_CORE_TYPES();

                using DefaultedThicknessType = decltype(Constants<Float, Spectrum>::defaultThickness);

                // IPythonClassIdentityCapability implementation.
                const char* className() const override;

                RadioMaterial();
                explicit RadioMaterial(nb::object obj);

                RadioMaterial(
                    const std::string& name,
                    Float64 conductivity = 0.0,
                    Float64 relativePermittivity = 1.0,
                    typename DefaultedThicknessType::Argument thickness = Constants<Float, Spectrum>::defaultThickness
                );

                Float64 relativePermittivity() const;
                void setRelativePermittivity(Float64 relativePermittivity);

                Float64 conductivity() const;
                void setConductivity(Float64 conductivity);

                Float64 thickness() const;
                void setThickness(Float64 thickness);
            };


        }
    }
}

SIONNA_BRIDGE_EXTERN_CLASS(artery::sionna::py::RadioMaterialBase)
SIONNA_BRIDGE_EXTERN_CLASS(artery::sionna::py::RadioMaterial)
