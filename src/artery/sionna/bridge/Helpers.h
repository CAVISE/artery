#pragma once

#include <cstdio>
#include <filesystem>
#include <type_traits>

#include <nanobind/nanobind.h>

#if not defined(SIONNA_OMNETPP_DETACHED)
    #include <omnetpp/cexception.h>
#else
    #include <stdexcept>
#endif

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

template<typename... Args>
auto wrapRuntimeError(const std::string& fmt, Args... args) {
    #if not defined(SIONNA_OMNETPP_DETACHED)
        return omnetpp::cRuntimeError(fmt.c_str(), args...);
    #else
        if constexpr (sizeof...(args) == 0) {
            return std::runtime_error(fmt);
        } else if (const int size = std::snprintf(nullptr, 0, fmt.c_str(), args...) + 1; size > 0) {
            auto buf = std::make_unique<char[]>(size);
            std::snprintf(buf.get(), size, fmt.c_str(), args...);
            return std::runtime_error(buf.get());
        }
        return std::runtime_error("error occurred: failed to format: std::snprintf returned invalid size");
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
    ScopedInterpreter(const std::filesystem::path& root, bool verbose = true) {
        namespace nb = nanobind;

        initialize(root);
        try {
            Py_InitializeFromConfig(&config_);            
        } catch (const nb::python_error& error) {
            PyConfig_Clear(&config_);
            throw wrapRuntimeError("sionna: failed to start interpreter, reported error: %s", error.what());
        }

        PyConfig_Clear(&config_);
    }

    ~ScopedInterpreter() {
        Py_Finalize();
    }

    // You may override the functions below to customize how this class
    // manages paths to various directories.
    virtual path program(const path& root) const {
        return root / "bin" / "python3";
    }

    virtual path pythonpath(const path& root) const {
        return root.parent_path();
    }

protected:

    /**
     * @brief Initialize configuration.
     * Update \ref config_ with options you want. This method is invoked before python 
     * is initialized, so take care: CPython API usage is heavily limited.
     */
    virtual void initialize(const path& root) {
        PyConfig_InitPythonConfig(&config_);
    
        // Program name is usually enough - the rest is discovered by python automatically.
        std::wstring program = this->program(root).wstring();
        PyConfig_SetString(&config_, &config_.program_name, program.c_str());

        // Add project root to PYTHONPATH when running in-place.
        #if not defined(NDEBUG)
            std::wstring python = this->pythonpath(root).wstring();
            PyConfig_SetString(&config_, &config_.pythonpath_env, python.c_str());
        #endif
    }

private:
    PyConfig config_;
};

// Some of sionna's objects use non-standard integer, floating point etc. types.
// Propagating them back to Artery may be done with this method - it also casts
// property (attribute) value to a standard python type before returning it.
template <typename T>
T access(const nanobind::object* obj, const std::string& attribute, bool convert = true) {
    namespace nb = nanobind;

    try {
        return nb::cast<T>(obj->attr(attribute.c_str()), convert);
    } catch (const nb::python_error& error) {
        throw wrapRuntimeError("sionna: failed to convert property %s for object at %p", attribute.c_str(), obj->ptr());
    }
}

// Set an attribute on a Sionna Python object, with automatic type wrapping.
template <typename T>
void set(nanobind::object* obj, const std::string& attribute, T value) {
    namespace nb = nanobind;

    try {
        if constexpr (std::is_base_of_v<nb::handle, T>) {
            nb::setattr(*obj, attribute.c_str(), value);
        } else {
            nb::setattr(*obj, attribute.c_str(), nb::cast(std::move(value)));
        }
    } catch (const nb::python_error& error) {
        throw wrapRuntimeError("sionna: failed to set property %s of object at %p", attribute.c_str(), obj->ptr());
    }
}

// Access Sionna python module.
inline nanobind::module_ sionna() {
    return nanobind::module_::import_("sionna");
}

// Access Sionna RT python submodule.
inline nanobind::module_ sionnaRt() {
    return nanobind::module_::import_("sionna.rt");
}

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
