package cavise.sionna.environment.hook;

import inet.common.InitStages;

@namespace(artery::sionna);
@class(SionnaPythonHook);

//
// A module that loads a user-specified Python module and calls its methods
// in response to various simulation events (TraCI signals, scene edits, etc.).
//
// The Python module should define a class or functions with the following methods:
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
// Alternatively, you can define a module-level 'hook' attribute that points to
// an instance of your hook class, or just define the functions at module level.
//
simple SionnaPythonHook like ISionnaPythonHook
{
    parameters:
        string pythonModulePath;  // Path to the Python module (file path or import path)
        string traciCoreModule;     // Path to the TraCI Core module (for subscribing to signals)

        @display("i=block/function");
}
