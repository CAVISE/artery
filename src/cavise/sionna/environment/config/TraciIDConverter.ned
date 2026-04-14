package cavise.sionna.environment.config;

//
// Converter for TraCI IDs into scene-compatible IDs.
//
simple TraciIDConverter like ITraciIDConverter
{
    parameters:
        @namespace(artery::sionna);
        @class(TraciIDConverter);
}
