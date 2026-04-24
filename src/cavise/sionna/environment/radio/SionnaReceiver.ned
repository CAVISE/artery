package cavise.sionna.environment.radio;

import cavise.sionna.environment.radio.ISionnaReceiver;

simple SionnaReceiver like ISionnaReceiver
{
    parameters:
        @class(SionnaReceiver);
        string physicalEnvironmentModule = default(absPath("^.^.^.^.physicalEnvironment"));
}
