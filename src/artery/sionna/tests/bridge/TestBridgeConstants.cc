#include <artery/sionna/bridge/Compat.h>
#include <artery/sionna/bridge/bindings/Constants.h>

#include <artery/sionna/tests/bridge/MitsubaFixture.h>

#include <cmath>
#include <memory>

namespace {

    class ConstantsLLVMFixture
        : public artery::sionna::tests::MitsubaLLVMFixture {
    public:
        void SetUp() override {
            MitsubaLLVMFixture::SetUp();
            nanobind::gil_scoped_acquire gil;
            module_ = std::make_unique<nanobind::module_> (nanobind::module_::import_("sionna.rt.constants"));
        }

        std::unique_ptr<nanobind::module_> module_;
    };

}

TEST_F(ConstantsLLVMFixture, DefaultThicknessViaBridgeResolves) {
    const Float64 value = Constants::defaultThickness.value();
    const double scalar = Compat::template toScalar<double>(value);
    EXPECT_TRUE(std::isfinite(scalar));
    EXPECT_GT(scalar, 0.0);
}

TEST_F(ConstantsLLVMFixture, InteractionTypesViaBridgeResolve) {
    Compat::template toScalar<int>(IntersectionTypes::none.value());
    Compat::template toScalar<int>(IntersectionTypes::specular.value());
    Compat::template toScalar<int>(IntersectionTypes::diffuse.value());
    Compat::template toScalar<int>(IntersectionTypes::refraction.value());
    Compat::template toScalar<int>(IntersectionTypes::diffraction.value());
}
