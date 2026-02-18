#include <artery/sionna/bridge/Compat.h>
#include <artery/sionna/bridge/bindings/Material.h>
#include <artery/sionna/bridge/Defaulted.h>

#include <artery/sionna/tests/bridge/MitsubaFixture.h>

#include <cmath>

namespace {

    class MaterialLLVMFixture
        : public artery::sionna::tests::MitsubaLLVMFixture {
    };

}

TEST_F(MaterialLLVMFixture, ConstructAndAccessProperties) {
    using Mat = artery::sionna::py::RadioMaterial<Float, Spectrum>;

    Mat material("test_mat", /*conductivity=*/2.5, /*relativePermittivity=*/3.5, /*thickness=*/0.2);

    EXPECT_EQ(material.materialName(), "test_mat");

    const double thickness = artery::sionna::py::Compat<Float, Spectrum>::template toScalar<double>(material.thickness());
    const double conductivity = artery::sionna::py::Compat<Float, Spectrum>::template toScalar<double>(material.conductivity());
    const double rel_perm = artery::sionna::py::Compat<Float, Spectrum>::template toScalar<double>(material.relativePermittivity());

    EXPECT_NEAR(thickness, 0.2, 1e-12);
    EXPECT_NEAR(conductivity, 2.5, 1e-12);
    EXPECT_NEAR(rel_perm, 3.5, 1e-12);
}

TEST_F(MaterialLLVMFixture, ColorRoundTrip) {
    using Mat = artery::sionna::py::RadioMaterial<Float, Spectrum>;

    Mat material("colored");
    material.setColor({ Float(0.1), Float(0.2), Float(0.3) });

    auto color = material.color();
    const double r = artery::sionna::py::Compat<Float, Spectrum>::template toScalar<double>(std::get<0>(color));
    const double g = artery::sionna::py::Compat<Float, Spectrum>::template toScalar<double>(std::get<1>(color));
    const double b = artery::sionna::py::Compat<Float, Spectrum>::template toScalar<double>(std::get<2>(color));

    EXPECT_NEAR(r, 0.1, 1e-12);
    EXPECT_NEAR(g, 0.2, 1e-12);
    EXPECT_NEAR(b, 0.3, 1e-12);
}

TEST_F(MaterialLLVMFixture, WrapExistingPythonObject) {
    nanobind::gil_scoped_acquire gil;
    using namespace artery::sionna::literals;
    auto rt = nanobind::module_::import_("sionna.rt");
    auto py_obj = rt.attr("RadioMaterial")("wrapped", "thickness"_a = 0.4);

    artery::sionna::py::RadioMaterial<Float, Spectrum> material(py_obj);

    EXPECT_EQ(material.materialName(), "wrapped");
    const double thickness = artery::sionna::py::Compat<Float, Spectrum>::template toScalar<double>(material.thickness());
    EXPECT_NEAR(thickness, 0.4, 1e-12);
}
