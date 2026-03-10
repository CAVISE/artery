#include "artery.pb.h"
#include "opencda.pb.h"
#include <capi.pb.h>
#include <omnetpp/csimplemodule.h>

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace cavise {

    class MessageRegistry
        : public omnetpp::cSimpleModule {
    public:
        MessageRegistry();

        void initialize() override;
        void finish() override;

        template<typename T>
        void append(const T&);

        template<typename T>
        const T& generate();

        template<>
        const capi::OpenCDAMessage& generate();

        template<>
        void append(const capi::ArteryMessage& message);
        template<>
        void append(const capi::OpenCDAMessage& message);

        const capi::Message& messageByOrder(std::uint64_t order) const;

        void confirm(const capi::Message& message);
        std::string summary() const;

    private:
        struct Holder {
            std::size_t timesConfirmed;
            capi::Message message;
        };

        std::vector<Holder> holders_;

        int minEntities_;
        int maxEntities_;
        std::int64_t nextOrder_;
    };

}
