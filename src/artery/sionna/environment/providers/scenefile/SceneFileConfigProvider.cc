#include "SceneFileConfigProvider.h"

#include <omnetpp/cexception.h>

using namespace artery::sionna;

Define_Module(SceneFileConfigProvider)

void SceneFileConfigProvider::initialize() {
    config_.sceneFile = par("sceneFile").stdstringValue();

    if (config_.sceneFile.empty()) {
        throw omnetpp::cRuntimeError("sceneFile parameter must not be empty");
    }
}

SceneConfig SceneFileConfigProvider::exportSceneConfig() const {
    return config_;
}
