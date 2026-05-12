#include "PythonRendererCallback.h"

#include <omnetpp/cexception.h>

#include <nanobind/nanobind.h>

#include <filesystem>
#include <stdexcept>

using namespace artery::sionna;

bool PythonRendererCallback::resolve(const std::string& rendererSpec) {
    if (rendererSpec.empty()) {
        return false;
    }

    try {
        nanobind::gil_scoped_acquire gil;

        // Parse "module:function" or "path/to/file.py:function" format
        auto pos = rendererSpec.find_last_of(':');
        if (pos == std::string::npos) {
            throw omnetpp::cRuntimeError(
                "PythonRendererCallback: Invalid renderer format '%s'. Expected 'module:function' or 'path/to/file.py:function'",
                rendererSpec.c_str());
        }

        std::string moduleName = rendererSpec.substr(0, pos);
        std::string functionName = rendererSpec.substr(pos + 1);

        nanobind::object module;

        // Check if moduleName is a file path (contains path separators or .py extension)
        bool isFilePath = (moduleName.find('/') != std::string::npos ||
                           moduleName.find('\\') != std::string::npos ||
                           moduleName.ends_with(".py"));

        if (isFilePath) {
            // Convert to absolute path to ensure we can find the file
            std::filesystem::path modulePath(moduleName);
            if (!modulePath.is_absolute()) {
                // Resolve relative to the current working directory
                modulePath = std::filesystem::absolute(modulePath);
            }

            if (!std::filesystem::exists(modulePath)) {
                throw omnetpp::cRuntimeError(
                    "PythonRendererCallback: Python renderer file not found: '%s' (resolved to '%s')",
                    moduleName.c_str(), modulePath.string().c_str());
            }

            // Use importlib.util to load module from file path
            auto importlib = nanobind::module_::import_("importlib");
            auto util = importlib.attr("util");

            // Create module spec from file location
            auto spec_from_file = util.attr("spec_from_file_location");
            auto spec = spec_from_file("sionna_dynamic_renderer", modulePath.string());

            if (spec.is_none()) {
                throw omnetpp::cRuntimeError(
                    "PythonRendererCallback: Failed to create module spec for '%s'",
                    modulePath.string().c_str());
            }

            // Create module from spec
            auto module_from_spec = util.attr("module_from_spec");
            module = module_from_spec(spec);

            // Execute the module
            auto loader = spec.attr("loader");
            loader.attr("exec_module")(module);

        } else {
            // Traditional module import (e.g., "my_package:my_module")
            module = nanobind::module_::import_(moduleName.c_str());
        }

        nanobind::object func = module.attr(functionName.c_str());

        if (!nanobind::isinstance<nanobind::callable>(func)) {
            throw omnetpp::cRuntimeError(
                "PythonRendererCallback: '%s' is not callable in module '%s'",
                functionName.c_str(), moduleName.c_str());
        }

        callback_ = func;
        return true;

    } catch (const nanobind::python_error& error) {
        throw omnetpp::cRuntimeError(
            "PythonRendererCallback: Failed to resolve renderer '%s': %s",
            rendererSpec.c_str(), error.what());
    }
}

void PythonRendererCallback::invoke(py::SionnaScene scene, double simTime, const std::string& outputDir, int frameIndex) {
    if (!callback_.is_valid()) {
        throw omnetpp::cRuntimeError("PythonRendererCallback: Cannot invoke null callback");
    }

    try {
        nanobind::gil_scoped_acquire gil;
        nanobind::dict kwargs;
        kwargs["scene"] = scene.object();
        kwargs["sim_time"] = simTime;
        kwargs["output_dir"] = outputDir;
        kwargs["frame_index"] = frameIndex;

        callback_(**kwargs);
    } catch (const nanobind::python_error& error) {
        throw omnetpp::cRuntimeError("PythonRendererCallback: Callback invocation failed: %s", error.what());
    }
}

bool PythonRendererCallback::isValid() const {
    return callback_.is_valid();
}