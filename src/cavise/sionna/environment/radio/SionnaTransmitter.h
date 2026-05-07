#pragma once

#include "SionnaRadioDeviceBase.h"

namespace artery::sionna {

    class SionnaTransmitter
        : public SionnaRadioDeviceBase {
    public:
        // omnetpp::cSimpleModule implementation.
        void finish() override;

        // Sionna transmitter access.
        py::Transmitter& device();
        const py::Transmitter& device() const;

        void setPowerDbm(float powerDbm);

    protected:
        void updatePhysics() override;
        void bindIntoScene() override;

    protected:
        std::optional<py::Transmitter> device_;
    };

} // namespace artery::sionna
