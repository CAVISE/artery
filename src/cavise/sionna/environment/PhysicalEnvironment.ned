package cavise.sionna.environment;

import inet.environment.common.PhysicalEnvironment;
import cavise.sionna.environment.config.IDynamicSceneConfigProvider;
import cavise.sionna.environment.config.IBootstrapSceneConfigProvider;
import cavise.sionna.environment.config.IMeshRegistry;
import cavise.sionna.environment.visualization.ISceneVisualizer;

//
// Extension of INET PhysicalEnvironment that mirrors the configured physical
// scene into a Sionna RT scene.
//
// For the first scenario, this module should stay opinionated:
// - a dedicated provider builds the initial static scene
// - a dedicated provider emits dynamic updates
// - scene/material mismatches fail fast by default
//
module SionnaPhysicalEnvironment extends PhysicalEnvironment
{
    parameters:
        @namespace(artery::sionna);
        @class(PhysicalEnvironment);

        // Scene providers. Keep source selection out of the environment itself.
        string sceneConfigProviderType = default("cavise.sionna.environment.config.scenes.FileSceneConfigProvider");
        string dynamicSceneConfigProviderType = default("");
        string meshRegistryType = default("");
        string sceneVisualizerType = default("");

        // Runtime policy.
        string rtBackend = default("llvm"); // safer default for a first scenario
        bool enableGradients = default(false);
        bool failOnMissingMeshes = default(true);
        bool failOnUnknownMaterial = default(true);

    submodules:
        sceneConfigProvider: <sceneConfigProviderType> like IBootstrapSceneConfigProvider {
            @display("p=140,60");
        }

        dynamicSceneConfigProvider: <dynamicSceneConfigProviderType> like IDynamicSceneConfigProvider if dynamicSceneConfigProviderType != "" {
            @display("p=140,100");
        }

        meshRegistry: <meshRegistryType> like IMeshRegistry if meshRegistryType != "" {
            @display("p=140,140");
        }

        sceneVisualizer: <sceneVisualizerType> like ISceneVisualizer if sceneVisualizerType != "" {
            @display("p=140,180");
        }
}
