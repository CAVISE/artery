#include "TraciIDConverter.h"

#include <omnetpp/cexception.h>

#include <cavise/sionna/bridge/Helpers.h>

using namespace artery::sionna;

Define_Module(TraciIDConverter);

void TraciIDConverter::initialize() {}

std::string TraciIDConverter::traciId(const std::string& sceneId) {
    auto& sceneIds = mapping_.right;
    if (auto iter = sceneIds.find(sceneId); iter != sceneIds.end()) {
        return iter->get_left();
    }

    throw omnetpp::cRuntimeError("failed to get traci ID: no scene ID found: %s", sceneId.c_str());
}

std::string TraciIDConverter::sceneId(const std::string& traciId) {
    auto& traciIds = mapping_.left;
    auto& sceneIds = mapping_.right;

    if (auto iter = traciIds.find(traciId); iter != traciIds.end()) {
        return iter->get_right();
    }

    std::string sceneId = traciId;
    if (sceneId.empty()) {
        throw omnetpp::cRuntimeError("could not convert TraCI ID: converted ID is empty");
    }

    // Dots are not allowed - underscores are fine though.
    std::replace(sceneId.begin(), sceneId.end(), '.', '_');

    std::size_t repeatedTimes = 0;
    auto formatId = [&sceneId, &repeatedTimes]() {
        return format("%s_%d", sceneId.c_str(), repeatedTimes);
    };

    // Scene IDs must be unique. This transforms ensures that
    // new scene ID will be unique.
    for (auto iter = sceneIds.begin(); iter != sceneIds.end(); iter = sceneIds.find(formatId())) {
        ++repeatedTimes;
    }

    mapping_.insert({traciId, sceneId});
    return sceneId;
}

void TraciIDConverter::removeByTraciId(const std::string& traciId) {
    auto& traciIds = mapping_.left;
    if (auto iter = traciIds.find(traciId); iter != traciIds.end()) {
        traciIds.erase(iter);
        return;
    }

    throw omnetpp::cRuntimeError("failed to remove ID: no traci ID found: %s", traciId.c_str());
}
