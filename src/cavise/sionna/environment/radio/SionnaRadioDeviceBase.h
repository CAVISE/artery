#pragma once

#include <cavise/sionna/bridge/SionnaBridge.h>
#include <cavise/sionna/environment/config/dynamic/TraciCoordinateTransformer.h>

#include <omnetpp/csimplemodule.h>

#include <inet/common/INETDefs.h>
#include <inet/mobility/contract/IMobility.h>
#include <inet/physicallayer/contract/packetlevel/IRadio.h>

namespace artery::sionna {

    class PhysicalEnvironment;

    class SionnaRadioDeviceBase
        : public omnetpp::cSimpleModule {
    public:
        static omnetpp::simsignal_t sceneRadioDevicesEditedSignal;

        // omnetpp::cSimpleModule implementation.
        int numInitStages() const override;
        void initialize(int stage) override;

        std::string sceneName() const;

    protected:
        const inet::physicallayer::IRadio* radio() const;
        inet::IMobility* mobility() const;
        PhysicalEnvironment* physicalEnvironment() const;
        ITraciCoordinateTransformer* coordinateTransformer() const;

        mitsuba::Resolve::Point3f scenePosition() const;
        mitsuba::Resolve::Point3f sceneOrientation() const;
        mitsuba::Resolve::Vector3f sceneVelocity() const;

        virtual void bindIntoScene() = 0;

    protected:
        mutable const inet::physicallayer::IRadio* radio_ = nullptr;
        mutable inet::IMobility* mobility_ = nullptr;
        mutable PhysicalEnvironment* physicalEnvironment_ = nullptr;
        mutable ITraciCoordinateTransformer* coordinateTransformer_ = nullptr;
    };

} // namespace artery::sionna
