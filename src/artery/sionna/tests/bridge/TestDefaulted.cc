#include <artery/sionna/bridge/Defaulted.h>
#include <artery/sionna/bridge/Helpers.h>
#include <gtest/gtest.h>
#include <nanobind/nanobind.h>

namespace nb = nanobind;

static nb::object make_kw_echo()
{
    nb::dict g;
    g["__builtins__"] = nb::module_::import_("builtins");

    nb::module_::import_("builtins")
        .attr("exec")(
            nb::str(R"PY(
def kw_echo(**kwargs):
    return kwargs
)PY"),
            g, g);

    return g["kw_echo"];
}

TEST(KwargsTest, PassingRegularArgsWorks)
{
    using namespace artery::sionna::literals;

    nb::object kw_echo = make_kw_echo();

    auto kw = kwargs("a"_a = 1, "b"_a = 2.5, "s"_a = nb::str("hello"));

    nb::object out = kw_echo(**kw);
    nb::dict d = nb::cast<nb::dict>(out);

    EXPECT_EQ(nb::cast<int>(d["a"]), 1);
    EXPECT_DOUBLE_EQ(nb::cast<double>(d["b"]), 2.5);
    // EXPECT_EQ(nb::cast<std::string>(d["s"]), "hello");
}

TEST(KwargsTest, ResolveWorks)
{
    using D = artery::sionna::Defaulted<int>;

    nb::module_ main = nb::module_::import_("__main__");
    nb::setattr(main, "MYCONST", nb::int_(123));

    D c{"__main__", "MYCONST"};

    D::Argument a_user = 7;
    EXPECT_EQ(D::resolve(a_user), 7);

    D::Argument a_def = std::cref(c);
    EXPECT_EQ(D::resolve(a_def), 123);
}

TEST(KwargsTest, DefaultedArgsAreNotPassed)
{
    using namespace artery::sionna::literals;
    using D = artery::sionna::Defaulted<int>;

    nb::object kw_echo = make_kw_echo();

    D def{"__main__", "SOME_DEFAULT_NAME"};

    {
        auto kw = kwargs("x"_a = def, "y"_a = 42);

        nb::dict d = nb::cast<nb::dict>(kw_echo(**kw));

        bool has_x = nb::cast<bool>(d.attr("__contains__")("x"));
        bool has_y = nb::cast<bool>(d.attr("__contains__")("y"));

        EXPECT_FALSE(has_x);
        EXPECT_TRUE(has_y);
        EXPECT_EQ(nb::cast<int>(d["y"]), 42);
    }

    {
        D::Argument x = 99;

        auto kw = kwargs("x"_a = x);

        nb::dict d = nb::cast<nb::dict>(kw_echo(**kw));

        bool has_x = nb::cast<bool>(d.attr("__contains__")("x"));
        EXPECT_TRUE(has_x);
        EXPECT_EQ(nb::cast<int>(d["x"]), 99);
    }
}
