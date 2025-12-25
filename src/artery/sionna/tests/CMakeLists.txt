################
# Sionna tests #
################

# Declare a test with special main() implementation, suited for most tests
# that work with Mitsuba/Sionna.
function(add_sionna_test target)
    set(one_value_args TEST_NAME)
    set(multi_value_args FILES)
    cmake_parse_arguments(args "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(args_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "add_sionna_test called with invalid arguments: ${args_UNPARSED_ARGUMENTS}")
    endif()

    if(TARGET ${target})
        message(FATAL_ERROR "add_sionna_test: target already exists ${target}")
    endif()

    if(NOT args_TEST_NAME)
        message(FATAL_ERROR "add_sionna_test: TEST_NAME is required")
    endif()

    if(NOT args_FILES)
        message(FATAL_ERROR "add_sionna_test: FILES is required")
    endif()

    add_executable(${target} ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Main.cc ${args_FILES})
    target_link_libraries(${target}
        PRIVATE
            GTest::gtest
            Artery::sionna::helpers
    )

    target_compile_definitions(${target} PRIVATE SIONNA_OMNETPP_DETACHED)
    add_test(NAME ${args_TEST_NAME} COMMAND $<TARGET_FILE:${target}>)
endfunction()

# Tests whether current configuration will work for Sionna bridge. Failing
# this test means bridge will not function correctly.
add_sionna_test(test_mitsuba_linkage
    TEST_NAME
        test_cmake_configuration
    FILES 
        TestTwoWayCast.cc
        TestStructAlignments.cc
)

# Runs light tests for compatibility layer.
add_sionna_test(test_mitsuba_compat
    TEST_NAME
        test_sionna_bridge_compat
    FILES
        TestMitsubaLikesShapes.cc
)

add_subdirectory(bridge)
