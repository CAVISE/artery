// SionnaPhysicalEnvironment.h
#pragma once

#include "omnetpp/csimplemodule.h"

#include <inet/environment/contract/IPhysicalEnvironment.h>


namespace artery {
    namespace sionna {

        class PhysicalEnvironment : public inet::physicalenvironment::IPhysicalEnvironment, public omnetpp::cSimpleModule {
        public:
            PhysicalEnvironment() = default;

            // inet::physicalenvironment::IPhysicalEnvironment implementation
            inet::physicalenvironment::IObjectCache* getObjectCache() const override;
            inet::physicalenvironment::IGround* getGround() const override;

            const inet::Coord& getSpaceMin() const override;
            const inet::Coord& getSpaceMax() const override;
            const inet::physicalenvironment::IMaterialRegistry* getMaterialRegistry() const override;

            int getNumObjects() const override;
            const inet::physicalenvironment::IPhysicalObject* getObject(int index) const override;
            const inet::physicalenvironment::IPhysicalObject* getObjectById(int id) const override;

            void visitObjects(const inet::IVisitor* visitor, const inet::LineSegment& lineSegment) const override;

        protected:
            int numInitStages() const override;
            void initialize(int stage) override;

            void handleParameterChange(const char* parname) override;
            void refreshDisplay() const override;

            virtual void buildSceneFromEnvironment();
            virtual void updateDynamicObjects();

        private:
            void initializeScene();
            void initializeSceneWithConfig();

            struct Parameters {
                std::string rtBackend;
                std::string materialSet;
                bool enableGradients;
            } parameters_;
        };

    }
}
