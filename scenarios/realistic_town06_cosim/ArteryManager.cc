//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 


#include "ArteryManager.h"
#include "artery/traci/VehicleController.h"
#include <omnetpp/cpacket.h>
#include <vanetza/btp/data_request.hpp>
#include <vanetza/dcc/profile.hpp>
#include <vanetza/geonet/interface.hpp>

#include <fstream>
#include <string>
#include <zmq.hpp>
#include "OpenCDA_message_structure.pb.h"
#include "Artery_message_structure.pb.h"
#include <google/protobuf/util/json_util.h>


using namespace omnetpp;
using namespace vanetza;


namespace artery
{
const std::string ArteryMessageFilename = "Memory_BIN/received_message_";
const std::string ArteryMessageJsonFilename = "Messages_JSON/received_message_";
const std::string OpenCDAMessageFilename = "Simulators_messages/OpenCDA_message.proto";
const std::string ArteryManagerIndicateLogsFilename = "Logs/ArteryManager_indicate_logs.log";
const std::string ArteryManagerTriggerLogsFilename = "Logs/ArteryManager_trigger_logs.log"; 





void logToFile(const std::string& filename, const std::string& message) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    char time_buffer[80];
    std::strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now_time));
    std::string time_str(time_buffer);

    std::ofstream outputFile(filename, std::ios::app);

    if (outputFile.is_open()) {
        outputFile << "[" << time_str << "] " << message << std::endl;
        outputFile.close();
    } else {
        std::cerr << "Error opening file " << filename << " for appending." << std::endl;
    }

}


static const simsignal_t scSignalCamReceived = cComponent::registerSignal("CamReceived");

Define_Module(ArteryManager)

ArteryManager::ArteryManager()
{
}
 
ArteryManager::~ArteryManager()
{
	cancelAndDelete(m_self_msg);
}

void ArteryManager::indicate(const btp::DataIndication& ind, cPacket* packet, const NetworkInterface& net)
{
    Enter_Method("indicate");

    if (packet->getByteLength() == 1024) {
		
		std::ofstream outputFile(ArteryManagerIndicateLogsFilename, std::ios::app);
		std::string log = "packet indication on channel " + std::to_string(net.channel) + "\n";
		logToFile(ArteryManagerIndicateLogsFilename, log);
        // Получаем данные из cPacket
        const char* json_c_str = packet->getName();
		int length = packet->getByteLength();
		
		structure_opencda::OpenCDA_message received_message;
		google::protobuf::util::Status status = google::protobuf::util::JsonStringToMessage(json_c_str, &received_message);
		if (!status.ok()) {
			EV_ERROR << "Error parsing JSON: " << status.ToString() << std::endl;
		}

		auto& vehicle = getFacilities().get_const<traci::VehicleController>();

		if (vehicle.getVehicleId().find("carla") != std::string::npos) {
			std::string Artery_filename = ArteryMessageFilename + vehicle.getVehicleId() + ".proto";
			std::string Artery_filename_json = ArteryMessageJsonFilename + vehicle.getVehicleId() + ".json";

			structure_artery::Artery_message artery_message;

			structure_artery::Artery_message::Received_information* received_info = artery_message.add_received_information();

			received_info->set_artery_vid(vehicle.getVehicleId());

			std::string prefix = "carla";
   			size_t pos = vehicle.getVehicleId().find(prefix);
			std::string numberStr = vehicle.getVehicleId().substr(pos + prefix.length());
			int vid = std::stoi(numberStr);

			auto& tp_cav = received_message.cav(vid).vid();

			received_info->set_vid(tp_cav);

			for(const auto& cav : received_message.cav()) {
				if (cav.vid() != received_info->vid()) {
					// Создаем новый объект Cav для сообщения Artery_message
					structure_artery::Artery_message::Received_information::Cav* new_cav = received_info->add_cav();
					// Заполняем новый объект значениями из объекта cav
					new_cav->set_vid(cav.vid());
					new_cav->set_ego_spd(cav.ego_spd());
					
					// Заполняем EgoPos
					structure_artery::Artery_message::Received_information::Cav::EgoPos* new_ego_pos = new_cav->mutable_ego_pos();
					new_ego_pos->set_x(cav.ego_pos().x());
					new_ego_pos->set_y(cav.ego_pos().y());
					new_ego_pos->set_z(cav.ego_pos().z());
					new_ego_pos->set_pitch(cav.ego_pos().pitch());
					new_ego_pos->set_yaw(cav.ego_pos().yaw());
					new_ego_pos->set_roll(cav.ego_pos().roll());
					
					// Заполняем BlueVehicles
					for (const auto& blue_cav : cav.blue_vehicles().blue_cav()) {
						structure_artery::Artery_message::Received_information::Cav::BlueVehicles::BlueCav* new_blue_cav = new_cav->mutable_blue_vehicles()->add_blue_cav();
						new_blue_cav->set_vid(blue_cav.vid());
						new_blue_cav->set_ego_spd(blue_cav.ego_spd());
						
						// Заполняем EgoPos для BlueCav
						structure_artery::Artery_message::Received_information::Cav::EgoPos* new_blue_ego_pos = new_blue_cav->mutable_ego_pos();
						new_blue_ego_pos->set_x(blue_cav.ego_pos().x());
						new_blue_ego_pos->set_y(blue_cav.ego_pos().y());
						new_blue_ego_pos->set_z(blue_cav.ego_pos().z());
						new_blue_ego_pos->set_pitch(blue_cav.ego_pos().pitch());
						new_blue_ego_pos->set_yaw(blue_cav.ego_pos().yaw());
						new_blue_ego_pos->set_roll(blue_cav.ego_pos().roll());
						
					}
					
					// Заполняем Vehicles
					for (const auto& cav_pos : cav.vehicles().cav_pos()) {
						structure_artery::Artery_message::Received_information::Cav::Vehicles::CavPos* new_cav_pos = new_cav->mutable_vehicles()->add_cav_pos();
						new_cav_pos->set_x(cav_pos.x());
						new_cav_pos->set_y(cav_pos.y());
						new_cav_pos->set_z(cav_pos.z());
					}

					// Заполняем TrafficLights
					for (const auto& tf_pos : cav.traffic_lights().tf_pos()) {
						structure_artery::Artery_message::Received_information::Cav::TrafficLights::TfPos* new_tf_pos = new_cav->mutable_traffic_lights()->add_tf_pos();
						new_tf_pos->set_x(tf_pos.x());
						new_tf_pos->set_y(tf_pos.y());
						new_tf_pos->set_z(tf_pos.z());
					}

					// Заполняем StaticObjects
					for (const auto& obj_pos : cav.static_objects().obj_pos()) {
						structure_artery::Artery_message::Received_information::Cav::StaticObjects::ObjPos* new_obj_pos = new_cav->mutable_static_objects()->add_obj_pos();
						new_obj_pos->set_x(obj_pos.x());
						new_obj_pos->set_y(obj_pos.y());
						new_obj_pos->set_z(obj_pos.z());
					}

					// Заполняем from_who_received
					for (const auto& from_who : cav.from_who_received()) {
						new_cav->add_from_who_received(from_who);
					}
					
				}
			}

            std::ofstream binary_file(Artery_filename, std::ios::binary);
            if (!binary_file.is_open()) {
                EV_ERROR << "Error opening Artery_message file for appending." << std::endl;
            } else {
                
                if (received_info->SerializeToOstream(&binary_file)) {
                    EV_INFO << "Received_information appended to Artery_message file successfully." << std::endl;
					std::string artery_message_str;
					artery_message.SerializeToString(&artery_message_str);


                } else {
                    EV_ERROR << "Error appending Received_information to Artery_message file." << std::endl;
                }
                binary_file.close();
            }
			google::protobuf::util::JsonOptions options;
    		options.add_whitespace = true; 
    		options.always_print_primitive_fields = true;
            std::string json_output;
			std::string json_output_to_logs;
            google::protobuf::util::MessageToJsonString(artery_message, &json_output, options);
			google::protobuf::util::MessageToJsonString(artery_message, &json_output_to_logs);
			std::string log = "JSON output: " + json_output_to_logs + '\n';
			logToFile(ArteryManagerIndicateLogsFilename, log);

			std::ofstream outputFile(Artery_filename_json);
			if (outputFile.is_open()) {
				outputFile << json_output << std::endl;
				EV_INFO << "JSON output writed" << std::endl;
				outputFile.close();
			} else {
				EV_ERROR << "Error opening JSON file " << ArteryMessageJsonFilename << std::endl;
			}
        }

    } 
    
    delete(packet);
}

void ArteryManager::initialize()
{
	ItsG5Service::initialize();
	m_self_msg = new cMessage("Artery Manager");
	subscribe(scSignalCamReceived);

	scheduleAt(simTime() + 3.0, m_self_msg);
}

void ArteryManager::finish()
{
	// you could record some scalars at this point
	ItsG5Service::finish();
}

void ArteryManager::handleMessage(cMessage* msg)
{
	Enter_Method("handleMessage");
	if (msg == m_self_msg) {
		EV_INFO << "self message\n";
	}
}

void ArteryManager::trigger()
{
	Enter_Method("trigger");

	// use an ITS-AID reserved for testing purposes
	static const vanetza::ItsAid example_its_aid = 16480;

	auto& mco = getFacilities().get_const<MultiChannelPolicy>();
	auto& networks = getFacilities().get_const<NetworkInterfaceTable>();

	for (auto channel : mco.allChannels(example_its_aid)) {
		auto network = networks.select(channel);
		if (network) {
			btp::DataRequestB req;
			// use same port number as configured for listening on this channel
			req.destination_port = host_cast(getPortNumber(channel));
			req.gn.transport_type = geonet::TransportType::SHB;
			req.gn.traffic_class.tc_id(static_cast<unsigned>(dcc::Profile::DP3));
			req.gn.communication_profile = geonet::CommunicationProfile::ITS_G5;
			req.gn.its_aid = example_its_aid;
			auto& vehicle = getFacilities().get_const<traci::VehicleController>();
			if (vehicle.getVehicleId().find("carla") != std::string::npos) {
				std::string OpenCDA_filename = OpenCDAMessageFilename;

				structure_opencda::OpenCDA_message opencda_message;
				std::ifstream file(OpenCDA_filename, std::ios::in | std::ios::binary);
				if (!file) {
					EV_ERROR << "Failed to open " << OpenCDA_filename << std::endl;
					throw cRuntimeError("Error while reading file");
				}
				if (!opencda_message.ParseFromIstream(&file)) {
					EV_ERROR << "Failed to parse " << OpenCDA_filename << std::endl;
					throw cRuntimeError("Error while parsing file");
				}
				file.close();

				int k = 0;
				for (const auto& cav : opencda_message.cav()) {
					std::string log = "Cav vid: " + cav.vid() + '\n';
					logToFile(ArteryManagerTriggerLogsFilename, log);
					EV_INFO << "Cav vid: " << cav.vid() << std::endl;
					k++;
				}
				std::string log = "Cav number: " + std::to_string(opencda_message.cav_size()) + '\n';
				logToFile(ArteryManagerTriggerLogsFilename, log);
				EV_INFO << "Cav number: " << k << std::endl;
				if (opencda_message.cav_size() > 0){
					std::string json_string;
					google::protobuf::util::JsonPrintOptions options;
					options.add_whitespace = true;
					options.always_print_primitive_fields = true;
					google::protobuf::util::MessageToJsonString(opencda_message, &json_string);

					const char *json_c_str = json_string.c_str();
					cPacket *packet = new cPacket(json_c_str);
					packet->setByteLength(1024);

					request(req, packet, network.get());
				}
			}
			cPacket *packet = new cPacket("Empty");
			packet->setByteLength(1);

			request(req, packet, network.get());

		} else {
			EV_ERROR << "No network interface available for channel " << channel << "\n";
		}
	}
}

void ArteryManager::receiveSignal(cComponent* source, simsignal_t signal, cObject*, cObject*)
{
	Enter_Method("receiveSignal");

	if (signal == scSignalCamReceived) {
		auto& vehicle = getFacilities().get_const<traci::VehicleController>();
		EV_INFO << "Vehicle " << vehicle.getVehicleId() << " received a CAM in sibling serivce\n";
	}
}

}