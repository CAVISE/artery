#include "Core.h"

#include "omnetpp/csimulation.h"
#include "omnetpp/simkerneldefs.h"

#include <memory>

using namespace cavise;

Define_Module(CAPICore);

const omnetpp::simsignal_t CAPICore::initSignal = cComponent::registerSignal("capi.init");
const omnetpp::simsignal_t CAPICore::stepSignal = cComponent::registerSignal("capi.step");
const omnetpp::simsignal_t CAPICore::closeSignal = cComponent::registerSignal("capi.close");

void CAPICore::initialize()
{
    if (omnetpp::cModule* parent = getParentModule(); !parent) {
        throw omnetpp::cRuntimeError("no parent module found");
    } else if (omnetpp::cModule* hmod = parent->getSubmodule("connection_handler"); !hmod) {
        throw omnetpp::cRuntimeError("connection_handler not found");
    } else {
        handler_ = reinterpret_cast<ICAPIConnectionHandler*>(hmod);
    }

    api_ = std::make_unique<CAPI>(this);

    updateInterval_ = par("updateInterval").doubleValue();
    stepMessage_ = new omnetpp::cMessage("CAPI step message");

    handler_->connect();
    emit(initSignal, true);

    scheduleAt(omnetpp::simTime() + updateInterval_, stepMessage_);
}

void CAPICore::finish()
{
    emit(closeSignal, true);

    cancelAndDelete(stepMessage_);
    handler_->stop();
}

void CAPICore::handleMessage(omnetpp::cMessage* msg)
{
    if (msg != stepMessage_) {
        return;
    }

    if (capi::Message message = handler_->cReceive(); !message.has_opencda()) {
        throw omnetpp::cRuntimeError("CAPICore: expected OpenCDA message");
    } else {
        OpenCDAMessage_.Swap(message.mutable_opencda());
    }

    emit(stepSignal, omnetpp::simTime());

    capi::ArteryMessage arteryMessage;
    while (!transmissions_.empty()) {
        arteryMessage.mutable_transmissions()->Add(std::move(transmissions_.front()));
        transmissions_.pop();
    }

    capi::Message message;
    message.mutable_artery()->Swap(&arteryMessage);
    handler_->cSend(message);

    scheduleAt(omnetpp::simTime() + updateInterval_, stepMessage_);
}

CAPI* CAPICore::getCAPI()
{
    return api_.get();
}

const CAPI* CAPICore::getCAPI() const
{
    return api_.get();
}

void CAPI::transmit(capi::ArteryMessage::Transmission message)
{
    core_->transmissions_.emplace(std::move(message));
}

capi::OpenCDAMessage& CAPI::OpenCDAState()
{
    return core_->OpenCDAMessage_;
}

const capi::OpenCDAMessage& CAPI::OpenCDAState() const
{
    return core_->OpenCDAMessage_;
}

CAPI::CAPI(CAPICore* core) : core_(core)
{
    ASSERT(core);
}
