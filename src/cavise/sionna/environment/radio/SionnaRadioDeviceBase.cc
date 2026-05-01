#include "SionnaRadioDeviceBase.h"

#include <artery/inet/AntennaMobility.h>
#include <artery/traci/Cast.h>

#include <cavise/sionna/environment/Compat.h>
#include <cavise/sionna/environment/PhysicalEnvironment.h>

#include <omnetpp/cmodule.h>
#include <omnetpp/cexception.h>

#include <inet/common/InitStages.h>

#include <cctype>

using namespace artery::sionna;

namespace {

    std::string sanitizeSceneName(std::string name) {
        for (auto& ch : name) {
            const auto uch = static_cast<unsigned char>(ch);
            if (!std::isalnum(uch) && ch != '_') {
                ch = '_';
            }
        }

        return name;
    }

    inet::IMobility* resolveBoundaryMobility(inet::IMobility* mobility) {
        auto* antennaMobility = dynamic_cast<artery::AntennaMobility*>(mobility);
        if (antennaMobility == nullptr) {
            return mobility;
        }

        auto* parentModule = antennaMobility->getModuleByPath(antennaMobility->par("mobilityModule"));
        auto* parentMobility = dynamic_cast<inet::IMobility*>(parentModule);
        if (parentMobility == nullptr) {
            throw omnetpp::cRuntimeError("AntennaMobility parent does not implement inet::IMobility");
        }

        return resolveBoundaryMobility(parentMobility);
    }

    traci::Boundary mobilityBoundary(inet::IMobility& mobility) {
        auto* boundaryMobility = resolveBoundaryMobility(&mobility);
        const auto min = boundaryMobility->getConstraintAreaMin();
        const auto max = boundaryMobility->getConstraintAreaMax();

        libsumo::TraCIPositionVector boundary;
        boundary.value.resize(2);
        boundary.value[0].x = min.x;
        boundary.value[0].y = min.y;
        boundary.value[0].z = min.z;
        boundary.value[1].x = max.x;
        boundary.value[1].y = max.y;
        boundary.value[1].z = max.z;
        return traci::Boundary(boundary);
    }

    mitsuba::Resolve::Vector3f reverseMobilityPosition(inet::IMobility& mobility) {
        const auto boundary = mobilityBoundary(mobility);
        const auto position = mobility.getCurrentPosition();
        return mitsuba::Resolve::Vector3f(
            fromScalar<mitsuba::Resolve::Float>(position.x + boundary.lowerLeftPosition().x),
            fromScalar<mitsuba::Resolve::Float>(boundary.upperRightPosition().y - position.y),
            fromScalar<mitsuba::Resolve::Float>(position.z));
    }

    traci::TraCIAngle reverseMobilityHeading(inet::IMobility& mobility) {
        const auto orientation = mobility.getCurrentAngularPosition();
        return traci::angle_cast(artery::Angle::from_radian(-orientation.alpha));
    }

} // namespace

omnetpp::simsignal_t SionnaRadioDeviceBase::sceneRadioDevicesEditedSignal = omnetpp::cComponent::registerSignal("sionnaSceneRadioDevicesEdited");

int SionnaRadioDeviceBase::numInitStages() const {
    return inet::NUM_INIT_STAGES;
}

void SionnaRadioDeviceBase::initialize(int stage) {
    if (stage == inet::INITSTAGE_LOCAL) {
        radio_ = dynamic_cast<const inet::physicallayer::IRadio*>(getParentModule());
        if (radio_ == nullptr) {
            throw omnetpp::cRuntimeError("%s must be placed directly under an IRadio module", getClassName());
        }

        mobility_ = radio_->getAntenna()->getMobility();
        if (mobility_ == nullptr) {
            auto* module = dynamic_cast<const omnetpp::cModule*>(radio_);
            throw omnetpp::cRuntimeError("Radio %s has no antenna mobility", module ? module->getFullPath().c_str() : "<unknown>");
        }
    } else if (stage == inet::INITSTAGE_LAST) {
        bindIntoScene();
    }
}

const inet::physicallayer::IRadio* SionnaRadioDeviceBase::radio() const {
    if (radio_ == nullptr) {
        throw omnetpp::cRuntimeError("%s has no bound parent radio", getClassName());
    }

    return radio_;
}

inet::IMobility* SionnaRadioDeviceBase::mobility() const {
    if (mobility_ == nullptr) {
        throw omnetpp::cRuntimeError("%s has no bound mobility", getClassName());
    }

    return mobility_;
}

PhysicalEnvironment* SionnaRadioDeviceBase::physicalEnvironment() const {
    if (physicalEnvironment_ != nullptr) {
        return physicalEnvironment_;
    }

    auto path = par("physicalEnvironmentModule").stdstringValue();
    if (path.empty()) {
        throw omnetpp::cRuntimeError("physicalEnvironmentModule was not specified");
    }

    auto* module = getModuleByPath(path.c_str());
    if (module == nullptr) {
        throw omnetpp::cRuntimeError("No physical environment found at path %s", path.c_str());
    }

    physicalEnvironment_ = dynamic_cast<PhysicalEnvironment*>(module);
    if (physicalEnvironment_ == nullptr) {
        throw omnetpp::cRuntimeError("Module at path %s is not a Sionna physical environment", path.c_str());
    }

    return physicalEnvironment_;
}

ITraciCoordinateTransformer* SionnaRadioDeviceBase::coordinateTransformer() const {
    if (coordinateTransformer_ != nullptr) {
        return coordinateTransformer_;
    }

    auto* environment = physicalEnvironment();
    auto* module = environment->getSubmodule("coordinateTransformer");
    if (module == nullptr) {
        throw omnetpp::cRuntimeError("Sionna physical environment has no coordinateTransformer submodule");
    }

    coordinateTransformer_ = dynamic_cast<ITraciCoordinateTransformer*>(module);
    if (coordinateTransformer_ == nullptr) {
        throw omnetpp::cRuntimeError("coordinateTransformer does not implement artery::sionna::ITraciCoordinateTransformer");
    }

    return coordinateTransformer_;
}

std::string SionnaRadioDeviceBase::sceneName() const {
    return sanitizeSceneName(getFullPath());
}

mitsuba::Resolve::Point3f SionnaRadioDeviceBase::scenePosition() const {
    const auto canonical = coordinateTransformer()->fromSumo(reverseMobilityPosition(*mobility()));
    return coordinateTransformer()->toLocalScene(canonical);
}

mitsuba::Resolve::Point3f SionnaRadioDeviceBase::sceneOrientation() const {
    const auto canonicalOrientation = convert<mitsuba::Resolve::Point3f>(reverseMobilityHeading(*mobility()));
    return coordinateTransformer()->toLocalScene(canonicalOrientation);
}

mitsuba::Resolve::Vector3f SionnaRadioDeviceBase::sceneVelocity() const {
    const auto canonical = coordinateTransformer()->vectorFromSumo(convert<mitsuba::Resolve::Vector3f>(mobility()->getCurrentSpeed()));
    return coordinateTransformer()->toLocalScene(canonical);
}
