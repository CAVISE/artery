#pragma once

#include <cavise/sionna/bridge/Fwd.h>
#include <cavise/sionna/bridge/bindings/Scene.h>
#include <cavise/sionna/bridge/bindings/SceneObject.h>

namespace artery::sionna {

    // Provides dynamic scene configuration, allows to create and remove objects dynamically.
    class IDynamicSceneConfigProvider {
    public:
        virtual void bindScene(py::SionnaScene scene) = 0;

        // Add object to the scene. This should cache the scene editing until edit() is called.
        virtual IDynamicSceneConfigProvider& add(const std::string& id, py::SceneObject object) = 0;
        // Remove object from the scene. This should cache the scene editing until edit() is called.
        virtual IDynamicSceneConfigProvider& remove(const std::string& id) = 0;
        // Fetch scene object. Pointer is valid until edit is called.
        virtual std::weak_ptr<py::SceneObject> fetch(const std::string& id) = 0;
        // Make changes to scene with all objects queued.
        virtual void edit() = 0;

        virtual ~IDynamicSceneConfigProvider() = default;
    };

} // namespace artery::sionna
