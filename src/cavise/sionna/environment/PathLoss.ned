package artery.sionna.environment;

import inet.physicallayer.contract.packetlevel.IPathLoss;

//
// Ray-traced path loss model backed by the Sionna physical environment.
//
// This module is intended to sit under `radioMedium.pathLoss` and query the
// sibling `physicalEnvironment` module in the world. A fallback submodule is
// kept in the contract because the first scenario should degrade cleanly while
// the Sionna runtime is still being completed.
//
module SionnaPathLoss like IPathLoss
{
    parameters:
        @display("i=block/wrxtx");
        @namespace(artery::sionna);
        @class(PathLoss);

        string physicalEnvironmentModule = default(absPath("^.^.physicalEnvironment"));
        bool requirePhysicalEnvironment = default(true);
        bool updateDynamicObjectsOnQuery = default(false);

        bool includeLineOfSight = default(true);
        bool includeReflections = default(true);
        bool includeDiffractions = default(false);
        int maxReflectionDepth = default(1);
        int maxDiffractionDepth = default(0);
        double maxRange @unit(m) = default(1000m);

        bool withFallback = default(true);
        bool failOnSceneError = default(true);

    submodules:
        fallback: <default("FreeSpacePathLoss")> like IPathLoss if withFallback {
            @display("p=20,20");
        }
}
