#pragma once

#include <nanobind/nanobind.h>

#include <artery/sionna/bridge/Capabilities.h>
#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Helpers.h>
#include <artery/sionna/bridge/bindings/Material.h>
#include <artery/sionna/bridge/bindings/SceneObject.h>

#include <unordered_map>
#include <string>

namespace artery {
    namespace sionna {
        namespace py {

            MI_VARIANT
            class SIONNA_BRIDGE_API SionnaScene
                : public SionnaRtModuleBase
                , public WrapPythonClassCapability {
            public:
                SIONNA_BRIDGE_IMPORT_RENDER_TYPES()

                // IPythonClassIdentityCapability implementation.
                const char* className() const override;

                SionnaScene();
                explicit SionnaScene(nb::object obj);

                void add(const SceneObject<Float, Spectrum>& obj);
                void add(const RadioMaterial<Float, Spectrum>& mat);
                void remove(const std::string& name);

                std::unordered_map<std::string, RadioMaterial<Float, Spectrum>> radioMaterials() const;
                std::unordered_map<std::string, SceneObject<Float, Spectrum>> sceneObjects() const;

                double frequency() const;
                void setFrequency(double f);

                double bandwidth() const;
                void setBandwidth(double bw);

                double temperature() const;
                void setTemperature(double t);

                mitsuba::ref<Scene> miScene() const;
            };

        }
    }
}

SIONNA_BRIDGE_EXTERN_CLASS(artery::sionna::py::SionnaScene)
