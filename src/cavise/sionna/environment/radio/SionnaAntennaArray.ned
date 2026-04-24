package cavise.sionna.environment.radio;

import inet.physicallayer.contract.packetlevel.IAntenna;

module SionnaAntennaArray like IAntenna
{
    parameters:
        string mobilityModule = default(".mobility");
        int numAntennas = default(1);
        @class(SionnaAntennaArray);
        double gain @unit(dB) = default(0dB);
        string sceneArrayRole @enum("", "tx", "rx") = default("");
        string physicalEnvironmentModule = default(absPath("^.^.^.^.physicalEnvironment"));
        @display("i=device/antennatower");
}
