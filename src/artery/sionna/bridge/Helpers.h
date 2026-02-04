#pragma once

#include <artery/sionna/bridge/Defaulted.h>
#include <nanobind/nanobind.h>

#include <cstdio>
#include <filesystem>
#include <memory>
#include <type_traits>

#if not defined(SIONNA_OMNETPP_DETACHED)
#include <omnetpp/cexception.h>
#else
#include <stdexcept>
#endif

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

/**
 * @brief Formats with std::snprintf and returns std::string.
 */
template <typename... Args>
std::string format(const std::string& fmt, Args... args) noexcept {
    if (sizeof...(args) == 0) {
        return fmt;
    } else if (const int size = std::snprintf(nullptr, 0, fmt.c_str(), args...) + 1; size > 0) {
        auto buf = std::make_unique<char[]>(size);
        std::snprintf(buf.get(), size, fmt.c_str(), args...);
        return buf.get();
    }

    return "failed to format string: std::snprintf returned invalid size";
}

/**
 * @brief Common exception factory. In OMNeT++ runtime return OMNeT++ runtime error,
 * and in detached mode returns basic std::runtime_error.
 */
template <typename... Args>
auto wrapRuntimeError(const std::string& fmt, Args... args) {
#if not defined(SIONNA_OMNETPP_DETACHED)
    return omnetpp::cRuntimeError(fmt.c_str(), args...);
#else
    return std::runtime_error(format(fmt, args...));
#endif
}

/**
 * @brief Manage embedded python interpreter with minimal config in RAII style, setting
 * essential paths to home, exec_dir, and package discovery paths.
 */
class ScopedInterpreter
{
public:
    using path = std::filesystem::path;

    /**
     * @brief Initialize interpreter, considering \ref root as root directory.
     * @param root prefix for python installation.
     */
    ScopedInterpreter(const std::filesystem::path& root, bool verbose = true) {
        if (PyStatus status = initialize(root); PyStatus_IsError(status)) {
            throw wrapRuntimeError("sionna: failed to initialize interpreter config, error: %s", status.err_msg);
        }

        if (PyStatus status = Py_InitializeFromConfig(&config_); PyStatus_IsError(status)) {
            throw wrapRuntimeError("sionna: failed to start interpreter, reported error: %s", status.err_msg);
        }

        PyConfig_Clear(&config_);
    }

    ~ScopedInterpreter() { Py_Finalize(); }

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
        PyConfig_InitPythonConfig(&config_);

        // Program name is usually enough - the rest is discovered by python automatically.
        std::wstring program = this->program(root).wstring();
        PyStatus status = PyConfig_SetString(&config_, &config_.program_name, program.c_str());
        if (PyStatus_IsError(status)) {
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
template <typename T>
T access(const nanobind::object obj, const std::string& attribute, bool convert) {
    namespace nb = nanobind;
    nb::gil_scoped_acquire gil;

    try {
        if constexpr (std::is_base_of_v<nb::handle, T>) {
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
template <typename T>
void set(nanobind::object obj, const std::string& attribute, T value) {
    namespace nb = nanobind;
    nb::gil_scoped_acquire gil;

    try {
        if constexpr (std::is_base_of_v<nb::handle, T>) {
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
ReturnT callObject(nanobind::object obj, Args&&... args) {
    namespace nb = nanobind;
    nb::gil_scoped_acquire gil;

    nb::dict kw = kwargs(std::forward<Args>(args)...);
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
ReturnT call(nanobind::object obj, const std::string& method, Args&&... args) {
    nanobind::gil_scoped_acquire gil;

    return callObject<ReturnT>(
        sionna::access<nanobind::object>(obj, method),
        std::forward<Args>(args)...
    );
}

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
