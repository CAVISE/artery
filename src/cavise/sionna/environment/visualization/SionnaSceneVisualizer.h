#pragma once

#include <cavise/sionna/environment/api/SionnaAPI.h>
#include <cavise/sionna/environment/visualization/ISceneVisualizer.h>
#include <cavise/sionna/bridge/bindings/Camera.h>

#include <omnetpp/ccomponent.h>
#include <omnetpp/csimplemodule.h>

#include <string>
#include <filesystem>
#include <vector>
#include <utility>

namespace artery::sionna {

    class PathLoss;
    class TraciDynamicSceneConfigProvider;

    class SionnaSceneVisualizer
        : public ISceneVisualizer
        , public omnetpp::cListener
        , public omnetpp::cSimpleModule {
    public:
        SionnaSceneVisualizer() = default;

        // omnetpp::cSimpleModule implementation.
        void initialize() override;
        void finish() override;

        // ISceneVisualizer implementation.
        void renderFrame() override;

        // omnetpp::cListener implementation.
        void receiveSignal(omnetpp::cComponent* source, omnetpp::simsignal_t signal, unsigned long value, omnetpp::cObject* details) override;

    private:
        std::vector<std::pair<std::string, py::Camera>> resolveCameras() const;
        std::filesystem::path framePath(const std::string& cameraId) const;

    private:
        ISionnaAPI* api_ = nullptr;
        PathLoss* pathLoss_ = nullptr;
        int frameIndex_ = 0;

        struct {
            std::string outputDir;
            std::string camera;
            int spp = 16;
            int width = 1280;
            int height = 720;
        } renderParams_;
    };

} // namespace artery::sionna
