#include "TraciDynamicSceneConfigProvider.h"

#include <traci/BasicNodeManager.h>
#include <traci/VariableCache.h>

#include <nanobind/nanobind.h>
#include <omnetpp/ccomponent.h>
#include <omnetpp/cexception.h>

#include <cavise/sionna/environment/Compat.h>
#include <cavise/sionna/bridge/bindings/SceneObject.h>
#include <cavise/sionna/environment/config/meshes/IMeshRegistry.h>
#include <cavise/sionna/environment/config/dynamic/TraciRelaxedCoordinateTransformer.h>

using namespace artery::sionna;

Define_Module(TraciDynamicSceneConfigProvider);

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
    meshRegistry_ = resolveModule<IMeshRegistry>(this, "meshRegistryModule");
    traciNodeManager_ = resolveModule<traci::BasicNodeManager>(this, "traciNodeManagerModule");
    IDConverter_ = resolveModule<ITraciIDConverter>(this, "idConverterModule");
    coordinateTransformer_ = resolveModule<ITraciCoordinateTransformer>(this, "coordinateTransformerModule");

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

    state_.clear();
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

void TraciDynamicSceneConfigProvider::bindScene(py::SionnaScene scene) {
    state_.scene = std::move(scene);
    if (auto* relaxedTransformer = dynamic_cast<TraciRelaxedCoordinateTransformer*>(coordinateTransformer_); relaxedTransformer) {
        relaxedTransformer->bindScene(*state_.scene);
    }
}

void TraciDynamicSceneConfigProvider::State::clear() {
    toRemove.clear();
    scene.reset();
    pendingObjects.clear();
    cachedObjects.clear();
    cachedTransforms.clear();
}

void TraciDynamicSceneConfigProvider::dispatchUpdateVehicle(traci::BasicNodeManager::VehicleObject* vehicle) {
    auto id = vehicle->getCache()->getVehicleId();

    if (!state_.pendingObjects.contains(id) && !state_.cachedObjects.contains(id)) {
        // Have to create a new scene object.
        auto mesh = meshRegistry_->asset(MeshAsset::LowPolyCar);
        auto material = meshRegistry_->material(MeshAsset::LowPolyCar);
        auto sceneId = IDConverter_->sceneId(id);
        add(id, py::SceneObject(mesh, sceneId, material));
    }

    TraCIVelocity velocity{
        .speed = vehicle->getSpeed(),
        .heading = vehicle->getHeading(),
    };

    const auto canonicalOrientation = convert<mitsuba::Resolve::Point3f>(vehicle->getHeading());
    const auto canonicalPosition = coordinateTransformer_->fromSumo(convert<mitsuba::Resolve::Vector3f>(vehicle->getPosition()));
    const auto canonicalVelocity = coordinateTransformer_->vectorFromSumo(convert<mitsuba::Resolve::Vector3f>(velocity));

    const auto scaling = meshRegistry_->scaling(MeshAsset::LowPolyCar);
    const auto localOrientation = coordinateTransformer_->toLocalScene(canonicalOrientation);
    const auto localVelocity = coordinateTransformer_->toLocalScene(canonicalVelocity);
    const auto localPosition = coordinateTransformer_->toLocalScene(canonicalPosition);

    applyTransform(id, [this, localOrientation, localPosition, localVelocity, scaling](py::SceneObject& obj) {
        EV_DEBUG
            << "Updating SUMO-controlled vehicle: "
            << "orientation: " << localOrientation << " "
            << "position: " << localPosition << " "
            << "velocity: " << localVelocity << "\n";

        obj.setOrientation(localOrientation);
        obj.setPosition(localPosition);
        obj.setScaling(scaling);
        obj.setVelocity(localVelocity);

        if (auto* relaxedTransformer = dynamic_cast<TraciRelaxedCoordinateTransformer*>(coordinateTransformer_); relaxedTransformer) {
            relaxedTransformer->adjust(obj);
        }
    });
}

void TraciDynamicSceneConfigProvider::dispatchUpdatePerson(traci::BasicNodeManager::PersonObject* person) {
    auto id = person->getCache()->getPersonId();

    if (!state_.pendingObjects.contains(id) && !state_.cachedObjects.contains(id)) {
        // Have to create a new scene object.
        auto mesh = meshRegistry_->asset(MeshAsset::LowPolyCar);
        auto material = meshRegistry_->material(MeshAsset::LowPolyCar);

        EV_WARN << "Persons right now are depicted as cars, mind that";
        auto sceneId = IDConverter_->sceneId(id);
        add(id, py::SceneObject(mesh, sceneId, material));
    }

    TraCIVelocity velocity{
        .speed = person->getSpeed(),
        .heading = person->getHeading(),
    };

    const auto canonicalOrientation = convert<mitsuba::Resolve::Point3f>(person->getHeading());
    const auto canonicalPosition = coordinateTransformer_->fromSumo(convert<mitsuba::Resolve::Vector3f>(person->getPosition()));
    const auto canonicalVelocity = coordinateTransformer_->vectorFromSumo(convert<mitsuba::Resolve::Vector3f>(velocity));

    const auto scaling = meshRegistry_->scaling(MeshAsset::LowPolyCar);
    const auto localOrientation = coordinateTransformer_->toLocalScene(canonicalOrientation);
    const auto localVelocity = coordinateTransformer_->toLocalScene(canonicalVelocity);
    const auto localPosition = coordinateTransformer_->toLocalScene(canonicalPosition);

    applyTransform(id, [this, localOrientation, localPosition, localVelocity, scaling](py::SceneObject& obj) {
        EV_DEBUG
            << "Updating SUMO-controlled person: "
            << "orientation: " << localOrientation << " "
            << "position: " << localPosition << " "
            << "velocity: " << localVelocity << "\n";

        obj.setOrientation(localOrientation);
        obj.setPosition(localPosition);
        obj.setScaling(scaling);
        obj.setVelocity(localVelocity);

        if (auto* relaxedTransformer = dynamic_cast<TraciRelaxedCoordinateTransformer*>(coordinateTransformer_); relaxedTransformer) {
            relaxedTransformer->adjust(obj);
        }
    });
}

void TraciDynamicSceneConfigProvider::applyTransform(const std::string& id, std::function<void(py::SceneObject&)> f) {
    if (auto cached = state_.cachedObjects.find(id); cached != state_.cachedObjects.end()) {
        if (auto locked = cached->second; locked) {
            f(*locked);
        } else {
            throw omnetpp::cRuntimeError("failed to lock cached object: possibly wrong cached objects view");
        }
    } else if (auto existing = state_.cachedTransforms.find(id); existing != state_.cachedTransforms.end()) {
        EV_WARN << "resetting transform: this is strange, scene updates may not be applied on time?";
    } else {
        state_.cachedTransforms.emplace(std::make_pair(id, std::move(f)));
    }
}

void TraciDynamicSceneConfigProvider::dispatchRemoveVehicle(const char* id) {
    EV_INFO << "Preparing to remove vehicle from the scene " << id;
    remove(id);
}

void TraciDynamicSceneConfigProvider::dispatchRemovePerson(const char* id) {
    EV_INFO << "Preparing to remove person from the scene " << id;
    remove(id);
}

void TraciDynamicSceneConfigProvider::dispatchAddVehicle(const char* id) {
    EV_INFO << "Adding new vehicle to the scene " << id << "\n";
}

void TraciDynamicSceneConfigProvider::dispatchAddPerson(const char* id) {
    EV_INFO << "Adding new person to the scene " << id << "\n";
}

IDynamicSceneConfigProvider& TraciDynamicSceneConfigProvider::add(const std::string& id, py::SceneObject object) {
    state_.pendingObjects.emplace(std::make_pair(id, std::make_shared<py::SceneObject>(object)));
    return *this;
}

IDynamicSceneConfigProvider& TraciDynamicSceneConfigProvider::remove(const std::string& id) {
    if (state_.pendingObjects.contains(id)) {
        state_.pendingObjects.extract(id);
    } else if (state_.cachedObjects.contains(id)) {
        state_.toRemove.emplace_back(id);
    }
    state_.cachedTransforms.erase(id);

    return *this;
}

std::weak_ptr<py::SceneObject> TraciDynamicSceneConfigProvider::fetch(const std::string& id) {
    if (auto pending = state_.pendingObjects.find(id); pending != state_.pendingObjects.end()) {
        return pending->second;
    } else if (auto cached = state_.cachedObjects.find(id); cached != state_.cachedObjects.end()) {
        return cached->second;
    } else {
        return {};
    }
}

void TraciDynamicSceneConfigProvider::edit() {
    if (state_.pendingObjects.empty() && state_.toRemove.empty()) {
        return;
    }

    std::vector<std::string> toRemove;
    toRemove.reserve(state_.toRemove.size());
    for (const auto& id : state_.toRemove) {
        toRemove.push_back(IDConverter_->sceneId(id));
    }

    std::vector<py::SceneObject> toAdd;
    toAdd.reserve(state_.pendingObjects.size());
    for (const auto& [_, object] : state_.pendingObjects) {
        toAdd.push_back(*object);
    }

    state_.scene->edit(toAdd, toRemove);

    for (auto& [addedId, added] : state_.pendingObjects) {
        state_.cachedObjects.insert_or_assign(addedId, added);
    }
    for (auto& [id, callback] : state_.cachedTransforms) {
        if (auto cached = state_.cachedObjects.find(id); cached != state_.cachedObjects.end()) {
            if (auto locked = cached->second; locked) {
                callback(*locked);
            }
        }
    }

    for (const auto& removedId : state_.toRemove) {
        state_.cachedObjects.erase(removedId);
        IDConverter_->removeByTraciId(removedId);
    }

    state_.toRemove.clear();
    state_.pendingObjects.clear();
    state_.cachedTransforms.clear();
}
