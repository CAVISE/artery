# Mitsuba3 exports shared libraries and even headers when installed from python
# package registry.

#####################
# Package Discovery #
#####################

# Required by nanobind and script later.
find_package(Python COMPONENTS Interpreter Development REQUIRED)

set(_collect_dist_path_script "collect-dist-path.py")
# This finds out current site directory for python packages.
execute_process(
    COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${_collect_dist_path_script}
    OUTPUT_VARIABLE _python_site_packages_path
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# For mitsuba to work, we also need DrJit
find_package(drjit CONFIG HINTS ${_python_site_packages_path}/drjit/cmake REQUIRED)
# Nanobind is required to access Mitsuba objects later
find_package(nanobind CONFIG HINTS ${_python_site_packages_path}/nanobind/cmake REQUIRED)

#########
# DrJit #
#########

# DrJit does not provide aliases for imported targets.
add_library(drjit::drjit ALIAS drjit)
add_library(drjit::drjit-core ALIAS drjit-core)

###############################
# Include directory discovery #
###############################

set(_mi_root ${_python_site_packages_path}/mitsuba)

find_path(MITSUBA_INCLUDE_DIRS
    NAMES mitsuba/mitsuba.h
    HINTS
        ${_mi_root}
        ${_mi_root}/include
    NO_DEFAULT_PATH
)

set(_mitsuba_header "${MITSUBA_INCLUDE_DIRS}/mitsuba/mitsuba.h")

if (EXISTS "${_mitsuba_header}")
    # Extract version macros from mitsuba.h
    file(STRINGS "${_mitsuba_header}" _mi_major_line REGEX "^#define[ \t]+MI_VERSION_MAJOR[ \t]+[0-9]+")
    file(STRINGS "${_mitsuba_header}" _mi_minor_line REGEX "^#define[ \t]+MI_VERSION_MINOR[ \t]+[0-9]+")
    file(STRINGS "${_mitsuba_header}" _mi_patch_line REGEX "^#define[ \t]+MI_VERSION_PATCH[ \t]+[0-9]+")

    string(REGEX MATCH "[0-9]+" MI_VERSION_MAJOR "${_mi_major_line}")
    string(REGEX MATCH "[0-9]+" MI_VERSION_MINOR "${_mi_minor_line}")
    string(REGEX MATCH "[0-9]+" MI_VERSION_PATCH "${_mi_patch_line}")

    set(MI_VERSION "${MI_VERSION_MAJOR}.${MI_VERSION_MINOR}.${MI_VERSION_PATCH}")
    message(STATUS "Detected Mitsuba version: ${MI_VERSION}")
else()
    set(MI_VERSION "")
    message(WARNING "Cannot find mitsuba.h at ${_mitsuba_header} - version not detected")
endif()

list(APPEND MITSUBA_INCLUDE_DIRS ${NB_DIR}/include)

######################################
# Mitsuba shared libraries discovery #
######################################

# To use with nanobind.
set(_domain drjit)

# Mitsuba compiles python bindings by MI_VARIANTS, which are types
# that are used in Mitsuba core library. All those variants are compiled into
# Mitsuba core library, and get chosen at runtime per user needs.

set(MITSUBA_LIBRARIES )
set(_libraries mitsuba nanobind-${_domain})

foreach(_library IN ITEMS ${_libraries})
    unset(_library_path CACHE)
    find_library(_library_path NAMES ${_library} HINTS ${_mi_root} NO_DEFAULT_PATH REQUIRED)

    set(_library_name mitsuba-${_library})
    set(_library_alias Mitsuba::${_library})

    add_library(${_library_name} SHARED IMPORTED)
    add_library(${_library_alias} ALIAS ${_library_name})

    target_include_directories(${_library_name} INTERFACE ${MITSUBA_INCLUDE_DIRS})
    list(APPEND MITSUBA_LIBRARIES ${_library_name})

    set_target_properties(${_library_name}
        PROPERTIES
            IMPORTED_NO_SONAME ON
            IMPORTED_LOCATION ${_library_path}
    )
endforeach()

add_library(mi INTERFACE)
add_library(Mitsuba::mi ALIAS mi)

set_target_properties(mi
    PROPERTIES
        INTERFACE_LINK_LIBRARIES "drjit;drjit-core;${MITSUBA_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${MITSUBA_INCLUDE_DIRS}"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Mitsuba
    REQUIRED_VARS MITSUBA_INCLUDE_DIRS MITSUBA_LIBRARIES
)
