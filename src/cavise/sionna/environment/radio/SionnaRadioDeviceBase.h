#pragma once

#include <cavise/sionna/bridge/SionnaBridge.h>
#include <cavise/sionna/environment/api/SionnaAPI.h>

#include <omnetpp/clistener.h>
#include <omnetpp/csimplemodule.h>

#include <inet/common/INETDefs.h>
#include <inet/mobility/contract/IMobility.h>
#include <inet/physicallayer/contract/packetlevel/IRadio.h>

namespace artery::sionna {

    class SionnaRadioDeviceBase
        : public omnetpp::cSimpleModule
        , public omnetpp::cListener {
    public:
        static omnetpp::simsignal_t sceneRadioDevicesEditedSignal;

        // omnetpp::cSimpleModule implementation.
        int numInitStages() const override;
        void initialize(int stage) override;
        void finish() override;

        // omnetpp::cListener implementation.
        void receiveSignal(omnetpp::cComponent* source, omnetpp::simsignal_t signal, omnetpp::cObject* object, omnetpp::cObject* details) override;

        std::string sceneName() const;
        const inet::physicallayer::IRadio* radio() const;

    protected:
        inet::IMobility* mobility() const;
        ISionnaAPI* api() const;

        mitsuba::Resolve::Point3f position() const;
        mitsuba::Resolve::Point3f orientation() const;
        mitsuba::Resolve::Vector3f velocity() const;

        virtual void bindIntoScene() = 0;
        virtual void updatePhysics() = 0;
    };

} // namespace artery::sionna
