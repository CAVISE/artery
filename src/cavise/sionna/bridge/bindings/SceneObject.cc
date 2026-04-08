#include <nanobind/nanobind.h>

#include <mitsuba/render/mesh.h>
#include <mitsuba/core/vector.h>

#include <cavise/sionna/bridge/Helpers.h>

#include "SceneObject.h"

using namespace artery::sionna;
using namespace artery::sionna::literals;

py::SceneObject::SceneObject(nanobind::object obj) {
    WrapPythonClassCapability::init(std::move(obj));
}

py::SceneObject::SceneObject(mitsuba::ref<mitsuba::Resolve::Mesh> mesh, const py::RadioMaterial& material) {
    using namespace literals;
    this->InitPythonClassCapability::init(
        "mi_mesh"_a = mesh.get(),
        "radio_material"_a = material);
}

py::SceneObject::SceneObject(mitsuba::ref<mitsuba::Resolve::Mesh> mesh, const std::string& name, const py::RadioMaterial& material) {
    using namespace literals;
    this->InitPythonClassCapability::init(
        "mi_mesh"_a = mesh.get(),
        "name"_a = name,
        "radio_material"_a = material);
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

std::string py::SceneObject::name() const {
    return sionna::access<std::string>(bound_, "name");
}

typename mitsuba::Resolve::Point3f py::SceneObject::position() const {
    return sionna::access<mitsuba::Resolve::Point3f>(bound_, "position");
}

typename mitsuba::Resolve::Point3f py::SceneObject::orientation() const {
    return sionna::access<mitsuba::Resolve::Point3f>(bound_, "orientation");
}

typename mitsuba::Resolve::Vector3f py::SceneObject::velocity() const {
    return sionna::access<mitsuba::Resolve::Vector3f>(bound_, "velocity");
}

mitsuba::ref<mitsuba::Resolve::Mesh> py::SceneObject::mesh() const {
    return sionna::access<mitsuba::ref<mitsuba::Resolve::Mesh>>(bound_, "_mi_mesh");
}

py::RadioMaterial py::SceneObject::material() const {
    return sionna::access<py::RadioMaterial>(bound_, "radio_material");
}

void py::SceneObject::setPosition(const mitsuba::Resolve::Point3f& position) {
    sionna::set(bound_, "position", position);
}

void py::SceneObject::setOrientation(const mitsuba::Resolve::Point3f& orientation) {
    sionna::set(bound_, "orientation", orientation);
}

void py::SceneObject::setVelocity(const mitsuba::Resolve::Vector3f& velocity) {
    sionna::set(bound_, "velocity", velocity);
}

void py::SceneObject::setMaterial(const py::RadioMaterial& material) {
    sionna::set(bound_, "radio_material", material);
}
