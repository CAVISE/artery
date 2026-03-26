###############
# Main target #
###############

set(SIONNA_BRIDGE_COMPILATION_MODES Basic Inet)

set(SIONNA_BRIDGE_TARGET_TRIPLETS
    "scalar|float|mitsuba::Color<float$<COMMA> 3>"
    "llvm-detached|drjit::LLVMArray<float>|mitsuba::Color<drjit::LLVMArray<float>$<COMMA> 3>"
    "llvm-ad|drjit::LLVMDiffArray<float>|mitsuba::Color<drjit::LLVMDiffArray<float>$<COMMA> 3>"
)
set(SIONNA_BRIDGE_TARGET_MODE_PAIRS)

# Main target(s). There is a separate library for each compilation mode and
# resolve profile.
foreach(_mode IN ITEMS ${SIONNA_BRIDGE_COMPILATION_MODES})
    string(TOLOWER ${_mode} _mode_lowercase)

    foreach(_profile_triplet IN LISTS SIONNA_BRIDGE_TARGET_TRIPLETS)
        string(REPLACE "|" ";" _profile_parts ${_profile_triplet})
        list(GET _profile_parts 0 _profile_suffix)
        list(GET _profile_parts 1 _resolve_float)
        list(GET _profile_parts 2 _resolve_spectrum)

        set(_target sionna-bridge-${_mode_lowercase}-${_profile_suffix})
        list(APPEND SIONNA_BRIDGE_TARGET_MODE_PAIRS "${_target}|${_mode}")
        add_library(${_target} SHARED)

        target_sources(${_target}
            PRIVATE
                bindings/Constants.cc
                bindings/Material.cc
                bindings/Scene.cc
                bindings/SceneObject.cc
            PUBLIC
                Fwd.h
                Helpers.h
                Compat.h
                Defaulted.h
                Capabilities.h
                bindings/Constants.h
                bindings/Material.h
                bindings/Scene.h
                bindings/SceneObject.h
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
                drjit::drjit-extra
                tsl::robin_map
                Python::Python
                sionna-compilation-flags
                Backward::Backward
        )

        target_compile_options(${_target}
            PUBLIC
                $<$<CONFIG:Debug>:-DNDEBUG>
        )

        set_target_properties(${_target}
            PROPERTIES
                SIONNA_BRIDGE_COMPILATION_MODE ${_mode}
                SIONNA_FLOAT_TYPE ${_resolve_float}
                SIONNA_SPECTRUM_TYPE ${_resolve_spectrum}
        )
    endforeach()
endforeach()

# Post configuration for various compilation modes.
foreach(_pair IN LISTS SIONNA_BRIDGE_TARGET_MODE_PAIRS)
    string(REPLACE "|" ";" _pair_parts ${_pair})
    list(GET _pair_parts 0 _target)
    list(GET _pair_parts 1 _mode)

    if(_mode STREQUAL "Inet")
        target_link_libraries(${_target} PUBLIC INET)
    endif()
endforeach()
