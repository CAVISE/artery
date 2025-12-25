##########
# Sionna #
##########

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(Mitsuba MODULE)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-march=native HAVE_NATIVE_ARCH_SUPPORT)

if(NOT HAVE_NATIVE_ARCH_SUPPORT)
    message(WARNING "Your compiler does not support optimizing for current arch. This may break linkage with Mitsuba!")
    set(SIONNA_CXX_COMPILER_FLAGS )
else()
    set(SIONNA_CXX_COMPILER_FLAGS $<$<COMPILE_LANGUAGE:CXX>:-march=native>)
endif()

if(NOT DEFINED ENV{VIRTUAL_ENV})
    message(SEND_ERROR 
        "It looks like you are not use virtual environment - which normally means something went wrong. "
        "If you really want to continue, set VIRTUAL_ENV environment variable to system python home."
    )
elseif(NOT IS_DIRECTORY $ENV{VIRTUAL_ENV})
    message(SEND_ERROR "Cannot find virtual environment directory, aborting.")
endif()

# Compile target detached from OmnetPP runtime dependencies.

macro(sionna_target_compile_detached target)
    if(NOT TARGET ${target})
        message(SEND_ERROR "sionna_target_compile_detached: could not find target ${target}")
    endif()
    target_compile_definitions(${target} PRIVATE SIONNA_OMNETPP_DETACHED)
endmacro()

# This variable hints Sionna bridge at virtual environment, as we know it
# during configuration phase. At runtime, Sionna should fall back to these if
# all other options fail.

set(SIONNA_VIRTUAL_ENV $ENV{VIRTUAL_ENV})

###############
# Main target #
###############

# Dummy that just populates settings.
add_library(sionna-bridge-common INTERFACE)

# Main target.
add_artery_feature(sionna-bridge)

# Composite targets, some of them may be used independently.
add_library(sionna-bridge-inet STATIC)
add_library(sionna-bridge-core INTERFACE)
add_library(sionna-bridge-helpers INTERFACE)

# Aliases.
add_library(Artery::sionna::common ALIAS sionna-bridge-common)
add_library(Artery::sionna::bridge ALIAS sionna-bridge)
add_library(Artery::sionna::inet ALIAS sionna-bridge-inet)
add_library(Artery::sionna::core ALIAS sionna-bridge-core)
add_library(Artery::sionna::helpers ALIAS sionna-bridge-helpers)

target_sources(sionna-bridge-inet
    PRIVATE
        environment/PathLoss.cc
        environment/PhysicalEnvironment.cc
    PUBLIC
        environment/Ground.h
        environment/PathLoss.h
        environment/PhysicalEnvironment.h
)

target_sources(sionna-bridge-core
    INTERFACE
        bridge/Scene.h
        bridge/Material.h
        bridge/Defaulted.h
        bridge/SceneObject.h
        bridge/Meshes.h
)

target_sources(sionna-bridge-helpers
    INTERFACE
        bridge/Fwd.h
        bridge/Helpers.h
)

target_compile_options(sionna-bridge-common INTERFACE ${SIONNA_CXX_COMPILER_FLAGS})
target_compile_definitions(sionna-bridge-common INTERFACE SIONNA_VENV_HINT="${SIONNA_VIRTUAL_ENV}")
target_include_directories(sionna-bridge-common INTERFACE $<TARGET_PROPERTY:Artery::Core,INTERFACE_INCLUDE_DIRECTORIES>)

target_compile_definitions(sionna-bridge-common
    INTERFACE
        NB_SHARED
        NB_DOMAIN=drjit
)

target_link_libraries(sionna-bridge-common
    INTERFACE
        Mitsuba::nanobind-drjit
        Mitsuba::mitsuba
        drjit::drjit
        drjit::drjit-core
        tsl::robin_map
)

target_link_libraries(sionna-bridge
    PUBLIC
        sionna-bridge-inet
        sionna-bridge-helpers
        sionna-bridge-core
)

target_link_libraries(sionna-bridge-inet
    PUBLIC
        INET
        sionna-bridge-core
        sionna-bridge-helpers
        sionna-bridge-common
)

target_link_libraries(sionna-bridge-core
    INTERFACE
        sionna-bridge-helpers
        sionna-bridge-common
)

target_link_libraries(sionna-bridge-helpers
    INTERFACE
        Python::Python
        sionna-bridge-common
)

if(WITH_TESTS)
    add_subdirectory(tests)
endif()
