#include "Spamming.h"

#include <artery/traci/VehicleController.h>

#include <omnetpp.h>

#include <memory>

using namespace cavise;

Define_Module(Spamming);

void Spamming::initialize() {
    CosimService::initialize();

    burstCount_ = par("burstCount").intValue();
    if (burstCount_ <= 0) {
        throw omnetpp::cRuntimeError("burstCount must be positive, got %d", burstCount_);
    }
}

void Spamming::trigger() {
    for (int i = 0; i < burstCount_; ++i) {
        replayingOldPayload_ = i != 0;
        CosimService::trigger();
    }

    replayingOldPayload_ = false;
}

bool Spamming::process(capi::Entity& message) {
    if (message.id().empty()) {
        return true;
    }

    const auto& vehicle = getFacilities().get_const<traci::VehicleController>();
    if (message.id() == vehicle.getVehicleId()) {
        return true;
    }

    payloads_.push_back(message);
    EV_WARN << "Spamming captured stale payload " << message.id()
            << " on vehicle " << vehicle.getVehicleId() << "\n";
    return true;
}

std::unique_ptr<capi::Entity> Spamming::make(std::shared_ptr<artery::NetworkInterface>& /* iface */) {
    if (!replayingOldPayload_ || payloads_.empty()) {
        return std::make_unique<capi::Entity>(current());
    }

    const auto index = intuniform(0, static_cast<int>(payloads_.size()) - 1);
    return std::make_unique<capi::Entity>(payloads_.at(index));
}
