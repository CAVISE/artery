#pragma once

#include <cavise/sionna/bridge/Fwd.h>

#include <gtest/gtest.h>

#include <drjit/array.h>
#include <drjit/autodiff.h>
#include <drjit-core/jit.h>

#include <mitsuba/core/fwd.h>
#include <mitsuba/mitsuba.h>

#include <nanobind/nanobind.h>

namespace artery::sionna::tests {

    class MitsubaLLVMFixture : public ::testing::Test {
    public:
        void SetUp() override {
            #if defined(MI_ENABLE_LLVM) && SIONNA_ENABLE_LLVM_TESTS
                jit_init((uint32_t)JitBackend::LLVM);
            #else
                GTEST_SKIP() << "LLVM tests disabled or LLVM backend not enabled.";
            #endif

            if (jit_has_backend(JitBackend::LLVM) == 0) {
                GTEST_FAIL() << "LLVM backend is not available!";
            }

            nanobind::gil_scoped_acquire gil;
            auto mi = nanobind::module_::import_("mitsuba");
            mi.attr("set_variant")("llvm_ad_rgb");
        }

        void TearDown() override {
            #if defined(MI_ENABLE_LLVM) && SIONNA_ENABLE_LLVM_TESTS
                jit_shutdown(1);
            #endif
        }
    };

} // namespace artery::sionna::tests
