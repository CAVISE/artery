// STD
#include "cavise/opencda.pb.h"
#include "omnetpp/cmessage.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>

// Artery
#include <artery/traci/VehicleController.h>
#include <omnetpp/cpacket.h>
#include <vanetza/btp/data_request.hpp>
#include <vanetza/dcc/profile.hpp>
#include <vanetza/geonet/interface.hpp>
#include <vanetza/net/osi_layer.hpp>
#include <vanetza/net/packet_variant.hpp>

// plog
#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Log.h>
#include <plog/Severity.h>

// proto
#include <google/protobuf/util/json_util.h>
// local
#include <cavise/application/CosimService.h>

using namespace cavise;

Define_Module(CosimService)

void CosimService::indicate(const vanetza::btp::DataIndication& /* ind */, omnetpp::cPacket* packet, const artery::NetworkInterface& /* interface */)
{
    // const auto& vehicle = getFacilities().get_const<traci::VehicleController>();
    // if (!strcmp(packet->getClassName(), STRINGIFY(OpencdaPayload))) {
    //     PLOG(plog::debug) << "receiving message " << STRINGIFY(OpencdaPayload) << " on vehicle " << vehicle.getVehicleId();
    // } else {
    //     PLOG(plog::warning) << "receiving unknown message " << packet->getClassName() << " on vehicle " << vehicle.getVehicleId();
    //     delete packet;
    //     return;
    // }

    // OpencdaPayload* payload = static_cast<OpencdaPayload*>(packet);
    // structure_capi::OpenCDA_message received_message;
    // if (auto status = google::protobuf::util::JsonStringToMessage(payload->getJson(), &received_message); !status.ok()) {
    //     PLOG(plog::debug) << "error parsing JSON: " << status.ToString();
    // }

    // std::string id = vehicle.getVehicleId();
    // PLOG(plog::debug) << "Vehicle ID: " << id;

    // if (!(id.rfind("rsu", 0) == 0 || id.rfind("cav", 0) == 0 || id.rfind("platoon", 0) == 0)) {
    //     PLOG(plog::debug) << "Skipping vehicle '" << id << "' — not RSU/CAV/Platoon";
    //     delete packet;
    //     return;
    // }

    // PLOG(plog::debug) << "Vehicle '" << id << "' passed prefix check";

    // auto message = std::make_unique<structure_capi::Artery_message>();
    // auto* received_info = message->add_received_information();
    // received_info->set_id(id);

    // std::string matchedPrefix;
    // for (const auto& prefix : {"rsu", "cav", "platoon"}) {
    //     if (id.rfind(prefix, 0) == 0) {
    //         matchedPrefix = prefix;
    //         PLOG(plog::debug) << "Matched prefix: " << matchedPrefix;
    //         break;
    //     }
    // }

    // if (matchedPrefix.empty()) {
    //     return;
    // }

    // for (const auto& entity : received_message.entity()) {
    //     PLOG(plog::debug) << "  - entity.id() = " << entity.id();
    // }

    // for (const auto& entity : received_message.entity()) {
    //     PLOG(plog::debug) << "Processing entity with ID: " << entity.id();
    //     if (entity.id() == received_info->id()) {
    //         PLOG(plog::debug) << "Skipping own entity ID: " << entity.id();
    //         continue;
    //     }

    //     auto* new_entity = received_info->add_entity();
    //     new_entity->set_id(entity.id());
    //     PLOG(plog::debug) << "Copying fields from entity " << entity.id();

    //     new_entity->CopyFrom(entity);
    // }

    // google::protobuf::util::JsonOptions options;
    // options.add_whitespace = true;
    // options.always_print_primitive_fields = true;

    // std::string json;
    // if (auto status = google::protobuf::util::MessageToJsonString(*message, &json, options); status.ok()) {
    //     PLOG(plog::debug) << "JSON: " << json;
    // } else {
    //     PLOG(plog::warning) << "failed to serialize to json: " << json;
    // }

    // // if (auto result = communicationManager_->push(id, std::move(message)); result.isError()) {
    // //     PLOG(plog::error) << "Error while adding artery message to the queue: " << result.error();
    // // }

    // delete packet;
}

void CosimService::trigger()
{
    static const vanetza::ItsAid its_aid = 16480;
    const auto& mco = getFacilities().get_const<artery::MultiChannelPolicy>();
    const auto& networks = getFacilities().get_const<artery::NetworkInterfaceTable>();

    for (const auto& channel : mco.allChannels(its_aid)) {
        std::shared_ptr<artery::NetworkInterface> network = networks.select(channel);
        if (!network) {
            PLOG(plog::warning) << "no network interface available for channel " << channel << "\n";
        }

        vanetza::btp::DataRequestB req;
        // use same port number as configured for listening on this channel
        req.destination_port = vanetza::host_cast(getPortNumber(channel));
        req.gn.transport_type = vanetza::geonet::TransportType::SHB;
        req.gn.traffic_class.tc_id(static_cast<unsigned int>(vanetza::dcc::Profile::DP3));
        req.gn.communication_profile = vanetza::geonet::CommunicationProfile::ITS_G5;
        req.gn.its_aid = its_aid;

        const auto& vehicle = getFacilities().get_const<traci::VehicleController>();
        std::string id = vehicle.getVehicleId();

        auto labels = {"rsu", "cav", "platoon"};
        if (!std::any_of(labels.begin(), labels.end(), [&id]() { return id.rfind(id, 0) == 0; })) {
            return;
        }

        EV_DEBUG << "Amount of CAVs and RSUs: " << current_->entity_size() << '\n';
        for (const auto& entity : current_->entity()) {
            EV_DEBUG << "Entity id: " << entity.id();
        }

        if (current_->entity_size() > 0) {
            auto* message = new omnetpp::cMessage;
            message->setName("");
            packet->setJson(json.c_str());
            packet->setSender(vehicle.getVehicleId().c_str());
            request(req, packet, network.get());
            continue;
        } else {
            PLOG(plog::debug) << "failed to serialize opencda payload: " << status.ToString();
        }
    }
}
}
