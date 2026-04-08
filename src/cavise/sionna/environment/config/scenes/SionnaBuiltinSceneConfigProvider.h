#pragma once

#include <omnetpp/csimplemodule.h>

#include <cavise/sionna/environment/config/scenes/FileSceneConfigProvider.h>

namespace artery::sionna {

    // Uses static mapping to extract built-in Sionna scenes. Inspect mapping
    // to get available scene names.
    class SionnaBuiltinSceneConfigProvider
        : public FileSceneConfigProvider  {
    public:
        SionnaBuiltinSceneConfigProvider() = default;

        // omnetpp::cSimpleModule implementation.
        void initialize() override;
    };

} // namespace artery::sionna
