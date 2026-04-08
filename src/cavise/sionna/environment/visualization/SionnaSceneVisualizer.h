#pragma once

#include <cavise/sionna/environment/visualization/ISceneVisualizer.h>

#include <omnetpp/cmessage.h>
#include <omnetpp/csimplemodule.h>

#include <optional>
#include <string>

namespace artery::sionna {

    class SionnaSceneVisualizer
        : public ISceneVisualizer
        , public omnetpp::cSimpleModule {
    public:
        SionnaSceneVisualizer() = default;

        // omnetpp::cSimpleModule implementation.
        void initialize() override;
        void handleMessage(omnetpp::cMessage* msg) override;
        void finish() override;

        // ISceneVisualizer implementation.
        void setScene(py::SionnaScene scene) override;
        void renderFrame() override;

    private:
        void scheduleNextRender();

    private:
        std::optional<py::SionnaScene> scene_;
        omnetpp::cMessage* renderTimer_ = nullptr;

        std::string outputDir_;
        int frameIndex_ = 0;
        int spp_ = 16;
        int width_ = 1280;
        int height_ = 720;
        omnetpp::SimTime renderInterval_ = omnetpp::SimTime::ZERO;
    };

} // namespace artery::sionna
