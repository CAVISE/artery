#pragma once

#include <cavise/sionna/bridge/bindings/Scene.h>
#include <cavise/sionna/bridge/Helpers.h>
#include <cavise/sionna/environment/api/SionnaAPI.h>
#include <cavise/sionna/environment/config/dynamic/TraciDynamicSceneConfigProvider.h>
#include <cavise/sionna/environment/config/scenes/IStaticSceneProvider.h>

#include <omnetpp/csimplemodule.h>

#include <inet/environment/contract/IPhysicalEnvironment.h>

#include <memory>
#include <optional>

namespace artery::sionna {

    class PhysicalEnvironment : public inet::physicalenvironment::IPhysicalEnvironment
        , public omnetpp::cSimpleModule
        , public ISionnaAPI {
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

        const py::SionnaScene& scene() const;
        py::SionnaScene& scene() override;

        // ISionnaAPI implementation.
        mitsuba::ref<mi::Scene> miScene() override;
        bool setTxArray(const py::AntennaArray& array) override;
        bool setRxArray(const py::AntennaArray& array) override;
        IDynamicSceneConfigProxy* dynamicConfiguration() override;
        ICoordinateTransformProxy* coordinateTransform() override;
        IIDConverterProxy* IDConversion() override;

    protected:
        int numInitStages() const override;
        void initialize(int stage) override;
        void finish() override;

        void handleParameterChange(const char* parname) override;
        void refreshDisplay() const override;

        virtual void buildSceneFromEnvironment();
        virtual void updateDynamicObjects();

    private:
        template <typename T>
        T* getSubmoduleAsType(const std::string& submodule) {
            if (auto* mod = getSubmodule(submodule.c_str()); !mod) {
                throw omnetpp::cRuntimeError("missing %s submodule", submodule);
            } else if (auto* casted = dynamic_cast<T*>(mod); !casted) {
                throw omnetpp::cRuntimeError("%s does not implement %s", submodule, typeid(T).name());
            } else {
                return casted;
            }
        }

        void initializePythonRuntime();
        void initializeScene();
        void initializeSionnaAPI();
        std::unique_ptr<ScopedInterpreter> interpreter_;
        std::optional<py::SionnaScene> scene_;
        std::shared_ptr<IDynamicSceneConfigProxy> dynamicConfiguration_;
        std::shared_ptr<ICoordinateTransformProxy> coordinateTransform_;
        std::shared_ptr<IIDConverterProxy> IDConversion_;
    };

} // namespace artery::sionna
