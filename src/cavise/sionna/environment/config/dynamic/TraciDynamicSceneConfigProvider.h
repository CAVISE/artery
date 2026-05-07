#pragma once

#include <traci/BasicNodeManager.h>

#include <omnetpp/clistener.h>
#include <omnetpp/csimplemodule.h>

#include <cavise/sionna/bridge/bindings/Scene.h>
#include <cavise/sionna/bridge/bindings/SceneObject.h>
#include <cavise/sionna/environment/api/SionnaAPI.h>
#include <cavise/sionna/environment/config/meshes/IMeshRegistry.h>

#include <unordered_set>

namespace artery::sionna {

    // Dynamic scene provider driven by TraCI node lifecycle signals.
    class TraciDynamicSceneConfigProvider
        : public omnetpp::cSimpleModule
        , public omnetpp::cListener {
    public:
        static omnetpp::simsignal_t sceneEditedSignal;

        TraciDynamicSceneConfigProvider() = default;

        // omnetpp::cSimpleModule implementation.
        void initialize() override;
        void finish() override;

        // omnetpp::cListener implementation.
        void receiveSignal(omnetpp::cComponent* /* source */, omnetpp::simsignal_t signal, const char* id, omnetpp::cObject* /* details */) override;
        void receiveSignal(omnetpp::cComponent* /* source */, omnetpp::simsignal_t signal, unsigned long value, omnetpp::cObject* /* details */) override;

    private:
        void edit();

        // dispatchers.
        void dispatchUpdateVehicle(traci::BasicNodeManager::VehicleObject* vehicle);
        void dispatchUpdatePerson(traci::BasicNodeManager::PersonObject* person);
        void dispatchRemoveVehicle(const char* id);
        void dispatchRemovePerson(const char* id);
        void dispatchAddVehicle(const char* id);
        void dispatchAddPerson(const char* id);

    private:
        ISionnaAPI* api_ = nullptr;
        traci::BasicNodeManager* traciNodeManager_ = nullptr;
        IMeshRegistry* meshRegistry_ = nullptr;
        std::unordered_set<std::string> knownSumoIds_;
    };

} // namespace artery::sionna
