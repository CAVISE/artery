#include "MessageRegistry.h"

#include <omnetpp/cexception.h>

#include <algorithm>
#include <sstream>
#include <string>

using namespace cavise;

Define_Module(MessageRegistry);

MessageRegistry::MessageRegistry()
    : nextOrder_(1)
    , minEntities_(1)
    , maxEntities_(1)
    , receivedOpenCdaCount_(0)
    , receivedArteryCount_(0)
{}

void MessageRegistry::initialize()
{
    nextOrder_ = par("firstOrder").intValue();
    minEntities_ = par("minEntities").intValue();
    maxEntities_ = par("minEntities").intValue(), par("maxEntities").intValue();
}

void MessageRegistry::finish()
{
    EV_INFO << summary() << "\n";
}

const capi::Message& MessageRegistry::appendMessage()
{
    capi::Message message;
    message.set_order(nextOrder_++);

    auto* openCda = message.mutable_opencda();
    const int entities = intuniform(minEntities_, maxEntities_);
    for (std::size_t i = 0; i < entities; ++i) {
        auto* entity = openCda->add_entity();
        entity->set_id("dummy-" + std::to_string(intrand(100000)));
        entity->set_velocity(uniform(0.0, 40.0));
    }

    holders_.push_back({
        .timesConfirmed = 0,
        .message = std::move(message)
    });

    return holders_.back().message;
}

const capi::Message& MessageRegistry::messageByOrder(std::uint64_t order) const {
    return holders_.at(order).message;
}

void MessageRegistry::confirm(const capi::Message& message)
{
    std::ostringstream out;
    out << "reply order=" << message.order();

    if (message.has_artery()) {
        ++receivedArteryCount_;
        out << " type=artery transmissions=" << message.artery().transmissions_size();
        details = out.str();
        return true;
    }

    if (message.has_opencda()) {
        ++receivedOpenCdaCount_;
        out << " type=opencda entities=" << message.opencda().entity_size();
        details = out.str();
        return true;
    }

    out << " type=other";
    details = out.str();
    return false;
}

std::string MessageRegistry::summary() const
{
    std::ostringstream out;
    out << "MessageRegistry summary:"
        << " sentOpenCda=" << sentOpenCdaCount_
        << " receivedOpenCda=" << receivedOpenCdaCount_
        << " receivedArtery=" << receivedArteryCount_
        << " stored=" << messages_.size();
    return out.str();
}
