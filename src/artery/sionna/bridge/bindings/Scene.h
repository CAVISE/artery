#pragma once

#include <nanobind/nanobind.h>

#include <artery/sionna/bridge/Capabilities.h>
#include <artery/sionna/bridge/bindings/Material.h>
#include <artery/sionna/bridge/bindings/SceneObject.h>

#include <unordered_map>
#include <string>

namespace artery::sionna::py {

    MI_VARIANT
    class SIONNA_BRIDGE_API SionnaScene
        : public SionnaRtModuleBase
        , public WrapPythonClassCapability {
    public:
        SIONNA_BRIDGE_IMPORT_RENDER_TYPES()

        // IPythonClassIdentityCapability implementation.
        const char* className() const override;

        SionnaScene();
        explicit SionnaScene(nanobind::object obj);

        void add(const SceneObject<Float, Spectrum>& obj);
        void add(const RadioMaterial<Float, Spectrum>& mat);
        void remove(const std::string& name);

        std::unordered_map<std::string, RadioMaterial<Float, Spectrum>> radioMaterials() const;
        std::unordered_map<std::string, SceneObject<Float, Spectrum>> sceneObjects() const;

        maybe_diff_t<Float> frequency() const;
        void setFrequency(maybe_diff_t<Float> f);

        maybe_diff_t<Float> wavelength() const;
        maybe_diff_t<Float> wavenumber() const;

        maybe_diff_t<Float> bandwidth() const;
        void setBandwidth(maybe_diff_t<Float> bw);

        maybe_diff_t<Float> temperature() const;
        void setTemperature(maybe_diff_t<Float> t);

        maybe_diff_t<Float> thermalNoisePower() const;
        maybe_diff_t<Float> angularFrequency() const;

        mitsuba::ref<Scene> miScene() const;
    };

} // namespace artery::sionna::py

SIONNA_BRIDGE_EXTERN_CLASS(artery::sionna::py::SionnaScene)
