#pragma once

#include <artery/sionna/bridge/Defaulted.h>
#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Helpers.h>
#include <nanobind/nanobind.h>

#include <memory>
#include <type_traits>
#include <utility>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

NAMESPACE_BEGIN(py)

namespace nb = nanobind;

using literals::operator""_a;

/**
 * @file Python to C++ bindings rely on capabilities, which are assigned to each
 * bound class, specifying what that class can do. Most capabilities have a base form, which
 * may be overridden to expand the functionality.
 */

class IPythonCapability {
public:
    virtual ~IPythonCapability() = default;
};

/**
 * @brief Capability to identify a module. Override with qualified path to
 * module, that gets implemented.
 */
class IPythonModuleIdentityCapability
    : public virtual IPythonCapability {
public:
    ~IPythonModuleIdentityCapability() override = default;
    virtual const char* moduleName() const = 0;
};

/**
 * @brief Capability to identify a class. Inherits from module identity capability,
 * since every class belongs to a module.
 */
class IPythonClassIdentityCapability
    : public virtual IPythonModuleIdentityCapability {
public:
    ~IPythonClassIdentityCapability() override = default;
    virtual const char* className() const = 0;
};

/**********************
 * Basic capabilities *
 **********************
 */

template <typename T>
struct PyDeleter {
    void operator()(T* p) const noexcept {
        nb::gil_scoped_acquire gil;
        delete p;
    }
};

template <typename T>
using OwnedPy = std::unique_ptr<T, PyDeleter<T>>;

/**
 * @brief Capability to call arbitrary methods in python.
 */
class CallAnyCapability
    : public virtual IPythonCapability {
public:
    template <typename ReturnType = nb::object, typename... Args>
    ReturnType callAny(nb::handle target, const char* name, Args&&... args) const {
        return sionna::call<ReturnType>(target, name, std::forward<Args>(args)...);
    }
};

/**
 * @brief Basic stateless python module import capability.
 */
class BasePythonImportCapability
    : public virtual IPythonModuleIdentityCapability {
public:
    /**
     * @brief Stateless module import.
     */
    virtual nb::module_ module() const {
        nb::gil_scoped_acquire gil;
        return nb::module_::import_(moduleName());
    }
};

/**
 * @brief Cached access to a module. Stores module upon first import, which
 * allows effective multiple access to it.
 */
class CachedImportCapability
    : public BasePythonImportCapability {
public:
    /**
     * @brief Stateful (cached) module import.
     */
    nb::module_ module() const override {
        if (!module_) {
            nb::gil_scoped_acquire gil;
            module_ = OwnedPy<nb::module_>(new nb::module_(BasePythonImportCapability::module()));
        }
        return nb::borrow<nb::module_>(*module_);
    }

protected:
    mutable OwnedPy<nb::module_> module_;
};

/**
 * @brief Basic class (type) stateless object access.
 */
class BasePythonFetchCapability
    : public virtual IPythonClassIdentityCapability
    , public BasePythonImportCapability {
public:
    /**
     * @brief Get type object from module (stateless).
     */
    virtual nb::object type() const {
        nb::gil_scoped_acquire gil;
        return sionna::access<nb::object>(module(), className());
    }
};

/**
 * @brief Class (type) stateful object access.
 */
class CachedFetchCapability
    : public BasePythonFetchCapability {
public:
    /**
     * @brief Get type object from module (stateful).
     */
    nb::object type() const override {
        if (!type_) {
            nb::gil_scoped_acquire gil;
            type_ = OwnedPy<nb::object>(new nb::object(BasePythonFetchCapability::type()));
        }
        return nb::borrow<nb::object>(*type_);
    }

protected:
    mutable OwnedPy<nb::object> type_;
};

/**
 * @brief Capability to initialize an object.
 */
class InitPythonClassCapability
    : public BasePythonFetchCapability
    , public CallAnyCapability {
public:
    /**
     * @brief Call a type object to initialize python object.
     */
    template <typename... Args>
    void init(Args&&... args) {
        bound_ = OwnedPy<nb::object>(new nb::object(callObject(type(), std::forward<Args>(args)...)));
    }

protected:
    OwnedPy<nb::object> bound_;

    template <typename T, typename>
    friend struct nanobind::detail::type_caster;
};

/**
 * @brief Capability to wrap existing object.
 */
class WrapPythonClassCapability
    : public InitPythonClassCapability  {
public:
    void init(nb::object obj) {
        bound_ = OwnedPy<nb::object>(new nb::object(std::move(obj)));
    }
};

/**
 * @brief Constructs defaulted attributes for a class.
 */
class DefaultedClassProviderCapability
    : public IPythonClassIdentityCapability {
protected:
    template <typename T>
    Defaulted<T> defaulted(const char* name) const {
        return Defaulted<T>(moduleName(), name, className());
    }
};

/**
 * @brief Constructs defaulted attributes for a module.
 * @see DefaultedClassProviderCapability
 */
class DefaultedModuleProviderCapability
    : public IPythonModuleIdentityCapability {
protected:
    template <typename T>
    Defaulted<T> defaulted(const char* name) const {
        return Defaulted<T>(moduleName(), name);
    }
};

MI_VARIANT class SceneObject;
MI_VARIANT class RadioMaterialBase;
MI_VARIANT class RadioMaterial;

NAMESPACE_END(py)

NAMESPACE_END(sionna)
NAMESPACE_END(artery)

NAMESPACE_BEGIN(nanobind)
NAMESPACE_BEGIN(detail)

template <typename T>
struct sionna_wrap_caster_enabled : std::false_type {};

template <typename Float, typename Spectrum>
struct sionna_wrap_caster_enabled<artery::sionna::py::SceneObject<Float, Spectrum>>
    : std::true_type {};

template <typename Float, typename Spectrum>
struct sionna_wrap_caster_enabled<artery::sionna::py::RadioMaterial<Float, Spectrum>>
    : std::true_type {};

template <typename T>
using enable_if_wrap_caster = std::enable_if_t<sionna_wrap_caster_enabled<T>::value &&
                                               std::is_default_constructible_v<T>, int>;

/**
 * @brief Generic caster for wrappers opted-in via nb_wrap_caster_enabled trait.
 */
template <typename T>
struct type_caster<T, enable_if_wrap_caster<T>> {
    NB_TYPE_CASTER(T, const_name<T>())

    bool from_python(handle src, uint8_t, cleanup_list *) noexcept {
        value = Value();
        value.init(borrow<object>(src));
        return true;
    }

    static handle from_cpp(const Value& src, rv_policy, cleanup_list *) noexcept {
        return src.bound_->release();
    }

    static handle from_cpp(Value&& src, rv_policy policy, cleanup_list *cleanup) noexcept {
        return from_cpp(static_cast<const Value&>(src), policy, cleanup);
    }
};

NAMESPACE_END(detail)
NAMESPACE_END(NB_NAMESPACE)
