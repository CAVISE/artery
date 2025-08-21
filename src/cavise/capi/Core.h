#pragma once

#include "ConnectionHandler.h"

#include <cavise/artery.pb.h>
#include <cavise/capi.pb.h>
#include <cavise/opencda.pb.h>
#include <omnetpp/cmessage.h>
#include <omnetpp/csimplemodule.h>
#include <omnetpp/simkerneldefs.h>

#include <memory>
#include <queue>

namespace cavise
{

class CAPI;

class CAPICore : public omnetpp::cSimpleModule
{
public:
    // CAPI & Core are realistically the same entity, but since
    // Omnet++ requires some unrelated to CAPI public methods,
    // we move them to separate class.
    friend CAPI;

    static const omnetpp::simsignal_t initSignal;
    static const omnetpp::simsignal_t stepSignal;
    static const omnetpp::simsignal_t closeSignal;

    /* cSimpleModule implementation */
    void finish() override;
    void initialize() override;
    void handleMessage(omnetpp::cMessage* msg) override;

    /* Access CAPI wrapper */
    CAPI* getCAPI();

    /* Access CAPI wrapper (readonly) */
    const CAPI* getCAPI() const;

private:
    capi::OpenCDAMessage OpenCDAMessage_;
    std::queue<capi::ArteryMessage::Transmission> transmissions_;

    omnetpp::cMessage* stepMessage_;
    omnetpp::SimTime updateInterval_;

    ICAPIConnectionHandler* handler_;
    std::unique_ptr<CAPI> api_;
};

class CAPI
{
public:
    CAPI(CAPICore* core);

    /* Add transmission to payload, upon sending CAPI will
     * concatenate transmissions into a single message.
     */
    void transmit(capi::ArteryMessage::Transmission message);

    /* Receive current OpenCDA message. */
    capi::OpenCDAMessage& OpenCDAState();

    /* Receive current OpenCDA message (readonly). */
    const capi::OpenCDAMessage& OpenCDAState() const;

private:
    CAPICore* core_;
};

}  // namespace cavise