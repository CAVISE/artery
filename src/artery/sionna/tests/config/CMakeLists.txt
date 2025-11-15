# Compatibility test for current Mitsuba distribution. If this compiles and runs,
# that means that nanobind object registry was shared and Sionna bridge module can operate.

add_executable(test_config test_two_way_cast.cpp)

target_link_libraries(test_config PRIVATE
    pybind11::embed
    Mitsuba::mi
    GTest::gtest_main
)

add_test(NAME test_cmake_configuration COMMAND test_config)
