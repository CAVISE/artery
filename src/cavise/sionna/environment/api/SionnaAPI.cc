#include "SionnaAPI.h"

#include <cavise/sionna/environment/PhysicalEnvironment.h>

#include <omnetpp/cexception.h>

using namespace artery::sionna;

Define_Module(BaseSionnaAPI);

BaseSionnaAPI* BaseSionnaAPI::get(const omnetpp::cModule* module) {
    constexpr const char* path = "^.sionnaApi";
    if (auto* api = module->getModuleByPath(path); api == nullptr) {
        throw omnetpp::cRuntimeError("could not resolve Sionna API at path %s", path);
    } else if (auto* typed = dynamic_cast<ISionnaAPI*>(api); typed == nullptr) {
        throw omnetpp::cRuntimeError("Sionna API submodule does not implement ISionnaAPI");
    } else {
        return typed;
    }
}

void ISionnaAPI::initialize() {
}
