#include "MessageRegistry.h"

#include "capi.pb.h"
#include "omnetpp/checkandcast.h"
#include "omnetpp/clog.h"
#include "opencda.pb.h"

#include <google/protobuf/util/message_differencer.h>
#include <omnetpp/cexception.h>

#include <sstream>
#include <typeindex>

using namespace cavise;

Define_Module(MessageRegistry);

MessageRegistry::MessageRegistry()
{
}

void MessageRegistry::initialize()
{
    for (SubmoduleIterator it(this); !it.end(); ++it) {
        if (auto generator = dynamic_cast<IMessageGenerator*>(*it); generator) {
            if (auto openCDAGenerator = dynamic_cast<OpenCDAMessageGenerator*>(generator); openCDAGenerator) {
                const auto type = std::type_index(typeid(capi::OpenCDAMessage));
                generators_[type] = openCDAGenerator;
            } else {
                // insert others above as needed.
                EV_INFO << "message registry: found non-generator submodule " << (*it)->getName() << ", skipping";
            }
        }
    }
}

void MessageRegistry::finish()
{
    EV_INFO << summary() << "\n";
}

template <>
capi::OpenCDAMessage MessageRegistry::generateMessage<capi::OpenCDAMessage>()
{
    const auto type = std::type_index(typeid(capi::OpenCDAMessage));
    if (const auto it = generators_.find(type); it == generators_.end()) {
        throw omnetpp::cRuntimeError("OpenCDAMessage generator is not registered");
    } else {
        return it->second->generate().opencda();
    }
}

template <>
void MessageRegistry::registerMessage<capi::ArteryMessage>(const capi::ArteryMessage& message)
{
    const auto type = std::type_index(typeid(capi::ArteryMessage));

    capi::Message forHolder;
    forHolder.mutable_artery()->CopyFrom(message);
    holders_[type].push_back(Holder{.timesConfirmed = 0, .message = forHolder});
}

template <>
void MessageRegistry::registerMessage<capi::OpenCDAMessage>(const capi::OpenCDAMessage& message)
{
    const auto type = std::type_index(typeid(capi::OpenCDAMessage));

    capi::Message forHolder;
    forHolder.mutable_opencda()->CopyFrom(message);
    holders_[type].push_back(Holder{.timesConfirmed = 0, .message = forHolder});
}

void MessageRegistry::confirm(const capi::Message& message)
{
    for (auto& [type, holders] : holders_) {
        EV_DEBUG << "confirm message: scanning " << type.name() << " message registry\n";

        for (auto& [times, heldMessage] : holders) {
            EV_DEBUG << "confirm message: comparing " << message.ShortDebugString() << " with " << heldMessage.ShortDebugString();

            if (google::protobuf::util::MessageDifferencer::Equals(message, heldMessage)) {
                EV_DEBUG << "confirm message: messages equal each other. Incrementing confirmation counter, curr: " << ++times;
                return;
            }
        }
    }
}

std::string MessageRegistry::summary() const
{
    std::ostringstream out;
    out << "MessageRegistry summary: \n";

    for (const auto& [type, holders] : holders_) {
        out << "holders for type: " << type.name() << ": \n";

        for (const auto& [times, heldMessage] : holders) {
            out << "- message: " << heldMessage.ShortDebugString() << " confirmed times " << times;
        }
    }

    return out.str();
}
