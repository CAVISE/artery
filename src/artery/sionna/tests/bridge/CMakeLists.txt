################
# Bridge tests #
################

add_sionna_test(test_bridge_module
    TEST_NAME
        test_bridge_module
    FILES
        TestDefaulted.cc
)

target_link_libraries(test_bridge_module
    PRIVATE
        Artery::sionna::core
)

set_tests_properties(test_bridge_module
    PROPERTIES
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
