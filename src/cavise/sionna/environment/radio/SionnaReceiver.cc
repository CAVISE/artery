#include "SionnaReceiver.h"

#include <cavise/sionna/environment/PhysicalEnvironment.h>

#include <omnetpp/cexception.h>

#include <inet/physicallayer/contract/packetlevel/IRadioMedium.h>

using namespace artery::sionna;

Define_Module(SionnaReceiver);

std::unordered_map<const inet::physicallayer::IRadio*, SionnaReceiver*> SionnaReceiver::registry_;

void SionnaReceiver::bindIntoScene() {
    auto* environment = physicalEnvironment();
    auto& scene = const_cast<py::SionnaScene&>(environment->scene());
    if (!std::holds_alternative<std::monostate>(scene.get(sceneName()))) {
        throw omnetpp::cRuntimeError("Sionna scene already contains an item named %s", sceneName().c_str());
    }

    py::Receiver device(
        sceneName(),
        scenePosition(),
        sceneOrientation(),
        sceneVelocity());
    scene.add(device);

    device_.emplace(std::move(device));
    registry_.insert_or_assign(radio(), this);
    environment->emit(sceneRadioDevicesEditedSignal, 1UL);
}

void SionnaReceiver::finish() {
    if (device_.has_value() && physicalEnvironment_ != nullptr) {
        auto& scene = const_cast<py::SionnaScene&>(physicalEnvironment_->scene());
        scene.remove(sceneName());
        device_.reset();
        physicalEnvironment_->emit(sceneRadioDevicesEditedSignal, 1UL);
    }

    registry_.erase(radio_);
    omnetpp::cSimpleModule::finish();
}

void SionnaReceiver::sync() {
    auto& rx = device();
    rx.setPosition(scenePosition());
    rx.setOrientation(sceneOrientation());
    rx.setVelocity(sceneVelocity());
}

py::Receiver& SionnaReceiver::device() {
    if (!device_.has_value()) {
        throw omnetpp::cRuntimeError("SionnaReceiver is not bound into the scene");
    }

    return *device_;
}

const py::Receiver& SionnaReceiver::device() const {
    if (!device_.has_value()) {
        throw omnetpp::cRuntimeError("SionnaReceiver is not bound into the scene");
    }

    return *device_;
}

SionnaReceiver* SionnaReceiver::resolve(const inet::physicallayer::IRadio* radio) {
    if (auto found = registry_.find(radio); found != registry_.end()) {
        return found->second;
    }

    return nullptr;
}

SionnaReceiver* SionnaReceiver::resolve(
    const inet::physicallayer::ITransmission* transmission,
    const inet::physicallayer::IArrival* arrival) {
    auto* medium = transmission->getTransmitter()->getMedium();
    for (const auto& [radio, receiver] : registry_) {
        if (radio->getMedium() != medium) {
            continue;
        }

        if (medium->getArrival(radio, transmission) == arrival) {
            return receiver;
        }
    }

    return nullptr;
}

const std::unordered_map<const inet::physicallayer::IRadio*, SionnaReceiver*>& SionnaReceiver::registered() {
    return registry_;
}
