#include <zmq.hpp>

#include <capi.pb.h>
#include <omnetpp/cexception.h>
#include <omnetpp/cmessage.h>
#include <omnetpp/csimplemodule.h>

#include <cstdint>

#include "MessageRegistry.h"

namespace cavise {

    class DummyDealer
        : public omnetpp::cSimpleModule {
    public:
        DummyDealer();

        void initialize() override;
        void handleMessage(omnetpp::cMessage* msg) override;
        void finish() override;

    private:
        void sendRandomOpenCdaMessage();
        void tryReceiveReply();

    private:
        zmq::context_t context_;
        zmq::socket_t socket_;

        omnetpp::cMessage* tick_;
        MessageRegistry* messageRegistry_;
    };

}
