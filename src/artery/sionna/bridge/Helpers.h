#pragma once

#include <artery/sionna/bridge/Defaulted.h>

#include <nanobind/nanobind.h>

#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <type_traits>

#if defined(SIONNA_BRIDGE_COMPILATION_MODE_BASIC)
#    include <stdexcept>
#elif defined(SIONNA_BRIDGE_COMPILATION_MODE_INET)
#    include <omnetpp/cexception.h>
#endif

namespace artery {

    namespace sionna {

        /**
        * @brief Formats with std::snprintf and returns std::string.
        */
        template <typename... Args>
        std::string format(const std::string& fmt, Args... args) {
            if constexpr (sizeof...(args) == 0) {
                return fmt;
            } else if (const int size = std::snprintf(nullptr, 0, fmt.c_str(), args...) + 1; size > 0) {
                auto buf = std::make_unique<char[]>(size);
                std::snprintf(buf.get(), size, fmt.c_str(), args...);
                return buf.get();
            }

            return "failed to format string: std::snprintf returned invalid size";
        }

        /**
        * @brief Common exception factory.
        */
        template <typename... Args>
        auto wrapRuntimeError(const std::string& fmt, Args... args) {
            #if defined(SIONNA_BRIDGE_COMPILATION_MODE_BASIC)
                return std::runtime_error(format(fmt, args...));
            #elif defined(SIONNA_BRIDGE_COMPILATION_MODE_INET)
                return omnetpp::cRuntimeError(fmt.c_str(), args...);
            #endif
        }

        /**
        * @brief Manage embedded python interpreter with minimal config in RAII style, setting
        * essential paths to home, exec_dir, and package discovery paths.
        */
        class ScopedInterpreter {
        public:
            using path = std::filesystem::path;

            /**
            * @brief Initialize interpreter, considering \ref root as root directory.
            * @param root prefix for python installation.
            */
            ScopedInterpreter(const std::filesystem::path& root) {
                PyConfig_InitPythonConfig(&config_);
                if (PyStatus status = initialize(root); PyStatus_IsError(status)) {
                    throw wrapRuntimeError("sionna: failed to initialize interpreter config, error: %s", status.err_msg);
                }

                if (PyStatus status = Py_InitializeFromConfig(&config_); PyStatus_IsError(status)) {
                    throw wrapRuntimeError("sionna: failed to start interpreter, reported error: %s", status.err_msg);
                }
            }

            ~ScopedInterpreter() {
                PyConfig_Clear(&config_);
                Py_Finalize();
            }

            // You may override the functions below to customize how this class
            // manages paths to various directories.
            virtual path program(const path& root) const { return root / "bin" / "python3"; }
            virtual path pythonpath(const path& root) const { return root.parent_path(); }

        protected:
            /**
            * @brief Initialize configuration.
            * Update @ref config_ with options you want. This method is invoked before python
            * is initialized, so take care: CPython API usage is heavily limited.
            */
            virtual PyStatus initialize(const path& root) {
                // Program name is usually enough - the rest is discovered by python automatically.
                std::wstring program = this->program(root).wstring();
                PyStatus status = PyConfig_SetString(&config_, &config_.program_name, program.c_str());
                if (PyStatus_IsError(status) != 0) {
                    return status;
                }

                // Add project root to PYTHONPATH when running in-place.
                #if not defined(NDEBUG)
                    std::wstring python = this->pythonpath(root).wstring();
                    status = PyConfig_SetString(&config_, &config_.pythonpath_env, python.c_str());
                #endif

                return status;
            }

        private:
            PyConfig config_;
        };

        /**
        * @brief Some of sionna's objects use non-standard integer, floating point etc. types.
        * Propagating them back to Artery may be done with this method - it also casts
        * property (attribute) value to a standard python type before returning it.
        */
        template <typename T = nanobind::object>
        T access(const nanobind::object& obj, const std::string& attribute, bool convert) {
            namespace nb = nanobind;
            nb::gil_scoped_acquire gil;

            try {
                if constexpr (std::is_base_of_v<nb::handle, std::decay_t<T>>) {
                    return obj.attr(attribute.c_str());
                } else {
                    return nb::cast<T>(obj.attr(attribute.c_str()), convert);
                }
            } catch (const nb::python_error& error) {
                throw wrapRuntimeError("sionna: failed to convert property \"%s\" for object at %p: %s", attribute.c_str(), obj.ptr(), error.what());
            }
        }

        /**
        * @brief Set an attribute on a Sionna Python object, with automatic type wrapping.
        */
        template <typename T = nanobind::object>
        void set(const nanobind::object& obj, const std::string& attribute, T value) {
            namespace nb = nanobind;
            nb::gil_scoped_acquire gil;

            try {
                if constexpr (std::is_base_of_v<std::decay<nb::handle>, T>) {
                    nb::setattr(obj, attribute.c_str(), value);
                } else {
                    nb::setattr(obj, attribute.c_str(), nb::cast(std::move(value)));
                }
            } catch (const nb::python_error& error) {
                throw wrapRuntimeError("sionna: failed to set property \"%s\" of object at %p: %s", attribute.c_str(), obj.ptr(), error.what());
            }
        }

        /**
        * @brief Call an object.
        */
        template <typename ReturnT = nanobind::object, typename... Args>
        ReturnT callObject(const nanobind::object& obj, Args&&... args) {
            namespace nb = nanobind;
            nb::gil_scoped_acquire gil;

            nb::dict kw = Kwargs::toDict(std::forward<Args>(args)...);
            try {
                return obj(**kw);
            } catch (const nb::python_error& error) {
                throw wrapRuntimeError("sionna: failed to call an object at %p: %s", obj.ptr(), error.what());
            }
        }

        /**
        * @brief Call an a specified attribute on an object.
        */
        template <typename ReturnT = nanobind::object, typename... Args>
        ReturnT call(const nanobind::object& obj, const std::string& method, Args&&... args) {
            nanobind::gil_scoped_acquire gil;

            return callObject<ReturnT>(
                sionna::access<nanobind::object>(obj, method),
                std::forward<Args>(args)...
            );
        }

    }
}
