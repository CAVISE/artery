#include "SionnaTransmitter.h"

#include <omnetpp/cexception.h>

using namespace artery::sionna;

Define_Module(SionnaTransmitter);

void SionnaTransmitter::bindIntoScene() {
    auto* api = this->api();
    auto* sceneConfig = api->dynamicConfiguration();

    const auto position = this->position();
    const auto orientation = this->orientation();
    const auto velocity = this->velocity();

    EV_INFO << "Binding Sionna transmitter " << sceneName()
            << ": mobility position=" << mobility()->getCurrentPosition()
            << " orientation=" << mobility()->getCurrentAngularPosition()
            << " velocity=" << mobility()->getCurrentSpeed()
            << " -> scene position=" << position
            << " orientation=" << orientation
            << " velocity=" << velocity << endl;

    auto device = py::Transmitter(sceneName(), position, orientation, velocity);
    if (!sceneConfig->addTransmitter(device)) {
        throw omnetpp::cRuntimeError("Sionna scene already contains an item named %s", sceneName().c_str());
    }

    device_.emplace(std::move(device));
    emit(sceneRadioDevicesEditedSignal, 1UL);
}

void SionnaTransmitter::finish() {
    if (device_.has_value()) {
        auto* api = this->api();
        api->dynamicConfiguration()->removeSceneItem(sceneName());
        device_.reset();
        emit(sceneRadioDevicesEditedSignal, 1UL);
    }

    SionnaRadioDeviceBase::finish();
}

void SionnaTransmitter::updatePhysics() {
    auto& tx = device();

    const auto position = this->position();
    const auto orientation = this->orientation();
    const auto velocity = this->velocity();

    EV_INFO << "Syncing Sionna transmitter " << sceneName()
            << ": mobility position=" << mobility()->getCurrentPosition()
            << " orientation=" << mobility()->getCurrentAngularPosition()
            << " velocity=" << mobility()->getCurrentSpeed()
            << " -> scene position=" << position
            << " orientation=" << orientation
            << " velocity=" << velocity << endl;

    tx.setPosition(position);
    tx.setOrientation(orientation);
    tx.setVelocity(velocity);
}

void SionnaTransmitter::setPowerDbm(float powerDbm) {
    device().setPowerDbm(fromScalar<mi::Float>(powerDbm));
}

py::Transmitter& SionnaTransmitter::device() {
    if (!device_.has_value()) {
        throw omnetpp::cRuntimeError("SionnaTransmitter is not bound into the scene");
    }

    return *device_;
}

const py::Transmitter& SionnaTransmitter::device() const {
    if (!device_.has_value()) {
        throw omnetpp::cRuntimeError("SionnaTransmitter is not bound into the scene");
    }

    return *device_;
}
