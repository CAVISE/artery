#pragma once

#include <omnetpp/csimplemodule.h>

#include <cavise/sionna/environment/Compat.h>

namespace artery::sionna {

    // SUMO <-> canonical scene coordinates mapper, with explicit conversion
    // to and from Sionna's local scene coordinate order.
    class ITraciCoordinateTransformer {
    public:
        virtual mitsuba::Resolve::Vector3f fromSumo(const mitsuba::Resolve::Vector3f& sumo) const = 0;
        virtual mitsuba::Resolve::Vector3f vectorFromSumo(const mitsuba::Resolve::Vector3f& sumo) const = 0;
        virtual mitsuba::Resolve::Vector3f fromScene(const mitsuba::Resolve::Vector3f& scene) const = 0;
        virtual mitsuba::Resolve::Vector3f toLocalScene(const mitsuba::Resolve::Vector3f& scene) const = 0;
        virtual mitsuba::Resolve::Vector3f fromLocalScene(const mitsuba::Resolve::Vector3f& localScene) const = 0;

        virtual ~ITraciCoordinateTransformer() = default;
    };

    // Maps coordinates using:
    // 1) rotation matrix if coordinate systems differ in angles
    // 2) translation vector to compensate coordinate system zero point locations
    // 3) remap matrix only at the Sionna API boundary, e.g. (x, y, z) -> (x, z, y)
    class TraciCoordinateTransformer
        : public ITraciCoordinateTransformer
        , public omnetpp::cSimpleModule {
    public:
        // omnetpp::cSimpleModule implementation.
        int numInitStages() const override;
        void initialize(int stage) override;

        // ITraciCoordinateTransformer implementation.
        mitsuba::Resolve::Vector3f fromSumo(const mitsuba::Resolve::Vector3f& sumo) const override;
        mitsuba::Resolve::Vector3f vectorFromSumo(const mitsuba::Resolve::Vector3f& sumo) const override;
        mitsuba::Resolve::Vector3f fromScene(const mitsuba::Resolve::Vector3f& scene) const override;
        mitsuba::Resolve::Vector3f toLocalScene(const mitsuba::Resolve::Vector3f& scene) const override;
        mitsuba::Resolve::Vector3f fromLocalScene(const mitsuba::Resolve::Vector3f& localScene) const override;

    private:
        mitsuba::Resolve::Matrix3f remap_;
        mitsuba::Resolve::Matrix3f inverseRemap_;
        mitsuba::Resolve::Matrix2f rotation_;
        mitsuba::Resolve::Matrix2f inverseRotation_;
        mitsuba::Resolve::Vector3f translation_;
    };

} // namespace artery::sionna
