#include "Replay.h"

#include <artery/traci/VehicleController.h>

#include <omnetpp.h>

#include <deque>
#include <memory>
#include <optional>
#include <string>

using namespace cavise;

Define_Module(Replay);

namespace {

    class FirstPayloadForeverStrategy : public IReplayStrategy {
    public:
        void save(const capi::Entity& message) override {
            if (!message_.has_value()) {
                message_ = message;
            }
        }

        std::optional<capi::Entity> replay() override {
            return message_;
        }

    private:
        std::optional<capi::Entity> message_;
    };

    class RepeatOldPayloadsStrategy : public IReplayStrategy {
    public:
        explicit RepeatOldPayloadsStrategy(int repeatCount)
            : repeatCount_(repeatCount) {
        }

        void save(const capi::Entity& message) override {
            messages_.push_back(message);
        }

        std::optional<capi::Entity> replay() override {
            if (!current_.has_value()) {
                if (messages_.empty()) {
                    return std::nullopt;
                }

                current_ = messages_.front();
                messages_.pop_front();
                remaining_ = repeatCount_;
            }

            auto result = current_;
            --remaining_;
            if (remaining_ <= 0) {
                current_.reset();
            }

            return result;
        }

    private:
        int repeatCount_;
        int remaining_ = 0;
        std::optional<capi::Entity> current_;
        std::deque<capi::Entity> messages_;
    };

    std::unique_ptr<IReplayStrategy> makeStrategy(omnetpp::cComponent* owner) {
        const auto strategy = owner->par("replayStrategy").stdstringValue();
        if (strategy == "first") {
            return std::make_unique<FirstPayloadForeverStrategy>();
        }

        if (strategy == "repeat") {
            const auto repeatCount = owner->par("replayCount").intValue();
            if (repeatCount <= 0) {
                throw omnetpp::cRuntimeError("replayCount must be positive, got %d", repeatCount);
            }

            return std::make_unique<RepeatOldPayloadsStrategy>(repeatCount);
        }

        throw omnetpp::cRuntimeError("Unknown replay strategy '%s'", strategy.c_str());
    }

} // namespace

void Replay::initialize() {
    CosimService::initialize();
    strategy_ = makeStrategy(this);
}

bool Replay::process(capi::Entity& message) {
    if (message.id().empty()) {
        return true;
    }

    const auto& vehicle = getFacilities().get_const<traci::VehicleController>();
    if (message.id() == vehicle.getVehicleId()) {
        return true;
    }

    strategy_->save(message);
    EV_WARN << "Replay captured stale payload " << message.id()
            << " on vehicle " << vehicle.getVehicleId() << "\n";
    return true;
}

std::unique_ptr<capi::Entity> Replay::make(std::shared_ptr<artery::NetworkInterface>& /* iface */) {
    if (auto replay = strategy_->replay(); replay.has_value()) {
        return std::make_unique<capi::Entity>(std::move(replay.value()));
    }

    return std::make_unique<capi::Entity>(current());
}
