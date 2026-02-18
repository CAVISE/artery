#pragma once

#include "artery/sionna/bridge/Fwd.h"
#include <nanobind/nanobind.h>

#include <artery/sionna/bridge/SionnaBridge.h>

#include <inet/common/INETDefs.h>
#include <inet/environment/common/Material.h>

#include <string>

namespace artery {

    namespace sionna {

        MI_VARIANT
        class InetRadioMaterial
            : public inet::physicalenvironment::Material {
        public:
            SIONNA_BRIDGE_IMPORT_BRIDGE_TYPES()

            using DefaultedThicknessType = decltype(Constants::defaultThickness);

            /**
            * @brief Sionna radio material constructor. Current implementation assumes non-magnetic materials, so relative permeability that
            * is different to 1 will raise error.
            */
            InetRadioMaterial(
                const std::string& name,
                Float64 conductivity = 0.0,
                Float64 relativePermittivity = 1.0,
                typename DefaultedThicknessType::Argument thickness = Constants::defaultThickness
            );

            explicit InetRadioMaterial(nanobind::object obj);
            explicit InetRadioMaterial(py::RadioMaterial<Float, Spectrum> material);

            // inet::physicalenvironment::IMaterial implementation.
            inet::physicalenvironment::Ohmm getResistivity() const override;
            double getRelativePermittivity() const override;
            double getRelativePermeability() const override;

            double getDielectricLossTangent(inet::physicalenvironment::Hz frequency) const override;
            double getRefractiveIndex() const override;
            inet::physicalenvironment::mps getPropagationSpeed() const override;

            py::RadioMaterial<Float, Spectrum>& object();
            const py::RadioMaterial<Float, Spectrum>& object() const;

        private:
            py::RadioMaterial<Float, Spectrum> py_;
        };

    }

}

SIONNA_BRIDGE_EXTERN_CLASS(artery::sionna::InetRadioMaterial)
