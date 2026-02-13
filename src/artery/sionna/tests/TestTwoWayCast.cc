#include <gtest/gtest.h>
#include <nanobind/nanobind.h>

#include <mitsuba/core/fwd.h>
#include <mitsuba/core/ray.h>
#include <mitsuba/core/config.h>

#include <artery/sionna/bridge/Helpers.h>

namespace nb = nanobind;

TEST(PythonEmbedTest, CastMitsubaRay) {
    using Float = float;
    using Spectrum = mitsuba::Color<Float, 3>;

    using namespace mitsuba;
    MI_IMPORT_CORE_TYPES();

    auto mi = nb::module_::import_("mitsuba");
    mi.attr("set_variant")("scalar_rgb");

    nb::handle pyType = nb::type<Vector3f>();
    ASSERT_TRUE(pyType.ptr() != nullptr)
        << "WARNING: Mitsuba did not register Vector3f. Probably bindings were not loaded?";

    std::string pyTypeName = nb::type_info(pyType).name();
    std::size_t pyTypeSize = nb::type_size(pyType);
    std::size_t pyTypeAlign = nb::type_align(pyType);

    ASSERT_EQ(pyTypeSize, sizeof(Vector3f))
        << "WARNING: nanobind was registered with type of different size. In this "
        << "translation unit Vector3f was compiled to "
        << sizeof(Vector3f) << "bytes, "
        << "but instead size should have been "
        << pyTypeSize << "bytes. Inspect compilation flags.";

    ASSERT_EQ(pyTypeAlign, alignof(Vector3f))
        << "WARNING: nanobind was registered with type of different alignment. In this "
        << "translation unit Vector3f was padded with "
        << alignof(Vector3f) << "bytes, "
        << "but instead size should have been "
        << pyTypeAlign << "bytes. Inspect compilation flags.";

    ASSERT_EQ(pyTypeName, typeid(mitsuba::CoreAliases<Float>::Vector3f).name())
        << "Mangled names should match.";

    auto ray3f = mi.attr("Ray3f");

    Point3f origin (0.1f, 0.2f, 0.3f);
    Vector3f destination (1.0f, 0.0f, 0.0f);

    // These args should be converted into python objects.
    nb::object pyRay = ray3f(nb::cast(origin), nb::cast(destination));

    // Same as above, but the other way around.
    auto cppRay = nb::cast<mitsuba::Ray<mitsuba::CoreAliases<Float>::Point3f, Spectrum>>(pyRay);

    EXPECT_FLOAT_EQ(cppRay.o.x(), 0.1f);
    EXPECT_FLOAT_EQ(cppRay.o.y(), 0.2f);
    EXPECT_FLOAT_EQ(cppRay.o.z(), 0.3f);

    EXPECT_FLOAT_EQ(cppRay.d.x(), 1.0f);
    EXPECT_FLOAT_EQ(cppRay.d.y(), 0.0f);
    EXPECT_FLOAT_EQ(cppRay.d.z(), 0.0f);
}
