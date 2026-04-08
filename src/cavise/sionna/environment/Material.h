#pragma once

#include <nanobind/nanobind.h>
#include <inet/environment/common/Material.h>

#include <cavise/sionna/bridge/SionnaBridge.h>

#include <string>

namespace artery::sionna {

    class RadioMaterial
        : public inet::physicalenvironment::Material {
    public:
        RadioMaterial(
            const std::string& name,
            mitsuba::Resolve::Float64 conductivity = 0.0,
            mitsuba::Resolve::Float64 relativePermittivity = 1.0,
            typename Defaulted<mitsuba::Resolve::Float64>::Argument thickness = py::Constants::defaultThickness);

        explicit RadioMaterial(nanobind::object obj);
        explicit RadioMaterial(py::RadioMaterial material);

        // inet::physicalenvironment::IMaterial implementation.
        inet::physicalenvironment::Ohmm getResistivity() const override;
        double getRelativePermittivity() const override;
        double getRelativePermeability() const override;

        double getDielectricLossTangent(inet::physicalenvironment::Hz frequency) const override;
        double getRefractiveIndex() const override;
        inet::physicalenvironment::mps getPropagationSpeed() const override;

        py::RadioMaterial& object();
        const py::RadioMaterial& object() const;

    private:
        py::RadioMaterial py_;
    };

} // namespace artery::sionna
