#pragma once

#include <traci/BasicNodeManager.h>

#include <omnetpp/clistener.h>
#include <omnetpp/csimplemodule.h>

#include <cavise/sionna/bridge/bindings/Scene.h>
#include <cavise/sionna/bridge/bindings/SceneObject.h>
#include <cavise/sionna/environment/config/IConfigProvider.h>
#include <cavise/sionna/environment/config/meshes/IMeshRegistry.h>
#include <cavise/sionna/environment/config/TraciIDConverter.h>
#include <cavise/sionna/environment/config/TraciCoordinateTransformer.h>

#include <optional>
#include <memory>
#include <functional>
#include <unordered_map>

namespace artery::sionna {

    // Dynamic scene provider driven by TraCI node lifecycle signals.
    class TraciDynamicSceneConfigProvider
        : public IDynamicSceneConfigProvider
        , public omnetpp::cSimpleModule
        , public omnetpp::cListener {
    public:
        TraciDynamicSceneConfigProvider() = default;

        // omnetpp::cSimpleModule implementation.
        void initialize() override;
        void finish() override;

        // omnetpp::cListener implementation.
        void receiveSignal(omnetpp::cComponent* /* source */, omnetpp::simsignal_t signal, const char* id, omnetpp::cObject* /* details */) override;
        void receiveSignal(omnetpp::cComponent* /* source */, omnetpp::simsignal_t signal, unsigned long value, omnetpp::cObject* /* details */) override;

        // IDynamicSceneConfigProvider implementation.
        void setScene(py::SionnaScene scene) override;
        IDynamicSceneConfigProvider& add(const std::string& id, py::SceneObject object) override;
        IDynamicSceneConfigProvider& remove(const std::string& id) override;
        std::weak_ptr<py::SceneObject> fetch(const std::string& id) override;
        void edit() override;

    private:
        void dispatchUpdateVehicle(traci::BasicNodeManager::VehicleObject* vehicle);
        void dispatchUpdatePerson(traci::BasicNodeManager::PersonObject* person);
        void dispatchRemoveVehicle(const char* id);
        void dispatchRemovePerson(const char* id);
        void dispatchAddVehicle(const char* id);
        void dispatchAddPerson(const char* id);

    private:
        struct State {
        public:
            // Full state clear.
            void clear();

        public:
            std::vector<std::string> toRemove;

            // Current scene.
            std::optional<py::SionnaScene> scene;

            // Pending objects queued to be added. Once edit() is called, these are flushed
            // to the actual scene and then added to cached objects. This map is always searched first
            // for public API calls.
            std::unordered_map<std::string, std::shared_ptr<py::SceneObject>> pendingObjects;
            // Already added, but cached for later updates objects.
            std::unordered_map<std::string, std::shared_ptr<py::SceneObject>> cachedObjects;

            // Latest staged update callbacks by TraCI id.
            std::unordered_map<std::string, std::function<void(py::SceneObject&)>> stagedUpdates;
        } state_;

        traci::BasicNodeManager* traciNodeManager_ = nullptr;
        IMeshRegistry* meshRegistry_ = nullptr;
        ITraciIDConverter* IDConverter_ = nullptr;
        ITraciCoordinateTransformer* coordinateTransformer_ = nullptr;
    };

} // namespace artery::sionna
