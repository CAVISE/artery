#include "RadioDevice.h"

#include <cavise/sionna/bridge/Helpers.h>

using namespace artery::sionna;
using namespace artery::sionna::literals;

py::RadioDevice::RadioDevice(nanobind::object obj) {
    WrapPythonClassCapability::init(std::move(obj));
}

std::string py::RadioDevice::name() const {
    return sionna::access<std::string>(bound_, "name");
}

mitsuba::Resolve::Point3f py::RadioDevice::position() const {
    return sionna::access<mitsuba::Resolve::Point3f>(bound_, "position");
}

void py::RadioDevice::setPosition(const mitsuba::Resolve::Point3f& position) {
    sionna::set(bound_, "position", position);
}

mitsuba::Resolve::Point3f py::RadioDevice::orientation() const {
    return sionna::access<mitsuba::Resolve::Point3f>(bound_, "orientation");
}

void py::RadioDevice::setOrientation(const mitsuba::Resolve::Point3f& orientation) {
    sionna::set(bound_, "orientation", orientation);
}

mitsuba::Resolve::Vector3f py::RadioDevice::velocity() const {
    return sionna::access<mitsuba::Resolve::Vector3f>(bound_, "velocity");
}

void py::RadioDevice::setVelocity(const mitsuba::Resolve::Vector3f& velocity) {
    sionna::set(bound_, "velocity", velocity);
}

auto py::RadioDevice::color() const -> TColor {
    return sionna::access<TColor>(bound_, "color");
}

void py::RadioDevice::setColor(TColor color) {
    sionna::set(bound_, "color", std::move(color));
}

void py::RadioDevice::lookAt(const mitsuba::Resolve::Point3f& target) {
    sionna::call(bound_, "look_at", "target"_a = target);
}

const char* py::Transmitter::className() const {
    return "Transmitter";
}

py::Transmitter::Transmitter(nanobind::object obj)
    : RadioDevice(std::move(obj)) {
}

py::Transmitter::Transmitter(
    const std::string& name,
    const mitsuba::Resolve::Point3f& position,
    std::optional<mitsuba::Resolve::Point3f> orientation,
    std::optional<mitsuba::Resolve::Vector3f> velocity,
    float powerDbm) {
    InitPythonClassCapability::init(
        "name"_a = name,
        "position"_a = position,
        "orientation"_a = std::move(orientation),
        "velocity"_a = std::move(velocity),
        "power_dbm"_a = powerDbm);
}

maybe_diff_t<mitsuba::Resolve::Float> py::Transmitter::power() const {
    return sionna::access<maybe_diff_t<mitsuba::Resolve::Float>>(bound_, "power");
}

maybe_diff_t<mitsuba::Resolve::Float> py::Transmitter::powerDbm() const {
    return sionna::access<maybe_diff_t<mitsuba::Resolve::Float>>(bound_, "power_dbm");
}

void py::Transmitter::setPowerDbm(maybe_diff_t<mitsuba::Resolve::Float> powerDbm) {
    sionna::set(bound_, "power_dbm", powerDbm);
}

const char* py::Receiver::className() const {
    return "Receiver";
}

py::Receiver::Receiver(nanobind::object obj)
    : RadioDevice(std::move(obj)) {
}

py::Receiver::Receiver(
    const std::string& name,
    const mitsuba::Resolve::Point3f& position,
    std::optional<mitsuba::Resolve::Point3f> orientation,
    std::optional<mitsuba::Resolve::Vector3f> velocity) {
    InitPythonClassCapability::init(
        "name"_a = name,
        "position"_a = position,
        "orientation"_a = std::move(orientation),
        "velocity"_a = std::move(velocity));
}
