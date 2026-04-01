#include "PhysicalEnvironment.h"

#include <inet/environment/common/MaterialRegistry.h>

#include <drjit-core/jit.h>
#include <nanobind/nanobind.h>

#include <omnetpp/cmodule.h>
#include <omnetpp/cexception.h>

#include <cstdlib>
#include <filesystem>

using namespace artery;

Define_Module(sionna::PhysicalEnvironment);

namespace {
    constexpr int NUM_INIT_STAGES = 2;

    enum class InitStage : int { LOAD_SCENE,
                                 INITIALIZE_SUBMODULES };

} // namespace

int sionna::PhysicalEnvironment::numInitStages() const {
    return NUM_INIT_STAGES;
}

void sionna::PhysicalEnvironment::initialize(int stage) {
    InitStage initStage = static_cast<InitStage>(stage);

    switch (initStage) {
        case InitStage::LOAD_SCENE:
            initializePythonRuntime();
            initializeScene();
            break;
        case InitStage::INITIALIZE_SUBMODULES:
            initializeDynamicConfigProvider();
            initializeSceneVisualizer();
            break;
    }
}

void sionna::PhysicalEnvironment::initializePythonRuntime() {
    if (interpreter_) {
        return;
    }

    parameters_.rtBackend = par("rtBackend").stdstringValue();
    parameters_.enableGradients = par("enableGradients").boolValue();

    interpreter_ = std::make_unique<ScopedInterpreter>(SIONNA_VENV_HINT);

    if (parameters_.rtBackend == "llvm") {
        if (std::getenv("DRJIT_LIBLLVM_PATH") == nullptr) {
            constexpr const char* kFallbackLlvmPath = "/usr/lib/libLLVM.so";
            if (std::filesystem::exists(kFallbackLlvmPath)) {
                setenv("DRJIT_LIBLLVM_PATH", kFallbackLlvmPath, 0);
            }
        }

        jit_init((uint32_t)JitBackend::LLVM);
        if (jit_has_backend(JitBackend::LLVM) == 0) {
            throw omnetpp::cRuntimeError("SionnaPhysicalEnvironment: LLVM backend is not available");
        }
    }

    nanobind::gil_scoped_acquire gil;
    auto mi = nanobind::module_::import_("mitsuba");

    if (parameters_.rtBackend == "llvm") {
        mi.attr("set_variant")("llvm_ad_rgb");
    } else if (parameters_.rtBackend == "scalar") {
        mi.attr("set_variant")("scalar_rgb");
    } else {
        throw omnetpp::cRuntimeError("SionnaPhysicalEnvironment: unsupported rtBackend \"%s\"", parameters_.rtBackend.c_str());
    }
}

void sionna::PhysicalEnvironment::initializeScene() {
    if (auto* providerModule = getSubmodule("sceneConfigProvider"); !providerModule) {
        throw omnetpp::cRuntimeError("SionnaPhysicalEnvironment: missing sceneConfigProvider submodule");
    } else if (auto* provider = dynamic_cast<IStaticSceneProvider*>(providerModule); !provider) {
        throw omnetpp::cRuntimeError("SionnaPhysicalEnvironment: sceneConfigProvider does not implement IStaticSceneProvider");
    } else {
        scene_.emplace(provider->getSceneConfig());
    }
}

void sionna::PhysicalEnvironment::initializeDynamicConfigProvider() {
    if (!scene_.has_value()) {
        throw omnetpp::cRuntimeError("SionnaPhysicalEnvironment: scene must be initialized before dynamic configuration");
    }

    if (auto* providerModule = getSubmodule("dynamicSceneConfigProvider"); !providerModule) {
        return;
    } else if (auto* provider = dynamic_cast<IDynamicSceneConfigProvider*>(providerModule); !provider) {
        throw omnetpp::cRuntimeError("SionnaPhysicalEnvironment: dynamicSceneConfigProvider does not implement IDynamicSceneConfigProvider");
    } else {
        provider->setScene(*scene_);
    }
}

void sionna::PhysicalEnvironment::initializeSceneVisualizer() {
    if (!scene_.has_value()) {
        throw omnetpp::cRuntimeError("SionnaPhysicalEnvironment: scene must be initialized before scene visualization");
    }

    if (auto* visualizerModule = getSubmodule("sceneVisualizer"); !visualizerModule) {
        return;
    } else if (auto* visualizer = dynamic_cast<ISceneVisualizer*>(visualizerModule); !visualizer) {
        throw omnetpp::cRuntimeError("SionnaPhysicalEnvironment: sceneVisualizer does not implement ISceneVisualizer");
    } else {
        visualizer->setScene(*scene_);
    }
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
