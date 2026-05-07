#pragma once

#include "SionnaRadioDeviceBase.h"

namespace artery::sionna {

    class SionnaReceiver
        : public SionnaRadioDeviceBase {
    public:
        // omnetpp::cSimpleModule implementation.
        void finish() override;

        // Sionna receiver access.
        py::Receiver& device();
        const py::Receiver& device() const;

    protected:
        void updatePhysics() override;
        void bindIntoScene() override;

    protected:
        std::optional<py::Receiver> device_;
    };

} // namespace artery::sionna
