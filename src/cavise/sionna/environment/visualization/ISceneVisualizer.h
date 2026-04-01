#pragma once

#include <cavise/sionna/bridge/bindings/Scene.h>

namespace artery::sionna {

    class ISceneVisualizer {
    public:
        virtual void setScene(py::SionnaScene scene) = 0;
        virtual void renderFrame() = 0;

        virtual ~ISceneVisualizer() = default;
    };

} // namespace artery::sionna
