#pragma once

#include "cavise/artery.pb.h"
#include "cavise/opencda.pb.h"
#include <omnetpp/csimplemodule.h>
#include <atomic>
#include <mutex>
#include <thread>

// protos
#include <cavise/capi.pb.h>
#include <zmq.hpp>


namespace cavise {

    class CAPIManager : public omnetpp::cSimpleModule {
    public:

        const omnetpp::simsignal_t initSignal = cComponent::registerSignal("capi.init");
        const omnetpp::simsignal_t stepSignal = cComponent::registerSignal("capi.step");
        const omnetpp::simsignal_t closeSignal = cComponent::registerSignal("capi.close");

        void initialize() override;
        void finish() override;

        CAPIManager();

        /*
         * Add message to pending transmissions, in the end of current tick
         * all messages in current queue are merged and sent back to OpenCDA.
         *
         * NOTE: this method should be called once per each id until next step signal
         * is received.
        */
        void transmit(const std::string& id, capi::ArteryMessage message);

        /* common getters */
        const std::string& address() const;

    private:
        void run();

    private:
        struct Worker {

            Worker(std::thread&& thread);
            ~Worker();

            std::mutex mutex;
            std::thread thread;
            std::atomic<bool> abort;
        };

        std::unique_ptr<Worker> worker_;
        std::unordered_map<std::string, capi::ArteryMessage> cavMessages_;

        capi::OpenCDAMessage opencdaMessage_;

        std::string address_;

        zmq::context_t context_;
        zmq::socket_t socket_;
    };

}