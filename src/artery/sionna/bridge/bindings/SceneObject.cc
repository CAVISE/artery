#include "SceneObject.h"

#include "artery/sionna/bridge/Fwd.h"
#include <mitsuba/core/vector.h>
#include <mitsuba/render/mesh.h>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)
NAMESPACE_BEGIN(py)

SIONNA_INSTANTIATE_CLASS(SceneObject)

MI_VARIANT
SceneObject<Float, Spectrum>::SceneObject() = default;

MI_VARIANT
SceneObject<Float, Spectrum>::SceneObject(nb::object obj) {
    WrapPythonClassCapability::init(std::move(obj));
}

MI_VARIANT
SceneObject<Float, Spectrum>::SceneObject(mitsuba::ref<Mesh> mesh) {
    using namespace literals;
    this->InitPythonClassCapability::init("mesh"_a = std::move(mesh));
}

MI_VARIANT
SceneObject<Float, Spectrum>::SceneObject(const std::string& fname, const std::string& name, mitsuba::ref<RadioMaterial> material) {
    using namespace literals;
    this->InitPythonClassCapability::init(
        "filename"_a = fname,
        "name"_a = name,
        "material"_a = material
    );
}

MI_VARIANT
const char* SceneObject<Float, Spectrum>::className() const {
    return "SceneObject";
}

MI_VARIANT
typename mitsuba::CoreAliases<Float>::Point3f SceneObject<Float, Spectrum>::position() const {
    return sionna::access<Point3f>(bound_, "position");
}

MI_VARIANT
typename mitsuba::CoreAliases<Float>::Vector3f SceneObject<Float, Spectrum>::orientation() const {
    return sionna::access<Vector3f>(bound_, "orientation");
}

MI_VARIANT
mitsuba::ref<typename SceneObject<Float, Spectrum>::Mesh> SceneObject<Float, Spectrum>::mesh() const {
    return sionna::access<mitsuba::ref<Mesh>>(bound_, "_mesh");
}

MI_VARIANT
typename SceneObject<Float, Spectrum>::RadioMaterial SceneObject<Float, Spectrum>::material() const {
    return sionna::access<RadioMaterial>(bound_, "material");
}

NAMESPACE_END(py)
NAMESPACE_END(sionna)
NAMESPACE_END(artery)
