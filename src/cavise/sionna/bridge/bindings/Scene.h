#pragma once

#include <nanobind/nanobind.h>

#include <cavise/sionna/bridge/Capabilities.h>
#include <cavise/sionna/bridge/bindings/Material.h>
#include <cavise/sionna/bridge/bindings/SceneObject.h>

#include <string>
#include <unordered_map>

namespace artery::sionna::py {

    class SIONNA_BRIDGE_API SionnaScene
        : public SionnaRtModule
        , public WrapPythonClassCapability {
    public:
        // IPythonClassIdentityCapability implementation.
        const char* className() const override;

        SionnaScene();
        explicit SionnaScene(nanobind::object obj);

        void add(const SceneObject& obj);
        void add(const RadioMaterial& mat);
        void remove(const std::string& name);

        std::unordered_map<std::string, RadioMaterial> radioMaterials() const;
        std::unordered_map<std::string, SceneObject> sceneObjects() const;

        maybe_diff_t<mitsuba::Resolve::Float> frequency() const;
        void setFrequency(maybe_diff_t<mitsuba::Resolve::Float> f);

        maybe_diff_t<mitsuba::Resolve::Float> wavelength() const;
        maybe_diff_t<mitsuba::Resolve::Float> wavenumber() const;

        maybe_diff_t<mitsuba::Resolve::Float> bandwidth() const;
        void setBandwidth(maybe_diff_t<mitsuba::Resolve::Float> bw);

        maybe_diff_t<mitsuba::Resolve::Float> temperature() const;
        void setTemperature(maybe_diff_t<mitsuba::Resolve::Float> t);

        maybe_diff_t<mitsuba::Resolve::Float> thermalNoisePower() const;
        maybe_diff_t<mitsuba::Resolve::Float> angularFrequency() const;

        mitsuba::ref<mitsuba::Resolve::Scene> miScene() const;
    };

} // namespace artery::sionna::py
