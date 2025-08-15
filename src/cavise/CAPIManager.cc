#include "CAPIManager.h"
#include "omnetpp/cexception.h"
#include "omnetpp/clog.h"
#include "omnetpp/simkerneldefs.h"

#include <omnetpp/ccomponent.h>
#include <omnetpp/cmodule.h>
#include <omnetpp/cevent.h>
#include <omnetpp/csimulation.h>

#include <google/protobuf/message.h>

#include <chrono>
#include <optional>
#include <stdexcept>
#include <utility>

namespace cavise {

CAPIManager::Worker::Worker(std::thread&& t)
    : thread(std::move(t)), abort(false) {}

CAPIManager::Worker::~Worker() {
    abort.store(true, std::memory_order_release);
    if (thread.joinable()) thread.join();
}

Define_Module(CAPIManager);

CAPIManager::CAPIManager()
    : context_(1)
    , socket_(context_, zmq::socket_type::pair) 
{}

const std::string& CAPIManager::address() const {
    return address_;
}

void CAPIManager::initialize() {
    // Parameters with defaults
    address_ = par("address").stdstringValue();
    if (address_.empty()) {
        address_ = "tcp://127.0.0.1:5555";
    }

    bool bind = par("bind").boolValue();
    EV_DEBUG << "CAPIManager: trying to pair with opencda on address: "
             << address_ << " (mode=" << ((bind) ? "bind" : "connect") << ")\n";

    const int rcvTimeoutMs = par("receiveTimeout").intValue();
    socket_.set(zmq::sockopt::rcvtimeo, rcvTimeoutMs);

    try {
        if (bind) {
            socket_.bind(address_);
            EV_INFO << "CAPIManager: bound at " << address_ << "\n";
        } else {
            socket_.connect(address_);
            EV_INFO << "CAPIManager: connected to " << address_ << "\n";
        }
    } catch (const zmq::error_t& e) {
        throw omnetpp::cRuntimeError(
            "CAPIManager: zmq error while trying to connect: %s",
            e.what()
        );
    }

    worker_ = std::make_unique<Worker>(std::thread(&CAPIManager::run, this));
    EV_DEBUG << "CAPIManager: worker thread started\n";

    emit(initSignal, true);
}

void CAPIManager::finish() {
    emit(closeSignal, true);

    worker_.reset();
    socket_.close();
    context_.close();
}

void CAPIManager::transmit(const std::string& id, capi::ArteryMessage message) {
    std::lock_guard<std::mutex> g(worker_->mutex);
    auto it = cavMessages_.find(id);
    if (it != cavMessages_.end()) {
        throw omnetpp::cRuntimeError("double transmission for cav: %s", id.c_str());
    }

    cavMessages_.emplace(id, std::move(message));
}

void CAPIManager::run() {
    using namespace std::chrono_literals;

    while (!worker_->abort) {
        zmq::message_t incomingMsg;
        std::size_t rx;

        if (
            std::optional<size_t> rx = socket_.recv(incomingMsg, zmq::recv_flags::none);
            !rx.has_value()
        ) {
            // timeout
            continue;
        }

        capi::OpenCDAMessage inProto;
        if (!inProto.ParseFromArray(incomingMsg.data(), static_cast<int>(incomingMsg.size()))) {
            LOG_EV << "CAPIManager: failed to parse OpenCDAMessage (" << incomingMsg.size() << " bytes)\n";
            // If parsing fails, skip this cycle
            continue;
        }

        {
            // Keep the most recent message visible to OMNeT++ side if needed
            std::lock_guard<std::mutex> g(worker_->mutex);
            opencdaMessage_.Swap(&inProto);  // move without extra copies
        }

        // Step boundary: OpenCDA has asked -> we respond with merged Artery view
        // Give OMNeT++ listeners a chance to react
        emit(stepSignal, true);

        // ---- Merge per-id Artery messages into one response ----
        capi::ArteryMessage outProto;
        {
            std::lock_guard<std::mutex> g(worker_->mutex);

            // Merge all per-id contributions
            for (auto &kv : cavMessages_) {
                outProto.MergeFrom(kv.second);
            }
            // Clear for the next step window
            cavMessages_.clear();
        }

        // ---- Send merged message back to OpenCDA (response) ----
        const auto bytes = outProto.ByteSizeLong();
        zmq::message_t outMsg(bytes);
        if (!outProto.SerializeToArray(outMsg.data(), static_cast<int>(bytes))) {
            LOG_EV << "CAPIManager: failed to serialize ArteryMessage (" << bytes << " bytes)\n";
            // If serialization fails, skip sending; proceed to next loop
            continue;
        }

        std::optional<size_t> tx = socket_.send(outMsg, zmq::send_flags::none);
        if (!tx.has_value()) {
            // EAGAIN or timeout; continue – OpenCDA should retry on its side
            LOG_EV << "CAPIManager: send() returned no value (EAGAIN/timeout)\n";
        }
    }
}

} // namespace cavise
