#include "SionnaTransmitter.h"

#include <cavise/sionna/environment/PhysicalEnvironment.h>

#include <omnetpp/cexception.h>

using namespace artery::sionna;

Define_Module(SionnaTransmitter);

std::unordered_map<const inet::physicallayer::IRadio*, SionnaTransmitter*> SionnaTransmitter::registry_;

void SionnaTransmitter::bindIntoScene() {
    auto* environment = physicalEnvironment();
    auto& scene = const_cast<py::SionnaScene&>(environment->scene());
    if (!std::holds_alternative<std::monostate>(scene.get(sceneName()))) {
        throw omnetpp::cRuntimeError("Sionna scene already contains an item named %s", sceneName().c_str());
    }

    const auto rawPosition = mobility()->getCurrentPosition();
    const auto rawOrientation = mobility()->getCurrentAngularPosition();
    const auto rawVelocity = mobility()->getCurrentSpeed();
    const auto position = scenePosition();
    const auto orientation = sceneOrientation();
    const auto velocity = sceneVelocity();

    EV_INFO << "Binding Sionna transmitter " << sceneName()
            << ": mobility position=" << rawPosition
            << " orientation=" << rawOrientation
            << " velocity=" << rawVelocity
            << " -> scene position=" << position
            << " orientation=" << orientation
            << " velocity=" << velocity << endl;

    py::Transmitter device(
        sceneName(),
        position,
        orientation,
        velocity);
    scene.add(device);

    device_.emplace(std::move(device));
    registry_.insert_or_assign(radio(), this);
    environment->emit(sceneRadioDevicesEditedSignal, 1UL);
}

void SionnaTransmitter::finish() {
    if (device_.has_value() && physicalEnvironment_ != nullptr) {
        auto& scene = const_cast<py::SionnaScene&>(physicalEnvironment_->scene());
        scene.remove(sceneName());
        device_.reset();
        physicalEnvironment_->emit(sceneRadioDevicesEditedSignal, 1UL);
    }

    registry_.erase(radio_);
    omnetpp::cSimpleModule::finish();
}

void SionnaTransmitter::sync() {
    auto& tx = device();
    const auto rawPosition = mobility()->getCurrentPosition();
    const auto rawOrientation = mobility()->getCurrentAngularPosition();
    const auto rawVelocity = mobility()->getCurrentSpeed();
    const auto position = scenePosition();
    const auto orientation = sceneOrientation();
    const auto velocity = sceneVelocity();

    EV_INFO << "Syncing Sionna transmitter " << sceneName()
            << ": mobility position=" << rawPosition
            << " orientation=" << rawOrientation
            << " velocity=" << rawVelocity
            << " -> scene position=" << position
            << " orientation=" << orientation
            << " velocity=" << velocity << endl;

    tx.setPosition(position);
    tx.setOrientation(orientation);
    tx.setVelocity(velocity);
}

void SionnaTransmitter::setPowerDbm(float powerDbm) {
    device().setPowerDbm(fromScalar<mitsuba::Resolve::Float>(powerDbm));
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

SionnaTransmitter* SionnaTransmitter::resolve(const inet::physicallayer::IRadio* radio) {
    if (auto found = registry_.find(radio); found != registry_.end()) {
        return found->second;
    }

    return nullptr;
}

const std::unordered_map<const inet::physicallayer::IRadio*, SionnaTransmitter*>& SionnaTransmitter::registered() {
    return registry_;
}
