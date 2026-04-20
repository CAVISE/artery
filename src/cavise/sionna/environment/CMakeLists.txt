add_artery_feature(sionna-phy)

target_sources(sionna-phy
    PRIVATE
        Ground.cc
        MaterialRegistry.cc
        MitsubaShape.cc
        PhysicalObject.cc
        Material.cc
        PathLoss.cc
        PhysicalEnvironment.cc
        config/scenes/FileSceneConfigProvider.cc
        config/meshes/SionnaAssetMeshRegistry.cc
        config/dynamic/TraciDynamicSceneConfigProvider.cc
        config/dynamic/TraciIDConverter.cc
        config/dynamic/TraciCoordinateTransformer.cc
        config/dynamic/TraciRelaxedCoordinateTransformer.cc
        visualization/SionnaSceneVisualizer.cc
    PUBLIC
        Compat.h
        Ground.h
        MaterialRegistry.h
        MitsubaShape.h
        PhysicalObject.h
        Material.h
        PathLoss.h
        PhysicalEnvironment.h
        config/dynamic/IDynamicSceneConfigProvider.h
        config/meshes/IMeshRegistry.h
        config/scenes/IStaticSceneProvider.h
        config/scenes/FileSceneConfigProvider.h
        config/meshes/SionnaAssetMeshRegistry.h
        config/dynamic/TraciDynamicSceneConfigProvider.h
        config/dynamic/TraciIDConverter.h
        config/dynamic/TraciCoordinateTransformer.h
        config/dynamic/TraciRelaxedCoordinateTransformer.h
        visualization/ISceneVisualizer.h
        visualization/SionnaSceneVisualizer.h
)

set_target_properties(sionna-phy
    PROPERTIES
        SIONNA_BRIDGE_COMPILATION_MODE Inet
        COMPATIBLE_INTERFACE_STRING "SIONNA_FLOAT_TYPE;SIONNA_SPECTRUM_TYPE"
)
target_link_libraries(sionna-phy PUBLIC sionna-bridge-inet-llvm-ad-mono-polarized)
target_compile_options(sionna-phy PUBLIC $<$<CONFIG:Debug>:-DNDEBUG>)
