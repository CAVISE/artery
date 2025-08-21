#include "Listener.h"

#include "Core.h"

#include <omnetpp/simkerneldefs.h>

using namespace cavise;

void CAPIOpenCDAListener::cSubscribe(CAPICore* c)
{
    ASSERT(c);

    if (core_) {
        cUnsubscribe();
    }

    core_ = c;
    core_->subscribe(CAPICore::closeSignal, this);
    core_->subscribe(CAPICore::initSignal, this);
    core_->subscribe(CAPICore::stepSignal, this);
}

void CAPIOpenCDAListener::cUnsubscribe()
{
    ASSERT(core_);

    core_->unsubscribe(CAPICore::closeSignal, this);
    core_->unsubscribe(CAPICore::initSignal, this);
    core_->unsubscribe(CAPICore::stepSignal, this);
}

void CAPIOpenCDAListener::receiveSignal(
    omnetpp::cComponent* /* c */, omnetpp::simsignal_t s, const omnetpp::SimTime& /* stamp */, omnetpp::cObject* /* payload */)
{
    if (s == CAPICore::initSignal) {
        cSetup(core_->getCAPI());
    } else if (s == CAPICore::stepSignal) {
        cStep(core_->getCAPI());
    } else if (s == CAPICore::closeSignal) {
        cTearDown(core_->getCAPI());
    }
}

void CAPIOpenCDAListener::cSetup(CAPI* /* api */)
{
}

void CAPIOpenCDAListener::cTearDown(CAPI* /* api */)
{
}

void CAPIOpenCDAListener::cStep(CAPI* /* api */)
{
}
