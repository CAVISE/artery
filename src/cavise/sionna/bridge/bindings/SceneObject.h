#pragma once

// Ensure nanobind included first for nb::ref
#include <nanobind/nanobind.h>

#include <cavise/sionna/bridge/bindings/Modules.h>
#include <cavise/sionna/bridge/bindings/Material.h>
#include <cavise/sionna/bridge/capabilities/Core.h>
#include <cavise/sionna/bridge/capabilities/Calling.h>

#include <mitsuba/core/object.h>

#include <string>

namespace artery::sionna::py {

    class SIONNA_BRIDGE_API SceneObject
        : public SionnaRtModule
        , public InitPythonClassCapability {
    public:
        // IPythonClassIdentityCapability implementation.
        const char* className() const override;

        SceneObject() = default;
        explicit SceneObject(nanobind::object obj);
        SceneObject(mitsuba::ref<mitsuba::Resolve::Mesh> mesh, const RadioMaterial& material);
        SceneObject(mitsuba::ref<mitsuba::Resolve::Mesh> mesh, const std::string& name, const RadioMaterial& material);
        SceneObject(const std::string& fname, const std::string& name, const RadioMaterial& material);

        std::string name() const;
        mitsuba::Resolve::Point3f position() const;
        mitsuba::Resolve::Point3f orientation() const;
        mitsuba::Resolve::Vector3f velocity() const;
        mitsuba::ref<mitsuba::Resolve::Mesh> mesh() const;
        RadioMaterial material() const;

        void setPosition(const mitsuba::Resolve::Point3f& position);
        void setOrientation(const mitsuba::Resolve::Point3f& orientation);
        void setVelocity(const mitsuba::Resolve::Vector3f& velocity);
        void setMaterial(const RadioMaterial& material);
    };

} // namespace artery::sionna::py
