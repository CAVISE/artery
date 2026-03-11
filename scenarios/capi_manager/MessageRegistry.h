#include "MessageGenerator.h"

#include <artery.pb.h>
#include <capi.pb.h>
#include <omnetpp/csimplemodule.h>
#include <opencda.pb.h>

#include <cstdint>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace cavise
{

template<typename G>
struct AlwaysFalse : std::false_type {};

class MessageRegistry : public omnetpp::cSimpleModule
{
public:
    MessageRegistry();

    /* omnetpp::cSimpleModule implementation. */
    void initialize() override;
    void finish() override;

    /**
     * @brief Register a message of a given type.
     */
    template <typename T>
    void registerMessage(const T&) {
        static_assert(AlwaysFalse<T>::value, "cannot register message of requested type - look for overloads");
    }

    template <typename T>
    T generateMessage() {
        static_assert(AlwaysFalse<T>::value, "cannot generate message of requested type - look for overloads");
    }

    void confirm(const capi::Message& message);
    std::string summary() const;

private:
    struct Holder {
        std::size_t timesConfirmed;
        capi::Message message;
    };

    std::unordered_map<std::type_index, IMessageGenerator*> generators_;
    std::unordered_map<std::type_index, std::vector<Holder>> holders_;
};

template <>
capi::OpenCDAMessage MessageRegistry::generateMessage<capi::OpenCDAMessage>();

template <>
void MessageRegistry::registerMessage<capi::ArteryMessage>(const capi::ArteryMessage& message);

template <>
void MessageRegistry::registerMessage<capi::OpenCDAMessage>(const capi::OpenCDAMessage& message);

}  // namespace cavise
