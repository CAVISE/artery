#include <artery/sionna/bridge/Defaulted.h>
#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Helpers.h>
#include <nanobind/nanobind.h>

#include <memory>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

NAMESPACE_BEGIN(py)

namespace nb = nanobind;

template <typename ModTag>
class Module : std::enable_shared_from_this<Module<ModTag>>
{
public:
    static constexpr const char* name() noexcept { return ModTag::name(); }

    template <typename FnTag, typename... Args>
    nanobind::object function(Args&&... args) const
    {
        return call(FnTag::name(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    nb::object call(const char* func, Args&&... args) const
    {
        nb::gil_scoped_acquire gil;

        nb::object f = module().attr(func);
        nb::dict kw = kwargs(std::forward<Args>(args)...);

        try {
            return f(**kw);
        } catch (const nb::python_error& err) {
            throw wrapRuntimeError("failed to invoke \"%s\" method on \"%s\" module, err: %s", func, ModTag::name(), err.what());
        }
    }

    nb::module_ module() const
    {
        if (!mod_) {
            mod_ = std::make_unique<nb::module_>(nb::module_::import_(name()));
        }
        return *mod_;
    }

protected:
    mutable std::unique_ptr<nb::module_> mod_ = nullptr;
};

template <typename ModTag, typename ClassTag>
class Class
{
public:
    using OwnerModule = Module<ModTag>;

    static constexpr const char* name() noexcept { return ClassTag::name(); }

    explicit Class(std::shared_ptr<const Module<ModTag>> owner) : owner_(owner), bound_(std::make_unique<nb::object>(ctor())) {}

    nb::object type() const
    {
        if (!type_) {
            nb::gil_scoped_acquire gil;
            type_ = std::make_unique<nb::object>(owner_->module().attr(name()));
        }
        return *type_;
    }

    template <typename ReturnT>
    ReturnT attr(const char* name) const
    {
        nb::gil_scoped_acquire gil;
        return access<ReturnT>(type(), name);
    }

    template <typename... Args>
    nb::object ctor(Args&&... args) const
    {
        nb::gil_scoped_acquire gil;
        nb::dict kw = kwargs(std::forward<Args>(args)...);
        try {
            return type()(**kw);
        } catch (const nb::python_error& err) {
            throw wrapRuntimeError("failed to construct \"%s\" from \"%s\": %s", name(), owner_->name(), err.what());
        }
    }

    template <typename FnTag, typename ReturnT, typename... Args>
    ReturnT method(Args&&... args) const
    {
        return call_method<ReturnT>(FnTag::name(), std::forward<Args>(args)...);
    }

    template <typename ReturnT, typename... Args>
    ReturnT call_method(const char* method, Args&&... args) const
    {
        nb::gil_scoped_acquire gil;

        nb::dict kw = kwargs(std::forward<Args>(args)...);
        try {
            nb::object m = type().attr(method);
            return m(**kw);
        } catch (const nb::python_error& err) {
            throw wrapRuntimeError("failed to call \"%s.%s\" in \"%s\": %s", name(), owner_->name(), err.what());
        }
    }

protected:
    std::unique_ptr<nb::object> type_ = nullptr;
    std::unique_ptr<nb::object> bound_ = nullptr;
    std::shared_ptr<const Module<ModTag>> owner_ = nullptr;
};


// Convenience macro for module declaration.
#define PY_MODULE(NAME, PYNAME) struct NAME : ::artery::sionna::py::ModuleCRTP<PYNAME>

// Convenience macro for tag declaration.
#define PY_IDENTITY_TAG(NAME) \
    struct NAME##Tag { \
        static constexpr const char* name() { return #NAME; } \
    };


NAMESPACE_END(py)

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
