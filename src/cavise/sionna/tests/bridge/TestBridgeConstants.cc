#include <cavise/sionna/bridge/Compat.h>
#include <cavise/sionna/bridge/bindings/Constants.h>

#include <cavise/sionna/tests/bridge/MitsubaFixture.h>

#include <gtest/gtest.h>

#include <cmath>
#include <memory>

using namespace artery::sionna;

namespace {

    class ConstantsLLVMFixture
        : public artery::sionna::tests::MitsubaLLVMFixture {
    public:
        void SetUp() override {
            MitsubaLLVMFixture::SetUp();
            nanobind::gil_scoped_acquire gil;
            module_ = std::make_unique<nanobind::module_>(nanobind::module_::import_("sionna.rt.constants"));
        }

        std::unique_ptr<nanobind::module_> module_;
    };

} // namespace

TEST_F(ConstantsLLVMFixture, DefaultThicknessViaBridgeResolves) {
    const mitsuba::Resolve::Float64 value = py::Constants::defaultThickness.value();
    EXPECT_TRUE(std::isfinite(toScalar<double>(value)));
}

TEST_F(ConstantsLLVMFixture, InteractionTypesViaBridgeResolve) {
    EXPECT_NO_FATAL_FAILURE(toScalar<int>(py::IntersectionTypes::none.value()));
    EXPECT_NO_FATAL_FAILURE(toScalar<int>(py::IntersectionTypes::specular.value()));
    EXPECT_NO_FATAL_FAILURE(toScalar<int>(py::IntersectionTypes::diffuse.value()));
    EXPECT_NO_FATAL_FAILURE(toScalar<int>(py::IntersectionTypes::refraction.value()));
    EXPECT_NO_FATAL_FAILURE(toScalar<int>(py::IntersectionTypes::diffraction.value()));
}
