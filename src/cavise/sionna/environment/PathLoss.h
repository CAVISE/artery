#pragma once

#include <cavise/sionna/bridge/SionnaBridge.h>
#include <cavise/sionna/environment/api/SionnaAPI.h>
#include <cavise/sionna/environment/radio/SionnaRadioDeviceBase.h>

#include <omnetpp/clistener.h>

#include <inet/common/INETDefs.h>
#include <inet/physicallayer/pathloss/FreeSpacePathLoss.h>

#include <optional>
#include <unordered_map>

namespace artery::sionna {

    class PathLoss
        : public inet::physicallayer::FreeSpacePathLoss
        , public omnetpp::cListener {
    public:
        static omnetpp::simsignal_t pathsSolvedSignal;

        // omnetpp::cSimpleModule implementation.
        int numInitStages() const override;
        void initialize(int stage) override;
        void finish() override;

        // omnetpp::cListener implementation.
        void receiveSignal(omnetpp::cComponent* source, omnetpp::simsignal_t signal, unsigned long value, omnetpp::cObject* details) override;

        // inet::physicallayer::IPrintableObject implementation.
        std::ostream& printToStream(std::ostream& stream, int level) const override;

        // inet::physicallayer::IPathLoss implementation.
        double computePathLoss(const inet::physicallayer::ITransmission* transmission, const inet::physicallayer::IArrival* arrival) const override;

        const std::optional<py::Paths>& cachedPaths() const;

    protected:
        void invalidateCachedPaths() const;
        void solveCachedPaths(double carrierFrequencyHz, double bandwidthHz) const;

    protected:
        mutable ISionnaAPI* api_ = nullptr;
        mutable omnetpp::cComponent* sceneNotifier_ = nullptr;
        mutable std::optional<py::Paths> cachedPaths_;
        mutable std::unordered_map<std::string, std::size_t> txIndices_;
        mutable std::unordered_map<std::string, std::size_t> rxIndices_;

        std::optional<py::PathSolver> solver_;
        double maxRange_ = 0.0;
        bool includeLineOfSight_ = true;
        bool includeReflections_ = true;
        bool includeDiffractions_ = false;
        int maxReflectionDepth_ = 1;
        int maxDiffractionDepth_ = 0;
    };

} // namespace artery::sionna
