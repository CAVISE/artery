#include <artery/sionna/bridge/Fwd.h>

#include "Constants.h"

SIONNA_BRIDGE_INSTANTIATE_CLASS(artery::sionna::py::Constants);
SIONNA_BRIDGE_INSTANTIATE_CLASS(artery::sionna::py::IntersectionTypes);

using namespace artery::sionna;

MI_VARIANT
const char* py::Constants<Float, Spectrum>::moduleName() const {
    return "sionna.rt.constants";
}

MI_VARIANT
const char* py::IntersectionTypes<Float, Spectrum>::className() const {
    return "ConstIntersectionTypes";
}

MI_VARIANT
const DefaultedWithDeferredResolution<typename mitsuba::CoreAliases<Float>::Float64, py::DefaultedModuleProviderCapability::ModuleResolver<py::Constants<Float, Spectrum>>>
    py::Constants<Float, Spectrum>::defaultThickness{
        "DEFAULT_THICKNESS",
        moduleResolver<py::Constants<Float, Spectrum>>()
    };

MI_VARIANT
const DefaultedWithDeferredResolution<typename mitsuba::CoreAliases<Float>::Int32, py::DefaultedClassProviderCapability::ModuleAndClassResolver<py::IntersectionTypes<Float, Spectrum>>>
    py::IntersectionTypes<Float, Spectrum>::none{
        "NONE",
        moduleAndClassResolver<py::IntersectionTypes<Float, Spectrum>>()
    };

MI_VARIANT
const DefaultedWithDeferredResolution<typename mitsuba::CoreAliases<Float>::Int32, py::DefaultedClassProviderCapability::ModuleAndClassResolver<py::IntersectionTypes<Float, Spectrum>>>
    py::IntersectionTypes<Float, Spectrum>::specular{
        "SPECULAR",
        moduleAndClassResolver<py::IntersectionTypes<Float, Spectrum>>()
    };

MI_VARIANT
const DefaultedWithDeferredResolution<typename mitsuba::CoreAliases<Float>::Int32, py::DefaultedClassProviderCapability::ModuleAndClassResolver<py::IntersectionTypes<Float, Spectrum>>>
    py::IntersectionTypes<Float, Spectrum>::diffuse{
        "DIFFUSE",
        moduleAndClassResolver<py::IntersectionTypes<Float, Spectrum>>()
    };

MI_VARIANT
const DefaultedWithDeferredResolution<typename mitsuba::CoreAliases<Float>::Int32, py::DefaultedClassProviderCapability::ModuleAndClassResolver<py::IntersectionTypes<Float, Spectrum>>>
    py::IntersectionTypes<Float, Spectrum>::refraction{
        "REFRACTION",
        moduleAndClassResolver<py::IntersectionTypes<Float, Spectrum>>()
    };

MI_VARIANT
const DefaultedWithDeferredResolution<typename mitsuba::CoreAliases<Float>::Int32, py::DefaultedClassProviderCapability::ModuleAndClassResolver<py::IntersectionTypes<Float, Spectrum>>>
    py::IntersectionTypes<Float, Spectrum>::diffraction{
        "DIFFRACTION",
        moduleAndClassResolver<py::IntersectionTypes<Float, Spectrum>>()
    };
