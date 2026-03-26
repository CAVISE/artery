#include "Scene.h"

#include <cavise/sionna/bridge/Helpers.h>

#include <nanobind/stl/string.h>
#include <mitsuba/render/scene.h>

using namespace artery::sionna;
using namespace artery::sionna::literals;

py::SionnaScene::SionnaScene() {
    InitPythonClassCapability::init();
}

py::SionnaScene::SionnaScene(nanobind::object obj) {
    WrapPythonClassCapability::init(std::move(obj));
}

const char* py::SionnaScene::className() const {
    return "Scene";
}

void py::SionnaScene::add(const py::SceneObject& obj) {
    callAny(bound_, "add", "item"_a = obj.object());
}

void py::SionnaScene::add(const py::RadioMaterial& mat) {
    callAny(bound_, "add", "item"_a = mat.object());
}

void py::SionnaScene::remove(const std::string& name) {
    callAny(bound_, "remove", "name"_a = name);
}

std::unordered_map<std::string, py::RadioMaterial>
py::SionnaScene::radioMaterials() const {
    nanobind::dict d = sionna::access<nanobind::dict>(bound_, "_radio_materials");
    return nanobind::cast<std::unordered_map<std::string, py::RadioMaterial>>(d);
}

std::unordered_map<std::string, py::SceneObject>
py::SionnaScene::sceneObjects() const {
    nanobind::dict d = sionna::access<nanobind::dict>(bound_, "_scene_objects");
    return nanobind::cast<std::unordered_map<std::string, py::SceneObject>>(d);
}

maybe_diff_t<mitsuba::Resolve::Float> py::SionnaScene::frequency() const {
    return sionna::access<maybe_diff_t<mitsuba::Resolve::Float>>(bound_, "frequency");
}

void py::SionnaScene::setFrequency(maybe_diff_t<mitsuba::Resolve::Float> f) {
    sionna::set(bound_, "frequency", f);
}

maybe_diff_t<mitsuba::Resolve::Float> py::SionnaScene::wavelength() const {
    return sionna::access<maybe_diff_t<mitsuba::Resolve::Float>>(bound_, "wavelength");
}

maybe_diff_t<mitsuba::Resolve::Float> py::SionnaScene::wavenumber() const {
    return sionna::access<maybe_diff_t<mitsuba::Resolve::Float>>(bound_, "wavenumber");
}

maybe_diff_t<mitsuba::Resolve::Float> py::SionnaScene::bandwidth() const {
    return sionna::access<maybe_diff_t<mitsuba::Resolve::Float>>(bound_, "bandwidth");
}

void py::SionnaScene::setBandwidth(maybe_diff_t<mitsuba::Resolve::Float> bw) {
    sionna::set(bound_, "bandwidth", bw);
}

maybe_diff_t<mitsuba::Resolve::Float> py::SionnaScene::temperature() const {
    return sionna::access<maybe_diff_t<mitsuba::Resolve::Float>>(bound_, "temperature");
}

void py::SionnaScene::setTemperature(maybe_diff_t<mitsuba::Resolve::Float> t) {
    sionna::set(bound_, "temperature", t);
}

maybe_diff_t<mitsuba::Resolve::Float> py::SionnaScene::thermalNoisePower() const {
    return sionna::access<maybe_diff_t<mitsuba::Resolve::Float>>(bound_, "thermal_noise_power");
}

maybe_diff_t<mitsuba::Resolve::Float> py::SionnaScene::angularFrequency() const {
    return sionna::access<maybe_diff_t<mitsuba::Resolve::Float>>(bound_, "angular_frequency");
}

mitsuba::ref<mitsuba::Resolve::Scene> py::SionnaScene::miScene() const {
    return sionna::access<mitsuba::ref<mitsuba::Resolve::Scene>>(bound_, "_scene");
}
