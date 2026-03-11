#include <capi.pb.h>

#include <omnetpp/csimplemodule.h>

namespace cavise {

class IMessageGenerator
{
public:
    /**
     * @brief Generate a single message, update state if needed.
     */
    virtual capi::Message generate() = 0;
};

class OpenCDAMessageGenerator : public IMessageGenerator, public omnetpp::cSimpleModule
{
public:
    OpenCDAMessageGenerator();

    /* omnetpp::cSimpleModule implementation. */
    void initialize() override;

    /* IMessageGenerator implementation. */
    capi::Message generate() override;

private:
    std::string generateEntityId();

private:
    struct State {
        /* entities range for message. */
        int minEntities;
        int maxEntities;
        /* id range for postfix, common prefix. */
        std::string idPrefix;
        int minId;
        int maxId;
        /* order for current message. */
        int currOrder;
    } state_;
};

}
