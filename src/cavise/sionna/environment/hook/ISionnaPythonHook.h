#pragma once

#include <omnetpp/csimplemodule.h>

namespace artery::sionna {

    // Interface for Sionna Python Hook modules.
    // This interface allows the PhysicalEnvironment to discover and manage
    // Python hook modules without knowing the concrete implementation.
    class ISionnaPythonHook : public omnetpp::cSimpleModule {
    public:
        virtual ~ISionnaPythonHook() = default;

        // Called when the Python module has been loaded successfully.
        virtual void onPythonModuleLoaded() = 0;

        // Called when the scene is about to be edited (before edit).
        virtual void onSceneEditBegin() = 0;

        // Called after the scene has been edited (after edit).
        virtual void onSceneEditEnd() = 0;
    };

} // namespace artery::sionna