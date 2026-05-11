#pragma once

#include <cavise/sionna/environment/api/SionnaAPI.h>
#include <cavise/sionna/environment/visualization/ISceneVisualizer.h>
#include <cavise/sionna/bridge/bindings/Camera.h>
#include <cavise/sionna/bridge/bindings/Constants.h>

#include <omnetpp/ccomponent.h>
#include <omnetpp/csimplemodule.h>

#include <string>
#include <filesystem>
#include <variant>
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
        using TCameraVariant = std::variant<std::string, std::pair<std::string, py::Camera>>;

        SionnaSceneVisualizer() = default;

        // omnetpp::cSimpleModule implementation.
        void initialize() override;
        void finish() override;

        // ISceneVisualizer implementation.
        void renderFrame() override;

        // omnetpp::cListener implementation.
        void receiveSignal(omnetpp::cComponent* source, omnetpp::simsignal_t signal, unsigned long value, omnetpp::cObject* details) override;

    private:
        std::filesystem::path framePath(const std::string& cameraId) const;

    private:
        ISionnaAPI* api_ = nullptr;
        PathLoss* pathLoss_ = nullptr;
        int frameIndex_ = 0;

        std::vector<TCameraVariant> cameras_;

        std::string outputDir_;
        py::RenderOptions renderOptions_;

        py::InteractionTypeColors interactionTypeColors_;
    };

} // namespace artery::sionna
