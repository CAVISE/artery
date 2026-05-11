#pragma once

#include <cavise/sionna/bridge/SionnaBridge.h>
#include <cavise/sionna/environment/api/SionnaAPI.h>
#include <cavise/sionna/environment/radio/SionnaRadioDeviceBase.h>
#include <cavise/sionna/environment/config/dynamic/DynamicSceneConfigListener.h>

#include <omnetpp/clistener.h>

#include <inet/common/INETDefs.h>
#include <inet/physicallayer/pathloss/FreeSpacePathLoss.h>

#include <optional>
#include <unordered_map>

namespace artery::sionna {

    class PathLoss
        : public inet::physicallayer::FreeSpacePathLoss
        , public DynamicSceneConfigListener  {
    public:
        static omnetpp::simsignal_t pathsSolved;

        // omnetpp::cSimpleModule implementation.
        int numInitStages() const override;
        void initialize(int stage) override;
        void finish() override;

        // DynamicSceneConfigListener implementation.
        void onDynamicSceneEdited() override;

        // inet::physicallayer::IPrintableObject implementation.
        std::ostream& printToStream(std::ostream& stream, int level) const override;

        // inet::physicallayer::IPathLoss implementation.
        double computePathLoss(const inet::physicallayer::ITransmission* transmission, const inet::physicallayer::IArrival* arrival) const override;
        double computePathLoss(const inet::physicallayer::IRadio* receiverRadio, const inet::physicallayer::ITransmission* transmission, const inet::physicallayer::IArrival* arrival) const;

        const std::optional<py::Paths>& cachedPaths() const;

    protected:
        void invalidateCachedPaths() const;
        void solveCachedPaths() const;

    private:
        mutable ISionnaAPI* api_ = nullptr;
        std::optional<py::PathSolver> solver_;

        mutable struct {
            std::optional<py::Paths> object;
            std::unordered_map<std::string, std::size_t> txIndices;
            std::unordered_map<std::string, std::size_t> rxIndices;
        } paths_;

        py::PathSolverOptions solverOptions_;
        double maxRange_ = 1000.0;

    };

} // namespace artery::sionna
