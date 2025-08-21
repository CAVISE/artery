#pragma once

#include "ConnectionHandler.h"

#include <omnetpp/cexception.h>
#include <zmq.hpp>


using namespace cavise;

Define_Module(ZmqCAPIConnectionHandler);

ZmqCAPIConnectionHandler::ZmqCAPIConnectionHandler() : context_(1), socket_(context_, zmq::socket_type::pair)
{
}

void ZmqCAPIConnectionHandler::connect()
{
    auto address = par("address").stdstringValue();
    EV_DEBUG << "trying to pair with opencda on address: " << address;

    if (int timeout = par("receiveTimeout").intValue(); timeout > 0) {
        socket_.set(zmq::sockopt::rcvtimeo, timeout);
    }

    if (int timeout = par("sendTimeout").intValue(); timeout > 0) {
        socket_.set(zmq::sockopt::sndtimeo, timeout);
    }

    try {
        socket_.connect(address);
        EV << "connected to " << address << "\n";
    } catch (const zmq::error_t& e) {
        throw omnetpp::cRuntimeError("zmq error while trying to connect: %s", e.what());
    }

    EV_DEBUG << "connected to OpenCDA\n";
}

void ZmqCAPIConnectionHandler::cSend(capi::Message message)
{
    const std::size_t bytes = message.ByteSizeLong();
    zmq::message_t out{bytes};

    if (!message.SerializeToArray(out.data(), bytes)) {
        throw omnetpp::cRuntimeError("could not serialize protobuf message");
    }

    if (auto res = socket_.send(out, zmq::send_flags::none); !res.has_value()) {
        throw omnetpp::cRuntimeError("could not send message: timeout expired");
    }
}

capi::Message ZmqCAPIConnectionHandler::cReceive()
{
    zmq::message_t in;

    if (std::optional<size_t> rx = socket_.recv(in, zmq::recv_flags::none); !rx.has_value()) {
        throw omnetpp::cRuntimeError("could not send message: timeout expired");
    }

    capi::Message message;
    if (!message.ParseFromArray(in.data(), in.size())) {
        throw omnetpp::cRuntimeError("failed to parse OpenCDA protobuf message");
    }

    return message;
}

void ZmqCAPIConnectionHandler::stop()
{
    socket_.close();
}
