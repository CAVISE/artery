#pragma once

// STD
#include <memory>

// artery
#include <artery/application/ItsG5Service.h>
#include <artery/application/NetworkInterface.h>
#include <omnetpp/cmessage.h>
#include <omnetpp/csimplemodule.h>
#include <omnetpp/simtime.h>

// communication
#include <cavise/comms/CommunicationManager.h>
#include <cavise/comms/SingletonHolder.h>

// protos
#include <capi.pb.h>


namespace cavise
{

class CosimService : public artery::ItsG5Service
{
public:
    using CommunicationManager = cavise::CommunicationManager<structure_capi::Artery_message, structure_capi::OpenCDA_message>;

    CosimService();

    void trigger() override;
    void indicate(const vanetza::btp::DataIndication& ind, omnetpp::cPacket* packet, const artery::NetworkInterface& interface) override;
    void receiveSignal(omnetpp::cComponent* source, omnetpp::simsignal_t signal, omnetpp::cObject* obj1, omnetpp::cObject* obj2) override;

protected:
    void initialize() override;

private:
    std::shared_ptr<CommunicationManager> communicationManager_;
};

}  // namespace cavise