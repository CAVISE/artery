#include <cavise_msgs/CosimMessage_m.h>

#include <omnetpp.h>

#include <memory>
#include <string>

#include <artery/traci/VehicleController.h>
#include <vanetza/btp/data_request.hpp>
#include <vanetza/dcc/profile.hpp>
#include <vanetza/geonet/interface.hpp>
#include <vanetza/net/osi_layer.hpp>
#include <vanetza/net/packet_variant.hpp>

#include <google/protobuf/util/json_util.h>

#include "CosimService.h"

using namespace cavise;

Define_Module(CosimService)

void CosimService::indicate(const vanetza::btp::DataIndication& /* ind */, omnetpp::cPacket* packet, const artery::NetworkInterface& /* interface */)
{
    const auto& vehicle = getFacilities().get_const<traci::VehicleController>();
    if (!strcmp(packet->getClassName(), "CosimMessage")) {
        EV_DEBUG << "receiving message (type=CosimMessage) on vehicle " << vehicle.getVehicleId();
    } else {
        EV_DEBUG << "receiving unknown message " << packet->getClassName() << " on vehicle " << vehicle.getVehicleId();
        delete packet;
        return;
    }

    auto payload = static_cast<CosimMessage*>(packet);
    capi::ArteryMessage::Transmission receivedMessage;
    if (auto status = receivedMessage.ParseFromString(payload->getContents()); !status) {
        EV_ERROR << "error parsing message contents: " << status;
    }

    accumulatedTransmissions_.push_back(std::move(receivedMessage));
    delete packet;
}

void CosimService::trigger()
{
    static const vanetza::ItsAid itsAid = 16480;
    const auto& mco = getFacilities().get_const<artery::MultiChannelPolicy>();
    const auto& networks = getFacilities().get_const<artery::NetworkInterfaceTable>();

    for (const auto& channel : mco.allChannels(itsAid)) {
        std::shared_ptr<artery::NetworkInterface> network = networks.select(channel);
        if (!network) {
            EV_DEBUG << "no network interface available for channel " << channel << '\n';
        }

        vanetza::btp::DataRequestB req;
        // use same port number as configured for listening on this channel
        req.destination_port = vanetza::host_cast(getPortNumber(channel));
        req.gn.transport_type = vanetza::geonet::TransportType::SHB;
        req.gn.traffic_class.tc_id(static_cast<unsigned int>(vanetza::dcc::Profile::DP3));
        req.gn.communication_profile = vanetza::geonet::CommunicationProfile::ITS_G5;
        req.gn.its_aid = itsAid;

        const auto& vehicle = getFacilities().get_const<traci::VehicleController>();
        std::string id = vehicle.getVehicleId();

        EV_DEBUG << "Amount of CAVs and RSUs: " << current_.mutable_entity()->size() << '\n';
        for (const auto& entity : *current_.mutable_entity()) {
            EV_DEBUG << "Entity id: " << entity.id();
        }

        if (current_.mutable_entity()->size() > 0) {
            capi::ArteryMessage::Transmission transmission;
            transmission.set_id(id);
            transmission.mutable_entity()->CopyFrom(*current_.mutable_entity());

            auto* message = new CosimMessage;
            auto string = transmission.SerializeAsString();
            message->setContents(string.c_str());
            request(req, message, network.get());
            continue;
        }
    }
}

void CosimService::cStep(CAPI* api) {
    for (auto& message : accumulatedTransmissions_) {
        api->transmit(std::move(message));
    }

    current_ = api->OpenCDAState();
}
