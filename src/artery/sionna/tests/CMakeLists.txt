################
# Sionna tests #
################

# Tests whether current configuration will work for Sionna bridge. Failing
# this test means bridge will not function correctly.
add_executable(test_mitsuba_linkage
    TestTwoWayCast.cc
    TestStructAlignments.cc
)

# Runs light tests for compatability layer.
add_executable(test_mitsuba_compat
    TestMitsubaLikesShapes.cc
)

target_link_libraries(test_mitsuba_linkage
    PRIVATE
        GTest::gtest
        Artery::sionna::helpers
)

target_link_libraries(test_mitsuba_compat
    PRIVATE
        GTest::gtest
        Artery::sionna::helpers
)

sionna_target_compile_detached(test_mitsuba_linkage)
sionna_target_compile_detached(test_mitsuba_compat)

add_test(NAME test_cmake_configuration COMMAND test_mitsuba_linkage)
add_test(NAME test_sionna_bridge_compat COMMAND test_mitsuba_compat)
