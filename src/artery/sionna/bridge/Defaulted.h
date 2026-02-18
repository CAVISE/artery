#pragma once

#include <artery/sionna/bridge/Fwd.h>
#include <nanobind/nanobind.h>

#include <drjit/array_traits.h>

#include <cstring>
#include <type_traits>
#include <utility>
#include <variant>

namespace artery {

    namespace sionna {

        template <typename T>
        class Defaulted {
        public:

            using ValueType = T;
            using Argument = std::variant<T, Defaulted<T>>;

            constexpr Defaulted(const char* module, const char* name, const char* cls = "")
                : module_(module)
                , name_(name)
                , cls_(cls)
            {}

            virtual ~Defaulted() = default;

            // Accessors.
            constexpr const char* name() const { return name_; }
            constexpr const char* module() const { return module_; }
            constexpr const char* cls() const { return cls_; }

            constexpr void setName(const char* name) { name_ = name; }
            constexpr void setModule(const char* module) const { module_ = module; }
            constexpr void setCls(const char* cls) const { cls_ = cls; }

            /**
            * @brief Request constant value from python module.
            */
            virtual T value() const {
                nanobind::gil_scoped_acquire gil;
                auto module = nanobind::module_::import_(module_);

                nanobind::object obj = (std::strlen(cls_) > 0)
                    ? sionna::access<nanobind::object>(nanobind::getattr(module, cls_), name_)
                    : sionna::access<nanobind::object>(module, name_);

                // This exists if fetched constant is not Jit array (hello, sionna!)
                if constexpr (drjit::is_array_v<T>) {
                    using Scalar = drjit::scalar_t<T>;
                    if (nanobind::isinstance<nanobind::bool_>(obj)) {
                        return T(static_cast<Scalar>(nanobind::cast<bool>(obj)));
                    }
                    if (nanobind::isinstance<nanobind::int_>(obj)) {
                        return T(static_cast<Scalar>(nanobind::cast<long long>(obj)));
                    }
                    if (nanobind::isinstance<nanobind::float_>(obj)) {
                        return T(static_cast<Scalar>(nanobind::cast<double>(obj)));
                    }
                }

                return nanobind::cast<T>(obj);
            }

            /**
            * @brief Resolve value: if user provided their own - use it, otherwise pull from Sionna presets.
            */
            static T resolve(Argument variadic) {
                return std::visit([](auto&& v) -> T {
                    using V = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<V, Defaulted<T>>) {
                        return v.value();
                    } else {
                        return v;
                    }
                }, variadic);
            }

        private:
            const char* name_;
            mutable const char* cls_;
            mutable const char* module_;
        };

        template <typename T, typename Resolver>
        class DefaultedWithDeferredResolution
            : public Defaulted<T> {
        public:

            constexpr DefaultedWithDeferredResolution(const char* name, Resolver resolver)
                : Defaulted<T>("", name, "")
                , resolver_(std::move(resolver))
            {}

            T value() const override {
                using Ret = std::invoke_result_t<Resolver&>;

                if constexpr (std::is_same_v<Ret, const char*>) {
                    this->setModule(resolver_());
                } else if constexpr (std::is_same_v<Ret, std::pair<const char*, const char*>>) {
                    auto [mod, cls] = resolver_();
                    this->setModule(mod);
                    this->setCls(cls);
                } else {
                    static_assert(std::is_same_v<Ret, void>,
                        "Resolver must return const char* or std::pair<const char*, const char*>");
                }

                return Defaulted<T>::value();
            }

        private:
            Resolver resolver_;
        };

        template<typename>
        struct IsDefaulted : std::false_type {};
        template<typename>
        struct IsDefaultedArgument : std::false_type {};

        template<typename T>
        struct IsDefaulted<Defaulted<T>> : std::true_type {};
        template<typename T, typename R>
        struct IsDefaulted<DefaultedWithDeferredResolution<T, R>> : std::true_type {};

        template<typename T>
        struct IsDefaultedArgument<std::variant<T, Defaulted<T>>> : std::true_type {};
        template<typename T, typename R>
        struct IsDefaultedArgument<std::variant<T, DefaultedWithDeferredResolution<T, R>>> : std::true_type {};

        template<typename V>
        inline constexpr bool IsDefaultedV = IsDefaulted<std::decay_t<V>>::value;
        template<typename V>
        inline constexpr bool IsDefaultedArgumentV = IsDefaultedArgument<std::decay_t<V>>::value;

        template<typename>
        struct DefaultedArgumentValueType;

        template<typename T>
        struct DefaultedArgumentValueType<std::variant<T, Defaulted<T>>> {
            using type = T;
        };

        template<typename T, typename R>
        struct DefaultedArgumentValueType<std::variant<T, DefaultedWithDeferredResolution<T, R>>> {
            using type = T;
        };

        class Kwargs {
        public:

            template <typename T>
            using Item = std::pair<const char*, T>;

            struct Key {
                const char* key;

                template <typename T>
                constexpr Item<T> operator=(T v) {
                    return std::make_pair(key, std::move(v));
                }
            };

            template <typename... Args>
            static nanobind::dict toDict(Item<Args>&&... items) {
                nanobind::dict d;
                (emplace(d, items.first, std::forward<decltype(items.second)>(items.second)), ...);
                return d;
            }

        private:

            template<typename T>
            static void emplace(nanobind::dict& d, const char* key, T&& v) {
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

        };

        namespace literals {

            constexpr Kwargs::Key operator""_a(const char* key, std::size_t /* size */) {
                return Kwargs::Key { .key = key };
            }

        }


    }

}
