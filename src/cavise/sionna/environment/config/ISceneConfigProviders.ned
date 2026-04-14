package cavise.sionna.environment.config;

//
// Provider for runtime scene edits (stage add/remove operations, then apply in batch).
//
moduleinterface IDynamicSceneConfigProvider
{
}

//
// Provider for initial scene bootstrap configuration.
//
moduleinterface IBootstrapSceneConfigProvider
{
}

//
// Provider for runtime mesh lookup by logical asset id.
//
moduleinterface IMeshRegistry
{
}

//
// Provider for TraCI<->scene id mapping.
//
moduleinterface ITraciIDConverter
{
}

//
// Provider for SUMO<->scene coordinate transforms.
//
moduleinterface ITraciCoordinateTransformer
{
}
