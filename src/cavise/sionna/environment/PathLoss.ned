package cavise.sionna.environment;

import inet.physicallayer.contract.packetlevel.IPathLoss;

//
// Ray-traced path loss model backed by the Sionna physical environment.
//
// This module is intended to sit under `radioMedium.pathLoss` and query the
// sibling `physicalEnvironment` module in the world.
//
module SionnaPathLoss like IPathLoss
{
    parameters:
        @class(PathLoss);
        @namespace(artery::sionna);

        @display("i=block/wrxtx");
        @signal[sionnaPathsSolved](type=unsigned long);
        @statistic[sionnaPathsSolved](source=sionnaPathsSolved; record=count,vector);

        double alpha = default(2);
        double systemLoss @unit(dB) = default(0dB);

        xml solverConfig = default(null);
        double maxRange @unit(m) = default(1000m);
        string physicalEnvironmentModule = default(absPath("^.^.physicalEnvironment"));

        // Direct cOutVector statistics emitted by the C++ implementation:
        // - "sionna.gain: <tx> -> <rx>"
        // - "sionna.strongestPathType: <tx> -> <rx>" (-1 means no valid path)
        // - "sionna.solvedLinks.{missing,los,specular,diffuse,refraction,diffraction}"
}
