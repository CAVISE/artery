#include "Scene.h"

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Compat.h>
#include <artery/sionna/bridge/Helpers.h>

#include <nanobind/stl/string.h>
#include <mitsuba/render/scene.h>

using namespace artery::sionna;
using namespace artery::sionna::literals;

SIONNA_BRIDGE_INSTANTIATE_CLASS(artery::sionna::py::SionnaScene)

MI_VARIANT
py::SionnaScene<Float, Spectrum>::SionnaScene() {
    InitPythonClassCapability::init();
}

MI_VARIANT
py::SionnaScene<Float, Spectrum>::SionnaScene(nanobind::object obj) {
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
    nanobind::dict d = sionna::access<nanobind::dict>(bound_, "_radio_materials");
    return nanobind::cast<std::unordered_map<std::string, py::RadioMaterial<Float, Spectrum>>>(d);
}

MI_VARIANT
std::unordered_map<std::string, py::SceneObject<Float, Spectrum>>
py::SionnaScene<Float, Spectrum>::sceneObjects() const {
    nanobind::dict d = sionna::access<nanobind::dict>(bound_, "_scene_objects");
    return nanobind::cast<std::unordered_map<std::string, py::SceneObject<Float, Spectrum>>>(d);
}

MI_VARIANT
maybe_diff_t<Float> py::SionnaScene<Float, Spectrum>::frequency() const {
    return sionna::access<maybe_diff_t<Float>>(bound_, "frequency");
}

MI_VARIANT
void py::SionnaScene<Float, Spectrum>::setFrequency(maybe_diff_t<Float> f) {
    sionna::set(bound_, "frequency", f);
}

MI_VARIANT
maybe_diff_t<Float> py::SionnaScene<Float, Spectrum>::wavelength() const {
    return sionna::access<maybe_diff_t<Float>>(bound_, "wavelength");
}

MI_VARIANT
maybe_diff_t<Float> py::SionnaScene<Float, Spectrum>::wavenumber() const {
    return sionna::access<maybe_diff_t<Float>>(bound_, "wavenumber");
}

MI_VARIANT
maybe_diff_t<Float> py::SionnaScene<Float, Spectrum>::bandwidth() const {
    return sionna::access<maybe_diff_t<Float>>(bound_, "bandwidth");
}

MI_VARIANT
void py::SionnaScene<Float, Spectrum>::setBandwidth(maybe_diff_t<Float> bw) {
    sionna::set(bound_, "bandwidth", bw);
}

MI_VARIANT
maybe_diff_t<Float> py::SionnaScene<Float, Spectrum>::temperature() const {
    return sionna::access<maybe_diff_t<Float>>(bound_, "temperature");
}

MI_VARIANT
void py::SionnaScene<Float, Spectrum>::setTemperature(maybe_diff_t<Float> t) {
    sionna::set(bound_, "temperature", t);
}

MI_VARIANT
maybe_diff_t<Float> py::SionnaScene<Float, Spectrum>::thermalNoisePower() const {
    return sionna::access<maybe_diff_t<Float>>(bound_, "thermal_noise_power");
}

MI_VARIANT
maybe_diff_t<Float> py::SionnaScene<Float, Spectrum>::angularFrequency() const {
    return sionna::access<maybe_diff_t<Float>>(bound_, "angular_frequency");
}

MI_VARIANT
mitsuba::ref<typename py::SionnaScene<Float, Spectrum>::Scene> py::SionnaScene<Float, Spectrum>::miScene() const {
    return sionna::access<mitsuba::ref<Scene>>(bound_, "_scene");
}
