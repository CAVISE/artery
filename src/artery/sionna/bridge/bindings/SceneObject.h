#pragma once

#include <nanobind/nanobind.h>

#include <artery/sionna/bridge/Capabilities.h>
#include <artery/sionna/bridge/bindings/Material.h>

#include <mitsuba/core/object.h>

#include <string>

namespace artery::sionna::py {

    MI_VARIANT
    class SIONNA_BRIDGE_API SceneObject
        : public SionnaRtModuleBase
        , public ExportBoundObjectCapability {
    public:
        SIONNA_BRIDGE_IMPORT_RENDER_TYPES()

        // IPythonClassIdentityCapability implementation.
        const char* className() const override;

        explicit SceneObject(nanobind::object obj);
        explicit SceneObject(mitsuba::ref<Mesh> mesh);
        SceneObject(const std::string& fname, const std::string& name, const RadioMaterial<Float, Spectrum>& material);

        Point3f position() const;
        Point3f orientation() const;
        mitsuba::ref<Mesh> mesh() const;
        RadioMaterial<Float, Spectrum> material() const;
    };

} // namespace artery::sionna::py

SIONNA_BRIDGE_EXTERN_CLASS(artery::sionna::py::SceneObject)
