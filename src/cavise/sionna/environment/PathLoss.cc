#include "PathLoss.h"

#include <cavise/sionna/environment/Compat.h>
#include <cavise/sionna/environment/api/SionnaAPI.h>
#include <cavise/sionna/environment/config/dynamic/TraciDynamicSceneConfigProvider.h>
#include <cavise/sionna/environment/radio/SionnaVanetReceiver.h>
#include <cavise/sionna/environment/radio/SionnaVanetTransmitter.h>

#include <omnetpp/cmodule.h>
#include <omnetpp/cexception.h>

#include <inet/common/INETMath.h>
#include <inet/common/InitStages.h>
#include <inet/physicallayer/contract/packetlevel/IRadio.h>

#include <algorithm>
#include <string>

using namespace artery::sionna;

Define_Module(PathLoss);

omnetpp::simsignal_t PathLoss::pathsSolved = omnetpp::cComponent::registerSignal("sionnaPathsSolved");

int PathLoss::numInitStages() const {
    return inet::NUM_INIT_STAGES;
}

void PathLoss::initialize(int stage) {
    FreeSpacePathLoss::initialize(stage);

    if (stage == inet::INITSTAGE_LOCAL) {
        // Configure Path Solver.
        if (omnetpp::cXMLElement* root = par("solverConfig").xmlValue(); root != nullptr) {
            auto valueOf = [](omnetpp::cXMLElement* element, const char* tag) {
                const char* value = element->getNodeValue();
                if (value == nullptr) {
                    throw omnetpp::cRuntimeError("path solver <%s> must define a value", tag);
                }
                return value;
            };

            auto configureInt = [root, &valueOf](const char* tag, int& target) {
                if (omnetpp::cXMLElement* element = root->getFirstChildWithTag(tag); element != nullptr) {
                    target = std::stoi(valueOf(element, tag));
                }
            };

            auto configureBool = [root, &valueOf](const char* tag, bool& target) {
                if (omnetpp::cXMLElement* element = root->getFirstChildWithTag(tag); element != nullptr) {
                    const char* value = valueOf(element, tag);
                    const std::string val = value;
                    if (val == "true") {
                        target = true;
                    } else if (val == "false") {
                        target = false;
                    } else {
                        throw omnetpp::cRuntimeError("invalid bool value for <%s>: %s", tag, value);
                    }
                }
            };

            configureInt("maxDepth", solverOptions_.maxDepth);
            configureInt("maxNumPathsPerSrc", solverOptions_.maxNumPathsPerSrc);
            configureInt("samplesPerSrc", solverOptions_.samplesPerSrc);
            configureBool("syntheticArray", solverOptions_.syntheticArray);
            configureBool("los", solverOptions_.los);
            configureBool("specularReflection", solverOptions_.specularReflection);
            configureBool("diffuseReflection", solverOptions_.diffuseReflection);
            configureBool("refraction", solverOptions_.refraction);
            configureBool("diffraction", solverOptions_.diffraction);
            configureBool("edgeDiffraction", solverOptions_.edgeDiffraction);
            configureBool("diffractionLitRegion", solverOptions_.diffractionLitRegion);
            configureInt("seed", solverOptions_.seed);
        }

        // Max range defines range where communication is not possible at all.
        maxRange_ = par("maxRange").doubleValue();
        api_ = ISionnaAPI::get(getModuleByPath(par("physicalEnvironmentModule").stringValue()));
        subscribeToDynamicSceneUpdates(getSystemModule());
    } else if (stage == inet::INITSTAGE_PHYSICAL_ENVIRONMENT_2) {
        solver_.emplace();

        solvedLinksByType_[-1].setName("sionna.solvedLinks.missing");
        solvedLinksByType_[py::InteractionTypes::none.value()].setName("sionna.solvedLinks.los");
        solvedLinksByType_[py::InteractionTypes::specular.value()].setName("sionna.solvedLinks.specular");
        solvedLinksByType_[py::InteractionTypes::diffuse.value()].setName("sionna.solvedLinks.diffuse");
        solvedLinksByType_[py::InteractionTypes::refraction.value()].setName("sionna.solvedLinks.refraction");
        solvedLinksByType_[py::InteractionTypes::diffraction.value()].setName("sionna.solvedLinks.diffraction");
    }
}

void PathLoss::finish() {
    unsubscribeFromDynamicSceneUpdates();
    invalidateCachedPaths();
    FreeSpacePathLoss::finish();
}

void PathLoss::onDynamicSceneEdited() {
    invalidateCachedPaths();
}

std::ostream& PathLoss::printToStream(std::ostream& stream, int level) const {
    stream << "SionnaPathLoss";
    if (level <= PRINT_LEVEL_DETAIL) {
        stream << ", maxRange = " << maxRange_
               << ", maxDepth = " << solverOptions_.maxDepth
               << ", los = " << solverOptions_.los
               << ", specularReflection = " << solverOptions_.specularReflection
               << ", diffuseReflection = " << solverOptions_.diffuseReflection
               << ", refraction = " << solverOptions_.refraction
               << ", diffraction = " << solverOptions_.diffraction;
    }
    return stream;
}

const std::optional<py::Paths>& PathLoss::cachedPaths() const {
    return paths_.object;
}

void PathLoss::invalidateCachedPaths() const {
    paths_.object.reset();
    paths_.txIndices.clear();
    paths_.rxIndices.clear();
}

void PathLoss::solveCachedPaths() const {
    if (paths_.object.has_value()) {
        return;
    }

    auto& scene = api_->scene();
    paths_.object = solver_->solve(scene, solverOptions_);

    paths_.txIndices.clear();
    paths_.rxIndices.clear();

    const auto txDevices = scene.orderedTransmitters();
    for (std::size_t i = 0; i < txDevices.size(); ++i) {
        paths_.txIndices.insert_or_assign(txDevices[i].first, i);
    }

    const auto rxDevices = scene.orderedReceivers();
    for (std::size_t i = 0; i < rxDevices.size(); ++i) {
        paths_.rxIndices.insert_or_assign(rxDevices[i].first, i);
    }

    recordPathStatistics();

    // Yeah I know, but how to emit differently?
    const_cast<PathLoss*>(this)->emit(pathsSolved, 1UL);
}

void PathLoss::recordPathStatistics() const {
    std::unordered_map<int, std::size_t> counts;
    for (const auto& [type, vector] : solvedLinksByType_) {
        counts[type] = 0;
    }

    for (const auto& [rxId, rxIndex] : paths_.rxIndices) {
        for (const auto& [txId, txIndex] : paths_.txIndices) {
            const std::string linkId = sionna::format("%s -> %s", txId.c_str(), rxId.c_str());

            auto& strongestLinkVec = strongestPathTypeVectors_[linkId];
            if (strongestLinkVec == nullptr) {
                strongestLinkVec = std::make_unique<omnetpp::cOutVector>();
                strongestLinkVec->setName(sionna::format("sionna.strongestPathType: %s", linkId.c_str()).c_str());
            }

            if (auto interaction = paths_.object->strongestPathInteraction(rxIndex, txIndex); interaction.has_value()) {
                const auto type = interaction.value();
                strongestLinkVec->record(type);
                ++counts[type];
            } else {
                strongestLinkVec->record(-1);
                ++counts[-1];
            }
        }
    }

    for (auto& [type, vector] : solvedLinksByType_) {
        vector.record(static_cast<double>(counts[type]));
    }
}

double PathLoss::computePathLoss(const inet::physicallayer::ITransmission* /* transmission */, const inet::physicallayer::IArrival* /* arrival */) const {
    // NOTE: The problem is that this call does not pass rx device. We could potentially
    // try to get that device from arrival coords, but this approach is highly ineffective
    // for large scenes. Much simpler option is just redefine the caller to supply rx device.
    EV_WARN << "Sionna Path Loss is only enabled for custom Analog Model, see base scenario for reference. "
            << "This method returns NaN always";

    return NaN;
}

double PathLoss::computePathLoss(const inet::physicallayer::IRadio* receiverRadio, const inet::physicallayer::ITransmission* transmission, const inet::physicallayer::IArrival* arrival) const {
    // Use sionna tx/rx devices to get their indices
    const SionnaVanetReceiver* sionnaRx = nullptr;
    const SionnaVanetTransmitter* sionnaTx = nullptr;

    const auto* tx = transmission->getTransmitter()->getTransmitter();
    if (sionnaTx = dynamic_cast<const SionnaVanetTransmitter*>(tx); sionnaTx == nullptr) {
        const auto* module = dynamic_cast<const omnetpp::cModule*>(transmission->getTransmitter());
        throw omnetpp::cRuntimeError("Radio %s has no Sionna transmitter", (module != nullptr) ? module->getFullPath().c_str() : "<unknown>");
    }

    if (sionnaRx = dynamic_cast<const SionnaVanetReceiver*>(receiverRadio->getReceiver()); sionnaRx == nullptr) {
        const auto* module = dynamic_cast<const omnetpp::cModule*>(receiverRadio);
        throw omnetpp::cRuntimeError("Radio %s has no Sionna receiver", (module != nullptr) ? module->getFullPath().c_str() : "<unknown>");
    }

    const auto start = transmission->getStartPosition();
    const auto end = arrival->getStartPosition();
    const auto distance = inet::m(start.distance(end));

    const std::string linkId = sionna::format("%s -> %s", sionnaTx->sceneID().c_str(), sionnaRx->sceneID().c_str());

    // This output vector stores gains per-link, which means tx, rx pair.
    auto& gainVec = sionnaGainVectors_[linkId];
    if (gainVec == nullptr) {
        gainVec = std::make_unique<omnetpp::cOutVector>();
        gainVec->setName(sionna::format("sionna.gain: %s", linkId.c_str()).c_str());
    }

    // Do not waste time calculating transmission if rx and tx are too far away.
    if (distance.get() > maxRange_) {
        gainVec->record(0.0);
        return 0.0;
    }

    solveCachedPaths();

    if (auto txIndex = paths_.txIndices.find(sionnaTx->sceneID()); txIndex == paths_.txIndices.end()) {
        throw omnetpp::cRuntimeError("Solved paths are missing transmitter index for %s", sionnaTx->sceneID().c_str());
    } else if (auto rxIndex = paths_.rxIndices.find(sionnaRx->sceneID()); rxIndex == paths_.rxIndices.end()) {
        throw omnetpp::cRuntimeError("Solved paths are missing receiver index for %s", sionnaRx->sceneID().c_str());
    } else {
        const double gain = std::clamp(paths_.object->pathGain(rxIndex->second, txIndex->second), 0.0, 1.0);
        gainVec->record(gain);
        return gain;
    }
}
