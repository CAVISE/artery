#include "SionnaAPI.h"

#include <cavise/sionna/environment/PhysicalEnvironment.h>

#include <omnetpp/cexception.h>

using namespace artery::sionna;

ISionnaAPI* ISionnaAPI::get(const omnetpp::cModule* module) {
    auto* current = const_cast<omnetpp::cModule*>(module);
    while (current != nullptr) {
        if (auto* api = dynamic_cast<ISionnaAPI*>(current); api != nullptr) {
            return api;
        }

        current = current->getParentModule();
    }

    throw omnetpp::cRuntimeError("could not resolve Sionna API from module %s", module != nullptr ? module->getFullPath().c_str() : "<null>");
}
