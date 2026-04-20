package cavise.sionna.environment;

import inet.environment.common.PhysicalEnvironment;
import cavise.sionna.environment.config.dynamic.IDynamicSceneConfigProvider;
import cavise.sionna.environment.config.dynamic.ITraciCoordinateTransformer;
import cavise.sionna.environment.config.dynamic.ITraciIDConverter;
import cavise.sionna.environment.config.meshes.IMeshRegistry;
import cavise.sionna.environment.config.scenes.IStaticSceneProvider;
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
        string dynamicSceneConfigProviderType = default("cavise.sionna.environment.config.dynamic.TraciDynamicSceneConfigProvider");
        string meshRegistryType = default("cavise.sionna.environment.config.meshes.SionnaDirectAssetMeshRegistry");
        string idConverterType = default("cavise.sionna.environment.config.dynamic.TraciIDConverter");
        string coordinateTransformerType = default("cavise.sionna.environment.config.dynamic.TraciCoordinateTransformer");
        string sceneVisualizerType = default("cavise.sionna.environment.visualization.SionnaSceneVisualizer");

    submodules:
        sceneConfigProvider: <sceneConfigProviderType> like IStaticSceneProvider {
            @display("p=140,60");
        }

        dynamicSceneConfigProvider: <dynamicSceneConfigProviderType> like IDynamicSceneConfigProvider if dynamicSceneConfigProviderType != "" {
            @display("p=140,100");
        }

        meshRegistry: <meshRegistryType> like IMeshRegistry if meshRegistryType != "" {
            @display("p=140,140");
        }

        idConverter: <idConverterType> like ITraciIDConverter if idConverterType != "" {
            @display("p=140,170");
        }

        coordinateTransformer: <coordinateTransformerType> like ITraciCoordinateTransformer if coordinateTransformerType != "" {
            @display("p=140,190");
        }

        sceneVisualizer: <sceneVisualizerType> like ISceneVisualizer if sceneVisualizerType != "" {
            @display("p=140,220");
        }
}
