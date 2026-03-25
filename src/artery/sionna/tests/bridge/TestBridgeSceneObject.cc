#include <artery/sionna/bridge/SionnaBridge.h>

#include <artery/sionna/tests/bridge/MitsubaFixture.h>
#include <gtest/gtest.h>

#include <limits>
#include <filesystem>

namespace {

    class SceneObjectLLVMFixture
        : public artery::sionna::tests::MitsubaLLVMFixture {
    public:
        const double eps_ = std::numeric_limits<float>::epsilon();
    };

}

TEST_F(SceneObjectLLVMFixture, WrapAndAccessProperties) {
    nanobind::gil_scoped_acquire gil;

    RadioMaterial material("test_mat", /* conductivity = */2.5, /* relativePermittivity = */3.5, /* thickness = */0.2);

    auto sample = std::filesystem::current_path() / "sample.ply";
    if (!std::filesystem::exists(sample)) {
        GTEST_FAIL() << "current working directory should be set to CMAKE_CURRENT_SOURCE_DIR so test may resolve all data files!";
    }

    SceneObject sceneObject (sample.string(), "some_obj", material);

    auto pos = sceneObject.position();
    EXPECT_NEAR(artery::sionna::toScalar<double>(pos.x()), 0.5, eps_);
    EXPECT_NEAR(artery::sionna::toScalar<double>(pos.y()), 0.5, eps_);
    EXPECT_NEAR(artery::sionna::toScalar<double>(pos.z()), 0.0, eps_);

    auto orient = sceneObject.orientation();
    EXPECT_NEAR(artery::sionna::toScalar<double>(orient.x()), 0.0, eps_);
    EXPECT_NEAR(artery::sionna::toScalar<double>(orient.y()), 0.0, eps_);
    EXPECT_NEAR(artery::sionna::toScalar<double>(orient.z()), 0.0, eps_);

    auto mat = sceneObject.material();
    EXPECT_EQ(mat.materialName(), "test_mat");
}
