#include <cavise/sionna/bridge/Compat.h>
#include <cavise/sionna/bridge/bindings/Constants.h>

#include <cavise/sionna/tests/bridge/MitsubaFixture.h>

#include <gtest/gtest.h>

#include <cmath>
#include <memory>
#include <tuple>

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
    using TInteraction = py::InteractionTypes;

    // clang-format off
    for (const auto& var : {
             TInteraction::none,
             TInteraction::specular,
             TInteraction::diffuse,
             TInteraction::refraction,
             TInteraction::diffraction
        }
    ) {
        EXPECT_NO_FATAL_FAILURE(toScalar<int>(var.value()));
    }
    // clang-format on
}

TEST_F(ConstantsLLVMFixture, InteractionTypeColorsCanBeChanged) {
    py::InteractionTypeColors colors;
    const auto original = colors.color(py::InteractionTypes::specular);
    const auto originalLos = colors.color(py::InteractionTypes::none);

    colors.setColor(py::InteractionTypes::specular, {0.1f, 0.2f, 0.3f});
    const auto changed = colors.color(py::InteractionTypes::specular);

    EXPECT_FLOAT_EQ(std::get<0>(changed), 0.1f);
    EXPECT_FLOAT_EQ(std::get<1>(changed), 0.2f);
    EXPECT_FLOAT_EQ(std::get<2>(changed), 0.3f);

    colors.setColor(py::InteractionTypes::none, {0.4f, 0.5f, 0.6f});
    const auto changedLos = colors.color(py::InteractionTypes::none);

    EXPECT_FLOAT_EQ(std::get<0>(changedLos), 0.4f);
    EXPECT_FLOAT_EQ(std::get<1>(changedLos), 0.5f);
    EXPECT_FLOAT_EQ(std::get<2>(changedLos), 0.6f);

    colors.setColor(py::InteractionTypes::specular, original);
    colors.setColor(py::InteractionTypes::none, originalLos);
}
