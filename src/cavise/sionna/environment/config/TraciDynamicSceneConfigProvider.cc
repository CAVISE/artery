#include "TraciDynamicSceneConfigProvider.h"

#include <traci/BasicNodeManager.h>
#include <traci/VariableCache.h>

#include <nanobind/nanobind.h>
#include <omnetpp/ccomponent.h>
#include <omnetpp/cexception.h>

#include <cavise/sionna/environment/Compat.h>
#include <cavise/sionna/bridge/bindings/SceneObject.h>
#include <cavise/sionna/environment/config/meshes/IMeshRegistry.h>

#include <algorithm>
#include <cmath>
#include <cstring>

using namespace artery::sionna;

Define_Module(TraciDynamicSceneConfigProvider);

namespace {

    mitsuba::Resolve::Vector3f velocityFromTraCI(double speed, traci::TraCIAngle heading) {
        const double yaw = (90.0 - heading.degree) * M_PI / 180.0;

        const double vx = speed * std::cos(yaw);
        const double vy = speed * -std::sin(yaw);

        return mitsuba::Resolve::Vector3f(
            fromScalar<mitsuba::Resolve::Float>(vx),
            fromScalar<mitsuba::Resolve::Float>(vy),
            fromScalar<mitsuba::Resolve::Float>(0.0));
    }

} // namespace

std::string TraciIdProvider::idFromTraci(const traci::BasicNodeManager::VehicleObject* vehicle) {
    const std::string id = vehicle->getCache()->getVehicleId();
    add(id);
    return traciToScene_.at(id);
}

std::string TraciIdProvider::idFromTraci(const traci::BasicNodeManager::PersonObject* person) {
    const std::string id = person->getCache()->getPersonId();
    add(id);
    return traciToScene_.at(id);
}

std::string TraciIdProvider::idToTraci(const std::string& id) {
    convertSceneID(id);
    return sceneToTraci_.at(id);
}

void TraciIdProvider::removeId(const traci::BasicNodeManager::VehicleObject* vehicle) {
    remove(vehicle->getCache()->getVehicleId());
}

void TraciIdProvider::removeId(const traci::BasicNodeManager::PersonObject* person) {
    remove(person->getCache()->getPersonId());
}

void TraciIdProvider::add(std::string id) {
    if (traciToScene_.contains(id)) {
        return;
    }

    std::string sceneId = id;
    std::replace(sceneId.begin(), sceneId.end(), '.', '_');

    if (sceneId.empty()) {
        throw omnetpp::cRuntimeError("could not convert TraCI ID: converted ID is empty");
    }

    const std::string base = sceneId;
    std::size_t suffix = 1;
    for (auto it = sceneToTraci_.find(sceneId);
         it != sceneToTraci_.end() && it->second != id;
         it = sceneToTraci_.find(sceneId)) {
        sceneId = base + "_" + std::to_string(suffix++);
    }

    traciToScene_.emplace(id, sceneId);
    sceneToTraci_.emplace(sceneId, std::move(id));
}

void TraciIdProvider::remove(std::string id) {
    convertTraciID(id);

    const std::string sceneId = traciToScene_.at(id);
    traciToScene_.erase(id);
    sceneToTraci_.erase(sceneId);
}

void TraciIdProvider::convertTraciID(const std::string& id) {
    if (!traciToScene_.contains(id)) {
        add(id);
    }
}

void TraciIdProvider::convertSceneID(const std::string& id) {
    if (!sceneToTraci_.contains(id)) {
        throw omnetpp::cRuntimeError("could not resolve TraCI ID for scene ID %s", id.c_str());
    }
}

std::string TraciIdProvider::sceneId(const std::string& id) {
    convertTraciID(id);
    return traciToScene_.at(id);
}

template <>
struct TraciDynamicSceneConfigProvider::EntityDispatchTraits<traci::BasicNodeManager::VehicleObject> {
    static std::string id(const traci::BasicNodeManager::VehicleObject* entity) {
        return entity->getCache()->getVehicleId();
    }

    static py::SceneObject& update(py::SceneObject& object, traci::BasicNodeManager::VehicleObject* entity) {
        object.setOrientation(convert<mitsuba::Resolve::Point3f>(entity->getHeading()));
        object.setPosition(convert<mitsuba::Resolve::Point3f>(entity->getPosition()));
        object.setVelocity(velocityFromTraCI(entity->getSpeed(), entity->getHeading()));
        return object;
    }
};

template <>
struct TraciDynamicSceneConfigProvider::EntityDispatchTraits<traci::BasicNodeManager::PersonObject> {
    static std::string id(const traci::BasicNodeManager::PersonObject* entity) {
        return entity->getCache()->getPersonId();
    }

    static py::SceneObject& update(py::SceneObject& object, traci::BasicNodeManager::PersonObject* entity) {
        object.setOrientation(convert<mitsuba::Resolve::Point3f>(entity->getHeading()));
        object.setPosition(convert<mitsuba::Resolve::Point3f>(entity->getPosition()));
        object.setVelocity(velocityFromTraCI(entity->getSpeed(), entity->getHeading()));
        return object;
    }
};

void TraciDynamicSceneConfigProvider::initialize() {
    if (const char* registryPath = par("meshRegistryModule").stringValue(); std::strlen(registryPath) > 0) {
        if (auto* registryModule = getModuleByPath(registryPath); registryModule == nullptr) {
            throw omnetpp::cRuntimeError("No mesh registry module found at path %s", registryPath);
        } else if (meshRegistry_ = dynamic_cast<IMeshRegistry*>(registryModule); meshRegistry_ == nullptr) {
            throw omnetpp::cRuntimeError("Module at path %s does not implement IMeshRegistry", registryPath);
        }
    }

    if (const char* managerPath = par("traciNodeManagerModule").stringValue(); std::strlen(managerPath) == 0) {
        throw omnetpp::cRuntimeError("traciNodeManagerModule was not specified: cannot continue");
    } else {
        if (traciNodeManager_ = getModuleByPath(managerPath); traciNodeManager_ == nullptr) {
            throw omnetpp::cRuntimeError("No TraCI node manager found at path %s", managerPath);
        }
    }

    if (meshRegistry_ == nullptr) {
        throw omnetpp::cRuntimeError("meshRegistryModule was not specified: cannot continue");
    }

    traciNodeManager_->subscribe(traci::BasicNodeManager::addVehicleSignal, this);
    traciNodeManager_->subscribe(traci::BasicNodeManager::removeVehicleSignal, this);
    traciNodeManager_->subscribe(traci::BasicNodeManager::updateVehicleSignal, this);
    traciNodeManager_->subscribe(traci::BasicNodeManager::addPersonSignal, this);
    traciNodeManager_->subscribe(traci::BasicNodeManager::removePersonSignal, this);
    traciNodeManager_->subscribe(traci::BasicNodeManager::updatePersonSignal, this);
    traciNodeManager_->subscribe(traci::BasicNodeManager::updateNodeSignal, this);
}

void TraciDynamicSceneConfigProvider::finish() {
    // Flush any remaining changes.
    edit();

    if (traciNodeManager_ == nullptr) {
        return;
    }

    traciNodeManager_->unsubscribe(traci::BasicNodeManager::addVehicleSignal, this);
    traciNodeManager_->unsubscribe(traci::BasicNodeManager::removeVehicleSignal, this);
    traciNodeManager_->unsubscribe(traci::BasicNodeManager::updateVehicleSignal, this);
    traciNodeManager_->unsubscribe(traci::BasicNodeManager::addPersonSignal, this);
    traciNodeManager_->unsubscribe(traci::BasicNodeManager::updatePersonSignal, this);
    traciNodeManager_->unsubscribe(traci::BasicNodeManager::removePersonSignal, this);
    traciNodeManager_->unsubscribe(traci::BasicNodeManager::updateNodeSignal, this);

    scene_.reset();
    pendingObjects_.clear();
    cachedObjects_.clear();
    toAdd_.clear();
    toRemove_.clear();
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

void TraciDynamicSceneConfigProvider::receiveSignal(omnetpp::cComponent* /* source */, omnetpp::simsignal_t signal, long /* value */, omnetpp::cObject* /* details */) {
    if (signal == traci::BasicNodeManager::updateNodeSignal) {
        edit();
    } else {
        throw omnetpp::cRuntimeError("could not dispatch signal: unknown signal received");
    }
}

void TraciDynamicSceneConfigProvider::receiveSignal(omnetpp::cComponent* /* source */, omnetpp::simsignal_t signal, unsigned long /* value */, omnetpp::cObject* /* details */) {
    if (signal == traci::BasicNodeManager::updateNodeSignal) {
        edit();
    } else {
        throw omnetpp::cRuntimeError("could not dispatch signal: unknown signal received");
    }
}

void TraciDynamicSceneConfigProvider::setScene(py::SionnaScene scene) {
    scene_ = std::move(scene);
}

void TraciDynamicSceneConfigProvider::dispatchUpdateVehicle(traci::BasicNodeManager::VehicleObject* vehicle) {
    dispatchUpdateEntity(vehicle);
}

void TraciDynamicSceneConfigProvider::dispatchUpdatePerson(traci::BasicNodeManager::PersonObject* person) {
    dispatchUpdateEntity(person);
}

void TraciDynamicSceneConfigProvider::dispatchRemoveVehicle(const char* id) {
    remove(id);
}

void TraciDynamicSceneConfigProvider::dispatchRemovePerson(const char* id) {
    remove(id);
}

void TraciDynamicSceneConfigProvider::dispatchAddVehicle(const char* id) {
    // NOTE: Update signal follows add signal. We may save vehicle to be added later.
    toAdd_.emplace(id);
}

void TraciDynamicSceneConfigProvider::dispatchAddPerson(const char* id) {
    // NOTE: Update signal follows add signal. We may save person to be added later.
    toAdd_.emplace(id);
}

void TraciDynamicSceneConfigProvider::receiveSignal(omnetpp::cComponent* /* source */, omnetpp::simsignal_t signal, omnetpp::cObject* object, omnetpp::cObject* /* details */) {
    if (signal != traci::BasicNodeManager::updatePersonSignal && signal != traci::BasicNodeManager::updateVehicleSignal) {
        throw omnetpp::cRuntimeError("could not dispatch signal: unknown signal received");
    }
}

IDynamicSceneConfigProvider& TraciDynamicSceneConfigProvider::add(const std::string& id, py::SceneObject object) {
    pendingObjects_.emplace(std::make_pair(id, object));
    return *this;
}

IDynamicSceneConfigProvider& TraciDynamicSceneConfigProvider::remove(const std::string& id) {
    if (cachedObjects_.contains(id)) {
        toRemove_.insert(id);
    } else if (pendingObjects_.contains(id)) {
        pendingObjects_.extract(id);
    }

    return *this;
}

void TraciDynamicSceneConfigProvider::edit() {
    if (pendingObjects_.empty() && toRemove_.empty()) {
        return;
    }

    std::vector<std::string> toRemove;
    toRemove.reserve(this->toRemove_.size());
    for (const auto& id : toRemove_) {
        toRemove.push_back(ids_.sceneId(id));
    }

    try {
        std::vector<py::SceneObject> toAdd;
        toAdd.reserve(pendingObjects_.size());
        for (const auto& [_, object] : pendingObjects_) {
            toAdd.push_back(object);
        }

        scene_->edit(toAdd, toRemove);
    } catch (const std::bad_cast&) {
        throw omnetpp::cRuntimeError(
            "failed to convert queued scene edit to python objects: add=%zu remove=%zu",
            pendingObjects_.size(),
            toRemove.size());
    }

    for (auto& [addedId, added] : pendingObjects_) {
        cachedObjects_.insert_or_assign(addedId, added);
    }

    toRemove_.clear();
    pendingObjects_.clear();
}
