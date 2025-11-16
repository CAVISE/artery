#include <gtest/gtest.h>
#include <pybind11/embed.h>
#include <nanobind/nanobind.h>

#include <mitsuba/core/fwd.h>
#include <mitsuba/core/ray.h>
#include <mitsuba/core/config.h>

namespace py = pybind11;
namespace nb = nanobind;


TEST(PythonEmbedTest, CastMitsubaRay) {
    py::scoped_interpreter s_python_guard;

    // 3. Cast Python Ray → C++ Ray
    using Float = float;
    using Spectrum = mitsuba::Color<Float, 3>;

    using namespace mitsuba;
    MI_IMPORT_CORE_TYPES();

    // 1. Import Mitsuba and set variant
    auto mi = nb::module_::import_("mitsuba");
    mi.attr("set_variant")("scalar_rgb");

    // 2. Create a Ray3f in Python
    auto ray3f = mi.attr("Ray3f");

    nb::object py_ray = ray3f(
        nb::make_tuple(0.1f, 0.2f, 0.3f),
        nb::make_tuple(1.0f, 0.0f, 0.0f)
    );

    auto ray_cpp = nb::cast<mitsuba::Ray<Point3f, Spectrum>>(py_ray);

    // 4. Validate origin/direction
    EXPECT_FLOAT_EQ(ray_cpp.o.x(), 0.1f);
    EXPECT_FLOAT_EQ(ray_cpp.o.y(), 0.2f);
    EXPECT_FLOAT_EQ(ray_cpp.o.z(), 0.3f);

    EXPECT_FLOAT_EQ(ray_cpp.d.x(), 0.0f);
    EXPECT_FLOAT_EQ(ray_cpp.d.y(), 1.0f);
    EXPECT_FLOAT_EQ(ray_cpp.d.z(), 0.0f);
}
