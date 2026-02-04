#include <artery/sionna/bridge/Helpers.h>
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    artery::sionna::ScopedInterpreter guard(SIONNA_VENV_HINT);
    return RUN_ALL_TESTS();
}
