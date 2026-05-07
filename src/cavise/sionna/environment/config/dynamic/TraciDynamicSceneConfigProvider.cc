#include "TraciDynamicSceneConfigProvider.h"

#include <traci/BasicNodeManager.h>
#include <traci/VariableCache.h>

#include <nanobind/nanobind.h>
#include <omnetpp/ccomponent.h>
#include <omnetpp/cexception.h>

#include <cavise/sionna/environment/Compat.h>
#include <cavise/sionna/environment/api/SionnaAPI.h>
#include <cavise/sionna/bridge/bindings/SceneObject.h>
#include <cavise/sionna/environment/config/meshes/IMeshRegistry.h>

using namespace artery::sionna;

Define_Module(TraciDynamicSceneConfigProvider);

omnetpp::simsignal_t TraciDynamicSceneConfigProvider::sceneEditedSignal = omnetpp::cComponent::registerSignal("sceneEdited");

namespace {

    template <typename T>
    T* resolveModule(omnetpp::cSimpleModule* owner, const char* moduleName) {
        if (auto path = owner->par(moduleName).stdstringValue(); path.size() == 0) {
            throw omnetpp::cRuntimeError("%s was not specified: cannot continue", moduleName);
        } else if (auto* module = owner->getModuleByPath(path.c_str()); module == nullptr) {
            throw omnetpp::cRuntimeError("No module found for %s at path %s", moduleName, path.c_str());
        } else if (auto* typed = dynamic_cast<T*>(module); typed == nullptr) {
            throw omnetpp::cRuntimeError("Module at path %s for %s does not implement required type", path.c_str(), moduleName);
        } else {
            return typed;
        }
    }

} // namespace

void TraciDynamicSceneConfigProvider::initialize() {
    api_ = ISionnaAPI::get(this);
    meshRegistry_ = resolveModule<IMeshRegistry>(this, "meshRegistryModule");
    traciNodeManager_ = resolveModule<traci::BasicNodeManager>(this, "traciNodeManagerModule");

    traciNodeManager_->subscribe(traci::BasicNodeManager::addVehicleSignal, this);
    traciNodeManager_->subscribe(traci::BasicNodeManager::removeVehicleSignal, this);
    traciNodeManager_->subscribe(traci::BasicNodeManager::updateVehicleSignal, this);
    traciNodeManager_->subscribe(traci::BasicNodeManager::addPersonSignal, this);
    traciNodeManager_->subscribe(traci::BasicNodeManager::removePersonSignal, this);
    traciNodeManager_->subscribe(traci::BasicNodeManager::updatePersonSignal, this);
    traciNodeManager_->subscribe(traci::BasicNodeManager::updateNodeSignal, this);
}

void TraciDynamicSceneConfigProvider::finish() {
    traciNodeManager_->unsubscribe(traci::BasicNodeManager::addVehicleSignal, this);
    traciNodeManager_->unsubscribe(traci::BasicNodeManager::removeVehicleSignal, this);
    traciNodeManager_->unsubscribe(traci::BasicNodeManager::updateVehicleSignal, this);
    traciNodeManager_->unsubscribe(traci::BasicNodeManager::addPersonSignal, this);
    traciNodeManager_->unsubscribe(traci::BasicNodeManager::updatePersonSignal, this);
    traciNodeManager_->unsubscribe(traci::BasicNodeManager::removePersonSignal, this);
    traciNodeManager_->unsubscribe(traci::BasicNodeManager::updateNodeSignal, this);

    knownSumoIds_.clear();
}

void TraciDynamicSceneConfigProvider::receiveSignal(omnetpp::cComponent* /* source */, omnetpp::simsignal_t signal, const char* id, omnetpp::cObject* details) {
    if (signal == traci::BasicNodeManager::addVehicleSignal) {
        dispatchAddVehicle(id);
    } else if (signal == traci::BasicNodeManager::addPersonSignal) {
        dispatchAddPerson(id);
    } else if (signal == traci::BasicNodeManager::removeVehicleSignal) {
        dispatchRemoveVehicle(id);
    } else if (signal == traci::BasicNodeManager::updateVehicleSignal) {
        if (auto* vehicle = dynamic_cast<traci::BasicNodeManager::VehicleObject*>(details); vehicle) {
            dispatchUpdateVehicle(vehicle);
        } else {
            throw omnetpp::cRuntimeError("could not dispatch signal: invalid vehicle update details for id %s", id);
        }
    } else if (signal == traci::BasicNodeManager::removePersonSignal) {
        dispatchRemovePerson(id);
    } else if (signal == traci::BasicNodeManager::updatePersonSignal) {
        if (auto* person = dynamic_cast<traci::BasicNodeManager::PersonObject*>(details); person) {
            dispatchUpdatePerson(person);
        } else {
            throw omnetpp::cRuntimeError("could not dispatch signal: invalid person update details for id %s", id);
        }
    } else {
        throw omnetpp::cRuntimeError("could not dispatch signal: unknown signal received");
    }
}

void TraciDynamicSceneConfigProvider::receiveSignal(omnetpp::cComponent* /* source */, omnetpp::simsignal_t signal, unsigned long /* value */, omnetpp::cObject* /* details */) {
    // This signals end-of-step, so scene is flushed here.
    if (signal == traci::BasicNodeManager::updateNodeSignal) {
        edit();
    } else {
        throw omnetpp::cRuntimeError("could not dispatch signal: unknown signal received");
    }
}

void TraciDynamicSceneConfigProvider::dispatchUpdateVehicle(traci::BasicNodeManager::VehicleObject* vehicle) {
    using idn = IDNamespace;
    using c = CoordinateSystem;

    const auto sumoId = vehicle->getCache()->getVehicleId();
    const auto sceneId = api_->IDConversion()->convertID(idn::SUMO, idn::SIONNA, sumoId);

    auto* config = api_->dynamicConfiguration();
    auto* transform = api_->coordinateTransform();

    std::unique_ptr<IDynamicSceneConfigProxy::ITransformProxy> object;
    if (!knownSumoIds_.contains(sumoId)) {
        auto mesh = meshRegistry_->asset(MeshAsset::LowPolyCar);
        auto material = meshRegistry_->material(MeshAsset::LowPolyCar);
        EV_WARN << "Persons right now are depicted as cars, mind that";

        object = config->addObject(py::SceneObject(mesh, sceneId, material));
        knownSumoIds_.insert(sumoId);
    } else {
        object = config->updateObject(sceneId);
    }

    TraCIVelocity traciVelocity{
        .speed = vehicle->getSpeed(),
        .heading = vehicle->getHeading(),
    };

    const auto canonicalOrientation = convert<mi::Point3f>(vehicle->getHeading());
    const auto canonicalPosition = transform->convertCoordinates(c::SUMO, c::SIONNA_SCENE, convert<mi::Vector3f>(vehicle->getPosition()));
    const auto canonicalVelocity = transform->convertCoordinates(c::SUMO, c::SIONNA_SCENE, convert<mi::Vector3f>(traciVelocity)) -
                                   transform->convertCoordinates(c::SUMO, c::SIONNA_SCENE, mi::Vector3f(0.0, 0.0, 0.0));

    const auto scaling = meshRegistry_->scaling(MeshAsset::LowPolyCar);
    const auto localOrientation = transform->convertCoordinates(c::SIONNA_SCENE, c::SIONNA_LOCAL, canonicalOrientation);
    const auto localVelocity = transform->convertCoordinates(c::SIONNA_SCENE, c::SIONNA_LOCAL, canonicalVelocity);
    const auto localPosition = transform->convertCoordinates(c::SIONNA_SCENE, c::SIONNA_LOCAL, canonicalPosition);

    object->transform([=](py::SceneObject& obj) {
        EV_DEBUG
            << "Updating SUMO-controlled vehicle: "
            << "orientation: " << localOrientation << " "
            << "position: " << localPosition << " "
            << "velocity: " << localVelocity << "\n";

        obj.setOrientation(localOrientation);
        obj.setPosition(localPosition);
        obj.setScaling(scaling);
        obj.setVelocity(localVelocity);

        transform->adjustVerticalComponent(obj);
    });
}

void TraciDynamicSceneConfigProvider::dispatchUpdatePerson(traci::BasicNodeManager::PersonObject* person) {
    using idn = IDNamespace;
    using c = CoordinateSystem;

    const auto sumoId = person->getCache()->getPersonId();
    const auto sceneId = api_->IDConversion()->convertID(idn::SUMO, idn::SIONNA, sumoId);

    auto* config = api_->dynamicConfiguration();
    auto* transform = api_->coordinateTransform();

    std::unique_ptr<IDynamicSceneConfigProxy::ITransformProxy> object;
    if (!knownSumoIds_.contains(sumoId)) {
        auto mesh = meshRegistry_->asset(MeshAsset::LowPolyCar);
        auto material = meshRegistry_->material(MeshAsset::LowPolyCar);
        EV_WARN << "Persons right now are depicted as cars, mind that";

        object = config->addObject(py::SceneObject(mesh, sceneId, material));
        knownSumoIds_.insert(sumoId);
    } else {
        object = config->updateObject(sceneId);
    }

    TraCIVelocity traciVelocity{
        .speed = person->getSpeed(),
        .heading = person->getHeading(),
    };

    const auto canonicalOrientation = convert<mi::Point3f>(person->getHeading());
    const auto canonicalPosition = transform->convertCoordinates(c::SUMO, c::SIONNA_SCENE, convert<mi::Vector3f>(person->getPosition()));
    const auto canonicalVelocity = transform->convertCoordinates(c::SUMO, c::SIONNA_SCENE, convert<mi::Vector3f>(traciVelocity)) -
                                   transform->convertCoordinates(c::SUMO, c::SIONNA_SCENE, mi::Vector3f(0.0, 0.0, 0.0));

    const auto scaling = meshRegistry_->scaling(MeshAsset::LowPolyCar);
    const auto localOrientation = transform->convertCoordinates(c::SIONNA_SCENE, c::SIONNA_LOCAL, canonicalOrientation);
    const auto localVelocity = transform->convertCoordinates(c::SIONNA_SCENE, c::SIONNA_LOCAL, canonicalVelocity);
    const auto localPosition = transform->convertCoordinates(c::SIONNA_SCENE, c::SIONNA_LOCAL, canonicalPosition);

    object->transform([=](py::SceneObject& obj) {
        EV_DEBUG
            << "Updating SUMO-controlled person: "
            << "orientation: " << localOrientation << " "
            << "position: " << localPosition << " "
            << "velocity: " << localVelocity << "\n";

        obj.setOrientation(localOrientation);
        obj.setPosition(localPosition);
        obj.setScaling(scaling);
        obj.setVelocity(localVelocity);

        transform->adjustVerticalComponent(obj);
    });
}

void TraciDynamicSceneConfigProvider::dispatchRemoveVehicle(const char* id) {
    EV_INFO << "Preparing to remove vehicle from the scene " << id;
    if (knownSumoIds_.erase(id) > 0) {
        const auto sceneId = api_->IDConversion()->convertID(IDNamespace::SUMO, IDNamespace::SIONNA, id);
        api_->dynamicConfiguration()->removeSceneItem(sceneId);
        api_->IDConversion()->removeID(IDNamespace::SUMO, id);
    }
}

void TraciDynamicSceneConfigProvider::dispatchRemovePerson(const char* id) {
    EV_INFO << "Preparing to remove person from the scene " << id;
    if (knownSumoIds_.erase(id) > 0) {
        const auto sceneId = api_->IDConversion()->convertID(IDNamespace::SUMO, IDNamespace::SIONNA, id);
        api_->dynamicConfiguration()->removeSceneItem(sceneId);
        api_->IDConversion()->removeID(IDNamespace::SUMO, id);
    }
}

void TraciDynamicSceneConfigProvider::dispatchAddVehicle(const char* id) {
    EV_INFO << "Adding new vehicle to the scene " << id << "\n";
}

void TraciDynamicSceneConfigProvider::dispatchAddPerson(const char* id) {
    EV_INFO << "Adding new person to the scene " << id << "\n";
}

void TraciDynamicSceneConfigProvider::edit() {
    api_->dynamicConfiguration()->edit();
    emit(sceneEditedSignal, 1UL);
}
