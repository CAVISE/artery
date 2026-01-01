#pragma once

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
class Module : public std::enable_shared_from_this<Module<ModTag>>
{
public:
    static constexpr const char* name() noexcept { return ModTag::name(); }

    template <typename FnTag, typename... Args>
    nanobind::object function(Args&&... args) const
    {
        return sionna::call(module(), FnTag::name(), std::forward<Args>(args)...);
    }

    nb::module_ module() const
    {
        if (!mod_) {
            nb::gil_scoped_acquire gil;
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
        return access<ReturnT>(type(), name);
    }

    template <typename... Args>
    nb::object ctor(Args&&... args) const
    {
        return sionna::call<nb::object>(owner_->module(), name(), std::forward<Args>(args)...);
    }

    template <typename FnTag, typename ReturnT, typename... Args>
    ReturnT method(Args&&... args) const
    {
        return sionna::call<ReturnT>(*bound_, FnTag::name(), std::forward<Args>(args)...);
    }

protected:
    std::unique_ptr<nb::object> type_ = nullptr;
    std::unique_ptr<nb::object> bound_ = nullptr;
    std::shared_ptr<const Module<ModTag>> owner_ = nullptr;
};


// Convenience macro for module declaration.
#define PY_MODULE(NAME, PYNAME) struct NAME : public ::artery::sionna::py::Module<PYNAME>

#define PY_BASE_IMPL(...) public __VA_ARGS__

// Pass bases.
#define PY_BASE(T) PY_BASE_IMPL T

// Convenience macro for class declaration (with optional extra bases/mixins).
// Extra bases must be default-constructible.
#define PY_CLASS(NAME, PYNAME, MODULE, ...)                     \
    struct NAME                                                 \
        : public ::artery::sionna::py::Class<MODULE, PYNAME>    \
        __VA_OPT__(, __VA_ARGS__)


// Convenience macro for tag declaration.
#define PY_IDENTITY_TAG(TAG, VALUE) \
    struct TAG##Tag { \
        static constexpr const char* name() { return #VALUE; } \
    }

NAMESPACE_END(py)

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
