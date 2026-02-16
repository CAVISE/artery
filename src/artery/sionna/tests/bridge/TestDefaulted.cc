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
    void SetUp() override {
        // For some reason WORKING_DIRECTORY is not enough, we have to append to system path
        // manually.
        auto sys = nb::module_::import_("sys");
        auto cwd = std::filesystem::current_path().string();
        sys.attr("path").attr("insert")(0, cwd);

        mod_ = std::make_unique<nb::module_>(nb::module_::import_("mod"));
        echo_ = std::make_unique<nb::object>(mod_->attr("echo_kwargs"));
    }

    bool dictContains(nb::dict d, const std::string& attribute) {
        return nb::cast<bool>(d.attr("__contains__")(attribute.c_str()));
    }

    template<typename T>
    T dictFetch(nb::dict d, const std::string& attribute) {
        return nb::cast<T>(d[attribute.c_str()]);
    }

    // Handles do not have defaults - so have to allocate them.
    std::unique_ptr<nb::object> echo_;
    std::unique_ptr<nb::module_> mod_;
};

}  // namespace

TEST_F(DefaultedTest, PassingRegularArgsWorks)
{
    using namespace artery::sionna::literals;
    using artery::sionna::kwargs;

    auto kw = kwargs("integer"_a = 1, "double"_a = 2.5, "string"_a = nb::str("hello"));
    nb::dict d = nb::cast<nb::dict>((*echo_)(**kw));

    ASSERT_TRUE(dictContains(d, "integer"));
    ASSERT_TRUE(dictContains(d, "double"));
    ASSERT_TRUE(dictContains(d, "string"));

    EXPECT_EQ(dictFetch<int>(d, "integer"), 1);
    EXPECT_DOUBLE_EQ(dictFetch<double>(d, "double"), 2.5);
    EXPECT_EQ(dictFetch<std::string>(d, "string"), "hello");
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
    using artery::sionna::kwargs;
    using D = artery::sionna::Defaulted<int>;

    D defaulted{"mod", "GLOBAL_CONST"};

    {
        // Make sure kwargs() resolves Defaulted type - if passed
        // to caster, this will break nanobind core library (nanobind-drjit).
        auto kw = kwargs("x"_a = defaulted, "y"_a = 42);

        nb::dict d = nb::cast<nb::dict>((*echo_)(**kw));

        // Default arguments are left out intentionally.
        ASSERT_FALSE(dictContains(d, "x"));
        ASSERT_TRUE(dictContains(d, "y"));

        EXPECT_EQ(dictFetch<int>(d, "y"), 42);
    }

    {
        D::Argument x = 99;

        auto kw = kwargs("x"_a = x);
        nb::dict d = nb::cast<nb::dict>((*echo_)(**kw));

        ASSERT_TRUE(dictContains(d, "x"));
        EXPECT_EQ(dictFetch<int>(d, "x"), 99);
    }
}
