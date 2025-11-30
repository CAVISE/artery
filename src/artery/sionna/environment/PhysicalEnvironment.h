// SionnaPhysicalEnvironment.h
#pragma once

#include "omnetpp/csimplemodule.h"

#include <artery/sionna/bridge/Scene.h>
#include <inet/environment/contract/IPhysicalEnvironment.h>

#include <memory>


namespace artery {
    namespace sionna {

        class PhysicalEnvironment : public inet::physicalenvironment::IPhysicalEnvironment, public omnetpp::cSimpleModule {
        public:
            PhysicalEnvironment() = default;

            // inet::physicalenvironment::IPhysicalEnvironment implementation
            virtual inet::physicalenvironment::IObjectCache* getObjectCache() const override;
            virtual inet::physicalenvironment::IGround* getGround() const override;

            virtual const inet::Coord& getSpaceMin() const override;
            virtual const inet::Coord& getSpaceMax() const override;
            virtual const inet::physicalenvironment::IMaterialRegistry* getMaterialRegistry() const override;

            virtual int getNumObjects() const override;
            virtual const inet::physicalenvironment::IPhysicalObject* getObject(int index) const override;
            virtual const inet::physicalenvironment::IPhysicalObject* getObjectById(int id) const override;

            virtual void visitObjects(const inet::IVisitor* visitor, const inet::LineSegment& lineSegment) const override;

        protected:
            virtual int numInitStages() const override;
            virtual void initialize(int stage) override;

            virtual void handleParameterChange(const char* parname) override;
            virtual void refreshDisplay() const override;

            virtual void buildSceneFromEnvironment();
            virtual void updateDynamicObjects();

        private:
            void initializeScene();
            void initializeSceneWithConfig();

        private:
            struct Parameters {
                std::string rtBackend;
                std::string materialSet;
                bool enableGradients;
            } parameters_;
        };

    }  // namespace sionna
}  // namespace artery
