#include <capi.pb.h>

#include <omnetpp/cexception.h>
#include <omnetpp/cmessage.h>
#include <omnetpp/csimplemodule.h>
#include <zmq.hpp>

#include <string>

#include "DummyDealer.h"

using namespace cavise;

Define_Module(DummyDealer);

namespace {



}

DummyDealer::DummyDealer()
    : context_(1)
    , socket_(context_, zmq::socket_type::dealer)
    , tick_(nullptr)
    , messageRegistry_(nullptr)
{}

void DummyDealer::initialize()
{
    const std::string address = par("address").stdstringValue();

    const int receiveTimeout = par("receiveTimeout").intValue();
    socket_.set(zmq::sockopt::rcvtimeo, receiveTimeout);

    const int sendTimeout = par("sendTimeout").intValue();
    socket_.set(zmq::sockopt::sndtimeo, sendTimeout);

    try {
        socket_.connect(address);
    } catch (const zmq::error_t& e) {
        throw omnetpp::cRuntimeError("failed to connect to %s: %s", address.c_str(), e.what());
    }

    EV_INFO << "connected to " << address << "\n";

    const char* registryPath = par("registryModule").stringValue();
    auto* registryModule = getModuleByPath(registryPath);
    messageRegistry_ = dynamic_cast<MessageRegistry*>(registryModule);
    if (!messageRegistry_) {
        throw omnetpp::cRuntimeError("DummyCapiDealer expected MessageRegistry module at path '%s'", registryPath);
    }

    tick_ = new omnetpp::cMessage("dummy-capi-tick");
    scheduleAt(omnetpp::simTime() + par("startDelay").doubleValue(), tick_);
}

void DummyDealer::handleMessage(omnetpp::cMessage* msg)
{
    if (msg != tick_) {
        delete msg;
        return;
    }

    sendRandomOpenCdaMessage();
    tryReceiveReply();

    scheduleAt(simTime() + par("sendInterval").doubleValue(), tick_);
}

void DummyDealer::finish()
{
    if (tick_) {
        if (tick_->isScheduled()) {
            cancelEvent(tick_);
        }

        delete tick_;
        tick_ = nullptr;
    }

    socket_.close();
}

void DummyDealer::sendRandomOpenCdaMessage()
{
    const capi::Message& message = messageRegistry_->appendMessage(*this);
    const int entities = message.opencda().entity_size();

    const std::size_t bytes = message.ByteSizeLong();
    zmq::message_t payload(bytes);
    if (!message.SerializeToArray(payload.data(), static_cast<int>(bytes))) {
        throw cRuntimeError("DummyCapiDealer failed to serialize OpenCDA message");
    }

    if (const auto res = socket_.send(payload, zmq::send_flags::none); !res.has_value()) {
        throw cRuntimeError("DummyCapiDealer send timeout expired");
    }

    EV_INFO << "DummyCapiDealer sent OpenCDA message order="
            << message.order() << " entities=" << entities << "\n";
}

void DummyDealer::tryReceiveReply()
{
    zmq::message_t payload;
    if (const auto rx = socket_.recv(payload, zmq::recv_flags::none); !rx.has_value()) {
        EV_WARN << "DummyCapiDealer did not receive reply before timeout\n";
        return;
    }

    capi::Message reply;
    if (!reply.ParseFromArray(payload.data(), static_cast<int>(payload.size()))) {
        EV_WARN << "DummyCapiDealer received non-protobuf reply payload\n";
        return;
    }

    std::string validation;
    if (messageRegistry_->confirmReceivedMessage(reply, validation)) {
        EV_INFO << "DummyCapiDealer validation success: " << validation << "\n";
    } else {
        EV_WARN << "DummyCapiDealer validation failure: " << validation << "\n";
    }
}
