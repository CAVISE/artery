package cavise.sionna.environment.config.dynamic;

simple TraciDynamicSceneConfigProvider like IDynamicSceneConfigProvider
{
    parameters:
        @namespace(artery::sionna);
        @class(TraciDynamicSceneConfigProvider);

        // Path to traci::BasicNodeManager (or compatible emitter of TraCI add/remove signals).
        string traciNodeManagerModule = default("^.^.traci.nodes");
        // Path to module implementing artery::sionna::IMeshRegistry.
        string meshRegistryModule = default("^.meshRegistry");
        // Path to module implementing artery::sionna::ITraciIDConverter.
        string idConverterModule = default("^.idConverter");
        // Path to module implementing artery::sionna::ITraciCoordinateTransformer.
        string coordinateTransformerModule = default("^.coordinateTransformer");

        // If true, TraCI add/remove signal reception triggers edit().
        bool autoEditOnTraCIEvents = default(true);
}
