#pragma once

#include <zmq.hpp>
#include <omnetpp/csimplemodule.h>

#include <capi/capi.pb.h>


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
        zmq::context_t context_;
        zmq::socket_t socket_;
    };

}