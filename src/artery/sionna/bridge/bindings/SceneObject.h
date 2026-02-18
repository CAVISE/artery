#pragma once

#include <nanobind/nanobind.h>

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Capabilities.h>
#include <artery/sionna/bridge/bindings/Material.h>

#include <mitsuba/core/object.h>

#include <string>

namespace artery {
    namespace sionna {
        namespace py {

            MI_VARIANT
            class SIONNA_BRIDGE_API SceneObject
                : public SionnaRtModuleBase
                , public ExportBoundObjectCapability {
            public:
                SIONNA_BRIDGE_IMPORT_RENDER_TYPES()

                // IPythonClassIdentityCapability implementation.
                const char* className() const override;

                SceneObject();
                explicit SceneObject(nb::object obj);
                explicit SceneObject(mitsuba::ref<Mesh> mesh);
                SceneObject(const std::string& fname, const std::string& name, mitsuba::ref<RadioMaterial<Float, Spectrum>> material);

                Point3f position() const;
                Vector3f orientation() const;
                mitsuba::ref<Mesh> mesh() const;
                RadioMaterial<Float, Spectrum> material() const;
            };

        }
    }
}

SIONNA_BRIDGE_EXTERN_CLASS(artery::sionna::py::SceneObject)
