#pragma once

#include "artery/application/ItsG5Service.h"
#include "artery/application/NetworkInterface.h"
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

    /* CAPIOpenCDAListener implementation */
    void cStep(CAPI* api) override;

private:
    capi::OpenCDAMessage* current_;
    std::vector<capi::OpenCDAMessage> accumulated_;
};

}  // namespace cavise
