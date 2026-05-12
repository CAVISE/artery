#include "TraciDynamicSceneConfigProvider.h"

#include <traci/BasicNodeManager.h>

#include <omnetpp/ccomponent.h>
#include <omnetpp/cexception.h>

#include <inet/common/ModuleAccess.h>

using namespace artery::sionna;

Define_Module(TraciDynamicSceneConfigProvider);

// Define static signal members
omnetpp::simsignal_t TraciDynamicSceneConfigProvider::sceneEditBeginSignal = omnetpp::cComponent::registerSignal("sceneEditBegin");
omnetpp::simsignal_t TraciDynamicSceneConfigProvider::sceneEditEndSignal = omnetpp::cComponent::registerSignal("sceneEditEnd");

void TraciDynamicSceneConfigProvider::initialize() {
    api_ = ISionnaAPI::get(this);
    traciNodeManager_ = inet::getModuleFromPar<traci::BasicNodeManager>(par("traciNodeManagerModule"), this);
    traciNodeManager_->subscribe(traci::BasicNodeManager::updateNodeSignal, this);
}

void TraciDynamicSceneConfigProvider::finish() {
    traciNodeManager_->unsubscribe(traci::BasicNodeManager::updateNodeSignal, this);
}

void TraciDynamicSceneConfigProvider::receiveSignal(omnetpp::cComponent* /* source */, omnetpp::simsignal_t signal, unsigned long /* value */, omnetpp::cObject* /* details */) {
    // This signals end-of-step, so scene is flushed here.
    if (signal == traci::BasicNodeManager::updateNodeSignal) {
        edit();
    } else {
        throw omnetpp::cRuntimeError("could not dispatch signal: unknown signal received");
    }
}

void TraciDynamicSceneConfigProvider::edit() {
    // Emit signal before scene edit
    emit(sceneEditBeginSignal, simTime());
    api_->dynamicConfiguration()->edit();
    // Emit signal after scene edit
    emit(sceneEditEndSignal, simTime());
}
