#include "Scene.h"
#include "nanobind/nanobind.h"

#include <cavise/sionna/bridge/Helpers.h>

#include <nanobind/stl/string.h>

#include <tuple>
#include <mitsuba/render/scene.h>

using namespace artery::sionna;
using namespace artery::sionna::literals;

py::SionnaScene::SionnaScene() {
    InitPythonClassCapability::init();
}

py::SionnaScene::SionnaScene(nanobind::object obj) {
    WrapPythonClassCapability::init(std::move(obj));
}

py::SionnaScene::SionnaScene(mitsuba::ref<mitsuba::Resolve::Scene> scene) {
    InitPythonClassCapability::init("mi_scene"_a = std::move(scene));
}

const char* py::SionnaScene::className() const {
    return "Scene";
}

void py::SionnaScene::edit(const std::vector<SceneObject>& add, const std::vector<std::string>& remove) {
    sionna::call(bound_, "edit", "add"_a = add, "remove"_a = remove);
}

void py::SionnaScene::add(const Transmitter& transmitter) {
    sionna::call(bound_, "add", "item"_a = transmitter);
}

void py::SionnaScene::add(const Receiver& receiver) {
    sionna::call(bound_, "add", "item"_a = receiver);
}

void py::SionnaScene::remove(const std::string& name) {
    sionna::call(bound_, "remove", "name"_a = name);
}

void py::SionnaScene::renderToFile(
    const std::string& camera,
    const std::string& filename,
    int numSamples,
    int width,
    int height,
    std::optional<float> fov,
    std::optional<std::string> envmap,
    std::optional<Paths> paths,
    bool showDevices,
    float lightingScale) const {
    if (paths.has_value()) {
        sionna::call(
            bound_,
            "render_to_file",
            "camera"_a = camera,
            "filename"_a = filename,
            "num_samples"_a = numSamples,
            "resolution"_a = std::make_tuple(width, height),
            "fov"_a = std::move(fov),
            "envmap"_a = std::move(envmap),
            "paths"_a = *paths,
            "show_devices"_a = showDevices,
            "lighting_scale"_a = lightingScale);
    } else {
        sionna::call(
            bound_,
            "render_to_file",
            "camera"_a = camera,
            "filename"_a = filename,
            "num_samples"_a = numSamples,
            "resolution"_a = std::make_tuple(width, height),
            "fov"_a = std::move(fov),
            "envmap"_a = std::move(envmap),
            "show_devices"_a = showDevices,
            "lighting_scale"_a = lightingScale);
    }
}

void py::SionnaScene::renderToFile(
    const Camera& camera,
    const std::string& filename,
    int numSamples,
    int width,
    int height,
    std::optional<float> fov,
    std::optional<std::string> envmap,
    std::optional<Paths> paths,
    bool showDevices,
    float lightingScale) const {
    if (paths.has_value()) {
        sionna::call(
            bound_,
            "render_to_file",
            "camera"_a = camera,
            "filename"_a = filename,
            "num_samples"_a = numSamples,
            "resolution"_a = std::make_tuple(width, height),
            "fov"_a = std::move(fov),
            "envmap"_a = std::move(envmap),
            "paths"_a = *paths,
            "show_devices"_a = showDevices,
            "lighting_scale"_a = lightingScale);
    } else {
        sionna::call(
            bound_,
            "render_to_file",
            "camera"_a = camera,
            "filename"_a = filename,
            "num_samples"_a = numSamples,
            "resolution"_a = std::make_tuple(width, height),
            "fov"_a = std::move(fov),
            "envmap"_a = std::move(envmap),
            "show_devices"_a = showDevices,
            "lighting_scale"_a = lightingScale);
    }
}

py::SionnaScene::TSceneElement py::SionnaScene::get(const std::string& name) const {
    nanobind::object value = sionna::call<nanobind::object>(bound_, "get", "name"_a = name);
    if (value.is_none()) {
        return std::monostate{};
    }

    if (nanobind::isinstance<py::SceneObject>(value)) {
        return nanobind::cast<py::SceneObject>(value);
    }

    if (nanobind::isinstance<py::RadioMaterial>(value)) {
        return nanobind::cast<py::RadioMaterial>(value);
    }

    if (nanobind::isinstance<py::Transmitter>(value)) {
        return nanobind::cast<py::Transmitter>(value);
    }

    if (nanobind::isinstance<py::Receiver>(value)) {
        return nanobind::cast<py::Receiver>(value);
    }

    throw sionna::wrapRuntimeError("Scene::get: unsupported item type");
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

std::unordered_map<std::string, py::Transmitter>
py::SionnaScene::transmitters() const {
    nanobind::dict d = sionna::access<nanobind::dict>(bound_, "_transmitters");
    return nanobind::cast<std::unordered_map<std::string, py::Transmitter>>(d);
}

std::unordered_map<std::string, py::Receiver>
py::SionnaScene::receivers() const {
    nanobind::dict d = sionna::access<nanobind::dict>(bound_, "_receivers");
    return nanobind::cast<std::unordered_map<std::string, py::Receiver>>(d);
}

std::vector<std::string> py::SionnaScene::transmitterNames() const {
    nanobind::dict d = sionna::access<nanobind::dict>(bound_, "_transmitters");
    std::vector<std::string> names;
    names.reserve(d.size());
    for (auto item : d) {
        names.push_back(nanobind::cast<std::string>(item.first));
    }
    return names;
}

std::vector<std::string> py::SionnaScene::receiverNames() const {
    nanobind::dict d = sionna::access<nanobind::dict>(bound_, "_receivers");
    std::vector<std::string> names;
    names.reserve(d.size());
    for (auto item : d) {
        names.push_back(nanobind::cast<std::string>(item.first));
    }
    return names;
}

bool py::SionnaScene::hasTxArray() const {
    nanobind::object value = sionna::access<nanobind::object>(bound_, "tx_array");
    return !value.is_none();
}

py::AntennaArray py::SionnaScene::txArray() const {
    return sionna::access<py::AntennaArray>(bound_, "tx_array");
}

void py::SionnaScene::setTxArray(const AntennaArray& array) {
    sionna::set(bound_, "tx_array", array);
}

bool py::SionnaScene::hasRxArray() const {
    nanobind::object value = sionna::access<nanobind::object>(bound_, "rx_array");
    return !value.is_none();
}

py::AntennaArray py::SionnaScene::rxArray() const {
    return sionna::access<py::AntennaArray>(bound_, "rx_array");
}

void py::SionnaScene::setRxArray(const AntennaArray& array) {
    sionna::set(bound_, "rx_array", array);
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

mitsuba::ref<mitsuba::Resolve::Scene> py::SionnaScene::miScene() const {
    return sionna::access<mitsuba::ref<mitsuba::Resolve::Scene>>(bound_, "_scene");
}
