#pragma once

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Helpers.h>
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

template<typename>
struct IsDefaulted : std::false_type {
};

template<typename T>
struct IsDefaulted<Defaulted<T>> : std::true_type {
};

template<typename V>
inline constexpr bool IsDefaultedV = IsDefaulted<std::decay_t<V>>::value;

template<typename>
struct IsDefaultedArgument : std::false_type {
};

template<typename T>
struct IsDefaultedArgument<std::variant<T, Defaulted<T>>> : std::true_type {
};

template<typename V>
inline constexpr bool IsDefaultedArgumentV = IsDefaultedArgument<std::decay_t<V>>::value;

template<typename>
struct DefaultedArgumentValueType;

template<typename T>
struct DefaultedArgumentValueType<std::variant<T, Defaulted<T>>> {
    using type = T;
};

template<typename T>
inline void emplace(nanobind::dict& d, const char* key, T&& v)
{
    using D = std::decay_t<T>;

    if constexpr (IsDefaultedV<D>) {
        // Let python handle defaults.
    } else if constexpr (IsDefaultedArgumentV<D>) {
        using ValueType = typename DefaultedArgumentValueType<D>::type;

        const D& view = v;
        if (const ValueType* value = std::get_if<ValueType>(&view)) {
            d[key] = *value;
        }
    } else {
        d[key] = std::forward<T>(v);
    }
}

template<typename... Items>
inline nanobind::dict kwargs(Items&&... items)
{
    nanobind::dict d;
    (emplace(d, items.first, std::forward<decltype(items.second)>(items.second)), ...);
    return d;
}


NAMESPACE_END(literals)

NAMESPACE_END(sionna)
NAMESPACE_END(artery)