"""
Example Python hooks for SionnaPhysicalEnvironment.

This module demonstrates how to use the SionnaPythonHook to react to
simulation events. There are multiple ways to define hooks:

1. Using the @sionna_hook decorator (recommended for new code)
2. Defining a class named 'Hook' (backward compatible)
3. Defining a module-level 'hook' attribute (backward compatible)
4. Defining functions at module level (backward compatible)

To use a single hook file, set in omnetpp.ini:
    *.physicalEnvironment.pythonHookType = "cavise.sionna.environment.hook.SionnaPythonHook"
    *.physicalEnvironment.pythonHook.pythonModulePath = "${ARTERY_ROOT}/src/cavise/sionna/environment/hook/example_hook.py"
    *.physicalEnvironment.pythonHook.traciCoreModule = "traci.core"

To use a package with multiple hooks, create a directory with multiple
Python files, each containing @sionna_hook decorated classes:
    *.physicalEnvironment.pythonHook.pythonModulePath = "${ARTERY_ROOT}/path/to/hooks_package"
"""

"""
Later do something like ```cavise.sionna.environment.hook.decorators```
"""
from decorators import sionna_hook


@sionna_hook
class MyHook:
    """
    Hook class using the @sionna_hook decorator.
    This is the recommended way to define hooks.
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


@sionna_hook
class AnotherHook:
    """
    Another hook class in the same module.
    Both MyHook and AnotherHook will be discovered and instantiated.
    """

    def __init__(self):
        print("AnotherHook: Initialized")

    def on_traci_init(self):
        print("AnotherHook: TraCI initialized")

    def on_traci_step(self):
        print("AnotherHook: TraCI step")

    def on_scene_edit_begin(self):
        print("AnotherHook: Scene edit begin")

    def on_scene_edit_end(self):
        print("AnotherHook: Scene edit end")

    def on_traci_close(self):
        print("AnotherHook: TraCI closing")


# For backward compatibility, you can also define a module-level instance:
# hook = MyHook()