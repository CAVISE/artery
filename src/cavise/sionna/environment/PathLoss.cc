#include "PathLoss.h"

#include <cavise/sionna/environment/Compat.h>
#include <cavise/sionna/environment/PhysicalEnvironment.h>
#include <cavise/sionna/environment/config/dynamic/TraciDynamicSceneConfigProvider.h>
#include <cavise/sionna/environment/radio/SionnaReceiver.h>
#include <cavise/sionna/environment/radio/SionnaTransmitter.h>

#include <nanobind/nanobind.h>
#include <omnetpp/cexception.h>

#include <inet/common/INETMath.h>
#include <inet/common/InitStages.h>
#include <inet/physicallayer/contract/packetlevel/IRadio.h>
#include <inet/physicallayer/contract/packetlevel/IRadioMedium.h>
#include <inet/physicallayer/contract/packetlevel/IRadioSignal.h>

#include <algorithm>
#include <cmath>

using namespace artery::sionna;

Define_Module(PathLoss);
omnetpp::simsignal_t PathLoss::lossComputedSignal = omnetpp::cComponent::registerSignal("sionnaPathLossComputed");
omnetpp::simsignal_t PathLoss::pathsSolvedSignal = omnetpp::cComponent::registerSignal("sionnaPathsSolved");

namespace {
    float wattsToDbm(inet::W power) {
        const double watts = power.get();
        if (watts <= 0.0) {
            return -std::numeric_limits<float>::infinity();
        }

        return static_cast<float>(10.0 * std::log10(watts / 1e-3));
    }

} // namespace

int PathLoss::numInitStages() const {
    return inet::NUM_INIT_STAGES;
}

void PathLoss::initialize(int stage) {
    PathLossBase::initialize(stage);

    if (stage == inet::INITSTAGE_LOCAL) {
        requirePhysicalEnvironment_ = par("requirePhysicalEnvironment").boolValue();
        updateDynamicObjectsOnQuery_ = par("updateDynamicObjectsOnQuery").boolValue();
        includeLineOfSight_ = par("includeLineOfSight").boolValue();
        includeReflections_ = par("includeReflections").boolValue();
        includeDiffractions_ = par("includeDiffractions").boolValue();
        maxReflectionDepth_ = par("maxReflectionDepth").intValue();
        maxDiffractionDepth_ = par("maxDiffractionDepth").intValue();
        maxRange_ = par("maxRange").doubleValue();
    } else if (stage == inet::INITSTAGE_PHYSICAL_ENVIRONMENT_2) {
        solver_.emplace();
    }
}

void PathLoss::finish() {
    if (subscribedToSceneEdits_ && sceneNotifier_ != nullptr) {
        sceneNotifier_->unsubscribe(TraciDynamicSceneConfigProvider::sceneEditedSignal, this);
    }
    if (subscribedToRadioDeviceEdits_ && physicalEnvironment_ != nullptr) {
        physicalEnvironment_->unsubscribe(SionnaRadioDeviceBase::sceneRadioDevicesEditedSignal, this);
    }

    cachedPaths_.reset();
    solver_.reset();
    txIndices_.clear();
    rxIndices_.clear();
    sceneNotifier_ = nullptr;
    physicalEnvironment_ = nullptr;
    subscribedToSceneEdits_ = false;
    subscribedToRadioDeviceEdits_ = false;
    cachedPathsDirty_ = true;

    PathLossBase::finish();
}

void PathLoss::receiveSignal(omnetpp::cComponent* /* source */, omnetpp::simsignal_t signal, unsigned long /* value */, omnetpp::cObject* /* details */) {
    if (signal == TraciDynamicSceneConfigProvider::sceneEditedSignal
        || signal == SionnaRadioDeviceBase::sceneRadioDevicesEditedSignal) {
        invalidateCachedPaths();
        if (lastCarrierFrequencyHz_.has_value() && lastBandwidthHz_.has_value()) {
            solveCachedPaths();
        }
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
    return PathLossBase::printToStream(stream, level);
}

const std::optional<py::Paths>& PathLoss::cachedPaths() const {
    return cachedPaths_;
}

PhysicalEnvironment* PathLoss::resolvePhysicalEnvironment() const {
    if (physicalEnvironment_ != nullptr) {
        return physicalEnvironment_;
    }

    auto path = par("physicalEnvironmentModule").stdstringValue();
    if (path.empty()) {
        if (requirePhysicalEnvironment_) {
            throw omnetpp::cRuntimeError("physicalEnvironmentModule was not specified");
        }
        return nullptr;
    }

    auto* module = getModuleByPath(path.c_str());
    if (module == nullptr) {
        if (requirePhysicalEnvironment_) {
            throw omnetpp::cRuntimeError("No physical environment found at path %s", path.c_str());
        }
        return nullptr;
    }

    physicalEnvironment_ = dynamic_cast<PhysicalEnvironment*>(module);
    if (physicalEnvironment_ == nullptr) {
        throw omnetpp::cRuntimeError("Module at path %s is not a Sionna physical environment", path.c_str());
    }

    if (!subscribedToSceneEdits_) {
        auto* notifier = resolveSceneNotifier();
        if (notifier != nullptr) {
            notifier->subscribe(TraciDynamicSceneConfigProvider::sceneEditedSignal, const_cast<PathLoss*>(this));
            sceneNotifier_ = notifier;
            subscribedToSceneEdits_ = true;
        }
    }
    if (!subscribedToRadioDeviceEdits_) {
        physicalEnvironment_->subscribe(SionnaRadioDeviceBase::sceneRadioDevicesEditedSignal, const_cast<PathLoss*>(this));
        subscribedToRadioDeviceEdits_ = true;
    }

    return physicalEnvironment_;
}

omnetpp::cComponent* PathLoss::resolveSceneNotifier() const {
    auto* environment = physicalEnvironment_;
    if (environment == nullptr) {
        return nullptr;
    }

    return dynamic_cast<omnetpp::cComponent*>(environment->getSubmodule("dynamicSceneConfigProvider"));
}

double PathLoss::freeSpacePathLoss(inet::mps propagationSpeed, inet::Hz frequency, inet::m distance) {
    if (distance.get() == 0.0) {
        return 1.0;
    }

    const inet::m waveLength = propagationSpeed / frequency;
    return (waveLength * waveLength).get() / (16.0 * M_PI * M_PI * distance.get() * distance.get());
}

inet::m PathLoss::freeSpaceRange(inet::mps propagationSpeed, inet::Hz frequency, double loss) {
    if (loss <= 0.0) {
        return inet::m(NaN);
    }

    const inet::m waveLength = propagationSpeed / frequency;
    return inet::m(std::sqrt((waveLength * waveLength).get() / (16.0 * M_PI * M_PI * loss)));
}

void PathLoss::invalidateCachedPaths() const {
    cachedPathsDirty_ = true;
    cachedPaths_.reset();
    txIndices_.clear();
    rxIndices_.clear();
}

void PathLoss::solveCachedPaths() const {
    if (!cachedPathsDirty_ && cachedPaths_.has_value()) {
        return;
    }

    auto* environment = resolvePhysicalEnvironment();
    if (environment == nullptr) {
        throw omnetpp::cRuntimeError("cannot solve Sionna paths without a physical environment");
    }
    if (!solver_.has_value()) {
        throw omnetpp::cRuntimeError("Sionna path solver is not initialized yet");
    }
    if (!lastCarrierFrequencyHz_.has_value() || !lastBandwidthHz_.has_value()) {
        throw omnetpp::cRuntimeError("Sionna path solver cannot run before carrier frequency and bandwidth are known");
    }

    auto& scene = const_cast<py::SionnaScene&>(environment->scene());
    scene.setFrequency(fromScalar<mitsuba::Resolve::Float>(*lastCarrierFrequencyHz_));
    scene.setBandwidth(fromScalar<mitsuba::Resolve::Float>(*lastBandwidthHz_));

    for (const auto& [_, tx] : SionnaTransmitter::registered()) {
        tx->sync();
    }
    for (const auto& [_, rx] : SionnaReceiver::registered()) {
        rx->sync();
    }

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

    auto txNames = scene.transmitterNames();
    for (std::size_t i = 0; i < txNames.size(); ++i) {
        txIndices_.insert_or_assign(txNames[i], i);
    }

    auto rxNames = scene.receiverNames();
    for (std::size_t i = 0; i < rxNames.size(); ++i) {
        rxIndices_.insert_or_assign(rxNames[i], i);
    }

    EV_INFO << "SionnaPathLoss solve: scene tx names = " << txNames.size()
            << ", scene rx names = " << rxNames.size()
            << ", paths.numTx = " << cachedPaths_->numTx()
            << ", paths.numRx = " << cachedPaths_->numRx() << endl;
    for (std::size_t i = 0; i < txNames.size(); ++i) {
        EV_INFO << "  tx[" << i << "] = " << txNames[i] << endl;
    }
    for (std::size_t i = 0; i < rxNames.size(); ++i) {
        EV_INFO << "  rx[" << i << "] = " << rxNames[i] << endl;
    }

    cachedPathsDirty_ = false;
    const_cast<PathLoss*>(this)->emit(pathsSolvedSignal, 1UL);
}

void PathLoss::ensureSolved(
    const inet::physicallayer::INarrowbandSignal* narrowbandSignal,
    const inet::physicallayer::IScalarSignal* /* scalarSignal */) const {
    lastCarrierFrequencyHz_ = narrowbandSignal->getCarrierFrequency().get();
    lastBandwidthHz_ = narrowbandSignal->getBandwidth().get();
    solveCachedPaths();
}

double PathLoss::computePathLoss(const inet::physicallayer::ITransmission* transmission, const inet::physicallayer::IArrival* arrival) const {
    const inet::m distance(arrival->getStartPosition().distance(transmission->getStartPosition()));
    if (distance.get() > maxRange_) {
        return 0.0;
    }

    auto* environment = resolvePhysicalEnvironment();
    if (environment == nullptr) {
        throw omnetpp::cRuntimeError("cannot compute Sionna path loss without a physical environment");
    }

    auto narrowbandSignal = check_and_cast<const inet::physicallayer::INarrowbandSignal*>(transmission->getAnalogModel());
    auto scalarSignal = check_and_cast<const inet::physicallayer::IScalarSignal*>(transmission->getAnalogModel());

    auto* tx = SionnaTransmitter::resolve(transmission->getTransmitter());
    if (tx == nullptr) {
        auto* module = dynamic_cast<const omnetpp::cModule*>(transmission->getTransmitter());
        throw omnetpp::cRuntimeError("No SionnaTransmitter module is registered for radio %s", module ? module->getFullPath().c_str() : "<unknown>");
    }

    auto* rx = SionnaReceiver::resolve(transmission, arrival);
    if (rx == nullptr) {
        throw omnetpp::cRuntimeError("No SionnaReceiver module matched arrival for transmission %d", transmission->getId());
    }

    tx->setPowerDbm(wattsToDbm(scalarSignal->getPower()));
    ensureSolved(narrowbandSignal, scalarSignal);

    auto txIndex = txIndices_.find(tx->sceneName());
    if (txIndex == txIndices_.end()) {
        throw omnetpp::cRuntimeError("Solved paths are missing transmitter index for %s", tx->sceneName().c_str());
    }

    auto rxIndex = rxIndices_.find(rx->sceneName());
    if (rxIndex == rxIndices_.end()) {
        throw omnetpp::cRuntimeError("Solved paths are missing receiver index for %s", rx->sceneName().c_str());
    }

    EV_INFO << "SionnaPathLoss query: tx=" << tx->sceneName()
            << " -> " << txIndex->second
            << ", rx=" << rx->sceneName()
            << " -> " << rxIndex->second
            << ", paths.numTx=" << cachedPaths_->numTx()
            << ", paths.numRx=" << cachedPaths_->numRx() << endl;

    const double gain = cachedPaths_->pathGain(rxIndex->second, txIndex->second);
    return std::clamp(gain, 0.0, 1.0);
}

double PathLoss::computePathLoss(inet::mps propagationSpeed, inet::Hz frequency, inet::m distance) const {
    if (distance.get() > maxRange_) {
        return 0.0;
    }

    return freeSpacePathLoss(propagationSpeed, frequency, distance);
}

inet::m PathLoss::computeRange(inet::mps propagationSpeed, inet::Hz frequency, double loss) const {
    return freeSpaceRange(propagationSpeed, frequency, loss);
}
