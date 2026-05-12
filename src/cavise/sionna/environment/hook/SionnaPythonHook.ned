package cavise.sionna.environment.hook;

import inet.common.InitStages;

@namespace(artery::sionna);
@class(SionnaPythonHook);

//
// A module that loads a user-specified Python module or package and calls
// its methods in response to various simulation events (TraCI signals,
// scene edits, etc.).
//
// The pythonModulePath parameter can be:
// 1. A single Python file path (e.g., "/path/to/my_hook.py")
// 2. A Python package directory containing multiple hook modules
//
// Hook classes can be defined in several ways:
//
// 1. Using the @sionna_hook decorator (recommended):
//    from cavise.sionna.environment.hook.decorators import sionna_hook
//
//    @sionna_hook
//    class MyHook:
//        def on_traci_init(self):
//            print("TraCI initialized")
//
// 2. Defining a class named 'Hook' (backward compatible)
//
// 3. Defining a module-level 'hook' attribute (backward compatible)
//
// 4. Defining functions at module level (backward compatible)
//
// When loading a package directory, all Python files (except __init__.py)
// are imported and scanned for @sionna_hook decorated classes. All decorated
// classes are instantiated and their methods are called in sequence.
//
// Required hook methods:
//     def on_traci_init(self):      # Called when TraCI initializes
//     def on_traci_step(self):      # Called on each TraCI step
//     def on_scene_edit_begin(self): # Called before scene edit
//     def on_scene_edit_end(self):   # Called after scene edit
//     def on_traci_close(self):     # Called when TraCI closes
//
simple SionnaPythonHook like ISionnaPythonHook
{
    parameters:
        string pythonModulePath;  // Path to Python module file or package directory
        string traciCoreModule;   // Path to the TraCI Core module (for subscribing to signals)

        @display("i=block/function");
}
