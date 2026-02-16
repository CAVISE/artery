#include "Scene.h"

#include "artery/sionna/bridge/Fwd.h"
#include <artery/sionna/bridge/Compat.h>

#include <nanobind/stl/string.h>
#include <mitsuba/render/scene.h>

using namespace artery::sionna;

MI_VARIANT
py::SionnaScene<Float, Spectrum>::SionnaScene() {
    InitPythonClassCapability::init();
}

MI_VARIANT
py::SionnaScene<Float, Spectrum>::SionnaScene(nb::object obj) {
    WrapPythonClassCapability::init(std::move(obj));
}

MI_VARIANT
const char* py::SionnaScene<Float, Spectrum>::className() const {
    return "Scene";
}

MI_VARIANT
void py::SionnaScene<Float, Spectrum>::add(const py::SceneObject<Float, Spectrum>& obj) {
    callAny(bound_, "add", "item"_a = obj.object());
}

MI_VARIANT
void py::SionnaScene<Float, Spectrum>::add(const py::RadioMaterial<Float, Spectrum>& mat) {
    callAny(bound_, "add", "item"_a = mat.object());
}

MI_VARIANT
void py::SionnaScene<Float, Spectrum>::remove(const std::string& name) {
    callAny(bound_, "remove", "name"_a = name);
}

MI_VARIANT
std::unordered_map<std::string, py::RadioMaterial<Float, Spectrum>>
py::SionnaScene<Float, Spectrum>::radioMaterials() const {
    nb::dict d = sionna::access<nb::dict>(bound_, "_radio_materials");
    return nb::cast<std::unordered_map<std::string,  py::RadioMaterial<Float, Spectrum>>>(d);
}

MI_VARIANT
std::unordered_map<std::string, py::SceneObject<Float, Spectrum>>
py::SionnaScene<Float, Spectrum>::sceneObjects() const {
    nb::dict d = sionna::access<nb::dict>(bound_, "_scene_objects");
    return nb::cast<std::unordered_map<std::string,  py::SceneObject<Float, Spectrum>>>(d);
}

MI_VARIANT
double py::SionnaScene<Float, Spectrum>::frequency() const {
    return Compat<Float, Spectrum>::toScalar(sionna::access<Float>(bound_, "frequency"));
}

MI_VARIANT
void py::SionnaScene<Float, Spectrum>::setFrequency(double f) {
    sionna::set(bound_, "frequency", static_cast<Float>(f));
}

MI_VARIANT
double py::SionnaScene<Float, Spectrum>::bandwidth() const {
    return Compat<Float, Spectrum>::toScalar(sionna::access<Float>(bound_, "bandwidth"));
}

MI_VARIANT
void py::SionnaScene<Float, Spectrum>::setBandwidth(double bw) {
    sionna::set(bound_, "bandwidth", static_cast<Float>(bw));
}

MI_VARIANT
double py::SionnaScene<Float, Spectrum>::temperature() const {
    return Compat<Float, Spectrum>::toScalar(sionna::access<Float>(bound_, "temperature"));
}

MI_VARIANT
void py::SionnaScene<Float, Spectrum>::setTemperature(double t) {
    sionna::set(bound_, "temperature", static_cast<Float>(t));
}

MI_VARIANT
mitsuba::ref<typename py::SionnaScene<Float, Spectrum>::Scene> py::SionnaScene<Float, Spectrum>::miScene() const {
    return sionna::access<mitsuba::ref<Scene>>(bound_, "_scene");
}
