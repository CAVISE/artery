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

#include <cavise_msgs/CosimMessage_m.h>

#include "CosimService.h"

using namespace cavise;

Define_Module(CosimService)

    CosimService::CosimService() = default;

void CosimService::initialize() {
    ItsG5Service::initialize();

    const auto* capiCoreModulePath = par("capiCoreModule").stringValue();
    auto* capiCore = dynamic_cast<CAPICore*>(getModuleByPath(capiCoreModulePath));
    if (!capiCore) {
        throw omnetpp::cRuntimeError("CosimService: could not find CAPICore at path '%s'", capiCoreModulePath);
    }

    cSubscribe(capiCore);
    EV_INFO << "CosimService subscribed to CAPICore signals\n";
}

void CosimService::finish() {
    cUnsubscribe();
    ItsG5Service::finish();
}

void CosimService::indicate(const vanetza::btp::DataIndication& /* ind */, omnetpp::cPacket* packet, const artery::NetworkInterface& /* interface */) {
    const auto& vehicle = getFacilities().get_const<traci::VehicleController>();
    if (!strcmp(packet->getClassName(), "CosimMessage")) {
        EV_DEBUG << "receiving message (type=CosimMessage) on vehicle " << vehicle.getVehicleId();
    } else {
        EV_DEBUG << "receiving unknown message " << packet->getClassName() << " on vehicle " << vehicle.getVehicleId();
        delete packet;
        return;
    }

    CosimMessage* payload = nullptr;
    if (payload = dynamic_cast<CosimMessage*>(packet); payload == nullptr) {
        throw omnetpp::cRuntimeError("message is not what it seems to be");
    }

    capi::Entity receivedMessage;
    if (auto status = google::protobuf::util::JsonStringToMessage(payload->getContents(), &receivedMessage); !status.ok()) {
        EV_ERROR << "error parsing message contents: " << status.ToString() << "\n";
        delete packet;
        return;
    }

    if (process(receivedMessage)) {
        accumulated_.push_back(std::move(receivedMessage));
        EV_INFO << "stored cosim payload on vehicle " << vehicle.getVehicleId()
                << ", accumulated entities=" << accumulated_.size() << "\n";
    }

    delete packet;
}

void CosimService::trigger() {
    static const vanetza::ItsAid itsAid = 16480;
    const auto& mco = getFacilities().get_const<artery::MultiChannelPolicy>();
    const auto& networks = getFacilities().get_const<artery::NetworkInterfaceTable>();

    for (const auto& channel : mco.allChannels(itsAid)) {
        std::shared_ptr<artery::NetworkInterface> network = networks.select(channel);
        if (!network) {
            EV_DEBUG << "no network interface available for channel " << channel << '\n';
            continue;
        }

        vanetza::btp::DataRequestB req;
        // use same port number as configured for listening on this channel
        req.destination_port = vanetza::host_cast(getPortNumber(channel));
        req.gn.transport_type = vanetza::geonet::TransportType::SHB;
        req.gn.traffic_class.tc_id(static_cast<unsigned int>(vanetza::dcc::Profile::DP3));
        req.gn.communication_profile = vanetza::geonet::CommunicationProfile::ITS_G5;
        req.gn.its_aid = itsAid;

        auto* message = new CosimMessage;
        auto current = make(network);
        if (current == nullptr) {
            delete message;
            continue;
        }

        std::string payload;
        if (auto status = google::protobuf::util::MessageToJsonString(*current, &payload); !status.ok()) {
            EV_ERROR << "error serializing message contents: " << status.ToString() << "\n";
            delete message;
            continue;
        }

        message->setContents(payload.c_str());
        const auto& vehicle = getFacilities().get_const<traci::VehicleController>();
        EV_INFO << "sending cosim payload from vehicle " << vehicle.getVehicleId()
                << " on channel " << channel
                << ", payload bytes=" << payload.size() << "\n";

        request(req, message, network.get());
    }
}

void CosimService::cStep(CAPI* api) {
    capi::ArteryMessage::Transmission transmission;
    for (auto& message : accumulated_) {
        transmission.mutable_entity()->Add(std::move(message));
    }

    const auto& vehicle = getFacilities().get_const<traci::VehicleController>();
    std::string id = vehicle.getVehicleId();
    transmission.set_id(id);
    EV_INFO << "cStep for vehicle " << id
            << ": transmitting " << transmission.entity_size()
            << " accumulated entities to CAPI\n";

    api->transmit(std::move(transmission));
    accumulated_.clear();

    capi::OpenCDAMessage message = api->OpenCDAState();
    EV_INFO << "cStep for vehicle " << id
            << ": received OpenCDA state with " << message.entity_size()
            << " entities\n";

    for (auto& entity : *message.mutable_entity()) {
        if (entity.id() != id) {
            continue;
        }

        current_ = entity;
        EV_INFO << "updated current cosim entity for vehicle " << id << "\n";
        return;
    }

    EV_DEBUG << "no matching OpenCDA entity found for vehicle " << id << "\n";
}

bool CosimService::process(capi::Entity& /* message */) {
    return true;
}

std::unique_ptr<capi::Entity> CosimService::make(std::shared_ptr<artery::NetworkInterface>& /* iface */) {
    return std::make_unique<capi::Entity>(current_);
}

capi::Entity& CosimService::current() {
    return current_;
}

const capi::Entity& CosimService::current() const {
    return current_;
}
