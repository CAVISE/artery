#pragma once

#include "artery/application/ItsG5Service.h"
#include "artery/application/NetworkInterface.h"
#include "cavise/artery.pb.h"
#include "cavise/capi/Listener.h"
#include "cavise/opencda.pb.h"

#include <cavise/capi.pb.h>
#include <omnetpp/cmessage.h>
#include <omnetpp/csimplemodule.h>
#include <omnetpp/simtime.h>


namespace cavise
{

class CosimService : public artery::ItsG5Service, public CAPIOpenCDAListener
{
public:
    CosimService();

    /* artery::ItsG5Service implementation */
    void trigger() override;
    void indicate(const vanetza::btp::DataIndication& ind, omnetpp::cPacket* packet, const artery::NetworkInterface& interface) override;

    /* CAPIOpenCDAListener implementation */
    void cStep(CAPI* api) override;

private:
    capi::Entity current_;
    std::vector<capi::ArteryMessage::Transmission> accumulatedTransmissions_;
};

}  // namespace cavise
