#pragma once

#include "cavise/sionna/bridge/Fwd.h"
#include "cavise/sionna/bridge/bindings/SceneObject.h"
#include "omnetpp/cexception.h"
#include <cstdint>
#include <memory>
#include <omnetpp/csimplemodule.h>

#include <cavise/sionna/bridge/bindings/Scene.h>
#include <type_traits>

namespace artery::sionna {

    using mi = mitsuba::Resolve;

    // Coordinate systems present in sim.
    enum class CoordinateSystem : std::uint8_t {
        SUMO,
        INET,
        SIONNA_LOCAL,
        SIONNA_SCENE,
    };

    // Different places that define IDs. Note, these
    // are created mainly to sync objects in various simulators.
    enum class IDNamespace : std::uint8_t {
        SUMO,
        SIONNA,
    };

    // This proxy class exposes scene layout operations that are processed in batch.
    class IDynamicSceneConfigProxy {
    public:
        // This class binds to objects which is queued to be added and allows to also
        // queue geometric transforms.
        class ITransformProxy {
        public:
            // Apply post-edit transform to object, mainly used for scene geometry.
            void transform(std::function<void(py::SceneObject&)> function);
        };

        // Queues object to add.
        virtual std::unique_ptr<ITransformProxy> addObject(py::SceneObject object) = 0;

        // Queues object to remove.
        virtual std::unique_ptr<ITransformProxy> removeObject(py::SceneObject object) = 0;

        // Flush pending changes. All queued objects to add are added, all queued
        // objects to remove are removed. Then all transforms are called. Invalidates
        // mi.Scene, since mitsuba scene is rebuilt on this action.
        virtual void edit() = 0;

        // Faster path to access mitsuba scene, currently used by Sionna. Note,
        // on scene edits it changes.
        mitsuba::ref<mi::Scene> miScene();

        virtual ~IDynamicSceneConfigProxy() = default;
    };

    // This proxy handles coordinate transforms. Coordinates consist of 3 components (x, y, z).
    // Return type maybe one of DrJit arrays with 3 components, like Vector or Point.
    // When extending, prefer chaining transforms to achieve more robust approach.
    class ICoordinateTransformProxy {
    public:
        // There are 4 coordinate systems in total when simulation runs on par with SUMO:
        // 1) SUMO coordinates, used by Node Manager when propagating events from SUMO
        // 2) INET coordinates, used by mobility module. Actually, this is the most robust way to acquire coordinates.
        // 3) Local Sionna coordinates. Sionna coordinates may be inverted, causing axes changing places, so (Z, Y, Z) or (Y, Z, X) all might be valid.
        // Local coordinates are guaranteed to be in a form of (X, Y, Z), so the third component is always responsible for vertical position, etc.
        // 4) Sionna coordinates - returned by scene objects, and passed to them. Failing to cast to these will result in broken scene.
        template <CoordinateSystem from, CoordinateSystem to, typename T>
        T convertCoordinates();
    };

    // This proxy additionally exports operations on coordinates in scene.
    class ICoordinateOperationsProxy {
        // When transferring object from SUMO or other 2D simulators, use this function
        // to place object on terrain or other mesh, like road for example.
        void adjustVerticalComponent(py::SceneObject& object);
    };

    // This proxy handles ID conversions.
    class IIDConverterProxy {
        // Currently, we need to be able to convert IDs between SUMO and Sionna, since
        // requirements for those IDs differ a bit. Ideally, use the same ID everywhere for single entity
        // if possible. Commonly ID transforms are implemented as bidirectional maps.
        template <IDNamespace from, IDNamespace to>
        std::string convertID();
    };

    // This is an API class for common utility methods, used while
    // interacting with Sionna. This class may be extended further or replaced
    // if needed.
    class BaseSionnaAPI
        : public omnetpp::cModule {
    public:

        // Gets API module, errors in case it was not found.
        static BaseSionnaAPI* get(const omnetpp::cModule* module);

        /****************
         * Scene basics *
         ****************
         */

        // Access sionna scene object.
        py::SionnaScene& scene();

        // Access proxy class to make dynamic changes to the scene.
        std::unique_ptr<IDynamicSceneConfigProxy> dynamicConfiguration();



    protected:
        void initialize() override;
    };

} // namespace artery::sionna
