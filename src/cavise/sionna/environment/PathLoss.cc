#include "PathLoss.h"

#include <cavise/sionna/environment/Compat.h>
#include <cavise/sionna/environment/api/SionnaAPI.h>
#include <cavise/sionna/environment/config/dynamic/TraciDynamicSceneConfigProvider.h>
#include <cavise/sionna/environment/radio/SionnaReceiver.h>
#include <cavise/sionna/environment/radio/SionnaTransmitter.h>

#include <omnetpp/cmodule.h>
#include <omnetpp/cexception.h>

#include <inet/common/INETMath.h>
#include <inet/common/InitStages.h>
#include <inet/physicallayer/contract/packetlevel/IRadio.h>
#include <inet/physicallayer/contract/packetlevel/IRadioMedium.h>
#include <inet/physicallayer/contract/packetlevel/IRadioSignal.h>

#include <algorithm>

using namespace artery::sionna;

Define_Module(PathLoss);

omnetpp::simsignal_t PathLoss::pathsSolvedSignal = omnetpp::cComponent::registerSignal("sionnaPathsSolved");

namespace {
    template <typename Device>
    Device* radioSubmodule(const inet::physicallayer::IRadio* radio, const char* submoduleName) {
        auto* module = dynamic_cast<const omnetpp::cModule*>(radio);
        if (module == nullptr) {
            return nullptr;
        }

        return dynamic_cast<Device*>(const_cast<omnetpp::cModule*>(module)->getSubmodule(submoduleName));
    }

    bool matchesArrival(SionnaReceiver* receiver, const inet::physicallayer::ITransmission* transmission, const inet::physicallayer::IArrival* arrival) {
        auto* medium = transmission->getTransmitter()->getMedium();
        return receiver->radio()->getMedium() == medium
            && medium->getArrival(receiver->radio(), transmission) == arrival;
    }

    SionnaReceiver* receiverForArrival(
        omnetpp::cModule* root,
        const inet::physicallayer::ITransmission* transmission,
        const inet::physicallayer::IArrival* arrival) {
        if (root == nullptr) {
            return nullptr;
        }

        if (auto* receiver = dynamic_cast<SionnaReceiver*>(root); receiver != nullptr && matchesArrival(receiver, transmission, arrival)) {
            return receiver;
        }

        for (omnetpp::cModule::SubmoduleIterator it(root); !it.end(); ++it) {
            if (auto* receiver = receiverForArrival(*it, transmission, arrival); receiver != nullptr) {
                return receiver;
            }
        }

        return nullptr;
    }

} // namespace

int PathLoss::numInitStages() const {
    return inet::NUM_INIT_STAGES;
}

void PathLoss::initialize(int stage) {
    FreeSpacePathLoss::initialize(stage);

    if (stage == inet::INITSTAGE_LOCAL) {
        includeLineOfSight_ = par("includeLineOfSight").boolValue();
        includeReflections_ = par("includeReflections").boolValue();
        includeDiffractions_ = par("includeDiffractions").boolValue();
        maxReflectionDepth_ = par("maxReflectionDepth").intValue();
        maxDiffractionDepth_ = par("maxDiffractionDepth").intValue();
        maxRange_ = par("maxRange").doubleValue();

        api_ = ISionnaAPI::get(getModuleByPath(par("physicalEnvironmentModule").stringValue()));
        auto* apiModule = dynamic_cast<omnetpp::cModule*>(api_);
        sceneNotifier_ = apiModule != nullptr ? dynamic_cast<omnetpp::cComponent*>(apiModule->getSubmodule("dynamicSceneConfigProvider")) : nullptr;
        if (sceneNotifier_ != nullptr) {
            sceneNotifier_->subscribe(TraciDynamicSceneConfigProvider::sceneEditedSignal, this);
        }

        getSimulation()->getSystemModule()->subscribe(SionnaRadioDeviceBase::sceneRadioDevicesEditedSignal, this);
    } else if (stage == inet::INITSTAGE_PHYSICAL_ENVIRONMENT_2) {
        solver_.emplace();
    }
}

void PathLoss::finish() {
    if (sceneNotifier_ != nullptr) {
        sceneNotifier_->unsubscribe(TraciDynamicSceneConfigProvider::sceneEditedSignal, this);
    }
    getSimulation()->getSystemModule()->unsubscribe(SionnaRadioDeviceBase::sceneRadioDevicesEditedSignal, this);

    cachedPaths_.reset();
    solver_.reset();
    txIndices_.clear();
    rxIndices_.clear();
    sceneNotifier_ = nullptr;
    api_ = nullptr;

    FreeSpacePathLoss::finish();
}

void PathLoss::receiveSignal(omnetpp::cComponent* /* source */, omnetpp::simsignal_t signal, unsigned long /* value */, omnetpp::cObject* /* details */) {
    if (signal == TraciDynamicSceneConfigProvider::sceneEditedSignal
        || signal == SionnaRadioDeviceBase::sceneRadioDevicesEditedSignal) {
        invalidateCachedPaths();
    }
}

std::ostream& PathLoss::printToStream(std::ostream& stream, int level) const {
    stream << "SionnaPathLoss";
    if (level <= PRINT_LEVEL_DETAIL) {
        stream << ", maxRange = " << maxRange_
               << ", los = " << includeLineOfSight_
               << ", reflections = " << includeReflections_
               << ", diffractions = " << includeDiffractions_;
    }
    return stream;
}

const std::optional<py::Paths>& PathLoss::cachedPaths() const {
    return cachedPaths_;
}

void PathLoss::invalidateCachedPaths() const {
    cachedPaths_.reset();
    txIndices_.clear();
    rxIndices_.clear();
}

void PathLoss::solveCachedPaths(double carrierFrequencyHz, double bandwidthHz) const {
    if (cachedPaths_.has_value()) {
        return;
    }

    auto& scene = api_->scene();
    scene.setFrequency(fromScalar<mitsuba::Resolve::Float>(carrierFrequencyHz));
    scene.setBandwidth(fromScalar<mitsuba::Resolve::Float>(bandwidthHz));

    const auto maxDepth = std::max(maxReflectionDepth_, maxDiffractionDepth_);
    cachedPaths_ = solver_->solve(
        scene,
        maxDepth,
        1000000,
        1000000,
        true,
        includeLineOfSight_,
        includeReflections_,
        false,
        false,
        includeDiffractions_,
        false,
        true,
        42);

    txIndices_.clear();
    rxIndices_.clear();

    const auto txDevices = scene.orderedTransmitters();
    for (std::size_t i = 0; i < txDevices.size(); ++i) {
        txIndices_.insert_or_assign(txDevices[i].first, i);
    }

    const auto rxDevices = scene.orderedReceivers();
    for (std::size_t i = 0; i < rxDevices.size(); ++i) {
        rxIndices_.insert_or_assign(rxDevices[i].first, i);
    }

    const_cast<PathLoss*>(this)->emit(pathsSolvedSignal, 1UL);
}

double PathLoss::computePathLoss(const inet::physicallayer::ITransmission* transmission, const inet::physicallayer::IArrival* arrival) const {
    const inet::m distance(arrival->getStartPosition().distance(transmission->getStartPosition()));
    if (distance.get() > maxRange_) {
        return 0.0;
    }

    auto narrowbandSignal = check_and_cast<const inet::physicallayer::INarrowbandSignal*>(transmission->getAnalogModel());
    auto scalarSignal = check_and_cast<const inet::physicallayer::IScalarSignal*>(transmission->getAnalogModel());

    auto* tx = radioSubmodule<SionnaTransmitter>(transmission->getTransmitter(), "sionnaTransmitter");
    if (tx == nullptr) {
        auto* module = dynamic_cast<const omnetpp::cModule*>(transmission->getTransmitter());
        throw omnetpp::cRuntimeError("No SionnaTransmitter submodule found for radio %s", module ? module->getFullPath().c_str() : "<unknown>");
    }

    auto* rx = receiverForArrival(getSimulation()->getSystemModule(), transmission, arrival);
    if (rx == nullptr) {
        throw omnetpp::cRuntimeError("No SionnaReceiver module matched arrival for transmission %d", transmission->getId());
    }

    tx->setPowerDbm(static_cast<float>(inet::math::mW2dBm(inet::mW(scalarSignal->getPower()).get())));
    solveCachedPaths(narrowbandSignal->getCarrierFrequency().get(), narrowbandSignal->getBandwidth().get());

    auto txIndex = txIndices_.find(tx->sceneName());
    if (txIndex == txIndices_.end()) {
        throw omnetpp::cRuntimeError("Solved paths are missing transmitter index for %s", tx->sceneName().c_str());
    }

    auto rxIndex = rxIndices_.find(rx->sceneName());
    if (rxIndex == rxIndices_.end()) {
        throw omnetpp::cRuntimeError("Solved paths are missing receiver index for %s", rx->sceneName().c_str());
    }

    const double gain = cachedPaths_->pathGain(rxIndex->second, txIndex->second);
    return std::clamp(gain, 0.0, 1.0);
}
