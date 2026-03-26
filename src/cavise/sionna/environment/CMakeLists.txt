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
    PUBLIC
        Compat.h
        Ground.h
        MaterialRegistry.h
        MitsubaShape.h
        PhysicalObject.h
        Material.h
        PathLoss.h
        PhysicalEnvironment.h
)

set_target_properties(sionna-phy PROPERTIES SIONNA_BRIDGE_COMPILATION_MODE Inet)
target_link_libraries(sionna-phy PUBLIC sionna-bridge-inet)
target_compile_options(sionna-phy PUBLIC $<$<CONFIG:Debug>:-DNDEBUG>)
