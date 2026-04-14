package cavise.sionna.environment.config;

simple TraciDynamicSceneConfigProvider like IDynamicSceneConfigProvider
{
    parameters:
        @namespace(artery::sionna);
        @class(TraciDynamicSceneConfigProvider);

        // Path to traci::BasicNodeManager (or compatible emitter of TraCI add/remove signals).
        string traciNodeManagerModule = default("");
        // Path to module implementing artery::sionna::IMeshRegistry.
        string meshRegistryModule = default("");
        // Path to module implementing artery::sionna::ITraciIDConverter.
        string idConverterModule = default("");
        // Path to module implementing artery::sionna::ITraciCoordinateTransformer.
        string coordinateTransformerModule = default("");

        // If true, TraCI add/remove signal reception triggers edit().
        bool autoEditOnTraCIEvents = default(true);
}
