package cavise.sionna.environment.config.dynamic;

simple TraciDynamicSceneConfigProvider
{
    parameters:
        @class(TraciDynamicSceneConfigProvider);

        // Path to traci::BasicNodeManager (or compatible emitter of TraCI add/remove signals).
        string traciNodeManagerModule = default("^.^.traci.nodes");
        // Path to module implementing artery::sionna::IMeshRegistry.
        string meshRegistryModule = default("^.meshRegistry");

        // If true, TraCI add/remove signal reception triggers edit().
        // Not advisable at all.
        bool autoEditOnTraCIEvents = default(true);
}
