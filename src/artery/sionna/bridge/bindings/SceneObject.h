#pragma once

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Capabilities.h>
#include <artery/sionna/bridge/Helpers.h>

#include <artery/sionna/bridge/bindings/Material.h>

#include <nanobind/nanobind.h>

#include <utility>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)
NAMESPACE_BEGIN(py)

MI_VARIANT
class SceneObject
    : public SionnaRtModuleBase
    , public CachedFetchCapability
    , public WrapPythonClassCapability {
public:
    SIONNA_IMPORT_CORE_TYPES(Point3f, Vector3f, Color3f)

    const char* className() const override {
        return "SceneObject";
    }

    SceneObject() = default;

    explicit SceneObject(nb::object obj) {
        WrapPythonClassCapability::init(std::move(obj));
    }

    Point3f position() const {
        return sionna::access<Point3f>(*bound_, "position");
    }

    Vector3f orientation() const {
        return sionna::access<Vector3f>(*bound_, "orientation");
    }

    RadioMaterial<Float, Spectrum> material() const {
        return sionna::access<RadioMaterial<Float, Spectrum>>(*bound_, "material");
    }
};

NAMESPACE_END(py)

NAMESPACE_END(sionna)
NAMESPACE_END(artery)

NAMESPACE_BEGIN(nanobind)
NAMESPACE_BEGIN(detail)

template <typename Float, typename Spectrum>
struct sionna_wrap_caster_enabled<artery::sionna::py::SceneObject<Float, Spectrum>>
    : std::true_type {};

NAMESPACE_END(detail)
NAMESPACE_END(nanobind)
