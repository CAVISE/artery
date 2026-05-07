#include "SionnaRadioDeviceBase.h"

#include <artery/traci/Cast.h>

#include <cavise/sionna/environment/Compat.h>

#include <omnetpp/cmodule.h>
#include <omnetpp/cexception.h>

#include <inet/common/InitStages.h>

using namespace artery::sionna;

namespace {

    traci::TraCIAngle reverseMobilityHeading(inet::IMobility* mobility) {
        const auto orientation = mobility->getCurrentAngularPosition();
        return traci::angle_cast(artery::Angle::from_radian(-orientation.alpha));
    }

} // namespace

omnetpp::simsignal_t SionnaRadioDeviceBase::sceneRadioDevicesEditedSignal = omnetpp::cComponent::registerSignal("sionnaSceneRadioDevicesEdited");

int SionnaRadioDeviceBase::numInitStages() const {
    return inet::NUM_INIT_STAGES;
}

void SionnaRadioDeviceBase::initialize(int stage) {
    if (stage == inet::INITSTAGE_LAST) {
        bindIntoScene();

        if (auto* component = dynamic_cast<omnetpp::cComponent*>(mobility()); component == nullptr) {
            throw omnetpp::cRuntimeError("Sionna radio device mobility is not an OMNeT++ component");
        } else {
            component->subscribe(inet::IMobility::mobilityStateChangedSignal, this);
        }
    }
}

void SionnaRadioDeviceBase::finish() {
    if (auto* component = dynamic_cast<omnetpp::cComponent*>(mobility()); component != nullptr) {
        component->unsubscribe(inet::IMobility::mobilityStateChangedSignal, this);
    }

    omnetpp::cSimpleModule::finish();
}

void SionnaRadioDeviceBase::receiveSignal(omnetpp::cComponent* /* source */, omnetpp::simsignal_t signal, omnetpp::cObject* /* object */, omnetpp::cObject* /* details */) {
    if (signal != inet::IMobility::mobilityStateChangedSignal) {
        throw omnetpp::cRuntimeError("SionnaRadioDeviceBase received unknown signal");
    }

    updatePhysics();
    emit(sceneRadioDevicesEditedSignal, 1UL);
}

const inet::physicallayer::IRadio* SionnaRadioDeviceBase::radio() const {
    if (auto* radio = dynamic_cast<const inet::physicallayer::IRadio*>(getParentModule()); radio == nullptr) {
        throw omnetpp::cRuntimeError("%s has no bound parent radio", getClassName());
    } else {
        return radio;
    }
}

inet::IMobility* SionnaRadioDeviceBase::mobility() const {
    if (auto* mobility = radio()->getAntenna()->getMobility(); mobility == nullptr) {
        auto* module = dynamic_cast<const omnetpp::cModule*>(radio());
        throw omnetpp::cRuntimeError("Radio %s has no antenna mobility", module ? module->getFullPath().c_str() : "<unknown>");
    } else {
        return mobility;
    }
}

ISionnaAPI* SionnaRadioDeviceBase::api() const {
    return ISionnaAPI::get(this);
}

std::string SionnaRadioDeviceBase::sceneName() const {
    return api()->IDConversion()->convertID(IDNamespace::SUMO, IDNamespace::SIONNA, getFullPath());
}

mi::Point3f SionnaRadioDeviceBase::position() const {
    using c = CoordinateSystem;
    return api()->coordinateTransform()->convertCoordinates(c::INET, c::SIONNA_LOCAL, convert<mi::Vector3f>(mobility()->getCurrentPosition()));
}

mi::Point3f SionnaRadioDeviceBase::orientation() const {
    using c = CoordinateSystem;
    const auto orientation = convert<mi::Point3f>(reverseMobilityHeading(mobility()));
    return api()->coordinateTransform()->convertCoordinates(c::SIONNA_SCENE, c::SIONNA_LOCAL, orientation);
}

mi::Vector3f SionnaRadioDeviceBase::velocity() const {
    using c = CoordinateSystem;
    auto* transform = api()->coordinateTransform();
    const auto canonical = transform->convertCoordinates(c::INET, c::SIONNA_SCENE, convert<mi::Vector3f>(mobility()->getCurrentSpeed())) -
                           transform->convertCoordinates(c::INET, c::SIONNA_SCENE, mi::Vector3f(0.0, 0.0, 0.0));
    return transform->convertCoordinates(c::SIONNA_SCENE, c::SIONNA_LOCAL, canonical);
}
