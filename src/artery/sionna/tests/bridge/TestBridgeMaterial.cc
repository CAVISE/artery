#include <artery/sionna/bridge/SionnaBridge.h>

#include <artery/sionna/tests/bridge/MitsubaFixture.h>
#include <gtest/gtest.h>

#include <limits>

namespace {

    class MaterialLLVMFixture
        : public artery::sionna::tests::MitsubaLLVMFixture {
    public:
        const double eps_ = std::numeric_limits<float>::epsilon();
    };

}

TEST_F(MaterialLLVMFixture, ConstructAndAccessProperties) {
    RadioMaterial material("test_mat", /* conductivity = */2.5, /* relativePermittivity = */3.5, /* thickness = */0.2);

    EXPECT_EQ(material.materialName(), "test_mat");

    const double thickness = artery::sionna::toScalar<double>(material.thickness());
    const double conductivity = artery::sionna::toScalar<double>(material.conductivity());
    const double permittivity = artery::sionna::toScalar<double>(material.relativePermittivity());

    EXPECT_NEAR(thickness, 0.2, eps_);
    EXPECT_NEAR(conductivity, 2.5, eps_);
    EXPECT_NEAR(permittivity, 3.5, eps_);

    material.setThickness(artery::sionna::fromScalar<Float>(0.1));
    material.setConductivity(artery::sionna::fromScalar<Float>(0.3));
    material.setRelativePermittivity(artery::sionna::fromScalar<Float>(1.5));

    EXPECT_NEAR(artery::sionna::toScalar<double>(material.thickness()), 0.1, eps_);
    EXPECT_NEAR(artery::sionna::toScalar<double>(material.conductivity()), 0.3, eps_);
    EXPECT_NEAR(artery::sionna::toScalar<double>(material.relativePermittivity()), 1.5, eps_);
}

TEST_F(MaterialLLVMFixture, ColorRoundTrip) {
    using Mat = artery::sionna::py::RadioMaterial<Float, Spectrum>;

    Mat material("colored");
    material.setColor({0.1, 0.2, 0.3});

    auto color = material.color();
    const double r = artery::sionna::toScalar<double>(std::get<0>(color));
    const double g = artery::sionna::toScalar<double>(std::get<1>(color));
    const double b = artery::sionna::toScalar<double>(std::get<2>(color));

    EXPECT_NEAR(r, 0.1, eps_);
    EXPECT_NEAR(g, 0.2, eps_);
    EXPECT_NEAR(b, 0.3, eps_);
}
