package cavise.sionna.environment.hook;

@namespace(artery::sionna);

//
// Interface for Sionna Python Hook modules.
// Modules implementing this interface can hook into the Sionna scene
// lifecycle and execute custom Python code at various points.
//
// The SionnaPythonHook implementation supports:
// - Loading single Python files or entire packages (directories)
// - Auto-discovery of classes decorated with @sionna_hook
// - Backward compatibility with 'Hook' class or 'hook' attribute
//
// See SionnaPythonHook.ned for detailed usage documentation.
//
moduleinterface ISionnaPythonHook
{
}
