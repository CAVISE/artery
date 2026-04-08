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
        config/TraciDynamicSceneConfigProvider.cc
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
        config/IConfigProvider.h
        config/meshes/IMeshRegistry.h
        config/scenes/FileSceneConfigProvider.h
        config/meshes/SionnaAssetMeshRegistry.h
        config/TraciDynamicSceneConfigProvider.h
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
