#include "ots/Core.h"
#include "ots/GtuObject.h"
#include <boost/container_hash/hash.hpp> // for hashing sim0mq::Identifier
#include <omnetpp/cmessage.h>
#include <sim0mqpp/buffer_serialization.hpp>
#include <sim0mqpp/quantity.hpp>
#include <sim0mqpp/message.hpp>
#include <zmq.h>
#include <array>
#include <fstream>
#include <sstream>

namespace ots
{

Define_Module(Core)

namespace
{

const sim0mqpp::Identifier gtu_move_msg = std::string("GTU move");
const sim0mqpp::Identifier gtu_move_get_current_msg = std::string("GTU move|GET_CURRENT");
const sim0mqpp::Identifier gtu_network_msg = std::string("GTUs in network");
const sim0mqpp::Identifier gtu_network_get_current_msg = std::string("GTUs in network|GET_CURRENT");
const sim0mqpp::Identifier gtu_network_subscribe_add_msg = std::string("GTUs in network|SUBSCRIBE_TO_ADD");
const sim0mqpp::Identifier gtu_network_subscribe_remove_msg = std::string("GTUs in network|SUBSCRIBE_TO_REMOVE");
const sim0mqpp::Identifier sim_start_msg = std::string("NEWSIMULATION");
const sim0mqpp::Identifier sim_stop_msg = std::string("DIE");
const sim0mqpp::Identifier sim_until_msg = std::string("SIMULATEUNTIL");

// subscription IDs are arbitrarily chosen, just need to be distinct
const std::int32_t gtu_add_subscription = 0x20201;
const std::int32_t gtu_remove_subscription = 0x20202;

using namespace omnetpp;
const simsignal_t gtu_add_signal = cComponent::registerSignal("ots-gtu-add");
const simsignal_t gtu_remove_signal = cComponent::registerSignal("ots-gtu-remove");
const simsignal_t gtu_position_signal = cComponent::registerSignal("ots-gtu-position");

bool isGoodReply(const sim0mqpp::Message& msg)
{
    auto success = msg.get_payload<bool>(0);
    return success && *success;
}

const std::string& getReplyMessage(const sim0mqpp::Message& msg)
{
    static const std::string fallback = "(no message)";
    auto message = msg.get_payload<std::string>(1);
    return message ? *message : fallback;
}

} // namespace


Core::Core()
{
    m_zmq_context = zmq_ctx_new();
    if (!m_zmq_context) {
        throw omnetpp::cRuntimeError("Creation of ZMQ context failed: %s", zmq_strerror(errno));
    }

    m_zmq_socket = zmq_socket(m_zmq_context, ZMQ_PAIR);
    if (!m_zmq_socket) {
        throw omnetpp::cRuntimeError("Creation of ZMQ socket failed: %s", zmq_strerror(errno));
    }

    m_step_event = new omnetpp::cMessage("OTS step");
}

Core::~Core()
{
    if (m_network_loaded) {
        stopSimulation();
    }
    cancelAndDelete(m_step_event);

    if (m_zmq_socket) {
        zmq_close(m_zmq_socket);
    }

    if (m_zmq_context) {
        zmq_ctx_term(m_zmq_context);
    }
}

void Core::initialize()
{
    m_sim_federation = par("simFederation").stdstringValue();
    m_sim_sender = par("simSender").stdstringValue();
    m_sim_receiver = par("simReceiver").stdstringValue();

    // connect socket to OTS endpoint
    if (zmq_connect(m_zmq_socket, par("otsEndpoint").stringValue()) != 0) {
        throw omnetpp::cRuntimeError("Connecting to OTS endpoint failed: %s", zmq_strerror(errno));
    }

    // schedule first OTS step
    m_step_length = par("stepLength");
    scheduleAt(omnetpp::simTime(), m_step_event);
}

void Core::handleMessage(omnetpp::cMessage* msg)
{
    if (msg == m_step_event) {
        if (!m_network_loaded) {
            startSimulation(par("otsNetworkFile").stringValue());
            sendCommand(gtu_network_subscribe_add_msg, gtu_add_subscription);
            sendCommand(gtu_network_subscribe_remove_msg, gtu_remove_subscription);
            m_network_loaded = true;
        }

        // trigger OTS to advance by step length
        simulateUntil(omnetpp::simTime());
        scheduleAt(omnetpp::simTime() + m_step_length, m_step_event);

        // request all initial GTU positions
        requestGtuPositions();
    }
}

bool Core::receive(std::size_t maxlen, bool block)
{
    m_buffer.resize(maxlen);
    int length = zmq_recv(m_zmq_socket, m_buffer.data(), m_buffer.size(), block ? 0 : ZMQ_NOBLOCK);
    if (length < 0) {
        if (!block && errno == EAGAIN) {
            return false;
        } else {
            throw omnetpp::cRuntimeError("Receiving from OTS endpoint failed: %s", zmq_strerror(errno));
        }
    } else if (length > m_buffer.size()) {
        throw omnetpp::cRuntimeError("Length of replied message exceeds buffer size");
    } else {
        m_buffer.resize(length);
    }

    return true;
}

void Core::sendCommand(const sim0mqpp::Identifier& cmd)
{
    sendCommand(cmd, 0);
}

void Core::sendCommand(const sim0mqpp::Identifier& cmd, std::int32_t cmd_id)
{
    sendCommand(cmd, cmd_id, std::vector<sim0mqpp::Any> {});
}

void Core::sendCommand(const sim0mqpp::Identifier& cmd, std::vector<sim0mqpp::Any>&& payload)
{
    sendCommand(cmd, 0, std::move(payload));
}

void Core::sendCommand(const sim0mqpp::Identifier& cmd, std::int32_t cmd_id, std::vector<sim0mqpp::Any>&& payload)
{
    sim0mqpp::Message msg;
    msg.federation_id = m_sim_federation;
    msg.sender_id = m_sim_sender;
    msg.receiver_id = m_sim_receiver;
    msg.message_type_id = cmd;
    msg.message_id = cmd_id;
    msg.payload = std::move(payload);

    sim0mqpp::Buffer buffer;
    sim0mqpp::BufferSerializer output(buffer);
    sim0mqpp::serialize(output, msg);
    if (!buffer.empty()) {
        if (zmq_send(m_zmq_socket, buffer.data(), buffer.size(), 0) < 0) {
            throw omnetpp::cRuntimeError("zmq_send failed: %s", zmq_strerror(errno));
        }
    } else {
        EV_ERROR << "Will not send empty buffer as sim0mqpp message\n";
    }
}

void Core::queryResponses(const sim0mqpp::Identifier& wait_for)
{
    m_pending.insert(wait_for);
    while (receive(8192, !m_pending.empty())) {
        sim0mqpp::BufferDeserializer input(m_buffer);
        sim0mqpp::Message msg;
        deserialize(input, msg);
        if (input.good()) {
            m_pending.erase(msg.message_type_id);

            if (msg.message_type_id == sim_until_msg) {
                processSimulationStep(msg);
            } else if (msg.message_type_id == gtu_move_msg) {
                processGtuMove(msg);
            } else if (msg.message_type_id == gtu_network_msg) {
                std::int32_t* msg_id = boost::get<std::int32_t>(&msg.message_id);
                if (msg_id) {
                    if (*msg_id == 0) {
                        processCurrentNetwork(msg);
                    } else if (*msg_id == gtu_add_subscription) {
                        if (m_gtu_add_subscribed) {
                            processGtuAdd(msg);
                        } else {
                            processSubscriptionReply(msg, "GTU add");
                            m_gtu_add_subscribed = isGoodReply(msg);
                        }
                    } else if (*msg_id == gtu_remove_subscription) {
                        if (m_gtu_remove_subscribed) {
                            processGtuRemove(msg);
                        } else {
                            processSubscriptionReply(msg, "GTU remove");
                            m_gtu_remove_subscribed = isGoodReply(msg);
                        }
                    } else {
                        EV_WARN << "do not know how to process network message with ID " << *msg_id << "\n";
                    }
                } else {
                    EV_ERROR << "network message contains invalid ID\n";
                }
            } else if (msg.message_type_id == sim_start_msg) {
                processSimulationStart(msg);
            } else {
                EV_WARN << "ignoring message " << sim0mqpp::to_string(msg.message_type_id) << "\n";
            }
        } else {
            throw omnetpp::cRuntimeError("Decoding sim0mqpp message failed: %s", input.error_message().c_str());
        }
    }
}

void Core::processGtuAdd(const sim0mqpp::Message& msg)
{
    const auto t = msg.get_payload<sim0mqpp::Unit::Time>(0);
    const auto id = msg.get_payload<std::string>(1);
    if (id) {
        EV_DETAIL << "add GTU " << *id << "\n";
        emit(gtu_add_signal, id->c_str());
    } else {
        EV_ERROR << "cannot read ID of GTU to be added\n";
    }
}

void Core::processGtuRemove(const sim0mqpp::Message& msg)
{
    const auto t = msg.get_payload<sim0mqpp::Unit::Time>(0);
    const auto id = msg.get_payload<std::string>(1);
    if (id) {
        EV_DETAIL << "remove GTU " << *id << "\n";
        emit(gtu_remove_signal, id->c_str());
    } else {
        EV_ERROR << "cannot read ID of GTU to be removed\n";
    }
}

void Core::processCurrentNetwork(const sim0mqpp::Message& msg)
{
    for (const auto& gtu_id : msg.payload) {
        requestGtuPosition(gtu_id);
    }
}

void Core::processGtuMove(const sim0mqpp::Message& msg)
{
    auto id = msg.get_payload<std::string>(0);
    auto type = msg.get_payload<std::string>(1);
    auto pos = msg.get_payload<sim0mqpp::VectorQuantity<double>, sim0mqpp::Unit::Position>(2);
    auto heading = msg.get_payload<sim0mqpp::Unit::Direction>(3);
    auto speed = msg.get_payload<sim0mqpp::Unit::Speed>(4);
    auto accel = msg.get_payload<sim0mqpp::Unit::Acceleration>(5);

    if (id && type && pos && pos->values().size() == 3 && heading && speed && accel) {
        GtuObject gtu;
        gtu.setId(*id);
        gtu.setType(*type);
        gtu.setPosition({ pos->values()[0], pos->values()[1], pos->values()[2] });
        gtu.setHeadingRad(heading->value());
        gtu.setSpeed(speed->value());
        gtu.setAcceleration(accel->value());
        emit(gtu_position_signal, &gtu);
        EV_DETAIL << "GTU position update for ID " << *id << "\n";
    } else {
        EV_ERROR << "received broken GTU position\n";
    }
}

void Core::processSubscriptionReply(const sim0mqpp::Message& msg, const std::string& event)
{
    if (isGoodReply(msg)) {
        EV_DETAIL << "Subscribed to " << event << " events\n";
    } else {
        EV_ERROR << "Subscribing to " << event << " events failed: " << getReplyMessage(msg) << "\n";
    }
}

void Core::processSimulationStart(const sim0mqpp::Message& msg)
{
    if (isGoodReply(msg)) {
        EV_INFO << "OTS simulation started\n";
    } else {
        EV_ERROR << "starting OTS simulation failed: " << getReplyMessage(msg) << "\n";
    }
}

void Core::processSimulationStep(const sim0mqpp::Message& msg)
{
    if (isGoodReply(msg)) {
        // TODO add check if simulation times are in sync as soon as OTS reports time as scalar quantity
        EV_DETAIL << "OTS time has changed\n";
    } else {
        EV_ERROR << "OTS reported a problem advancing in time: " << getReplyMessage(msg) << "\n";
    }
}

void Core::startSimulation(const std::string& path)
{
    std::string ots_network;
    std::ifstream ifs(path);
    if (ifs) {
        // not really fast, but gets the job done for now
        std::ostringstream ss;
        ss << ifs.rdbuf();
        ots_network = ss.str();
    } else {
        throw omnetpp::cRuntimeError("Cannot read OTS network from %s", path);
    }

    omnetpp::SimTime ots_warmup = par("otsWarmupDuration");
    omnetpp::SimTime ots_run = par("otsRunDuration");
    std::int64_t ots_seed = par("otsSeed");

    std::vector<sim0mqpp::Any> payload;
    payload.push_back(ots_network);
    payload.push_back(sim0mqpp::ScalarQuantity<double> { ots_run.dbl(), sim0mqpp::Unit::Duration });
    payload.push_back(sim0mqpp::ScalarQuantity<double> { ots_warmup.dbl(), sim0mqpp::Unit::Duration });
    payload.push_back(ots_seed);

    sendCommand(sim_start_msg, std::move(payload));
}

void Core::stopSimulation()
{
    sendCommand(sim_stop_msg);
}

void Core::simulateUntil(omnetpp::SimTime time)
{
    std::vector<sim0mqpp::Any> payload;
    payload.push_back(sim0mqpp::ScalarQuantity<double> { time.dbl(), sim0mqpp::Unit::Time });
    sendCommand(sim_until_msg, std::move(payload));
    queryResponses(sim_until_msg);
}

void Core::requestGtuPositions()
{
    sendCommand(gtu_network_get_current_msg);
    queryResponses(gtu_network_msg);
}

void Core::requestGtuPosition(const sim0mqpp::Any& gtu_id)
{
    std::vector<sim0mqpp::Any> payload;
    payload.push_back(gtu_id);
    sendCommand(gtu_move_get_current_msg, std::move(payload));
    queryResponses(gtu_move_msg);
}

} // namespace ots
