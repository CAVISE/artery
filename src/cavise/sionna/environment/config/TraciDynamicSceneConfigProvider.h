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

            // If object was added recently, we need to add it to the scene.
            if (auto iter = toAdd_.find(id); iter != toAdd_.end()) {
                toAdd_.extract(iter);

                // Create new object and call update on it to initialize.
                py::SceneObject object = [&]() {
                    try {
                        auto asset = meshRegistry_->getAsset(MeshAsset::LowPolyCar);
                        return py::SceneObject(asset.mesh, id, asset.material);
                    } catch (const std::bad_cast&) {
                        throw omnetpp::cRuntimeError("failed to construct SceneObject for entity %s from mesh registry result", id.c_str());
                    }
                }();
                add(id, Traits::update(object, entity));

                if (toAdd_.empty()) {
                    edit();
                }
            }

            if (auto pending = pendingObjects_.find(id); pending != pendingObjects_.end()) {
                Traits::update(pending->second, entity);
                return;
            }

            if (!cachedObjects_.contains(id)) {
                auto item = scene_->get(id);
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

        std::set<std::string> toAdd_;
        std::set<std::string> toRemove_;

        // Pending objects queued to be added.
        std::unordered_map<std::string, py::SceneObject> pendingObjects_;
        // Already added, but cached for later updates objects.
        std::unordered_map<std::string, py::SceneObject> cachedObjects_;

        std::optional<py::SionnaScene> scene_;
    };

} // namespace artery::sionna
