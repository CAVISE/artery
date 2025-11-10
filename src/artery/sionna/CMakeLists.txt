find_package(Mitsuba MODULE)

# Ensure domains are shared with Mitsuba, and DrJit in turn.
set(MI_DOMAIN drjit)

add_artery_feature(sionna
    bridge/Scene.cc
    bridge/Scene.h
    bridge/Material.h
    bridge/SceneObject.h
    bridge/meshes/Cube.cc
    bridge/meshes/Sphere.cc
    bridge/meshes/Prism.cc
    bridge/meshes/Polyhedron.cc
    bridge/meshes/Meshes.h
    environment/PhysicalEnvironment.h
    environment/PhysicalEnvironment.cc
)

target_link_libraries(sionna PRIVATE
    INET
    pybind11::pybind11
    Eigen3::Eigen
    Mitsuba::Mitsuba
    tsl::robin_map
)

set_target_properties(sionna PROPERTIES CXX_VISIBILITY_PRESET hidden)
