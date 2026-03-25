#include <gtest/gtest.h>

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Compat.h>

#include <drjit-core/jit.h>
#include <drjit/array.h>
#include <drjit/jit.h>

namespace dr = drjit;

namespace {

    class LLVMCompatTest
        : public ::testing::Test {
    public:
        using Float = drjit::LLVMArray<float>;
        using Spectrum = mitsuba::Color<Float, 3>;

        SIONNA_BRIDGE_IMPORT_BRIDGE_TYPES()

        void SetUp() override {
            #if defined(MI_ENABLE_LLVM) && SIONNA_ENABLE_LLVM_TESTS
                jit_init((uint32_t) JitBackend::LLVM);
            #else
                GTEST_SKIP() << "LLVM tests disabled or LLVM backend not enabled.";
            #endif

            if (jit_has_backend(JitBackend::LLVM) == 0) {
                GTEST_FAIL() << "LLVM backend is not available!";
            }
        }

        void TearDown() override {
            #if defined(MI_ENABLE_LLVM) && SIONNA_ENABLE_LLVM_TESTS
                jit_shutdown(1);
            #endif
        }
    };

    struct DummyValue {
        float value;
    };

    class ScalarCompatTest
        : public ::testing::Test {
    public:
        using Float = float;
        using Spectrum = mitsuba::Color<Float, 3>;

        SIONNA_BRIDGE_IMPORT_BRIDGE_TYPES()
    };

}

namespace artery::sionna {

    template <typename Float, typename Spectrum>
    struct CompatImpl<Float, Spectrum, int, DummyValue, void> {
        static int convert(const DummyValue& input) {
            return toScalar<int>(input.value);
        }
    };

}

TEST_F(ScalarCompatTest, ToScalarExtractsScalar) {
    const double scalar = artery::sionna::toScalar<double>(Float(1.0));
    EXPECT_DOUBLE_EQ(scalar, 1.0);
}

TEST_F(ScalarCompatTest, FromScalarProducesScalar) {
    const Float produced = artery::sionna::fromScalar<Float>(1.0F);
    EXPECT_FLOAT_EQ(artery::sionna::toScalar<float>(produced), 1.0F);
}

TEST_F(LLVMCompatTest, ToScalarExtractsFirstLane) {
    const Float values = drjit::arange<Float>(4);
    const double scalar = artery::sionna::toScalar(values);
    EXPECT_DOUBLE_EQ(scalar, 0.0);
}

TEST_F(LLVMCompatTest, FromScalarProducesLLVMArray) {
    const Float produced = artery::sionna::fromScalar<Float>(4.5);
    drjit::eval(produced);

    ASSERT_EQ(drjit::width(produced), 1);
    EXPECT_FLOAT_EQ(artery::sionna::toScalar<float>(produced), 4.5F);
}
