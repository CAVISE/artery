#include "ConnectionHandler.h"
#include <capi.pb.h>

#include <google/protobuf/message.h>
#include <omnetpp/cexception.h>
#include <zmq.hpp>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeinfo>

using namespace cavise;

Define_Module(ZmqCAPIConnectionHandler);

namespace {

    class RoutingMessage
        : public std::enable_shared_from_this<RoutingMessage> {
    public:

        /* construct message from frames. Parse fields optionally. */
        RoutingMessage(std::vector<zmq::message_t> frames);

        /* construct from actual payload */
        RoutingMessage(const std::string& identity, capi::Message message);

        /* receive routing message on socket */
        static std::shared_ptr<RoutingMessage> receive(zmq::socket_t& sock);

        /* send this message via a socket */
        void send(zmq::socket_t& sock);

        /* return sender identity from received frames */
        std::string identity() const;

        /* return message parsed from received frames */
        capi::Message payload() const;

    private:
        std::vector<zmq::message_t> frames_;
    };

    RoutingMessage::RoutingMessage(std::vector<zmq::message_t> frames)
        : frames_(std::move(frames))
    {}

    RoutingMessage::RoutingMessage(const std::string& identity, capi::Message message) {
        const std::size_t bytes = message.ByteSizeLong();
        zmq::message_t payload(bytes);
        if (!message.SerializeToArray(payload.data(), static_cast<int>(bytes))) {
            throw omnetpp::cRuntimeError("could not serialize ACK protobuf message");
        }

        frames_.emplace_back(identity.data(), identity.size());
        frames_.emplace_back(std::move(payload));
    }

    std::shared_ptr<RoutingMessage> RoutingMessage::receive(zmq::socket_t& sock) {
        std::vector<zmq::message_t> frames;

        while (true) {
            zmq::message_t part;
            if (auto rx = sock.recv(part, zmq::recv_flags::none); !rx.has_value()) {
                return nullptr;
            }

            const bool hasMore = part.more();
            frames.emplace_back(std::move(part));

            if (!hasMore) {
                break;
            }
        }

        return std::make_shared<RoutingMessage>(std::move(frames));
    }

    void RoutingMessage::send(zmq::socket_t& sock) {
        for (size_t i = 0; i < frames_.size(); ++i) {
            auto flags = (i + 1 < frames_.size()) ? zmq::send_flags::sndmore : zmq::send_flags::none;
            if (auto res = sock.send(frames_[i], flags); !res.has_value()) {
                throw omnetpp::cRuntimeError("could not send message part: timeout expired");
            }
        }
    }

    std::string RoutingMessage::identity() const {
        if (frames_.empty()) {
            throw std::out_of_range("frames should not be empty: cannot access identity");
        }

        const auto& identity = frames_.front();
        return std::string(static_cast<const char*>(identity.data()), identity.size());
    }

    capi::Message RoutingMessage::payload() const {
        if (frames_.empty()) {
            throw std::out_of_range("frames should not be empty: cannot access message");
        }

        capi::Message message;
        const zmq::message_t& payload = frames_.back();

        if (!message.ParseFromArray(payload.data(), static_cast<int>(payload.size()))) {
            throw std::bad_cast();
        }

        return message;
    }

}

ZmqCAPIConnectionHandler::ZmqCAPIConnectionHandler()
    : context_(1)
    , lastOrder_(0)
    , socket_(context_, zmq::socket_type::router)
{}

void ZmqCAPIConnectionHandler::connect()
{
    auto address = par("address").stdstringValue();
    EV_DEBUG << "binding ROUTER for OpenCDA on address: " << address;

    if (int timeout = par("receiveTimeout").intValue(); timeout > 0) {
        socket_.set(zmq::sockopt::rcvtimeo, timeout);
    }

    if (int timeout = par("sendTimeout").intValue(); timeout > 0) {
        socket_.set(zmq::sockopt::sndtimeo, timeout);
    }

    try {
        socket_.bind(address);
        EV << "bound to " << address << "\n";
    } catch (const zmq::error_t& e) {
        throw omnetpp::cRuntimeError("zmq error while trying to bind: %s", e.what());
    }

    EV_DEBUG << "ROUTER bound, waiting for OpenCDA DEALER\n";
}

void ZmqCAPIConnectionHandler::cSend(capi::Message message)
{
    if (identity_.empty()) {
        throw omnetpp::cRuntimeError("cannot send: missing ROUTER identity");
    }

    message.set_order(lastOrder_);

    RoutingMessage routingMessage { identity_, std::move(message) };
    routingMessage.send(socket_);

    identity_.clear();
}

capi::Message ZmqCAPIConnectionHandler::cReceive()
{
    while (true) {
        auto routingMessage = RoutingMessage::receive(socket_);
        if (!routingMessage) {
            throw omnetpp::cRuntimeError("could not receive ROUTER message: timeout expired");
        }

        const capi::Message& incoming = routingMessage->payload();
        if (!incoming.has_opencda()) {
            EV_WARN << "unexpected message type from OpenCDA peer; waiting for opencda payload\n";
            continue;
        }

        lastOrder_ = incoming.order();
        identity_ = routingMessage->identity();

        capi::Message ack;
        ack.mutable_ack();
        cSend(std::move(ack));

        return incoming;
    }
}

void ZmqCAPIConnectionHandler::stop()
{
    socket_.close();
}
