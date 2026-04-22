#include "Camera.h"

#include <cavise/sionna/bridge/Helpers.h>

using namespace artery::sionna;
using namespace artery::sionna::literals;

const char* py::Camera::className() const {
    return "Camera";
}

py::Camera::Camera(nanobind::object obj) {
    WrapPythonClassCapability::init(std::move(obj));
}

py::Camera::Camera(
    const mitsuba::Resolve::Point3f& position,
    const mitsuba::Resolve::Point3f& orientation) {
    InitPythonClassCapability::init(
        "position"_a = position,
        "orientation"_a = orientation);
}

mitsuba::Resolve::Point3f py::Camera::position() const {
    return sionna::access<mitsuba::Resolve::Point3f>(bound_, "position");
}

void py::Camera::setPosition(const mitsuba::Resolve::Point3f& position) {
    sionna::set(bound_, "position", position);
}

mitsuba::Resolve::Point3f py::Camera::orientation() const {
    return sionna::access<mitsuba::Resolve::Point3f>(bound_, "orientation");
}

void py::Camera::setOrientation(const mitsuba::Resolve::Point3f& orientation) {
    sionna::set(bound_, "orientation", orientation);
}
