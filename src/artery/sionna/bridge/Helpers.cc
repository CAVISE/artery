#include <string>
#include <cstring>
#include <filesystem>

#include <Python.h>
#include <nanobind/nanobind.h>

#include "artery/sionna/bridge/Helpers.h"

using namespace artery::sionna;

ScopedInterpreter::ScopedInterpreter(const path& root, bool verbose) {
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

ScopedInterpreter::path ScopedInterpreter::program(const path& root) const {
    return root / "bin" / "python3";
}

ScopedInterpreter::path ScopedInterpreter::pythonpath(const path& root) const {
    return root.parent_path();
}

void ScopedInterpreter::initialize(const path& root) {
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

ScopedInterpreter::~ScopedInterpreter() {
    Py_Finalize();
}
