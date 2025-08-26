package artery.sionna.environment;

import inet.environment.common.PhysicalEnvironment;

//
// Extension of INET PhysicalEnvironment that also mirrors geometry and
// materials into a Sionna RT scene.
//
simple SionnaPhysicalEnvironment extends PhysicalEnvironment
{
    parameters:
        @class(SionnaPhysicalEnvironment);

        string rtBackend = default("cuda"); // or "optix", "vulkan" later
        string materialSet = default("inet_compat");
        bool enableGradients = default(false); // RT run mode
}
