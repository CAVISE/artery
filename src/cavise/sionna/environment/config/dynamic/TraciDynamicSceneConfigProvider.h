#pragma once

#include <traci/BasicNodeManager.h>

#include <omnetpp/clistener.h>
#include <omnetpp/csimplemodule.h>

#include <cavise/sionna/environment/api/SionnaAPI.h>

namespace artery::sionna {

    // Dynamic scene provider driven by TraCI node lifecycle signals.
    class TraciDynamicSceneConfigProvider
        : public omnetpp::cSimpleModule
        , public omnetpp::cListener {
    public:
        static omnetpp::simsignal_t sceneEditBeginSignal;
        static omnetpp::simsignal_t sceneEditEndSignal;

        TraciDynamicSceneConfigProvider() = default;

        // omnetpp::cSimpleModule implementation.
        void initialize() override;
        void finish() override;

        // omnetpp::cListener implementation.
        void receiveSignal(omnetpp::cComponent* /* source */, omnetpp::simsignal_t signal, unsigned long value, omnetpp::cObject* /* details */) override;

    private:
        void edit();

    private:
        ISionnaAPI* api_ = nullptr;
        traci::BasicNodeManager* traciNodeManager_ = nullptr;
        // Signals for notifying scene edit operations.
    };

} // namespace artery::sionna
