#pragma once

#include <nanobind/nanobind.h>

#include <cavise/sionna/bridge/bindings/Modules.h>
#include <cavise/sionna/bridge/capabilities/Calling.h>

namespace artery::sionna::py {

    class SIONNA_BRIDGE_API Camera
        : public SionnaRtModule
        , public InitPythonClassCapability {
    public:
        const char* className() const override;

        Camera() = default;
        explicit Camera(nanobind::object obj);
        Camera(
            const mitsuba::Resolve::Point3f& position,
            const mitsuba::Resolve::Point3f& orientation = mitsuba::Resolve::Point3f(0.f, 0.f, 0.f));

        mitsuba::Resolve::Point3f position() const;
        void setPosition(const mitsuba::Resolve::Point3f& position);

        mitsuba::Resolve::Point3f orientation() const;
        void setOrientation(const mitsuba::Resolve::Point3f& orientation);
    };

} // namespace artery::sionna::py
