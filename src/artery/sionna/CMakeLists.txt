add_artery_feature(sionna
    bridge/Scene.cc
    bridge/Scene.h
    environment/PhysicalEnvironment.h
    environment/PhysicalEnvironment.cc
)

target_link_libraries(sionna PRIVATE pybind11::pybind11 INET)
