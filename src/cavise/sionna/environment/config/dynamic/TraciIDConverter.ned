package cavise.sionna.environment.config.dynamic;

//
// Provider for TraCI<->scene id mapping.
//
moduleinterface ITraciIDConverter
{
}

//
// Converter for TraCI IDs into scene-compatible IDs.
//
simple TraciIDConverter like ITraciIDConverter
{
    parameters:
        @namespace(artery::sionna);
        @class(TraciIDConverter);
}
