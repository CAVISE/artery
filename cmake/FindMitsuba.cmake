# Mitsuba3 exports shared libraries and even headers when installed from python
# package registry.

find_package(Python COMPONENTS Interpreter Development REQUIRED)

# This finds out current site directory for python packages.
execute_process(
    COMMAND ${Python_EXECUTABLE} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())"
    OUTPUT_VARIABLE PYTHON_SITE_PACKAGES
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# For mitsuba to work, we also need DrJit
find_package(drjit CONFIG HINTS ${PYTHON_SITE_PACKAGES}/drjit/cmake REQUIRED)
# Nanobind also leaks into headers - do not use it with pybind!
find_package(nanobind CONFIG HINTS ${PYTHON_SITE_PACKAGES}/nanobind/cmake REQUIRED)

set(PYBIND_INCLUDE_DIR ${NB_DIR}/include)

set(Mitsuba_ROOT ${PYTHON_SITE_PACKAGES}/mitsuba)
message(STATUS "Mitsuba root: ${Mitsuba_ROOT}")

find_path(MITSUBA_INCLUDE_DIRS
    NAMES mitsuba/mitsuba.h
    HINTS
        ${Mitsuba_ROOT}
        ${Mitsuba_ROOT}/include
    NO_DEFAULT_PATH
)

set(_mitsuba_header "${MITSUBA_INCLUDE_DIRS}/mitsuba/mitsuba.h")

if (EXISTS "${_mitsuba_header}")
    # Extract version macros from mitsuba.h
    file(STRINGS "${_mitsuba_header}" _mitsuba_major_line REGEX "^#define[ \t]+MI_VERSION_MAJOR[ \t]+[0-9]+")
    file(STRINGS "${_mitsuba_header}" _mitsuba_minor_line REGEX "^#define[ \t]+MI_VERSION_MINOR[ \t]+[0-9]+")
    file(STRINGS "${_mitsuba_header}" _mitsuba_patch_line REGEX "^#define[ \t]+MI_VERSION_PATCH[ \t]+[0-9]+")

    string(REGEX MATCH "[0-9]+" MITSUBA_VERSION_MAJOR "${_mitsuba_major_line}")
    string(REGEX MATCH "[0-9]+" MITSUBA_VERSION_MINOR "${_mitsuba_minor_line}")
    string(REGEX MATCH "[0-9]+" MITSUBA_VERSION_PATCH "${_mitsuba_patch_line}")

    set(MITSUBA_VERSION "${MITSUBA_VERSION_MAJOR}.${MITSUBA_VERSION_MINOR}.${MITSUBA_VERSION_PATCH}")
    message(STATUS "Detected Mitsuba version: ${MITSUBA_VERSION}")
else()
    set(MITSUBA_VERSION "")
    message(WARNING "Cannot find mitsuba.h at ${_mitsuba_header} - version not detected")
endif()

set(_libraries mitsuba)
set(MITSUBA_LIBRARIES )

foreach(_library IN LISTS _libraries)
    add_library(Mitsuba::${_library} SHARED IMPORTED)
    find_library(_library_path 
        NAMES ${_library}
        HINTS
            ${Mitsuba_ROOT}
            ${Mitsuba_ROOT}/lib
        NO_DEFAULT_PATH
    )
    list(APPEND MITSUBA_LIBRARIES Mitsuba::${_library})
    set_target_properties(Mitsuba::${_library} PROPERTIES IMPORTED_LOCATION ${_library_path})
    set_property(TARGET Mitsuba::${_library} PROPERTY IMPORTED_NO_SONAME ON)
    message(STATUS "Found ${_library} with path ${_library_path}")
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Mitsuba
    REQUIRED_VARS MITSUBA_INCLUDE_DIRS MITSUBA_LIBRARIES
)

if(MITSUBA_FOUND)
    add_library(Mitsuba::Mitsuba INTERFACE IMPORTED)
    set_target_properties(Mitsuba::Mitsuba PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${MITSUBA_INCLUDE_DIRS};${drjit_INCLUDE_DIR};${PYBIND_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${MITSUBA_LIBRARIES}"
    )
endif()
