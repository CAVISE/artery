#pragma once

#include <artery/sionna/bridge/Defaulted.h>
#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Helpers.h>

#include <nanobind/nanobind.h>

#include <utility>
#include <type_traits>

/**
* @file Python to C++ bindings rely on capabilities, which are assigned to each
* bound class, specifying what that class can do. Most capabilities have a base form, which
* may be overridden to expand the functionality.
*/

namespace artery {
    namespace sionna {
        namespace py {

            namespace nb = nanobind;
            using literals::operator""_a;

            class SIONNA_BRIDGE_API IPythonCapability {
            public:
                virtual ~IPythonCapability() = default;
            };

            /**
            * @brief Capability to identify a module. Override with qualified path to
            * module, that gets implemented.
            */
            class SIONNA_BRIDGE_API IPythonModuleIdentityCapability
                : public virtual IPythonCapability {
            public:
                ~IPythonModuleIdentityCapability() override = default;
                constexpr virtual const char* moduleName() const = 0;
            };

            /**
            * @brief Capability to identify a class. Inherits from module identity capability,
            * since every class belongs to a module.
            */
            class SIONNA_BRIDGE_API IPythonClassIdentityCapability
                : public virtual IPythonModuleIdentityCapability {
            public:
                ~IPythonClassIdentityCapability() override = default;
                constexpr virtual const char* className() const = 0;
            };

            /**********************
            * Basic capabilities *
            **********************
            */

            /**
            * @brief Capability to call arbitrary methods in python.
            */
            class SIONNA_BRIDGE_API CallAnyCapability
                : public virtual IPythonCapability {
            public:
                template <typename ReturnType = nb::object, typename... Args>
                ReturnType callAny(nb::handle target, const char* name, Args&&... args) const {
                    return sionna::call<ReturnType>(
                        nb::borrow<nb::object>(target),
                        name,
                        std::forward<Args>(args)...
                    );
                }
            };

            /**
            * @brief Basic stateless python module import capability.
            */
            class SIONNA_BRIDGE_API BasePythonImportCapability
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
            class SIONNA_BRIDGE_API CachedImportCapability
                : public BasePythonImportCapability {
            public:
                /**
                * @brief Stateful (cached) module import.
                */
                nb::module_ module() const override {
                    if (!module_.is_valid()) {
                        nb::gil_scoped_acquire gil;
                        module_ = BasePythonImportCapability::module();
                    }
                    return nb::borrow<nb::module_>(module_.ptr());
                }

            protected:
                mutable nb::object module_;
            };

            /**
            * @brief Basic class (type) stateless object access.
            */
            class SIONNA_BRIDGE_API BasePythonFetchCapability
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
            class SIONNA_BRIDGE_API CachedFetchCapability
                : public BasePythonFetchCapability {
            public:
                /**
                * @brief Get type object from module (stateful).
                */
                nb::object type() const override {
                    if (!type_.is_valid()) {
                        nb::gil_scoped_acquire gil;
                        type_ = BasePythonFetchCapability::type();
                    }
                    return type_;
                }

            protected:
                mutable nb::object type_;
            };

            /**
            * @brief Capability to initialize an object.
            */
            class SIONNA_BRIDGE_API InitPythonClassCapability
                : public BasePythonFetchCapability
                , public CallAnyCapability {
            public:
                /**
                * @brief Call a type object to initialize python object.
                */
                template <typename... Args>
                void init(Args&&... args) {
                    bound_ = callObject(type(), std::forward<Args>(args)...);
                }

            protected:
                nb::object bound_;

                template <typename T, typename>
                friend struct nanobind::detail::type_caster;
            };

            /**
            * @brief Capability to wrap existing object.
            */
            class SIONNA_BRIDGE_API WrapPythonClassCapability
                : public InitPythonClassCapability  {
            public:
                void init(nb::object obj) {
                    bound_ = std::move(obj);
                }
            };

            /**
            * @brief Capability to expose the underlying bound Python object.
            */
            class SIONNA_BRIDGE_API ExportBoundObjectCapability
                : public WrapPythonClassCapability {
            public:
                nb::object object() const {
                    return this->bound_;
                }
            };

            class DefaultedDeferredFactoryCapability {
            protected:
                template <typename>
                using ModuleResolver = const char* (*)();

                template <typename>
                using ModuleAndClassResolver = std::pair<const char*, const char*> (*)();

                template <typename T, typename Resolver>
                static DefaultedWithDeferredResolution<T, std::decay_t<Resolver>>
                makeDeferredDefaulted(const char* name, Resolver&& resolver) {
                    return DefaultedWithDeferredResolution<T, std::decay_t<Resolver>>(
                        name,
                        std::forward<Resolver>(resolver)
                    );
                }
            };

            class SIONNA_BRIDGE_API DefaultedModuleProviderCapability
                : public virtual IPythonModuleIdentityCapability
                , protected DefaultedDeferredFactoryCapability {
            protected:
                template <typename T>
                static const char* moduleResolverFn() {
                    T obj;
                    return obj.moduleName();
                }

                template <typename T>
                static ModuleResolver<T> moduleResolver() {
                    return &moduleResolverFn<T>;
                }
            };

            class SIONNA_BRIDGE_API DefaultedClassProviderCapability
                : public virtual IPythonClassIdentityCapability
                , protected DefaultedDeferredFactoryCapability {
            protected:
                template <typename T>
                static std::pair<const char*, const char*> moduleAndClassResolverFn() {
                    T obj;
                    return std::make_pair(obj.moduleName(), obj.className());
                }

                template <typename T>
                static ModuleAndClassResolver<T> moduleAndClassResolver() {
                    return &moduleAndClassResolverFn<T>;
                }
            };

            MI_VARIANT class SceneObject;
            MI_VARIANT class RadioMaterialBase;
            MI_VARIANT class RadioMaterial;

        }
    }
}
