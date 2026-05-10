#include "Masquerade.h"

#include <omnetpp.h>

#include <artery/traci/VehicleController.h>

#include <deque>
#include <memory>
#include <optional>
#include <algorithm>

using namespace cavise;

Define_Module(Masquerade);

namespace {

    class TakeFirstAndHoldStrategy : public IMasqueradeStrategy {
    public:
        void newId(const std::string& newID) override {
            if (!id_.has_value()) {
                id_ = newID;
            }
        }

        std::optional<std::string> takeNewID() override {
            return id_;
        }

    private:
        std::optional<std::string> id_;
    };

    class SwapAsFastAsNewIdsArriveStrategy : public IMasqueradeStrategy {
    public:
        void newId(const std::string& newID) override {
            id_ = newID;
        }
        std::optional<std::string> takeNewID() override {
            return id_;
        }

    private:
        std::optional<std::string> id_;
    };

    class HoldIdsAndChooseRandomlyStrategy : public IMasqueradeStrategy {
    public:
        explicit HoldIdsAndChooseRandomlyStrategy(omnetpp::cComponent* owner)
            : owner_(owner) {
        }

        void newId(const std::string& newID) override {
            if (std::find(ids_.begin(), ids_.end(), newID) == ids_.end()) {
                ids_.push_back(newID);
            }
        }

        std::optional<std::string> takeNewID() override {
            if (ids_.empty()) {
                return std::nullopt;
            }

            return ids_.at(owner_->intuniform(0, static_cast<int>(ids_.size()) - 1));
        }

    private:
        omnetpp::cComponent* owner_;
        std::deque<std::string> ids_;
    };

    std::unique_ptr<IMasqueradeStrategy> makeStrategy(const std::string& strategy, omnetpp::cComponent* owner) {
        if (strategy == "first") {
            return std::make_unique<TakeFirstAndHoldStrategy>();
        }
        if (strategy == "swap") {
            return std::make_unique<SwapAsFastAsNewIdsArriveStrategy>();
        }
        if (strategy == "random") {
            return std::make_unique<HoldIdsAndChooseRandomlyStrategy>(owner);
        }

        throw omnetpp::cRuntimeError("Unknown masquerade strategy '%s'", strategy.c_str());
    }

} // namespace

void Masquerade::initialize() {
    CosimService::initialize();
    strategy_ = makeStrategy(par("masqueradeStrategy").stdstringValue(), this);
}

bool Masquerade::process(capi::Entity& message) {
    if (message.id().empty()) {
        return true;
    }

    const auto& vehicle = getFacilities().get_const<traci::VehicleController>();
    if (message.id() == vehicle.getVehicleId()) {
        return true;
    }

    strategy_->newId(message.id());
    EV_WARN << "Masquerade observed identity " << message.id() << " for vehicle " << vehicle.getVehicleId() << "\n";
    return true;
}

std::unique_ptr<capi::Entity> Masquerade::make(std::shared_ptr<artery::NetworkInterface>& /* iface */) {
    auto cur = std::make_unique<capi::Entity>(current());
    if (auto id = strategy_->takeNewID(); id.has_value()) {
        cur->set_id(id.value());
    }

    return cur;
}
