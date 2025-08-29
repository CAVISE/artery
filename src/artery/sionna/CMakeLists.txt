add_artery_feature(sionna
    bridge/Scene.cc
    bridge/Scene.h
    bridge/Material.cc
    bridge/Material.h
    bridge/SceneObject.cc
    bridge/SceneObject.h
    bridge/meshes/Cube.cc
    bridge/meshes/Sphere.cc
    bridge/meshes/Prism.cc
    bridge/meshes/Polyhedron.cc
    bridge/meshes/Meshes.cc
    environment/PhysicalEnvironment.h
    environment/PhysicalEnvironment.cc
)

target_link_libraries(sionna PRIVATE pybind11::pybind11 INET)
