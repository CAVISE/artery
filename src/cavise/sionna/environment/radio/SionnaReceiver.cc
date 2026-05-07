#include "SionnaReceiver.h"

#include <omnetpp/cexception.h>

using namespace artery::sionna;

Define_Module(SionnaReceiver);

void SionnaReceiver::bindIntoScene() {
    auto* api = this->api();
    auto* sceneConfig = api->dynamicConfiguration();

    const auto rawPosition = mobility()->getCurrentPosition();
    const auto rawOrientation = mobility()->getCurrentAngularPosition();
    const auto rawVelocity = mobility()->getCurrentSpeed();
    const auto position = this->position();
    const auto orientation = this->orientation();
    const auto velocity = this->velocity();

    EV_INFO << "Binding Sionna receiver " << sceneName()
            << ": mobility position=" << rawPosition
            << " orientation=" << rawOrientation
            << " velocity=" << rawVelocity
            << " -> scene position=" << position
            << " orientation=" << orientation
            << " velocity=" << velocity << endl;

    auto device = py::Receiver(sceneName(), position, orientation, velocity);
    if (!sceneConfig->addReceiver(device)) {
        throw omnetpp::cRuntimeError("Sionna scene already contains an item named %s", sceneName().c_str());
    }

    device_.emplace(std::move(device));
    emit(sceneRadioDevicesEditedSignal, 1UL);
}

void SionnaReceiver::finish() {
    if (device_.has_value()) {
        auto* api = this->api();
        api->dynamicConfiguration()->removeSceneItem(sceneName());
        device_.reset();
        emit(sceneRadioDevicesEditedSignal, 1UL);
    }

    SionnaRadioDeviceBase::finish();
}

void SionnaReceiver::updatePhysics() {
    auto& rx = device();

    const auto position = this->position();
    const auto orientation = this->orientation();
    const auto velocity = this->velocity();

    EV_INFO << "Syncing Sionna receiver " << sceneName()
            << ": mobility position=" << mobility()->getCurrentPosition()
            << " orientation=" << mobility()->getCurrentAngularPosition()
            << " velocity=" << mobility()->getCurrentSpeed()
            << " -> scene position=" << position
            << " orientation=" << orientation
            << " velocity=" << velocity << endl;

    rx.setPosition(position);
    rx.setOrientation(orientation);
    rx.setVelocity(velocity);
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
