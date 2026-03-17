#pragma once

#include <zmq.hpp>
#include <omnetpp/cexception.h>
#include <omnetpp/csimplemodule.h>

#include <capi.pb.h>
#include <cstddef>
#include <cstring>
#include <string>
#include <utility>

namespace cavise {

    /*
     * Abstract class that is used to make CAPI
     * sync receive/send calls.
     */
    class ICAPIConnectionHandler {
    public:
        virtual void connect() = 0;
        virtual void cSend(capi::Message message) = 0;
        virtual capi::Message cReceive() = 0;
        virtual void stop() = 0;
    };

    /* connector with zmq sockets */
    class ZmqCAPIConnectionHandler
        : public ICAPIConnectionHandler
        , public omnetpp::cSimpleModule {
    public:
        ZmqCAPIConnectionHandler();

        /* ICAPIConnectionHandler implementation */
        virtual void connect() override;
        virtual void cSend(capi::Message message) override;
        virtual capi::Message cReceive() override;
        virtual void stop() override;

    private:
        zmq::message_t toMessage(const std::string& payload) const;
        zmq::message_t toMessage(std::size_t payload) const;
        zmq::message_t toMessage(const capi::Message& payload) const;

        void fromMessage(const zmq::message_t& message, std::string& payload) const;
        void fromMessage(const zmq::message_t& message, std::size_t& payload) const;
        void fromMessage(const zmq::message_t& message, capi::Message& payload) const;

        template <typename... Args>
        void sendFrames(Args&&... args);

        template <typename... Args>
        void receiveFrames(Args&... args);

        template <typename T>
        void sendFrame(const T& payload, zmq::send_flags flags);

        template <typename T>
        void receiveFrame(T& payload, bool expectMore);

    private:
        zmq::context_t context_;
        zmq::socket_t socket_;

        std::string identity_;
        std::uint64_t lastOrder_;
    };

    template <typename T>
    void ZmqCAPIConnectionHandler::sendFrame(const T& payload, zmq::send_flags flags) {
        zmq::message_t message = toMessage(payload);
        if (auto res = socket_.send(message, flags); !res.has_value()) {
            throw omnetpp::cRuntimeError("could not send message part: timeout expired");
        }
    }

    template <typename... Args>
    void ZmqCAPIConnectionHandler::sendFrames(Args&&... args) {
        constexpr std::size_t size = sizeof...(Args);
        std::size_t index = 0;
        (sendFrame(std::forward<Args>(args), (++index < size) ? zmq::send_flags::sndmore : zmq::send_flags::none), ...);
    }

    template <typename... Args>
    void ZmqCAPIConnectionHandler::receiveFrames(Args&... args) {
        constexpr std::size_t total = sizeof...(Args);
        std::size_t index = 0;
        (receiveFrame(args, ++index < total), ...);
    }

    template <typename T>
    void ZmqCAPIConnectionHandler::receiveFrame(T& payload, bool expectMore) {
        zmq::message_t message;
        if (auto rx = socket_.recv(message, zmq::recv_flags::none); !rx.has_value()) {
            throw omnetpp::cRuntimeError("could not receive message part: timeout expired");
        }

        if (message.more() != expectMore) {
            throw omnetpp::cRuntimeError("received unexpected multipart ROUTER payload shape");
        }

        fromMessage(message, payload);
    }

} // namespace cavise
