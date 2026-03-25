#pragma once

#include <string>

namespace artery::sionna {

    struct SceneConfig {
        std::string sceneFile;
    };

    class ISceneConfigProvider {
    public:
        virtual ~ISceneConfigProvider() = default;
        virtual SceneConfig exportSceneConfig() const = 0;
    };

} // namespace artery::sionna
