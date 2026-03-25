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
    const double scalar = artery::sionna::toScalar<double>(value);
    EXPECT_TRUE(std::isfinite(scalar));
    EXPECT_GT(scalar, 0.0);
}

TEST_F(ConstantsLLVMFixture, InteractionTypesViaBridgeResolve) {
    artery::sionna::toScalar<int>(IntersectionTypes::none.value());
    artery::sionna::toScalar<int>(IntersectionTypes::specular.value());
    artery::sionna::toScalar<int>(IntersectionTypes::diffuse.value());
    artery::sionna::toScalar<int>(IntersectionTypes::refraction.value());
    artery::sionna::toScalar<int>(IntersectionTypes::diffraction.value());
}
