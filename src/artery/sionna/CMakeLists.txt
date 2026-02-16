##########
# Sionna #
##########

if(NOT DEFINED ENV{VIRTUAL_ENV})
    message(SEND_ERROR
        "It looks like you are not use virtual environment - which normally means something went wrong. "
        "If you really want to continue, set VIRTUAL_ENV environment variable to system python home."
    )
elseif(NOT IS_DIRECTORY $ENV{VIRTUAL_ENV})
    message(SEND_ERROR "Cannot find virtual environment directory, aborting.")
endif()

# Handle packages and configurations stuff.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(DiscoverCompileOptions)

find_package(Python COMPONENTS Interpreter Development REQUIRED)

find_package(Nanobind MODULE REQUIRED)
find_package(DrJit MODULE REQUIRED)
find_package(Mitsuba MODULE REQUIRED)

###############
# Main target #
###############

set(SIONNA_BRIDGE_COMPILATION_MODES Basic Inet)

# Main target(s). There is a separate library for each compilation mode.
foreach(_mode IN ITEMS ${SIONNA_BRIDGE_COMPILATION_MODES})
    string(TOLOWER ${_mode} _mode_lowercase)
    set(_target sionna-bridge-${_mode_lowercase})

    add_library(${_target} SHARED)

    target_sources(${_target}
        PRIVATE
            bridge/bindings/Constants.cc
            bridge/bindings/Material.cc
            bridge/bindings/Scene.cc
            bridge/bindings/SceneObject.cc
        PUBLIC
            bridge/Fwd.h
            bridge/Helpers.h
            bridge/Compat.h
            bridge/Defaulted.h
            bridge/Capabilities.h
            bridge/bindings/Constants.h
            bridge/bindings/Material.h
            bridge/bindings/Scene.h
            bridge/bindings/SceneObject.h
    )

    target_include_directories(${_target}
        PUBLIC
            ${CMAKE_SOURCE_DIR}/src
    )

    target_link_libraries(${_target}
        PUBLIC
            Mitsuba::nanobind-drjit
            Mitsuba::mitsuba
            drjit::drjit
            drjit::drjit-core
            tsl::robin_map
            Python::Python
            sionna-compilation-flags
    )

    set_target_properties(${_target}
        PROPERTIES
            SIONNA_BRIDGE_COMPILATION_MODE ${_mode}
    )
endforeach()

# Post configuration for various compilation modes.
target_link_libraries(sionna-bridge-inet PUBLIC INET)

# Extensions. Implement abstractions necessary to integrate Sionna with
# various network simulation kernels.
add_artery_feature(sionna-phy)

target_sources(sionna-phy
    PRIVATE
        environment/Ground.cc
        environment/MaterialRegistry.cc
        environment/MitsubaShape.cc
        environment/PhysicalObject.cc
        environment/Material.cc
        environment/PathLoss.cc
        environment/PhysicalEnvironment.cc
    PUBLIC
        environment/Ground.h
        environment/MaterialRegistry.h
        environment/MitsubaShape.h
        environment/PhysicalObject.h
        environment/Material.h
        environment/PathLoss.h
        environment/PhysicalEnvironment.h
)

set_target_properties(sionna-phy PROPERTIES SIONNA_BRIDGE_COMPILATION_MODE Inet)
target_link_libraries(sionna-phy PUBLIC sionna-bridge-inet)

# if(WITH_TESTS)
#     add_subdirectory(tests)
# endif()
