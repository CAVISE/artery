package artery.sionna.environment.providers;

//
// Supplies the initial Sionna scene description consumed by the physical
// environment. Implementations may derive the scene from INET's XML
// configuration, an external scene file, or some future generator.
//
moduleinterface ISceneConfigProvider
{
    parameters:
        @display("i=block/cogwheel");
}

simple InetSceneConfigProvider like ISceneConfigProvider
{
    parameters:
        @namespace(artery::sionna);
        @class(InetSceneConfigProvider);
        string sourcePhysicalEnvironmentModule = default("^");
}
