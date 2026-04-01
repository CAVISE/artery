package cavise.sionna.environment.config;

simple TraciDynamicSceneConfigProvider like IDynamicSceneConfigProvider
{
    parameters:
        @namespace(artery::sionna);
        @class(TraciDynamicSceneConfigProvider);

        // Path to traci::BasicNodeManager (or compatible emitter of TraCI add/remove signals).
        string traciNodeManagerModule = default("");
        // Optional path to module implementing artery::sionna::IMeshRegistry.
        string meshRegistryModule = default("");

        // If true, TraCI add/remove signal reception triggers edit().
        bool autoEditOnTraCIEvents = default(true);
}
