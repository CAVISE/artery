#include "SceneObject.h"
#include "artery/sionna/bridge/Fwd.h"

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
const char* SceneObject<Float, Spectrum>::className() const {
    return "SceneObject";
}

MI_VARIANT
typename SceneObject<Float, Spectrum>::Point3f SceneObject<Float, Spectrum>::position() const {
    return sionna::access<Point3f>(*bound_, "position");
}

MI_VARIANT
typename SceneObject<Float, Spectrum>::Vector3f SceneObject<Float, Spectrum>::orientation() const {
    return sionna::access<Vector3f>(*bound_, "orientation");
}

MI_VARIANT
RadioMaterial<Float, Spectrum> SceneObject<Float, Spectrum>::material() const {
    return sionna::access<RadioMaterial<Float, Spectrum>>(*bound_, "material");
}

NAMESPACE_END(py)
NAMESPACE_END(sionna)
NAMESPACE_END(artery)
