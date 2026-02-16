#pragma once

#include <nanobind/nanobind.h>

#include <artery/sionna/bridge/Capabilities.h>
#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Helpers.h>
#include <artery/sionna/bridge/bindings/Material.h>
#include <artery/sionna/bridge/bindings/SceneObject.h>

#include <unordered_map>
#include <string>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)
NAMESPACE_BEGIN(py)

MI_VARIANT
class SIONNA_BRIDGE_API SionnaScene
    : public SionnaRtModuleBase
    , public WrapPythonClassCapability {
public:
    SIONNA_IMPORT_CORE_TYPES(Point3f, Vector3f)
    SIONNA_IMPORT_RENDER_TYPES(Scene)
    SIONNA_IMPORT_BRIDGE_TYPES(SceneObject, RadioMaterial)

    // IPythonClassIdentityCapability implementation.
    const char* className() const override;

    SionnaScene();
    explicit SionnaScene(nb::object obj);

    void add(const SceneObject& obj);
    void add(const RadioMaterial& mat);
    void remove(const std::string& name);

    std::unordered_map<std::string, RadioMaterial> radioMaterials() const;
    std::unordered_map<std::string, SceneObject> sceneObjects() const;

    double frequency() const;
    void setFrequency(double f);

    double bandwidth() const;
    void setBandwidth(double bw);

    double temperature() const;
    void setTemperature(double t);

    mitsuba::ref<Scene> miScene() const;
};

SIONNA_EXTERN_CLASS(SionnaScene)

NAMESPACE_END(py)
NAMESPACE_END(sionna)
NAMESPACE_END(artery)
