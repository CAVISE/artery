package artery.sionna.environment.providers;

//
// Supplies dynamic updates for objects that move independently from the static
// physical environment configuration, such as TraCI-managed vehicles.
//
moduleinterface IDynamicSceneConfigProvider
{
    parameters:
        double updateInterval @unit(s) = default(0.1s);
        @display("i=block/cogwheel");
}

simple TraciDynamicSceneConfigProvider like IDynamicSceneConfigProvider
{
    parameters:
        @namespace(artery::sionna);
        @class(TraciDynamicSceneConfigProvider);
        string traciModule = default(absPath("^.^.traci"));
        bool importVehicles = default(true);
        bool importPersons = default(false);
}
