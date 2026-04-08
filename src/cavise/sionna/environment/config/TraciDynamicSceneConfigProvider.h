#pragma once

#include <traci/BasicNodeManager.h>

#include <omnetpp/clistener.h>
#include <omnetpp/csimplemodule.h>

#include <cavise/sionna/bridge/bindings/Scene.h>
#include <cavise/sionna/bridge/bindings/SceneObject.h>
#include <cavise/sionna/environment/config/IConfigProvider.h>
#include <cavise/sionna/environment/config/meshes/IMeshRegistry.h>

#include <set>
#include <optional>
#include <unordered_map>

namespace artery::sionna {

    // Resolving ID's from traci to Sionna is actually not that
    // trivial, that class does that.
    class TraciIdProvider {
    public:

        // Query scene object ID from respective traci object.
        std::string idFromTraci(const traci::BasicNodeManager::VehicleObject* vehicle);
        // Query scene object ID from respective traci object.
        std::string idFromTraci(const traci::BasicNodeManager::PersonObject* person);
        // Query scene object ID from respective traci object.
        std::string idToTraci(const std::string& id);
        // Query scene-safe object ID from raw TraCI id.
        std::string sceneId(const std::string& id);

        // Remove ID associated with respective traci object.
        void removeId(const traci::BasicNodeManager::VehicleObject* vehicle);
        // Remove ID associated with respective traci object.
        void removeId(const traci::BasicNodeManager::PersonObject* person);

    private:
        // Removal is for traci ids only (scene cannot delete object by itself).
        void add(std::string id);
        void remove(std::string id);

        void convertTraciID(const std::string& id);
        void convertSceneID(const std::string& id);

    private:
        std::unordered_map<std::string, std::string> traciToScene_;
        std::unordered_map<std::string, std::string> sceneToTraci_;

    };

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
        void receiveSignal(omnetpp::cComponent* /* source */, omnetpp::simsignal_t signal, long value, omnetpp::cObject* /* details */) override;
        void receiveSignal(omnetpp::cComponent* /* source */, omnetpp::simsignal_t signal, unsigned long value, omnetpp::cObject* /* details */) override;
        void receiveSignal(omnetpp::cComponent* /* source */, omnetpp::simsignal_t signal, omnetpp::cObject* object, omnetpp::cObject* /* details */) override;

        // IDynamicSceneConfigProvider implementation.
        void setScene(py::SionnaScene scene) override;
        IDynamicSceneConfigProvider& add(const std::string& id, py::SceneObject object) override;
        IDynamicSceneConfigProvider& remove(const std::string& id) override;
        void edit() override;

    private:
        template <typename TEntity>
        struct EntityDispatchTraits;

        void dispatchUpdateVehicle(traci::BasicNodeManager::VehicleObject* vehicle);
        void dispatchUpdatePerson(traci::BasicNodeManager::PersonObject* person);

        template <typename TEntity>
        void dispatchUpdateEntity(TEntity* entity) {
            using Traits = EntityDispatchTraits<TEntity>;
            std::string id = Traits::id(entity);
            const std::string sceneId = ids_.idFromTraci(entity);

            // If object was added recently, we need to attach it to the scene
            // before Sionna allows transform updates.
            if (auto iter = toAdd_.find(id); iter != toAdd_.end()) {
                toAdd_.extract(iter);

                py::SceneObject object = [&]() {
                    try {
                        auto mesh = meshRegistry_->asset(MeshAsset::LowPolyCar);
                        auto material = meshRegistry_->material(MeshAsset::LowPolyCar);

                        return py::SceneObject(mesh, sceneId, material);
                    } catch (const std::bad_cast&) {
                        throw omnetpp::cRuntimeError("failed to construct SceneObject for entity %s from mesh registry result", id.c_str());
                    }
                }();
                add(id, object);
                edit();
            }

            if (auto pending = pendingObjects_.find(id); pending != pendingObjects_.end()) {
                Traits::update(pending->second, entity);
                return;
            }

            if (!cachedObjects_.contains(id)) {
                auto item = scene_->get(sceneId);
                if (auto* object = std::get_if<py::SceneObject>(&item); object != nullptr) {
                    cachedObjects_.insert_or_assign(id, *object);
                } else {
                    return;
                }
            }

            auto cached = cachedObjects_.find(id);
            Traits::update(cached->second, entity);
        }

        void dispatchRemoveVehicle(const char* id);
        void dispatchRemovePerson(const char* id);
        void dispatchAddVehicle(const char* id);
        void dispatchAddPerson(const char* id);

    private:
        omnetpp::cComponent* traciNodeManager_ = nullptr;
        IMeshRegistry* meshRegistry_ = nullptr;
        TraciIdProvider ids_;

        std::set<std::string> toAdd_;
        std::set<std::string> toRemove_;

        // Pending objects queued to be added.
        std::unordered_map<std::string, py::SceneObject> pendingObjects_;
        // Already added, but cached for later updates objects.
        std::unordered_map<std::string, py::SceneObject> cachedObjects_;

        std::optional<py::SionnaScene> scene_;
    };

} // namespace artery::sionna
