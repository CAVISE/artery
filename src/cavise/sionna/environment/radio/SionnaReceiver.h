#pragma once

#include "SionnaRadioDeviceBase.h"

#include <unordered_map>

namespace artery::sionna {

    class SionnaReceiver
        : public SionnaRadioDeviceBase {
    public:
        // omnetpp::cSimpleModule implementation.
        void finish() override;

        // Sionna receiver access.
        void sync();
        py::Receiver& device();
        const py::Receiver& device() const;

        static SionnaReceiver* resolve(const inet::physicallayer::IRadio* radio);
        static SionnaReceiver* resolve(
            const inet::physicallayer::ITransmission* transmission,
            const inet::physicallayer::IArrival* arrival);
        static const std::unordered_map<const inet::physicallayer::IRadio*, SionnaReceiver*>& registered();

    protected:
        void bindIntoScene() override;

    protected:
        static std::unordered_map<const inet::physicallayer::IRadio*, SionnaReceiver*> registry_;

        std::optional<py::Receiver> device_;
    };

} // namespace artery::sionna
