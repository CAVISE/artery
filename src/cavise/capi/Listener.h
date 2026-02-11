#pragma once

#include <cavise/capi/Core.h>
#include <omnetpp/clistener.h>

namespace cavise
{

class CAPIOpenCDAListener : omnetpp::cListener
{
public:
    void cSubscribe(CAPICore* c);
    void cUnsubscribe();

    /* omnetpp::cListener implementation */
    void receiveSignal(omnetpp::cComponent* c, omnetpp::simsignal_t s, const omnetpp::SimTime& stamp, omnetpp::cObject* payload) override;

protected:
    virtual void cSetup(CAPI* api);
    virtual void cTearDown(CAPI* api);
    virtual void cStep(CAPI* api);

private:
    CAPICore* core_;
};

}  // namespace cavise
