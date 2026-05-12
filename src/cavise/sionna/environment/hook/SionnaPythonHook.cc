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
        // Load the Python module(s).
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
    pythonHookInstances_.clear();
    pythonModules_.clear();
}

void SionnaPythonHook::loadPythonModule() {
    if (pythonModulePath_.empty()) {
        throw omnetpp::cRuntimeError("SionnaPythonHook: pythonModulePath parameter is empty");
    }

    std::filesystem::path path(pythonModulePath_);

    // Check if it's a directory (package) or a file (module)
    if (std::filesystem::is_directory(path)) {
        loadPackage(pythonModulePath_);
    } else {
        loadSingleModule(pythonModulePath_);
    }

    // Notify that Python module(s) are loaded.
    onPythonModuleLoaded();

    EV_INFO << "SionnaPythonHook: Loaded " << pythonHookInstances_.size() << " hook instance(s)" << std::endl;
}

void SionnaPythonHook::loadSingleModule(const std::string& modulePath) {
    try {
        gil_scoped_acquire gil;

        std::string moduleName = modulePath;
        std::filesystem::path path(modulePath);

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
        auto module = std::make_unique<object>(nanobind::module_::import_(moduleName.c_str()));
        pythonModules_.push_back(std::move(module));

        // Discover hook classes in the module.
        discoverHooksInModule(*pythonModules_.back());

    } catch (const nanobind::python_error& e) {
        throw omnetpp::cRuntimeError("SionnaPythonHook: Failed to load Python module '%s': %s",
                                     modulePath.c_str(), e.what());
    } catch (const std::exception& e) {
        throw omnetpp::cRuntimeError("SionnaPythonHook: Failed to load Python module '%s': %s",
                                     modulePath.c_str(), e.what());
    }
}

void SionnaPythonHook::loadPackage(const std::string& packagePath) {
    try {
        gil_scoped_acquire gil;

        std::filesystem::path path(packagePath);

        // Add package directory to sys.path if not already there.
        nanobind::module_ sys = nanobind::module_::import_("sys");
        nanobind::object pathList = sys.attr("path");

        bool found = false;
        for (auto item : pathList) {
            if (nanobind::cast<std::string>(item) == path.string()) {
                found = true;
                break;
            }
        }
        if (!found) {
            pathList.attr("append")(path.string());
        }

        // Iterate through all Python files in the directory.
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.path().extension() == ".py") {
                std::string stem = entry.path().stem().string();

                // Skip __init__.py
                if (stem == "__init__") {
                    continue;
                }

                EV_INFO << "SionnaPythonHook: Loading Python module '" << stem
                        << "' from package" << std::endl;

                try {
                    auto module = std::make_unique<object>(nanobind::module_::import_(stem.c_str()));
                    pythonModules_.push_back(std::move(module));

                    // Discover hook classes in the module.
                    discoverHooksInModule(*pythonModules_.back());
                } catch (const nanobind::python_error& e) {
                    EV_WARN << "SionnaPythonHook: Failed to load module '" << stem << "': " << e.what() << std::endl;
                }
            }
        }

    } catch (const nanobind::python_error& e) {
        throw omnetpp::cRuntimeError("SionnaPythonHook: Failed to load Python package '%s': %s",
                                     packagePath.c_str(), e.what());
    } catch (const std::exception& e) {
        throw omnetpp::cRuntimeError("SionnaPythonHook: Failed to load Python package '%s': %s",
                                     packagePath.c_str(), e.what());
    }
}

void SionnaPythonHook::discoverHooksInModule(const nanobind::object& module) {
    try {
        gil_scoped_acquire gil;

        object mod = module;
        bool hookFound = false;

        // First, check if the module has a 'hook' attribute (an instance).
        try {
            object hookAttr = mod.attr("hook");
            pythonHookInstances_.push_back(std::make_unique<object>(hookAttr));
            EV_INFO << "SionnaPythonHook: Using 'hook' attribute from module" << std::endl;
            hookFound = true;
        } catch (const nanobind::python_error&) {
            // No 'hook' attribute, try to find a Hook class.
            try {
                object hookClass = mod.attr("Hook");
                // Check if it's marked with @sionna_hook decorator
                try {
                    bool isMarked = nanobind::cast<bool>(hookClass.attr("_sionna_hook_marked"));
                    if (isMarked) {
                        // Instantiate the class.
                        pythonHookInstances_.push_back(std::make_unique<object>(hookClass()));
                        EV_INFO << "SionnaPythonHook: Instantiated decorated 'Hook' class from module" << std::endl;
                        hookFound = true;
                    }
                } catch (const nanobind::python_error&) {
                    // Not decorated, instantiate anyway for backward compatibility
                    pythonHookInstances_.push_back(std::make_unique<object>(hookClass()));
                    EV_INFO << "SionnaPythonHook: Instantiated 'Hook' class from module (not decorated)" << std::endl;
                    hookFound = true;
                }
            } catch (const nanobind::python_error&) {
                // No Hook class - scan for all classes marked with @sionna_hook
                try {
                    // Use Python's dir() to get all attribute names in the module
                    nanobind::module_ builtins = nanobind::module_::import_("builtins");
                    nanobind::object dir_func = builtins.attr("dir");
                    nanobind::object dir_list = dir_func(mod);
                    int n = nanobind::cast<int>(dir_list.attr("__len__")());
                    for (int i = 0; i < n; ++i) {
                        nanobind::object name_obj = dir_list.attr("__getitem__")(i);
                        std::string name = nanobind::cast<std::string>(name_obj);
                        nanobind::object obj = mod.attr(name.c_str());
                        // Check if obj is a class (type)
                        nanobind::object type_func = builtins.attr("type");
                        nanobind::object obj_type = type_func(obj);
                        nanobind::object type_type = type_func(type_func);
                        if (obj_type.equal(type_type)) {
                            // It's a class, check if it's marked with @sionna_hook
                            try {
                                bool isMarked = nanobind::cast<bool>(obj.attr("_sionna_hook_marked"));
                                if (isMarked) {
                                    // Instantiate the class
                                    pythonHookInstances_.push_back(std::make_unique<object>(obj()));
                                    EV_INFO << "SionnaPythonHook: Instantiated decorated class from module" << std::endl;
                                    hookFound = true;
                                }
                            } catch (const nanobind::python_error&) {
                                // Not marked, skip
                            }
                        }
                    }
                } catch (const nanobind::python_error& e) {
                    EV_WARN << "SionnaPythonHook: Error scanning for decorated classes: " << e.what() << std::endl;
                }
            }
        }

        // If no hook found, use the module itself as the hook object (backward compatibility)
        if (!hookFound) {
            pythonHookInstances_.push_back(std::make_unique<object>(mod));
            EV_INFO << "SionnaPythonHook: Using module itself as hook object (no decorated classes found)" << std::endl;
        }

    } catch (const nanobind::python_error& e) {
        EV_WARN << "SionnaPythonHook: Error discovering hooks in module: " << e.what() << std::endl;
    }
}

void SionnaPythonHook::callPythonMethod(const std::string& methodName) {
    if (pythonHookInstances_.empty()) {
        EV_WARN << "SionnaPythonHook: No Python hook instances loaded, skipping " << methodName << std::endl;
        return;
    }

    try {
        gil_scoped_acquire gil;

        // Call the method on all hook instances.
        for (auto& hookInstance : pythonHookInstances_) {
            // Use call from SB helpers (in artery::sionna namespace)
            call(*hookInstance, methodName);
        }

        EV_DEBUG << "SionnaPythonHook: Called Python method '" << methodName << "' on "
                 << pythonHookInstances_.size() << " hook instance(s)" << std::endl;

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