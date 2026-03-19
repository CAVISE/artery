#include "ConnectionHandler.h"

#include <capi.pb.h>
#include <google/protobuf/message.h>
#include <omnetpp/cexception.h>
#include <zmq.hpp>
#include <string>

using namespace cavise;

Define_Module(ZmqCAPIConnectionHandler);

namespace {

    const char* messageKind(const capi::Message& message) {
        if (message.has_opencda()) {
            return "opencda";
        } else if (message.has_artery()) {
            return "artery";
        } else if (message.has_ack()) {
            return "ack";
        } else {
            return "unknown";
        }
    }

} // namespace

ZmqCAPIConnectionHandler::ZmqCAPIConnectionHandler()
    : context_(1)
    , lastOrder_(0)
    , socket_(context_, zmq::socket_type::router) {
}

void ZmqCAPIConnectionHandler::connect() {
    auto address = par("address").stdstringValue();
    EV_DEBUG << "binding ROUTER for OpenCDA on address: " << address;

    const int receiveTimeout = par("receiveTimeout").intValue();
    const int sendTimeout = par("sendTimeout").intValue();

    if (receiveTimeout > 0) {
        socket_.set(zmq::sockopt::rcvtimeo, receiveTimeout);
    }

    if (sendTimeout > 0) {
        socket_.set(zmq::sockopt::sndtimeo, sendTimeout);
    }

    try {
        socket_.bind(address);
        EV << "bound to " << address << "\n";
    } catch (const zmq::error_t& e) {
        throw omnetpp::cRuntimeError("zmq error while trying to bind: %s", e.what());
    }

    EV_DEBUG << "ROUTER bound, waiting for OpenCDA DEALER\n";
}

void ZmqCAPIConnectionHandler::cSend(capi::Message message) {
    if (identity_.empty()) {
        throw omnetpp::cRuntimeError("cannot send: missing ROUTER identity");
    }

    message.set_order(lastOrder_);
    EV_INFO << "sending CAPI message to peer '" << identity_
            << "' kind=" << messageKind(message)
            << " order=" << message.order() << "\n";
    sendFrames(identity_, message);

    identity_.clear();
}

capi::Message ZmqCAPIConnectionHandler::cReceive() {
    while (true) {
        capi::Message incoming;
        EV_INFO << "awaiting CAPI message from OpenCDA\n";
        try {
            receiveFrames(identity_, incoming);
        } catch (const zmq::error_t& e) {
            if (e.num() == EAGAIN) {
                EV_DEBUG << "receive timeout expired while waiting for OpenCDA; continuing without payload\n";
                identity_.clear();
                return {};
            }

            throw omnetpp::cRuntimeError("zmq error while receiving: %s", e.what());
        }
        EV_INFO << "received CAPI message from peer '" << identity_
                << "' kind=" << messageKind(incoming)
                << " order=" << incoming.order() << "\n";

        if (!incoming.has_opencda()) {
            EV_WARN << "unexpected message type from peer '" << identity_
                    << "' kind=" << messageKind(incoming)
                    << "; waiting for opencda payload\n";
            continue;
        }

        lastOrder_ = incoming.order();
        EV_INFO << "accepting OpenCDA payload from peer '" << identity_
                << "' with order=" << lastOrder_ << "\n";
        return incoming;
    }
}

void ZmqCAPIConnectionHandler::stop() {
    socket_.close();
}

zmq::message_t ZmqCAPIConnectionHandler::toMessage(const std::string& payload) const {
    return zmq::message_t(payload.data(), payload.size());
}

zmq::message_t ZmqCAPIConnectionHandler::toMessage(std::size_t payload) const {
    return zmq::message_t(&payload, sizeof(payload));
}

zmq::message_t ZmqCAPIConnectionHandler::toMessage(const capi::Message& payload) const {
    const auto bytes = payload.ByteSizeLong();
    zmq::message_t message(bytes);
    if (!payload.SerializeToArray(message.data(), bytes)) {
        throw omnetpp::cRuntimeError("could not serialize protobuf message");
    }

    return message;
}

void ZmqCAPIConnectionHandler::fromMessage(const zmq::message_t& message, std::string& payload) const {
    payload.assign(static_cast<const char*>(message.data()), message.size());
}

void ZmqCAPIConnectionHandler::fromMessage(const zmq::message_t& message, std::size_t& payload) const {
    if (message.size() != sizeof(std::size_t)) {
        throw omnetpp::cRuntimeError("could not deserialize size_t: unexpected payload size");
    }

    std::memcpy(&payload, message.data(), sizeof(payload));
}

void ZmqCAPIConnectionHandler::fromMessage(const zmq::message_t& message, capi::Message& payload) const {
    if (!payload.ParseFromArray(message.data(), message.size())) {
        throw omnetpp::cRuntimeError("could not deserialize protobuf message");
    }
}
