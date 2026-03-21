#include "Core.h"

#include "omnetpp/csimulation.h"
#include "omnetpp/simkerneldefs.h"

#include <limits>
#include <memory>

using namespace cavise;

Define_Module(CAPICore);

const omnetpp::simsignal_t CAPICore::initSignal = cComponent::registerSignal("capi.init");
const omnetpp::simsignal_t CAPICore::stepSignal = cComponent::registerSignal("capi.step");
const omnetpp::simsignal_t CAPICore::closeSignal = cComponent::registerSignal("capi.close");

CAPICore::~CAPICore()
{
    cancelAndDelete(stepMessage_);
}

void CAPICore::initialize()
{
    if (omnetpp::cModule* parent = getParentModule(); !parent) {
        throw omnetpp::cRuntimeError("no parent module found");
    } else if (omnetpp::cModule* hmod = parent->getSubmodule("connection_handler"); !hmod) {
        throw omnetpp::cRuntimeError("connection_handler not found");
    } else {
        handler_ = dynamic_cast<ICAPIConnectionHandler*>(hmod);
        if (!handler_) {
            throw omnetpp::cRuntimeError("connection_handler does not implement ICAPIConnectionHandler");
        }
    }

    api_ = std::make_unique<CAPI>(this);

    updateInterval_ = par("updateInterval").doubleValue();
    stepMessage_ = new omnetpp::cMessage("CAPI step message");
    stepMessage_->setSchedulingPriority(std::numeric_limits<short>::min());

    auto* manager = getParentModule();
    auto* traciCore = manager ? manager->getModuleByPath(par("traciCoreModule")) : nullptr;
    if (!traciCore) {
        throw omnetpp::cRuntimeError("No traci.Core module found at %s", par("traciCoreModule").stringValue());
    }

    subscribeTraCI(traciCore);
}

void CAPICore::finish()
{
    unsubscribeTraCI();
    emit(closeSignal, true);
    if (started_) {
        handler_->stop();
    }
}

void CAPICore::handleMessage(omnetpp::cMessage* msg)
{
    if (msg != stepMessage_) {
        return;
    }

    runStep();
}

void CAPICore::runStep()
{
    if (capi::Message message = handler_->cReceive(); !message.has_opencda()) {
        EV_DEBUG << "CAPICore: no OpenCDA message available for this TraCI tick\n";
        return;
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
}

void CAPICore::traciInit()
{
    if (started_) {
        return;
    }

    handler_->connect();
    started_ = true;
    emit(initSignal, true);
}

void CAPICore::traciStep()
{
    if (!started_) {
        return;
    }

    runStep();
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
