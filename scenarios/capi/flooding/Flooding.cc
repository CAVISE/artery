#include "Flooding.h"

#include <artery/traci/VehicleController.h>

#include <omnetpp.h>

#include <memory>
#include <string>

using namespace cavise;

Define_Module(Flooding);

namespace {

    class RandomDropStrategy : public IFloodingStrategy {
    public:
        RandomDropStrategy(omnetpp::cComponent* owner, double dropProbability)
            : owner_(owner)
            , dropProbability_(dropProbability) {
        }

        bool shouldDrop() override {
            return owner_->uniform(0.0, 1.0) < dropProbability_;
        }

    private:
        omnetpp::cComponent* owner_;
        double dropProbability_;
    };

    class FloodedPeriodsStrategy : public IFloodingStrategy {
    public:
        FloodedPeriodsStrategy(omnetpp::cComponent* owner, const omnetpp::SimTime& clearMean, const omnetpp::SimTime& floodedMean)
            : owner_(owner)
            , clearMean_(clearMean)
            , floodedMean_(floodedMean) {
            generateNextPeriod(omnetpp::simTime());
        }

        bool shouldDrop() override {
            const auto now = omnetpp::simTime();
            while (now >= floodedUntil_) {
                generateNextPeriod(now);
            }

            return now >= floodedFrom_ && now < floodedUntil_;
        }

    private:
        omnetpp::SimTime sampleDuration(const omnetpp::SimTime& mean) {
            return omnetpp::SimTime(owner_->exponential(mean.dbl()));
        }

        void generateNextPeriod(const omnetpp::SimTime& after) {
            floodedFrom_ = after + sampleDuration(clearMean_);
            floodedUntil_ = floodedFrom_ + sampleDuration(floodedMean_);
        }

    private:
        omnetpp::cComponent* owner_;
        omnetpp::SimTime clearMean_;
        omnetpp::SimTime floodedMean_;
        omnetpp::SimTime floodedFrom_ = omnetpp::SimTime::ZERO;
        omnetpp::SimTime floodedUntil_ = omnetpp::SimTime::ZERO;
    };

    std::unique_ptr<IFloodingStrategy> makeStrategy(omnetpp::cComponent* owner) {
        const auto strategy = owner->par("floodingStrategy").stdstringValue();
        if (strategy == "random") {
            const auto dropProbability = owner->par("dropProbability").doubleValue();
            if (dropProbability < 0.0 || dropProbability > 1.0) {
                throw omnetpp::cRuntimeError("dropProbability must be in [0, 1], got %g", dropProbability);
            }

            return std::make_unique<RandomDropStrategy>(owner, dropProbability);
        }

        if (strategy == "periods") {
            const auto clearMean = omnetpp::SimTime(owner->par("clearPeriodMean").doubleValue());
            const auto floodedMean = omnetpp::SimTime(owner->par("floodedPeriodMean").doubleValue());
            if (clearMean <= omnetpp::SimTime::ZERO || floodedMean <= omnetpp::SimTime::ZERO) {
                throw omnetpp::cRuntimeError("clearPeriodMean and floodedPeriodMean must be positive");
            }

            return std::make_unique<FloodedPeriodsStrategy>(owner, clearMean, floodedMean);
        }

        throw omnetpp::cRuntimeError("Unknown flooding strategy '%s'", strategy.c_str());
    }

} // namespace

void Flooding::initialize() {
    CosimService::initialize();
    strategy_ = makeStrategy(this);
}

bool Flooding::process(capi::Entity& message) {
    if (!strategy_->shouldDrop()) {
        return true;
    }

    const auto& vehicle = getFacilities().get_const<traci::VehicleController>();
    EV_WARN << "Flooding dropped cosim payload " << message.id()
            << " on vehicle " << vehicle.getVehicleId() << "\n";
    return false;
}
