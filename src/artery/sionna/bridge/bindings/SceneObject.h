#pragma once

#include <nanobind/nanobind.h>

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Capabilities.h>
#include <artery/sionna/bridge/bindings/Material.h>

#include <mitsuba/core/object.h>

#include <string>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)
NAMESPACE_BEGIN(py)

MI_VARIANT
class SIONNA_BRIDGE_API SceneObject
    : public SionnaRtModuleBase
    , public ExportBoundObjectCapability {
public:
    SIONNA_IMPORT_CORE_TYPES(Point3f, Vector3f, Color3f)
    SIONNA_IMPORT_RENDER_TYPES(Mesh)
    SIONNA_IMPORT_BRIDGE_TYPES(RadioMaterial)

    // IPythonClassIdentityCapability implementation.
    const char* className() const override;

    SceneObject();
    explicit SceneObject(nb::object obj);
    explicit SceneObject(mitsuba::ref<Mesh> mesh);
    SceneObject(const std::string& fname, const std::string& name, mitsuba::ref<RadioMaterial> material);

    Point3f position() const;
    Vector3f orientation() const;
    mitsuba::ref<Mesh> mesh() const;
    RadioMaterial material() const;
};

SIONNA_EXTERN_CLASS(SceneObject)

NAMESPACE_END(py)

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
