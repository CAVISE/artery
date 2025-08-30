#pragma once

#include <pybind11/pybind11.h>

#include <functional>
#include <optional>
#include <string>

namespace artery {

    namespace sionna {

        using Color = std::tuple<double, double, double>;

        // Maps RadioMaterialBase, serves to ease later implementations of other materials.
        // This class is abstract - sionna does not allow creation of instances.
        class RadioMaterialBase : public pybind11::object {
        public:
            // Move-only ctor. Children are expected to pass their instance here.
            RadioMaterialBase(pybind11::object object);

            // Access outer color of the material.
            Color color() const;

            // Access name, given to the material.
            std::string name() const;

            // Set outer color of the material.
            void setColor(Color color);
        };

        // Maps RadioMaterial from sionna, see https://nvlabs.github.io/sionna/rt/api/radio_materials.html#sionna.rt.RadioMaterial.
        // Current implementation assumes non-magnetic materials, so relative permeability that
        // is different to 1 will raise error.
        class RadioMaterial : public RadioMaterialBase {
        public:
            // This callback takes frequency and returns a pair of (relative_permittivity, conductivity).
            using FrequencyUpdateCallbackType = std::function<std::tuple<double, double>(double)>;

            struct CtorArguments {
                // name for material.
                const std::string& name;
                // thickness of material.
                double thickness = 0.1;
                // permittivity calculated relevant to vacuum.
                double relativePermittivity = 1.0;
                // conductivity of this material/
                double conductivity = 0.0;
                // TODO: document
                double scatteringCoefficient = 0.0;
                // TODO: document
                double xpdCoefficient = 0.0;
                // should always equal 1.
                double relativePermeability = 1.0;
                // TODO: document
                const std::string& pattern = "lambertian";
                // function for frequency -> (relative_permittivity, conductivity)
                std::optional<FrequencyUpdateCallbackType> callback = std::nullopt;
            };

            RadioMaterial(CtorArguments args);

            void setRelativePermittivity(double relativePermittivity);
            void setConductivity(double conductivity);
            void setThickness(double thickness);
            void setScatteringCoefficient(double scatteringCoefficient);
            void setXpdCoefficient(double xpdCoefficient);

            void setScatteringPattern(const std::string& pattern);
            void setFrequencyUpdateCallback(const FrequencyUpdateCallbackType& callback);

            double relativePermittivity() const;
            double conductivity() const;
            double thickness() const;
            double scatteringCoefficient() const;
            double xpdCoefficient() const;

            std::string scatteringPattern() const;

            virtual ~RadioMaterial() = default;

        private:
            pybind11::object initialize(CtorArguments args);

            pybind11::function makeFrequencyUpdateCallback(const FrequencyUpdateCallbackType& cb);
        };

    }  // namespace sionna

}  // namespace artery
