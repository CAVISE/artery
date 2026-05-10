#pragma once

#include <cavise/capi/Listener.h>
#include <artery/application/ItsG5Service.h>
#include <artery/application/NetworkInterface.h>

#include <capi.pb.h>
#include <cavise_msgs/CosimMessage_m.h>

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
    void initialize() override;
    void finish() override;
    void trigger() override;
    void indicate(const vanetza::btp::DataIndication& ind, omnetpp::cPacket* packet, const artery::NetworkInterface& interface) override;

    /* CAPIOpenCDAListener implementation */
    void cStep(CAPI* api) override;

protected:
    // Process single message on indicate(). Return false to drop.
    virtual bool process(capi::Entity& message);
    // Make message for iface to send over it. Return nullptr to skip.
    virtual std::unique_ptr<capi::Entity> make(std::shared_ptr<artery::NetworkInterface>& iface);

    // Returns current payload.
    capi::Entity& current();
    // Returns current payload.
    const capi::Entity& current() const;

private:
    capi::Entity current_;
    std::vector<capi::Entity> accumulated_;
};

}  // namespace cavise
