# Compatibility test for current Mitsuba distribution. If this compiles and runs,
# that means that nanobind object registry was shared and Sionna bridge module can operate.

add_executable(test_config test_two_way_cast.cpp)

# Make this target behave like a nanobind "shared runtime" consumer:
# - NB_SHARED: use shared nanobind core (matches libnanobind-drjit.so)
# - NB_DOMAIN=drjit: same domain as Mitsuba/DrJit
target_compile_definitions(test_config PRIVATE
    NB_SHARED
    NB_DOMAIN=drjit
)

# Do *not* hide everything in this executable; hidden UND references
# won't link cleanly against the GLOBAL DEFAULT symbols in libnanobind-drjit.so
set_target_properties(test_config PROPERTIES
    CXX_VISIBILITY_PRESET default
    VISIBILITY_INLINES_HIDDEN OFF
)

# If you have explicit include dirs for Mitsuba/nanobind, keep them here.
# (Optional, if FindMitsuba already does this via imported targets, you can skip.)
# target_include_directories(test_config PRIVATE
#     ${Python_INCLUDE_DIRS}
#     ${NANOBIND_INCLUDE_DIR}
#     ${MITSUBA_INCLUDE_DIRS}
# )

# Enable sanitizers for this test only (GCC/Clang)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(SANITIZERS "-fsanitize=address,undefined")

    target_compile_options(test_config PRIVATE ${SANITIZERS})
    target_link_options(test_config PRIVATE ${SANITIZERS})
endif()


target_link_libraries(test_config PRIVATE
    pybind11::embed
    Mitsuba::mitsuba
    Mitsuba::nanobind-drjit
    Mitsuba::mitsuba-ext
    drjit
    GTest::gtest_main
)

add_test(NAME test_cmake_configuration COMMAND test_config)
