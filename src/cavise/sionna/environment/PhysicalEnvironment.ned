package cavise.sionna.environment;

import inet.environment.common.PhysicalEnvironment;
import cavise.sionna.environment.config.dynamic.TraciDynamicSceneConfigProvider;
import cavise.sionna.environment.config.meshes.IMeshRegistry;
import cavise.sionna.environment.config.scenes.IStaticSceneProvider;
import cavise.sionna.environment.visualization.ISceneVisualizer;
import cavise.sionna.environment.hook.ISionnaPythonHook;

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
        string meshRegistryType = default("cavise.sionna.environment.config.meshes.SionnaDirectAssetMeshRegistry");
        string sceneVisualizerType = default("cavise.sionna.environment.visualization.SionnaSceneVisualizer");
        string traciNodeManagerModule = default("^.traci.nodes");
        string pythonHookType = default("");  // Set to "cavise.sionna.environment.hook.SionnaPythonHook" to enable Python hook
        xml remapMatrix = default(xmldoc("transforms.xml", "/transforms/identity"));
        xml rotationMatrix = default(xmldoc("transforms.xml", "/transforms/identity"));
        xml translationVector = default(xmldoc("transforms.xml", "/transforms/zero"));

    submodules:
        sceneConfigProvider: <sceneConfigProviderType> like IStaticSceneProvider {
            @display("p=140,60");
        }

        dynamicSceneConfigProvider: TraciDynamicSceneConfigProvider {
            @display("p=140,100");
        }

        meshRegistry: <meshRegistryType> like IMeshRegistry if meshRegistryType != "" {
            @display("p=140,140");
        }

        sceneVisualizer: <sceneVisualizerType> like ISceneVisualizer if sceneVisualizerType != "" {
            @display("p=140,220");
        }

        pythonHook: <pythonHookType> like ISionnaPythonHook if pythonHookType != "" {
            @display("p=140,250");
        }
}
