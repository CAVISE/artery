#include "PhysicalEnvironment.h"
#include <cstdint>
#include <inet/environment/common/MaterialRegistry.h>

#include <drjit-core/jit.h>
#include <nanobind/nanobind.h>

#include <omnetpp/cmodule.h>
#include <omnetpp/cexception.h>

using namespace artery;

Define_Module(sionna::PhysicalEnvironment);

int sionna::PhysicalEnvironment::numInitStages() const {
    return inet::NUM_INIT_STAGES;
}

void sionna::PhysicalEnvironment::initialize(int stage) {
    switch (stage) {
        case inet::InitStages::INITSTAGE_LOCAL:
            initializePythonRuntime();
            break;
        case inet::InitStages::INITSTAGE_PHYSICAL_ENVIRONMENT:
            initializeScene();
            initializeDynamicConfigProvider();
            break;
        case inet::InitStages::INITSTAGE_LAST:
            initializeSceneVisualizer();
            break;
        default:
            break;
    }
}

void sionna::PhysicalEnvironment::finish() {
    scene_.reset();
    interpreter_.reset();
    omnetpp::cSimpleModule::finish();
}

void sionna::PhysicalEnvironment::initializePythonRuntime() {
    if (interpreter_) {
        return;
    }

    if (hasPar("virtualEnvironmentPath")) {
        interpreter_ = std::make_unique<ScopedInterpreter>(par("virtualEnvironmentPath").stdstringValue());
    } else {
        // clang-format off
        #if defined(SIONNA_VENV_HINT)
            interpreter_ = std::make_unique<ScopedInterpreter>(SIONNA_VENV_HINT);
        #else
            throw omnetpp::cRuntimeError("failed to initialize interpreter, python virtual environment is not set");
        #endif
        // clang-format on
    }

    nanobind::gil_scoped_acquire gil;
    auto mi = nanobind::module_::import_("mitsuba");

    // Variant is defined during compilation.
    using VariantName = mitsuba::detail::variant<mitsuba::Resolve::Float, mitsuba::Resolve::Spectrum>;
    mi.attr("set_variant")(VariantName::name);
}

void sionna::PhysicalEnvironment::initializeScene() {
    auto* provider = getSubmoduleAsType<IStaticSceneProvider>("sceneConfigProvider");
    scene_.emplace(provider->getSceneConfig());
}

void sionna::PhysicalEnvironment::initializeDynamicConfigProvider() {
    auto* provider = getSubmoduleAsType<IDynamicSceneConfigProvider>("dynamicSceneConfigProvider");
    provider->bindScene(*scene_);
}

void sionna::PhysicalEnvironment::initializeSceneVisualizer() {
    auto* visualizer = getSubmoduleAsType<ISceneVisualizer>("sceneVisualizer");
    visualizer->setScene(*scene_);
}

void sionna::PhysicalEnvironment::handleParameterChange(const char* /* parname */) {
}

void sionna::PhysicalEnvironment::refreshDisplay() const {
}

void sionna::PhysicalEnvironment::buildSceneFromEnvironment() {
}

void sionna::PhysicalEnvironment::updateDynamicObjects() {
}

inet::physicalenvironment::IObjectCache* sionna::PhysicalEnvironment::getObjectCache() const {
    return nullptr;
}

inet::physicalenvironment::IGround* sionna::PhysicalEnvironment::getGround() const {
    return nullptr;
}

const inet::Coord& sionna::PhysicalEnvironment::getSpaceMin() const {
    static const inet::Coord kZero;
    return kZero;
}

const inet::Coord& sionna::PhysicalEnvironment::getSpaceMax() const {
    static const inet::Coord kZero;
    return kZero;
}

const inet::physicalenvironment::IMaterialRegistry* sionna::PhysicalEnvironment::getMaterialRegistry() const {
    return &inet::physicalenvironment::MaterialRegistry::singleton;
}

int sionna::PhysicalEnvironment::getNumObjects() const {
    return 0;
}

const inet::physicalenvironment::IPhysicalObject* sionna::PhysicalEnvironment::getObject(int /* index */) const {
    return nullptr;
}

const inet::physicalenvironment::IPhysicalObject* sionna::PhysicalEnvironment::getObjectById(int /* id */) const {
    return nullptr;
}

void sionna::PhysicalEnvironment::visitObjects(
    const inet::IVisitor* visitor,
    const inet::LineSegment& lineSegment) const {
    if (!visitor) {
        return;
    }

    const int count = getNumObjects();
    for (int i = 0; i < count; ++i) {
        const auto* object = getObject(i);
        if (!object) {
            continue;
        }

        const auto* shape = object->getShape();
        if (!shape) {
            continue;
        }

        inet::Coord intersection1;
        inet::Coord intersection2;
        inet::Coord normal1;
        inet::Coord normal2;

        if (shape->computeIntersection(lineSegment, intersection1, intersection2, normal1, normal2)) {
            visitor->visit(dynamic_cast<const omnetpp::cObject*>(object));
        }
    }
}

const artery::sionna::py::SionnaScene& sionna::PhysicalEnvironment::scene() const {
    if (!scene_.has_value()) {
        throw omnetpp::cRuntimeError("physical environment scene is not initialized");
    }

    return *scene_;
}
