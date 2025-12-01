#pragma once

#include <cstdio>
#include <filesystem>
#include <type_traits>

#include <nanobind/nanobind.h>

#if defined(SIONNA_OMNETPP_DETACHED)
    #include <omnetpp/cexception.h>
#else
    #include <stdexcept>
#endif

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

/**
 * @brief Throw an exception - either one of OmnetPP's or runtime error.
 * @return auto constructed exception object.
 */
template<typename... Args>
auto wrapRuntimeError(const char* fmt, Args... args) {
    #if defined(SIONNA_OMNETPP_DETACHED)
        return omnetpp::cRuntimeError(fmt, args...);
    #else
        char buf[256];
        std::snprintf(buf, sizeof(buf), fmt, args...);
        return std::runtime_error(buf);
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
    ScopedInterpreter(const std::filesystem::path& root, bool verbose = true);
    ~ScopedInterpreter();

    // You may override the functions below to customize how this class
    // manages paths to various directories.

    virtual path program(const path& root) const;
    virtual path pythonpath(const path& root) const;

protected:

    /**
     * @brief Initialize configuration.
     * Update \ref config_ with options you want. This method is invoked before python 
     * is initialized, so take care: CPython API usage is heavily limited.
     */
    virtual void initialize(const path& root);

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
