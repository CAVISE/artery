#pragma once

#include <string>

#include <boost/bimap.hpp>

#include <omnetpp/csimplemodule.h>

#include <traci/BasicNodeManager.h>

namespace artery::sionna {

    class ITraciIDConverter {
    public:
        // Query scene object ID from respective scene object.
        virtual std::string traciId(const std::string& sceneId) = 0;
        // Query scene object ID from raw Traci id.
        virtual std::string sceneId(const std::string& traciId) = 0;

        // Remove IDs by traci ID.
        virtual void removeByTraciId(const std::string& traciId) = 0;

        virtual ~ITraciIDConverter() = default;
    };

    // Resolving ID's from traci to Sionna is actually not that
    // trivial, that class does that.
    class TraciIDConverter
        : public ITraciIDConverter
        , public omnetpp::cSimpleModule {
    public:
        // omnetpp::cSimpleModule implementation.
        void initialize() override;

        // ITraciIDConverter implementation.
        std::string traciId(const std::string& sceneId) override;
        std::string sceneId(const std::string& traciId) override;
        void removeByTraciId(const std::string& traciId) override;

    private:
        // left is for Traci IDs, right is for scene IDs.
        boost::bimap<std::string, std::string> mapping_;
    };

} // namespace artery::sionna
