#pragma once

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Layout.h>
#include <nanobind/nanobind.h>

#include <variant>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

template <typename T>
class Defaulted
{
public:
    /**
     * @brief Alias for convenient usage within arguments.
     */
    using Argument = std::variant<T, Defaulted<T>>;

    Defaulted(const std::string& module, const std::string& name, const std::string& cls = "") : module_(module), name_(name), cls_(cls) {}

    // Accessors.
    const std::string& name() const { return name_; }
    const std::string& module() const { return module_; }

    /**
     * @brief Request constant value from python module.
     */
    T value() const
    {
        namespace nb = nanobind;

        auto module = nb::module_::import_(module_.c_str());
        return access<T>((cls_ != "") ? nb::getattr(module, cls_.c_str()) : module, name_);
    }

    /**
     * @brief Resolve value: if user provided their own - use it, otherwise pull from Sionna presets.
     */
    static T resolve(Argument variadic)
    {
        struct Visitor {
            T operator()(T value) const { return value; }
            T operator()(const Defaulted<T>& constant) const { return constant.value(); }
        };

        return std::visit(Visitor(), variadic);
    }

private:
    std::string cls_;
    std::string name_;
    std::string module_;
};

NAMESPACE_BEGIN(literals)

template <typename T>
using Item = std::pair<const char*, T>;

struct Key {
    const char* name;

    template <typename T>
    constexpr Item<std::decay_t<T>> operator=(T v)
    {
        return std::make_pair(name, std::forward<T>(v));
    }
};

constexpr Key operator""_a(const char* s, std::size_t /* size */)
{
    return Key{s};
}

template <typename T>
inline void put(nanobind::dict& d, const char* key, const typename Defaulted<T>::Argument& a)
{
    if (T v = std::get_if<T>(&a); v) {
        d[key] = *v;
    }

    // Omit key here, as python constant should already be provided.
}

template <typename V>
inline void put(nanobind::dict& d, const char* key, V&& v)
{
    d[key] = std::forward<V>(v);
}

template <typename... Items>
inline nanobind::dict kwargs(Items&&... items)
{
    nanobind::dict d;
    (put(d, items.first, items.second), ...);
    return d;
}

NAMESPACE_END(literals)

MI_VARIANT
class Constants
{
public:
    SIONNA_IMPORT_CORE_TYPES(Float64, Int32)

    inline static Defaulted<Float64> DEFAULT_THICKNESS{ModuleLayout::sionnaConstants, "DEFAULT_THICKNESS"};
    inline static Defaulted<Int32> INTERSECTION_NONE{ModuleLayout::sionnaConstants, "NONE", ModuleLayout::Classes::IntersectionTypes};
    inline static Defaulted<Int32> INTERSECTION_SPECULAR{ModuleLayout::sionnaConstants, "SPECULAR", ModuleLayout::Classes::IntersectionTypes};
    inline static Defaulted<Int32> INTERSECTION_DIFFUSE{ModuleLayout::sionnaConstants, "DIFFUSE", ModuleLayout::Classes::IntersectionTypes};
    inline static Defaulted<Int32> INTERSECTION_REFRACTION{ModuleLayout::sionnaConstants, "REFRACTION", ModuleLayout::Classes::IntersectionTypes};
    inline static Defaulted<Int32> INTERSECTION_DIFFRACTION{ModuleLayout::sionnaConstants, "DIFFRACTION", ModuleLayout::Classes::IntersectionTypes};
};

NAMESPACE_END(sionna)
NAMESPACE_END(artery)