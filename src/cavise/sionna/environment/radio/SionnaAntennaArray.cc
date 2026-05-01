#include "SionnaAntennaArray.h"

#include <cavise/sionna/environment/PhysicalEnvironment.h>

#include <omnetpp/cexception.h>

#include <inet/common/INETMath.h>
#include <inet/common/InitStages.h>

using namespace artery::sionna;

Define_Module(SionnaAntennaArray);

SionnaAntennaArray::SionnaAntennaArray()
    : inet::physicallayer::AntennaBase() {
}

int SionnaAntennaArray::numInitStages() const {
    return inet::NUM_INIT_STAGES;
}

void SionnaAntennaArray::initialize(int stage) {
    inet::physicallayer::AntennaBase::initialize(stage);
    if (stage == inet::INITSTAGE_LOCAL) {
        gain_ = inet::math::dB2fraction(par("gain"));
    } else if (stage == inet::INITSTAGE_LAST) {
        bindIntoScene();
    }
}

std::ostream& SionnaAntennaArray::printToStream(std::ostream& stream, int level) const {
    stream << "SionnaAntennaArray";
    if (level <= PRINT_LEVEL_DETAIL) {
        stream << ", gain = " << gain_
               << ", numAntennas = " << numAntennas;
    }
    return inet::physicallayer::AntennaBase::printToStream(stream, level);
}

double SionnaAntennaArray::getMaxGain() const {
    return gain_;
}

double SionnaAntennaArray::computeGain(const inet::EulerAngles /* direction */) const {
    return gain_;
}

bool SionnaAntennaArray::hasArray() const {
    return array_.has_value();
}

const py::AntennaArray& SionnaAntennaArray::array() const {
    if (!array_.has_value()) {
        throw omnetpp::cRuntimeError("SionnaAntennaArray has no bound Sionna array");
    }

    return *array_;
}

void SionnaAntennaArray::setArray(py::AntennaArray array) {
    numAntennas = static_cast<int>(array.numAntennas());
    array_.emplace(std::move(array));
}

SionnaAntennaArray::SceneArrayRole SionnaAntennaArray::sceneArrayRole() const {
    auto role = par("sceneArrayRole").stdstringValue();
    if (role.empty()) {
        return SceneArrayRole::None;
    } else if (role == "tx") {
        return SceneArrayRole::Tx;
    } else if (role == "rx") {
        return SceneArrayRole::Rx;
    } else {
        throw omnetpp::cRuntimeError("Unsupported sceneArrayRole '%s'", role.c_str());
    }
}

void SionnaAntennaArray::bindIntoScene() {
    if (sceneArrayRole() == SceneArrayRole::None) {
        return;
    }

    auto path = par("physicalEnvironmentModule").stdstringValue();
    if (path.empty()) {
        throw omnetpp::cRuntimeError("physicalEnvironmentModule was not specified");
    }

    auto* module = getModuleByPath(path.c_str());
    if (module == nullptr) {
        throw omnetpp::cRuntimeError("No physical environment found at path %s", path.c_str());
    }

    auto* environment = dynamic_cast<PhysicalEnvironment*>(module);
    if (environment == nullptr) {
        throw omnetpp::cRuntimeError("Module at path %s is not a Sionna physical environment", path.c_str());
    }

    auto& scene = const_cast<py::SionnaScene&>(environment->scene());
    switch (sceneArrayRole()) {
        case SceneArrayRole::None:
            return;
        case SceneArrayRole::Tx:
            if (scene.hasTxArray()) {
                throw omnetpp::cRuntimeError("Sionna scene already has tx_array configured");
            }
            scene.setTxArray(array());
            break;
        case SceneArrayRole::Rx:
            if (scene.hasRxArray()) {
                throw omnetpp::cRuntimeError("Sionna scene already has rx_array configured");
            }
            scene.setRxArray(array());
            break;
    }
}
