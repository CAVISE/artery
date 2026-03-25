#pragma once

#include <artery/sionna/environment/providers/SceneConfigProvider.h>

#include <omnetpp/csimplemodule.h>

namespace artery::sionna {

    class SceneFileConfigProvider
        : public omnetpp::cSimpleModule
        , public ISceneConfigProvider {
    public:
        SceneConfig exportSceneConfig() const override;
        void initialize() override;

    private:
        SceneConfig config_;
    };

} // namespace artery::sionna
