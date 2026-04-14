#pragma once

#include <omnetpp/csimplemodule.h>

#include <cavise/sionna/environment/Compat.h>

#include <optional>

namespace artery::sionna {

    class ITraciCoordinateTransformer {
    public:
        virtual mitsuba::Resolve::Vector3f fromSumo(const mitsuba::Resolve::Vector3f& sumo) const = 0;
        virtual mitsuba::Resolve::Vector3f fromScene(const mitsuba::Resolve::Vector3f& scene) const = 0;

        virtual ~ITraciCoordinateTransformer() = default;
    };

    class TraciCoordinateTransformer
        : public ITraciCoordinateTransformer
        , public omnetpp::cSimpleModule {
    public:
        // omnetpp::cSimpleModule implementation.
        int numInitStages() const override;
        void initialize(int stage) override;

        // ITraciCoordinateTransformer implementation.
        mitsuba::Resolve::Vector3f fromSumo(const mitsuba::Resolve::Vector3f& sumo) const override;
        mitsuba::Resolve::Vector3f fromScene(const mitsuba::Resolve::Vector3f& scene) const override;

    private:
        std::optional<mitsuba::Resolve::Matrix2f> rotation_;
        std::optional<mitsuba::Resolve::Matrix2f> inverseRotation_;
        std::optional<mitsuba::Resolve::Vector3f> translation_;
    };

} // namespace artery::sionna
