list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(Mitsuba MODULE)

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
    Eigen3::Eigen
    tsl::robin_map
    ${MITSUBA_LIBRARIES}
)

set_target_properties(sionna PROPERTIES CXX_VISIBILITY_PRESET hidden)

if(WITH_TESTS)
    add_subdirectory(tests)
endif()
