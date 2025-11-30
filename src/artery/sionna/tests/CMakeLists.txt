################
# Sionna tests #
################

# Tests whether current configuration will work for Sionna bridge. Failing
# this test means bridge will not function correctly.
add_executable(test_mitsuba_linkage
    TestTwoWayCast.cc
    TestStructAlignments.cc
)

# Prints arch flags.
add_executable(arch-flags-unix ArchFlagsUnix.cc)

target_link_libraries(test_mitsuba_linkage
    PRIVATE
        GTest::gtest
        Artery::sionna-core
)

target_compile_options(arch-flags-unix PRIVATE -march=native)

add_test(NAME test_cmake_configuration COMMAND test_mitsuba_linkage)
