################
# Bridge tests #
################

option(SIONNA_ENABLE_LLVM_TESTS "Enable LLVM-based bridge tests" ON)

add_sionna_test(test_bridge_module
    TEST_NAME
        test_bridge_module
    FILES
        TestCapabilities.cc
        TestBridgeConstants.cc
        TestBridgeMaterial.cc
        TestDefaulted.cc
        TestCompat.cc
)

target_link_libraries(test_bridge_module
    PRIVATE
        sionna-bridge-basic
)

target_compile_definitions(test_bridge_module
    PRIVATE
        SIONNA_ENABLE_LLVM_TESTS=$<BOOL:${SIONNA_ENABLE_LLVM_TESTS}>
)

set_tests_properties(test_bridge_module
    PROPERTIES
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR}:$ENV{PYTHONPATH}"
)
