#include <mitsuba/render/mesh.h>
#include <mitsuba/core/vector.h>

#include <cavise/sionna/bridge/Helpers.h>

#include "SceneObject.h"

using namespace artery::sionna;
using namespace artery::sionna::literals;

py::SceneObject::SceneObject(nanobind::object obj) {
    WrapPythonClassCapability::init(std::move(obj));
}

py::SceneObject::SceneObject(mitsuba::ref<mitsuba::Resolve::Mesh> mesh) {
    using namespace literals;
    this->InitPythonClassCapability::init("mesh"_a = std::move(mesh));
}

py::SceneObject::SceneObject(const std::string& fname, const std::string& name, const py::RadioMaterial& material) {
    using namespace literals;
    this->InitPythonClassCapability::init(
        "fname"_a = fname,
        "name"_a = name,
        "radio_material"_a = material);
}

const char* py::SceneObject::className() const {
    return "SceneObject";
}

typename mitsuba::Resolve::Point3f py::SceneObject::position() const {
    return sionna::access<mitsuba::Resolve::Point3f>(bound_, "position");
}

typename mitsuba::Resolve::Point3f py::SceneObject::orientation() const {
    return sionna::access<mitsuba::Resolve::Point3f>(bound_, "orientation");
}

mitsuba::ref<mitsuba::Resolve::Mesh> py::SceneObject::mesh() const {
    return sionna::access<mitsuba::ref<mitsuba::Resolve::Mesh>>(bound_, "_mesh");
}

py::RadioMaterial py::SceneObject::material() const {
    return sionna::access<py::RadioMaterial>(bound_, "radio_material");
}
