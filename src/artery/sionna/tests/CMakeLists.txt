################
# Sionna tests #
################

# Tests whether current configuration will work for Sionna bridge. Failing
# this test means bridge will not function correctly.
add_executable(test_mitsuba_linkage
    TestTwoWayCast.cc
    TestStructAlignments.cc
)

target_link_libraries(test_mitsuba_linkage
    PRIVATE
        GTest::gtest
        Artery::sionna::helpers
)

sionna_target_compile_detached(test_mitsuba_linkage)

add_test(NAME test_cmake_configuration COMMAND test_mitsuba_linkage)
