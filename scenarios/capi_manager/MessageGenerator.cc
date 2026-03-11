#include "MessageGenerator.h"

#include <omnetpp/cexception.h>
#include <omnetpp/csimplemodule.h>
#include <omnetpp/regmacros.h>

#include <cstdio>
#include <memory>
#include <string>

using namespace cavise;

Define_Module(OpenCDAMessageGenerator)

OpenCDAMessageGenerator::OpenCDAMessageGenerator() : state_{.minEntities = 1, .maxEntities = 1, .idPrefix = "dummy", .minId = 1, .maxId = 1, .currOrder = 0}
{
}

void OpenCDAMessageGenerator::initialize()
{
    state_.maxEntities = par("maxEntities").intValue();
    state_.minEntities = par("minEntities").intValue();

    if (state_.minEntities > state_.maxEntities) {
        throw omnetpp::cRuntimeError("minEntities is more than maxEntities, minEntities=%d, maxEntities=%d", state_.minEntities, state_.maxEntities);
    }

    if (state_.minEntities < 0) {
        throw omnetpp::cRuntimeError("minEntities is less than 0");
    }

    state_.maxId = par("maxId").intValue();
    state_.minId = par("minId").intValue();

    if (state_.minId > state_.maxId) {
        throw omnetpp::cRuntimeError("minId is more than maxId, minId=%d, maxId=%d", state_.minId, state_.maxId);
    }

    if (state_.minId < 0) {
        throw omnetpp::cRuntimeError("minId is less than 0");
    }

    state_.idPrefix = par("idPrefix").stdstringValue();
}

std::string OpenCDAMessageGenerator::generateEntityId()
{
    int id = uniform(state_.minId, state_.maxId);

    if (std::size_t size = std::snprintf(nullptr, 0, "%s-%d", state_.idPrefix.c_str(), id); size > 0) {
        auto buffer = std::make_unique<char[]>(size);
        std::snprintf(buffer.get(), size, "%s-%d", state_.idPrefix.c_str(), id);
        return std::string(buffer.get(), size);
    }

    throw omnetpp::cRuntimeError("could not format string: could not dermine string size");
}

capi::Message OpenCDAMessageGenerator::generate()
{
    capi::Message message;
    message.set_order(state_.currOrder++);

    auto* payload = message.mutable_opencda();
    const int entities = intuniform(state_.minEntities, state_.maxEntities);
    for (std::size_t i = 0; i < entities; ++i) {
        auto* entity = payload->add_entity();
        entity->set_id(generateEntityId());
    }

    return message;
}
