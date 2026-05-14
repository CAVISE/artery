#include "Constants.h"

#include <cavise/sionna/bridge/Compat.h>
#include <cavise/sionna/bridge/Helpers.h>

namespace nb = nanobind;

using namespace artery::sionna;

const char* py::Constants::moduleName() const {
    return "sionna.rt.constants";
}

const char* py::InteractionTypes::className() const {
    return "InteractionType";
}

const DefaultedWithDeferredResolution<typename mi::Float64, py::DefaultedModuleProviderCapability::ModuleResolver<py::Constants>>
    py::Constants::defaultThickness{
        "DEFAULT_THICKNESS",
        moduleResolver<py::Constants>()};

const py::InteractionTypes::TInteraction
    py::InteractionTypes::none{
        "NONE",
        moduleAndClassResolver<py::InteractionTypes>()};

const py::InteractionTypes::TInteraction
    py::InteractionTypes::specular{
        "SPECULAR",
        moduleAndClassResolver<py::InteractionTypes>()};

const py::InteractionTypes::TInteraction
    py::InteractionTypes::diffuse{
        "DIFFUSE",
        moduleAndClassResolver<py::InteractionTypes>()};

const py::InteractionTypes::TInteraction
    py::InteractionTypes::refraction{
        "REFRACTION",
        moduleAndClassResolver<py::InteractionTypes>()};

const py::InteractionTypes::TInteraction
    py::InteractionTypes::diffraction{
        "DIFFRACTION",
        moduleAndClassResolver<py::InteractionTypes>()};

const py::InteractionTypeColors::TDeferredColor py::InteractionTypeColors::los{
    "LOS_COLOR",
    moduleResolver<py::Constants>()};

const py::InteractionTypeColors::TDeferredColor py::InteractionTypeColors::specular{
    "SPECULAR_COLOR",
    moduleResolver<py::Constants>()};

const py::InteractionTypeColors::TDeferredColor py::InteractionTypeColors::diffuse{
    "DIFFUSE_COLOR",
    moduleResolver<py::Constants>()};

const py::InteractionTypeColors::TDeferredColor py::InteractionTypeColors::refraction{
    "REFRACTION_COLOR",
    moduleResolver<py::Constants>()};

const py::InteractionTypeColors::TDeferredColor py::InteractionTypeColors::diffraction{
    "DIFFRACTION_COLOR",
    moduleResolver<py::Constants>()};

py::InteractionTypeColors::TColor py::InteractionTypeColors::color(const InteractionTypes::TInteraction& interaction) {
    const nb::dict mapping = sionna::access<nb::dict>(module(), "INTERACTION_TYPE_TO_COLOR");
    if (const auto key = interaction.value(); key == InteractionTypes::none.value()) {
        return nb::cast<TColor>(mapping[nb::none()]);
    } else {
        return nb::cast<TColor>(mapping[nb::cast(key)]);
    }
}

void py::InteractionTypeColors::setColor(const InteractionTypes::TInteraction& interaction, const TColor& color) {
    nb::dict mapping = sionna::access<nb::dict>(module(), "INTERACTION_TYPE_TO_COLOR");
    if (const auto key = interaction.value(); key == InteractionTypes::none.value()) {
        mapping[nb::none()] = nb::cast(color);
    } else {
        mapping[nb::cast(key)] = nb::cast(color);
    }
}
