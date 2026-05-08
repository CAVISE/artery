"""
Example Python hook for SionnaPhysicalEnvironment.

This module demonstrates how to use the SionnaPythonHook to react to
simulation events. To use this hook, set the pythonModulePath parameter to this file.

Usage in omnetpp.ini:
    *.physicalEnvironment.pythonHookType = "cavise.sionna.environment.hook.SionnaPythonHook"
    *.physicalEnvironment.pythonHook.pythonModulePath = "${ARTERY_ROOT}/src/cavise/sionna/environment/hook/example_hook.py"
    *.physicalEnvironment.pythonHook.traciCoreModule = "traci.core"
    *.physicalEnvironment.pythonHook.sceneConfigProviderModule = "physicalEnvironment.dynamicSceneConfigProvider"
"""

class MyHook:
    """
    Hook class that gets instantiated by SionnaPythonHook.
    Alternatively, you can define a module-level 'hook' attribute pointing to an instance,
    or just define the functions at module level.
    """

    def __init__(self):
        self.step_count = 0
        print("MyHook: Initialized")

    def on_traci_init(self):
        """Called when TraCI initializes."""
        print("MyHook: TraCI initialized")

    def on_traci_step(self):
        """Called on each TraCI step."""
        self.step_count += 1
        if self.step_count % 10 == 0:
            print(f"MyHook: TraCI step {self.step_count}")

    def on_scene_edit_begin(self):
        """Called before scene edit."""
        print("MyHook: Scene edit begin")

    def on_scene_edit_end(self):
        """Called after scene edit."""
        print("MyHook: Scene edit end")

    def on_traci_close(self):
        """Called when TraCI closes."""
        print("MyHook: TraCI closing")

# If you want to use a module-level instance, you can do:
# hook = MyHook()