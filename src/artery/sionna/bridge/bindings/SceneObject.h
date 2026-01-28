#pragma once

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Capabilities.h>

#include <artery/sionna/bridge/bindings/Material.h>

#include <nanobind/nanobind.h>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)
NAMESPACE_BEGIN(py)

MI_VARIANT
class SceneObject
    : public SionnaRtModuleBase
    , public WrapPythonClassCapability {
public:
    SIONNA_IMPORT_CORE_TYPES(Point3f, Vector3f, Color3f)

    // IPythonClassIdentityCapability implementation.
    const char* className() const override;

    SceneObject();
    explicit SceneObject(nb::object obj);

    Point3f position() const;
    Vector3f orientation() const;
    RadioMaterial<Float, Spectrum> material() const;
};

SIONNA_EXTERN_CLASS(SceneObject)

NAMESPACE_END(py)

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
