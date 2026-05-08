#pragma once

#include <cavise/sionna/environment/visualization/ISceneVisualizer.h>
#include <cavise/sionna/bridge/bindings/Camera.h>
#include <cavise/sionna/bridge/bindings/Scene.h>

#include <nanobind/nanobind.h>

#include <omnetpp/cmessage.h>
#include <omnetpp/csimplemodule.h>

#include <optional>
#include <string>
#include <filesystem>
#include <vector>
#include <utility>

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
        std::vector<std::pair<std::string, py::Camera>> resolveCameras() const;
        std::filesystem::path framePath(const std::string& cameraId) const;
        void scheduleNextRender();
        nanobind::object resolvePythonRenderer(const std::string& rendererSpec) const;

    private:
        std::optional<py::SionnaScene> scene_;
        omnetpp::cMessage* renderTimer_ = nullptr;
        nanobind::object pythonRenderer_;

        std::string outputDir_;
        std::string camera_;
        std::string pythonRendererSpec_;
        int frameIndex_ = 0;
        int spp_ = 16;
        int width_ = 1280;
        int height_ = 720;
        omnetpp::SimTime renderInterval_ = omnetpp::SimTime::ZERO;
    };

} // namespace artery::sionna
