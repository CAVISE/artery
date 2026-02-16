#include <mitsuba/render/mesh.h>
#include <mitsuba/core/vector.h>

#include "artery/sionna/bridge/Fwd.h"

#include "SceneObject.h"

using namespace artery::sionna;

SIONNA_BRIDGE_INSTANTIATE_CLASS(py::SceneObject)

MI_VARIANT
py::SceneObject<Float, Spectrum>::SceneObject() = default;

MI_VARIANT
py::SceneObject<Float, Spectrum>::SceneObject(nb::object obj) {
    WrapPythonClassCapability::init(std::move(obj));
}

MI_VARIANT
py::SceneObject<Float, Spectrum>::SceneObject(mitsuba::ref<Mesh> mesh) {
    using namespace literals;
    this->InitPythonClassCapability::init("mesh"_a = std::move(mesh));
}

MI_VARIANT
py::SceneObject<Float, Spectrum>::SceneObject(const std::string& fname, const std::string& name, mitsuba::ref<py::RadioMaterial<Float, Spectrum>> material) {
    using namespace literals;
    this->InitPythonClassCapability::init(
        "filename"_a = fname,
        "name"_a = name,
        "material"_a = material
    );
}

MI_VARIANT
const char* py::SceneObject<Float, Spectrum>::className() const {
    return "SceneObject";
}

MI_VARIANT
typename mitsuba::CoreAliases<Float>::Point3f py::SceneObject<Float, Spectrum>::position() const {
    return sionna::access<Point3f>(bound_, "position");
}

MI_VARIANT
typename mitsuba::CoreAliases<Float>::Vector3f py::SceneObject<Float, Spectrum>::orientation() const {
    return sionna::access<Vector3f>(bound_, "orientation");
}

MI_VARIANT
mitsuba::ref<typename py::SceneObject<Float, Spectrum>::Mesh> py::SceneObject<Float, Spectrum>::mesh() const {
    return sionna::access<mitsuba::ref<Mesh>>(bound_, "_mesh");
}

MI_VARIANT
typename py::RadioMaterial<Float, Spectrum> py::SceneObject<Float, Spectrum>::material() const {
    return sionna::access<py::RadioMaterial<Float, Spectrum>>(bound_, "material");
}
