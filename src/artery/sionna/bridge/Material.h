// #pragma once

// #include <string>
// #include <optional>
// #include <functional>

// #include <mitsuba/core/fwd.h>
// #include <nanobind/nanobind.h>
// #include <mitsuba/core/properties.h>

// #include "Constants.h"
// #include "artery/sionna/bridge/Helpers.h"


// namespace artery {

//     namespace sionna {

//         using Color = std::tuple<double, double, double>;

//         // Maps RadioMaterial from sionna, see https://nvlabs.github.io/sionna/rt/api/radio_materials.html#sionna.rt.RadioMaterial.
//         // Current implementation assumes non-magnetic materials, so relative permeability that
//         // is different to 1 will raise error.
//         MI_VARIANT class RadioMaterial 
//             : public RadioMaterialBase<Float, Spectrum> {
//         public:
//             // This callback takes frequency and returns a pair of (relative_permittivity, conductivity).
//             using FrequencyUpdateCallback = std::function<std::tuple<double, double>(double)>;

//             static RadioMaterial fromProperties(
//                 // name for material.
//                 const std::string& name,
//                 // thickness of material.
//                 SimpleConstant<double, false>::Argument thickness = constants::DEFAULT_THICKNESS,
//                 // permittivity calculated relevant to vacuum.
//                 Float relativePermittivity = 1.0,
//                 // conductivity of this material
//                 Float conductivity = 0.0,
//                 // TODO: document
//                 Float scatteringCoefficient = 0.0,
//                 // TODO: document
//                 Float xpdCoefficient = 0.0,
//                 // should always equal 1.
//                 Float relativePermeability = 1.0,
//                 // TODO: document
//                 const std::string& pattern = "lambertian",
//                 // function for frequency -> (relative_permittivity, conductivity)
//                 std::optional<FrequencyUpdateCallback> callback = std::nullopt
//             );

//             // Property setters.
//             void setRelativePermittivity(Float relativePermittivity);
//             void setConductivity(Float conductivity);
//             void setThickness(Float thickness);
//             void setScatteringCoefficient(Float scatteringCoefficient);
//             void setXpdCoefficient(Float xpdCoefficient);
//             void setScatteringPattern(const std::string& pattern);
//             void setFrequencyUpdateCallback(FrequencyUpdateCallback callback);

//             // Property getters.
//             Float relativePermittivity() const;
//             Float conductivity() const;
//             Float thickness() const;
//             Float scatteringCoefficient() const;
//             Float xpdCoefficient() const;
//             std::string scatteringPattern() const;
//         };

//     }  // namespace sionna

// }  // namespace artery


// MI_VARIANT artery::sionna::RadioMaterial<Float, Spectrum> artery::sionna::RadioMaterial<Float, Spectrum>::fromProperties(
//     const std::string& name,
//     SimpleConstant<double, false>::Argument thickness,
//     Float relativePermittivity,
//     Float conductivity,
//     Float scatteringCoefficient,
//     Float xpdCoefficient,
//     Float relativePermeability,
//     const std::string& pattern,
//     std::optional<FrequencyUpdateCallback> callback
// ) {
//     namespace nb = nanobind;
//     using namespace nanobind::literals;

//     if (relativePermeability != 1.0) {
//         throw wrapRuntimeError("sionna::RadioMaterial is non-magnetic, relativePermeability should be 1.0, but is %f", relativePermeability);
//     }

//     nb::object cb = nb::none();
//     if (callback.has_value()) {
//         cb = nb::cpp_function(callback.value());
//     }

//     nb::object radioMaterial = sionna::sionnaRt().attr("RadioMaterial");
//     return radioMaterial(
//         "name"_a = name,
//         "thickness"_a = thickness,
//         "relative_permittivity"_a = relativePermittivity,
//         "conductivity"_a = conductivity,
//         "scattering_coefficient"_a = scatteringCoefficient,
//         "xpd_coefficient"_a = xpdCoefficient,
//         "scattering_pattern"_a = pattern,
//         "frequency_update_callback"_a = cb
//     );
// }

// MI_VARIANT void artery::sionna::RadioMaterial<Float, Spectrum>::setRelativePermittivity(Float relativePermittivity) {
//     sionna::set(this, "relative_permittivity", relativePermittivity);
// }

// MI_VARIANT void artery::sionna::RadioMaterial<Float, Spectrum>::setConductivity(Float conductivity) {
//     sionna::set(this, "conductivity", conductivity);
// }

// MI_VARIANT void artery::sionna::RadioMaterial<Float, Spectrum>::setThickness(Float thickness) {
//     sionna::set(this, "thickness", thickness);
// }

// MI_VARIANT void artery::sionna::RadioMaterial<Float, Spectrum>::setScatteringCoefficient(Float scatteringCoefficient) {
//     sionna::set(this, "scattering_coefficient", scatteringCoefficient);
// }

// MI_VARIANT void artery::sionna::RadioMaterial<Float, Spectrum>::setXpdCoefficient(Float xpdCoefficient) {
//     sionna::set(this, "xpd_coefficient", xpdCoefficient);
// }

// MI_VARIANT void artery::sionna::RadioMaterial<Float, Spectrum>::setScatteringPattern(const std::string& pattern) {
//     sionna::set(this, "scattering_pattern", pattern);
// }

// MI_VARIANT void artery::sionna::RadioMaterial<Float, Spectrum>::setFrequencyUpdateCallback(FrequencyUpdateCallback callback) {
//     sionna::set(this, "frequency_update_callback", nanobind::cpp_function(callback));
// }

// MI_VARIANT Float artery::sionna::RadioMaterial<Float, Spectrum>::relativePermittivity() const {
//     return sionna::access<double>(this, "relative_permittivity");
// }

// MI_VARIANT Float artery::sionna::RadioMaterial<Float, Spectrum>::conductivity() const {
//     return sionna::access<double>(this, "conductivity");
// }

// MI_VARIANT Float artery::sionna::RadioMaterial<Float, Spectrum>::thickness() const {
//     return sionna::access<double>(this, "thickness");
// }

// MI_VARIANT Float artery::sionna::RadioMaterial<Float, Spectrum>::scatteringCoefficient() const {
//     return sionna::access<double>(this, "scattering_coefficient");
// }

// MI_VARIANT Float artery::sionna::RadioMaterial<Float, Spectrum>::xpdCoefficient() const {
//     return sionna::access<double>(this, "xpd_coefficient");
// }

// MI_VARIANT std::string artery::sionna::RadioMaterial<Float, Spectrum>::scatteringPattern() const {
//     return sionna::access<std::string>(this, "scattering_pattern");
// }

