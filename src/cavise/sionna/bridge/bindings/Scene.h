#pragma once

#include <nanobind/nanobind.h>

#include <cavise/sionna/bridge/bindings/Camera.h>
#include <cavise/sionna/bridge/bindings/Material.h>
#include <cavise/sionna/bridge/bindings/SceneObject.h>
#include <cavise/sionna/bridge/capabilities/Calling.h>

#include <optional>
#include <string>
#include <unordered_map>
#include <variant>

namespace artery::sionna::py {

    class SIONNA_BRIDGE_API SionnaScene
        : public SionnaRtModule
        , public InitPythonClassCapability {
    public:
        // IPythonClassIdentityCapability implementation.
        const char* className() const override;

        SionnaScene();
        explicit SionnaScene(nanobind::object obj);
        explicit SionnaScene(mitsuba::ref<mitsuba::Resolve::Scene> scene);

        using SceneElement = std::variant<std::monostate, SceneObject, RadioMaterial>;

        void edit(const std::vector<SceneObject>& add, const std::vector<std::string>& remove);
        void renderToFile(
            const std::string& camera,
            const std::string& filename,
            int numSamples = 512,
            int width = 655,
            int height = 500,
            std::optional<float> fov = std::nullopt,
            std::optional<std::string> envmap = std::nullopt,
            float lightingScale = 1.0f) const;
        void renderToFile(
            const Camera& camera,
            const std::string& filename,
            int numSamples = 512,
            int width = 655,
            int height = 500,
            std::optional<float> fov = std::nullopt,
            std::optional<std::string> envmap = std::nullopt,
            float lightingScale = 1.0f) const;
        SceneElement get(const std::string& name) const;

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
