#pragma once

#include <cavise/sionna/environment/hook/ISionnaPythonHook.h>
#include <cavise/sionna/bridge/Helpers.h>

#include <traci/Listener.h>

#include <omnetpp/csimplemodule.h>
#include <omnetpp/simtime.h>

#include <nanobind/nanobind.h>

#include <string>
#include <memory>

namespace artery::sionna {

    // A module that loads a user-specified Python module and calls its methods
    // in response to various simulation events (TraCI signals, scene edits, etc.).
    //
    // The Python module should define a class (or module-level functions) with
    // the following methods:
    //
    // class MyHook:
    //     def on_traci_init(self):
    //         """Called when TraCI initializes"""
    //         pass
    //
    //     def on_traci_step(self):
    //         """Called on each TraCI step"""
    //         pass
    //
    //     def on_scene_edit_begin(self):
    //         """Called before scene edit"""
    //         pass
    //
    //     def on_scene_edit_end(self):
    //         """Called after scene edit"""
    //         pass
    //
    //     def on_traci_close(self):
    //         """Called when TraCI closes"""
    //         pass
    //
    class SionnaPythonHook : public ISionnaPythonHook, public traci::Listener {
    public:
        SionnaPythonHook() = default;
        ~SionnaPythonHook() override = default;

        // omnetpp::cSimpleModule overrides.
        int numInitStages() const override;
        void initialize(int stage) override;
        void finish() override;

        // ISionnaPythonHook overrides.
        void onPythonModuleLoaded() override {}
        void onSceneEditBegin() override;
        void onSceneEditEnd() override;

        // traci::Listener override.
        void receiveSignal(omnetpp::cComponent* source, omnetpp::simsignal_t signal,
                           const omnetpp::SimTime& value, omnetpp::cObject* details) override;

        // traci::Listener virtual methods.
        void traciInit() override;
        void traciStep() override;
        void traciClose() override;

    private:
        // Load the Python module specified by the user.
        void loadPythonModule();

        // Call a method on the Python hook object.
        void callPythonMethod(const std::string& methodName);

    private:
        // Path to the Python module (parameter).
        std::string pythonModulePath_;

        // The Python module object (imported).
        std::unique_ptr<nanobind::object> pythonModule_;

        // The Python hook class instance (if the module contains a class).
        std::unique_ptr<nanobind::object> pythonHookInstance_;

        // Cached signal IDs for scene edit.
        static omnetpp::simsignal_t sceneEditBeginSignal_;
        static omnetpp::simsignal_t sceneEditEndSignal_;

        // Reference to the TraciDynamicSceneConfigProvider (for subscribing to scene edit signals).
        omnetpp::cModule* sceneConfigProviderModule_ = nullptr;
    };

} // namespace artery::sionna
