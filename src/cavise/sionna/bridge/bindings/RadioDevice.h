#pragma once

#include <nanobind/nanobind.h>

#include <cavise/sionna/bridge/Compat.h>
#include <cavise/sionna/bridge/bindings/Modules.h>
#include <cavise/sionna/bridge/capabilities/Calling.h>

#include <optional>
#include <string>
#include <tuple>

namespace artery::sionna::py {

    class SIONNA_BRIDGE_API RadioDevice
        : public SionnaRtModule
        , public InitPythonClassCapability {
    public:
        using TColor = std::tuple<float, float, float>;

        RadioDevice() = default;
        explicit RadioDevice(nanobind::object obj);

        std::string name() const;

        mitsuba::Resolve::Point3f position() const;
        void setPosition(const mitsuba::Resolve::Point3f& position);

        mitsuba::Resolve::Point3f orientation() const;
        void setOrientation(const mitsuba::Resolve::Point3f& orientation);

        mitsuba::Resolve::Vector3f velocity() const;
        void setVelocity(const mitsuba::Resolve::Vector3f& velocity);

        TColor color() const;
        void setColor(TColor color);

        void lookAt(const mitsuba::Resolve::Point3f& target);
    };

    class SIONNA_BRIDGE_API Transmitter
        : public RadioDevice {
    public:
        const char* className() const override;

        Transmitter() = default;
        explicit Transmitter(nanobind::object obj);
        Transmitter(
            const std::string& name,
            const mitsuba::Resolve::Point3f& position,
            std::optional<mitsuba::Resolve::Point3f> orientation = std::nullopt,
            std::optional<mitsuba::Resolve::Vector3f> velocity = std::nullopt,
            float powerDbm = 44.0f);

        maybe_diff_t<mitsuba::Resolve::Float> power() const;
        maybe_diff_t<mitsuba::Resolve::Float> powerDbm() const;
        void setPowerDbm(maybe_diff_t<mitsuba::Resolve::Float> powerDbm);
    };

    class SIONNA_BRIDGE_API Receiver
        : public RadioDevice {
    public:
        const char* className() const override;

        Receiver() = default;
        explicit Receiver(nanobind::object obj);
        Receiver(
            const std::string& name,
            const mitsuba::Resolve::Point3f& position,
            std::optional<mitsuba::Resolve::Point3f> orientation = std::nullopt,
            std::optional<mitsuba::Resolve::Vector3f> velocity = std::nullopt);
    };

} // namespace artery::sionna::py
