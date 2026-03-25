package artery.sionna.environment;

import inet.environment.common.PhysicalEnvironment;
import artery.sionna.environment.providers.IDynamicSceneConfigProvider;
import artery.sionna.environment.providers.ISceneConfigProvider;

//
// Extension of INET PhysicalEnvironment that mirrors the configured physical
// scene into a Sionna RT scene.
//
// For the first scenario, this module should stay opinionated:
// - a dedicated provider builds the initial static scene
// - a dedicated provider emits dynamic updates
// - scene/material mismatches fail fast by default
//
simple SionnaPhysicalEnvironment extends PhysicalEnvironment
{
    parameters:
        @namespace(artery::sionna);
        @class(PhysicalEnvironment);

        // Keep INET defaults explicit so scenarios can override them centrally.
        string objectCacheType = default("BVHObjectCache");
        string groundType = default("FlatGround");

        // Scene providers. Keep source selection out of the environment itself.
        string sceneConfigProviderType = default("artery.sionna.environment.providers.InetSceneConfigProvider"); // scene-file provider: artery.sionna.environment.providers.scenefile.SceneFileConfigProvider
        string dynamicSceneConfigProviderType = default("");

        // Runtime policy.
        string rtBackend = default("llvm"); // safer default for a first scenario
        bool enableGradients = default(false);
        bool failOnMissingMeshes = default(true);
        bool failOnUnknownMaterial = default(true);

    submodules:
        sceneConfigProvider: <sceneConfigProviderType> like ISceneConfigProvider {
            @display("p=140,60");
        }

        dynamicSceneConfigProvider: <dynamicSceneConfigProviderType> like IDynamicSceneConfigProvider if dynamicSceneConfigProviderType != "" {
            @display("p=140,100");
        }
}
