#pragma once

#include <cavise/sionna/bridge/SionnaBridge.h>
#include <cavise/sionna/environment/radio/SionnaRadioDeviceBase.h>

#include <omnetpp/clistener.h>

#include <inet/common/INETDefs.h>
#include <inet/physicallayer/base/packetlevel/PathLossBase.h>

#include <optional>
#include <unordered_map>

namespace artery::sionna {

    class PhysicalEnvironment;

    class PathLoss
        : public inet::physicallayer::PathLossBase
        , public omnetpp::cListener {
    public:
        static omnetpp::simsignal_t lossComputedSignal;
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
        double computePathLoss(inet::mps propagationSpeed, inet::Hz frequency, inet::m distance) const override;
        inet::m computeRange(inet::mps propagationSpeed, inet::Hz frequency, double loss) const override;

        const std::optional<py::Paths>& cachedPaths() const;

    protected:
        PhysicalEnvironment* resolvePhysicalEnvironment() const;
        omnetpp::cComponent* resolveSceneNotifier() const;
        void invalidateCachedPaths() const;
        void solveCachedPaths() const;
        void ensureSolved(
            const inet::physicallayer::INarrowbandSignal* narrowbandSignal,
            const inet::physicallayer::IScalarSignal* scalarSignal) const;

        static double freeSpacePathLoss(inet::mps propagationSpeed, inet::Hz frequency, inet::m distance);
        static inet::m freeSpaceRange(inet::mps propagationSpeed, inet::Hz frequency, double loss);

    protected:
        mutable PhysicalEnvironment* physicalEnvironment_ = nullptr;
        mutable omnetpp::cComponent* sceneNotifier_ = nullptr;
        mutable bool subscribedToSceneEdits_ = false;
        mutable bool subscribedToRadioDeviceEdits_ = false;
        mutable bool cachedPathsDirty_ = true;
        mutable std::optional<double> lastCarrierFrequencyHz_;
        mutable std::optional<double> lastBandwidthHz_;
        mutable std::optional<py::Paths> cachedPaths_;
        mutable std::unordered_map<std::string, std::size_t> txIndices_;
        mutable std::unordered_map<std::string, std::size_t> rxIndices_;

        std::optional<py::PathSolver> solver_;
        double maxRange_ = 0.0;
        bool requirePhysicalEnvironment_ = true;
        bool updateDynamicObjectsOnQuery_ = false;
        bool includeLineOfSight_ = true;
        bool includeReflections_ = true;
        bool includeDiffractions_ = false;
        int maxReflectionDepth_ = 1;
        int maxDiffractionDepth_ = 0;
    };

} // namespace artery::sionna
