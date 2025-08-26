#pragma once

#include <pybind11/pytypes.h>

#include <functional>
#include <optional>
#include <string>

namespace artery {

    namespace sionna {

        struct Color {
            Color(double r, double g, double b);

            double r;
            double g;
            double b;
        };

        // Maps RadioMaterialBase, serves to ease later implementations of other materials.
        // This class is abstract - sionna does not allow creation of instances.
        class RadioMaterialBase {
        public:
            void setColor(Color color);

            std::string name();
            Color color() const;

        protected:
            virtual ~RadioMaterialBase() = 0;

        protected:
            pybind11::object handle_;
        };

        // Maps RadioMaterial from sionna, see https://nvlabs.github.io/sionna/rt/api/radio_materials.html#sionna.rt.RadioMaterial.
        // Current implementation assumes non-magnetic materials, so relative permeability that
        // is different to 1 will raise error.
        class RadioMaterial : protected RadioMaterialBase {
        public:
            // This callback takes frequency and returns a pair of (relative_permittivity, conductivity).
            using FrequencyUpdateCallbackType = std::function<std::tuple<double, double>(double)>;

            RadioMaterial(
                const std::string& name,
                double thickness = 0.1,
                double relativePermittivity = 1.0,
                double conductivity = 0.0,
                double scatteringCoefficient = 0.0,
                double xpdCoefficient = 0.0,
                double relativePermeability = 1,
                const std::string& pattern = "lambertian",
                std::optional<FrequencyUpdateCallbackType> callback = std::nullopt);

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
            pybind11::function makeFrequencyUpdateCallback(const FrequencyUpdateCallbackType& cb);
        };

    }  // namespace sionna

}  // namespace artery