#pragma once

#include "SionnaRadioDeviceBase.h"

#include <unordered_map>

namespace artery::sionna {

    class SionnaTransmitter
        : public SionnaRadioDeviceBase {
    public:
        // omnetpp::cSimpleModule implementation.
        void finish() override;

        // Sionna transmitter access.
        void sync();
        void setPowerDbm(float powerDbm);
        py::Transmitter& device();
        const py::Transmitter& device() const;

        static SionnaTransmitter* resolve(const inet::physicallayer::IRadio* radio);
        static const std::unordered_map<const inet::physicallayer::IRadio*, SionnaTransmitter*>& registered();

    protected:
        void bindIntoScene() override;

    protected:
        static std::unordered_map<const inet::physicallayer::IRadio*, SionnaTransmitter*> registry_;

        std::optional<py::Transmitter> device_;
    };

} // namespace artery::sionna
