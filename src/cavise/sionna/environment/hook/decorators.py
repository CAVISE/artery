"""
Decorators for marking Sionna Python hook classes.

This module provides decorators that can be used to mark classes as Sionna hooks.
When a package/directory is loaded, the SionnaPythonHook will discover and
instantiate all decorated classes.
"""

def sionna_hook(cls):
    """
    Decorator to mark a class as a Sionna Python hook.

    Classes decorated with @sionna_hook will be automatically discovered and
    instantiated when loading a Python package/directory.

    Example usage:
        from cavise.sionna.environment.hook.decorators import sionna_hook

        @sionna_hook
        class MyCustomHook:
            def on_traci_init(self):
                print("TraCI initialized")

            def on_traci_step(self):
                pass

            def on_scene_edit_begin(self):
                pass

            def on_scene_edit_end(self):
                pass

            def on_traci_close(self):
                pass
    """
    cls._sionna_hook_marked = True
    return cls


def is_sionna_hook(cls):
    """
    Check if a class is marked as a Sionna hook.

    Args:
        cls: The class to check

    Returns:
        True if the class is marked with @sionna_hook
    """
    return getattr(cls, '_sionna_hook_marked', False)