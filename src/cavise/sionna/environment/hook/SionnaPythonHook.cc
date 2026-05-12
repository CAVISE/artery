#include "SionnaPythonHook.h"

#include <traci/Core.h>

#include <nanobind/nanobind.h>

#include <omnetpp/ccomponent.h>
#include <omnetpp/cexception.h>
#include <omnetpp/cmodule.h>
#include <omnetpp/cregistrationlist.h>
#include <omnetpp/csimulation.h>
#include <omnetpp/simtime.h>

#include <filesystem>
#include <inet/common/InitStages.h>

using namespace artery::sionna;
using namespace nanobind;

Define_Module(SionnaPythonHook);

// Define static signal members for scene edit
omnetpp::simsignal_t SionnaPythonHook::sceneEditBeginSignal_ = omnetpp::cComponent::registerSignal("sceneEditBegin");
omnetpp::simsignal_t SionnaPythonHook::sceneEditEndSignal_ = omnetpp::cComponent::registerSignal("sceneEditEnd");

int SionnaPythonHook::numInitStages() const {
    return inet::NUM_INIT_STAGES;
}

void SionnaPythonHook::initialize(int stage) {
    if (stage == inet::InitStages::INITSTAGE_LOCAL) {
        // Get parameters.
        pythonModulePath_ = par("pythonModulePath").stdstringValue();
    } else if (stage == inet::InitStages::INITSTAGE_PHYSICAL_ENVIRONMENT) {
        // Python runtime should be initialized by now (done at INITSTAGE_LOCAL by PhysicalEnvironment).
        // Load the Python module.
        loadPythonModule();

        // Subscribe to TraCI signals using traci::Listener.
        omnetpp::cModule* traciCoreModule = getModuleByPath(par("traciCoreModule").stringValue());
        if (!traciCoreModule) {
            throw omnetpp::cRuntimeError("SionnaPythonHook: traciCoreModule not found at path '%s'",
                                         par("traciCoreModule").stringValue());
        }
        subscribeTraCI(traciCoreModule);

        // Subscribe to scene edit signals via system module (signals propagate to top).
        getSystemModule()->subscribe(sceneEditBeginSignal_, this);
        getSystemModule()->subscribe(sceneEditEndSignal_, this);
    }
}

void SionnaPythonHook::finish() {
    // Unsubscribe from TraCI signals using traci::Listener.
    unsubscribeTraCI();

    // Unsubscribe from scene edit signals via system module.
    getSystemModule()->unsubscribe(sceneEditBeginSignal_, this);
    getSystemModule()->unsubscribe(sceneEditEndSignal_, this);

    // Clear Python objects (they will be destroyed when the interpreter shuts down).
    pythonHookInstance_.reset();
    pythonModule_.reset();
}

void SionnaPythonHook::loadPythonModule() {
    if (pythonModulePath_.empty()) {
        throw omnetpp::cRuntimeError("SionnaPythonHook: pythonModulePath parameter is empty");
    }

    try {
        gil_scoped_acquire gil;

        // The pythonModulePath_ can be:
        // 1. A module path like "myapp.hooks.my_hook" (Python import path)
        // 2. A file path like "/path/to/my_hook.py"
        // For file paths, we need to add the parent directory to sys.path.

        std::string moduleName = pythonModulePath_;
        std::filesystem::path path(pythonModulePath_);

        if (path.extension() == ".py" || std::filesystem::exists(path)) {
            // It's a file path - need to add parent to sys.path and import by stem name.
            auto parentPath = path.parent_path();
            auto stem = path.stem().string();

            // Add parent path to sys.path if not already there.
            nanobind::module_ sys = nanobind::module_::import_("sys");
            nanobind::object pathList = sys.attr("path");

            // Check if parent path is already in sys.path.
            bool found = false;
            for (auto item : pathList) {
                if (nanobind::cast<std::string>(item) == parentPath.string()) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                pathList.attr("append")(parentPath.string());
            }

            moduleName = stem;
        }

        EV_INFO << "SionnaPythonHook: Loading Python module '" << moduleName << "'" << std::endl;

        // Import the module.
        pythonModule_ = std::make_unique<object>(nanobind::module_::import_(moduleName.c_str()));

        object module = *pythonModule_;

        // Try to find a hook instance.
        // First, check if the module has a 'hook' attribute (an instance).
        try {
            object hookAttr = module.attr("hook");
            pythonHookInstance_ = std::make_unique<object>(hookAttr);
            EV_INFO << "SionnaPythonHook: Using 'hook' attribute from module" << std::endl;
        } catch (const nanobind::python_error&) {
            // No 'hook' attribute, try to find a Hook class.
            try {
                object hookClass = module.attr("Hook");
                // Instantiate the class.
                pythonHookInstance_ = std::make_unique<object>(hookClass());
                EV_INFO << "SionnaPythonHook: Instantiated 'Hook' class from module" << std::endl;
            } catch (const nanobind::python_error&) {
                // No Hook class either - use the module itself as the hook object.
                pythonHookInstance_ = std::make_unique<object>(module);
                EV_INFO << "SionnaPythonHook: Using module itself as hook object" << std::endl;
            }
        }

        // Notify that Python module is loaded.
        onPythonModuleLoaded();

        EV_INFO << "SionnaPythonHook: Python module loaded successfully" << std::endl;

    } catch (const nanobind::python_error& e) {
        throw omnetpp::cRuntimeError("SionnaPythonHook: Failed to load Python module '%s': %s",
                                     pythonModulePath_.c_str(), e.what());
    } catch (const std::exception& e) {
        throw omnetpp::cRuntimeError("SionnaPythonHook: Failed to load Python module '%s': %s",
                                     pythonModulePath_.c_str(), e.what());
    }
}

void SionnaPythonHook::callPythonMethod(const std::string& methodName) {
    if (!pythonHookInstance_) {
        EV_WARN << "SionnaPythonHook: Python hook instance not loaded, skipping " << methodName << std::endl;
        return;
    }

    try {
        gil_scoped_acquire gil;
        object hook = *pythonHookInstance_;
        
        // Use call from SB helpers
        call<object>(hook, methodName);
        EV_DEBUG << "SionnaPythonHook: Called Python method '" << methodName << "'" << std::endl;

    } catch (const nanobind::python_error& e) {
        EV_ERROR << "SionnaPythonHook: Error calling Python method '" << methodName << "': " << e.what() << std::endl;
    }
}

void SionnaPythonHook::receiveSignal(omnetpp::cComponent* source, omnetpp::simsignal_t signal,
                                     const omnetpp::SimTime& value, omnetpp::cObject* details) {
    // Let traci::Listener handle TraCI signals (traciInit, traciStep, traciClose).
    traci::Listener::receiveSignal(source, signal, value, details);

    // Handle scene edit signals.
    if (signal == sceneEditBeginSignal_) {
        onSceneEditBegin();
    } else if (signal == sceneEditEndSignal_) {
        onSceneEditEnd();
    }
}

void SionnaPythonHook::traciInit() {
    callPythonMethod("on_traci_init");
}

void SionnaPythonHook::traciStep() {
    callPythonMethod("on_traci_step");
}

void SionnaPythonHook::traciClose() {
    callPythonMethod("on_traci_close");
}

void SionnaPythonHook::onSceneEditBegin() {
    callPythonMethod("on_scene_edit_begin");
}

void SionnaPythonHook::onSceneEditEnd() {
    callPythonMethod("on_scene_edit_end");
}