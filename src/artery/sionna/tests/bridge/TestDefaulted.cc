#include <artery/sionna/bridge/Defaulted.h>
#include <artery/sionna/bridge/Helpers.h>
#include <gtest/gtest.h>
#include <nanobind/nanobind.h>

// For casters
#include <nanobind/stl/string.h>

#include <memory>

namespace nb = nanobind;

namespace
{

class DefaultedTest : public testing::Test
{
public:
    void SetUp() override
    {
        auto sys = nb::module_::import_("sys");
        auto cwd = std::filesystem::current_path().string();
        sys.attr("path").attr("insert")(0, cwd);

        mod_ = std::make_unique<nb::module_>(nb::module_::import_("mod"));
        echo_ = std::make_unique<nb::object>(mod_->attr("echo_kwargs"));
    }

    std::unique_ptr<nb::object> echo_;
    std::unique_ptr<nb::module_> mod_;
};

}  // namespace

TEST_F(DefaultedTest, PassingRegularArgsWorks)
{
    using namespace artery::sionna::literals;

    auto kw = kwargs("a"_a = 1, "b"_a = 2.5, "s"_a = nb::str("hello"));
    nb::dict d = nb::cast<nb::dict>((*echo_)(**kw));

    EXPECT_EQ(nb::cast<int>(d["a"]), 1);
    EXPECT_DOUBLE_EQ(nb::cast<double>(d["b"]), 2.5);
    EXPECT_EQ(nb::cast<std::string>(d["s"]), "hello");
}

TEST_F(DefaultedTest, ResolveWorks)
{
    using D = artery::sionna::Defaulted<int>;

    D global{"mod", "GLOBAL_CONST"};
    D belongsToCls{"mod", "CONST", "Constants"};

    D::Argument arg = 7;
    EXPECT_EQ(D::resolve(arg), 7);

    D::Argument defaulted = global;
    EXPECT_EQ(D::resolve(defaulted), 1);

    defaulted = belongsToCls;
    EXPECT_EQ(D::resolve(defaulted), 2);
}

TEST_F(DefaultedTest, DefaultedArgsAreNotPassed)
{
    using namespace artery::sionna::literals;
    using D = artery::sionna::Defaulted<int>;

    D defaulted{"mod", "GLOBAL_CONST"};

    {
        auto kw = kwargs("x"_a = defaulted, "y"_a = 42);

        nb::dict d = nb::cast<nb::dict>((*echo_)(**kw));

        bool hasX = nb::cast<bool>(d.attr("__contains__")("x"));
        bool hasY = nb::cast<bool>(d.attr("__contains__")("y"));

        EXPECT_FALSE(hasX);
        EXPECT_TRUE(hasY);

        EXPECT_EQ(nb::cast<int>(d["y"]), 42);
    }

    {
        D::Argument x = 99;

        auto kw = kwargs("x"_a = x);
        nb::dict d = nb::cast<nb::dict>((*echo_)(**kw));

        bool hasX = nb::cast<bool>(d.attr("__contains__")("x"));
        EXPECT_TRUE(hasX);

        EXPECT_EQ(nb::cast<int>(d["x"]), 99);
    }
}
