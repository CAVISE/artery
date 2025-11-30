##########
# Sionna #
##########

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Sionna is based on Mitsuba rendering library, which provides native cpp API.
find_package(Mitsuba MODULE)

###########
# Options #
###########

set(SIONNA_COMPILATION_FLAGS "-mavx2" CACHE STRING "Additional compilation arguments. Make sure to adjust this option to match Mitsuba's")

###############
# Main target #
###############

# Sionna dummy target that populates all settings that most Sionna targets would prefer.
add_library(sionna-core INTERFACE)
add_library(Artery::sionna-core ALIAS sionna-core)

target_include_directories(sionna-core
    INTERFACE
        $<TARGET_PROPERTY:Artery::Core,INTERFACE_INCLUDE_DIRECTORIES>
)

target_compile_definitions(sionna-core
    INTERFACE
        NB_SHARED
        NB_DOMAIN=drjit
)

target_compile_options(sionna-core
    INTERFACE
        ${SIONNA_COMPILATION_FLAGS}
)

target_link_libraries(sionna-core
    INTERFACE
        Mitsuba::nanobind-drjit
        Mitsuba::mitsuba
        drjit::drjit
        drjit::drjit-core
        tsl::robin_map
)

add_artery_feature(sionna
    bridge/Scene.h
    bridge/Material.h
    bridge/Constants.h
    bridge/SceneObject.h
    bridge/meshes/Cube.cc
    bridge/meshes/Sphere.cc
    bridge/meshes/Prism.cc
    bridge/meshes/Polyhedron.cc
    bridge/meshes/Meshes.h
    environment/Ground.h
    environment/PathLoss.h
    environment/PathLoss.cc
    environment/PhysicalEnvironment.h
    environment/PhysicalEnvironment.cc
)

target_link_libraries(sionna PRIVATE
    INET
    Eigen3::Eigen
    Artery::sionna-core
)

if(WITH_TESTS)
    add_subdirectory(tests)
endif()
