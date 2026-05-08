#include "SionnaSceneVisualizer.h"

#include <omnetpp/cexception.h>
#include <omnetpp/cxmlelement.h>

#include <nanobind/nanobind.h>

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <sstream>

using namespace artery::sionna;

Define_Module(SionnaSceneVisualizer);

void SionnaSceneVisualizer::initialize() {
    outputDir_ = par("outputDir").stdstringValue();
    camera_ = par("camera").stdstringValue();
    spp_ = par("spp").intValue();
    width_ = par("width").intValue();
    height_ = par("height").intValue();
    renderInterval_ = par("renderInterval");
    pythonRendererSpec_ = par("pythonRenderer").stdstringValue();

    if (!pythonRendererSpec_.empty()) {
        pythonRenderer_ = resolvePythonRenderer(pythonRendererSpec_);
    }

    renderTimer_ = new omnetpp::cMessage("sionna-render-timer");
}

void SionnaSceneVisualizer::handleMessage(omnetpp::cMessage* msg) {
    if (msg != renderTimer_) {
        throw omnetpp::cRuntimeError("SionnaSceneVisualizer received unknown message");
    }

    renderFrame();
    scheduleNextRender();
}

void SionnaSceneVisualizer::finish() {
    scene_.reset();

    if (renderTimer_ != nullptr) {
        cancelAndDelete(renderTimer_);
        renderTimer_ = nullptr;
    }
}

void SionnaSceneVisualizer::setScene(py::SionnaScene scene) {
    scene_ = std::move(scene);

    if (renderInterval_ <= omnetpp::SimTime::ZERO) {
        renderFrame();
    } else {
        scheduleNextRender();
    }
}

void SionnaSceneVisualizer::renderFrame() {
    if (!scene_.has_value()) {
        return;
    }

    if (pythonRenderer_) {
        // Use custom Python renderer callback
        try {
            nanobind::gil_scoped_acquire gil;
            nanobind::dict kwargs;
            kwargs["scene"] = scene_->object();
            kwargs["sim_time"] = omnetpp::simTime().dbl();
            kwargs["output_dir"] = outputDir_;
            kwargs["frame_index"] = frameIndex_;

            pythonRenderer_(**kwargs);
        } catch (const nanobind::python_error& error) {
            throw omnetpp::cRuntimeError("SionnaSceneVisualizer: Python renderer callback failed: %s", error.what());
        }

        ++frameIndex_;
        return;
    }

    // Default rendering logic
    const auto cameras = resolveCameras();
    for (const auto& [cameraId, camera] : cameras) {
        const auto filename = framePath(cameraId);
        std::filesystem::create_directories(filename.parent_path());

        scene_->renderToFile(
            camera,
            filename.string(),
            spp_,
            width_,
            height_);
    }

    ++frameIndex_;
}

std::vector<std::pair<std::string, py::Camera>> SionnaSceneVisualizer::resolveCameras() const {
    omnetpp::cXMLElement* root = par("cameraConfig").xmlValue();
    if (root == nullptr) {
        throw omnetpp::cRuntimeError("SionnaSceneVisualizer cameraConfig does not resolve to XML; provide xmldoc(...) and select a camera id");
    }

    const char* selectedId = camera_.empty() ? nullptr : camera_.c_str();
    std::vector<std::pair<std::string, py::Camera>> result;

    for (omnetpp::cXMLElement* child = root->getFirstChild(); child != nullptr; child = child->getNextSibling()) {
        if (std::strcmp(child->getTagName(), "camera") != 0) {
            continue;
        }

        const char* id = child->getAttribute("id");
        if (selectedId != nullptr && id != nullptr && camera_ != id) {
            continue;
        } else if (selectedId != nullptr && id == nullptr) {
            continue;
        }

        const char* positionAttr = child->getAttribute("position");
        const char* orientationAttr = child->getAttribute("orientation");
        if (positionAttr == nullptr || orientationAttr == nullptr) {
            throw omnetpp::cRuntimeError("camera entry must define both position and orientation attributes");
        }

        double px, py, pz, ox, oy, oz;
        if (std::sscanf(positionAttr, "%lf %lf %lf", &px, &py, &pz) != 3) {
            throw omnetpp::cRuntimeError("invalid camera position '%s'", positionAttr);
        }
        if (std::sscanf(orientationAttr, "%lf %lf %lf", &ox, &oy, &oz) != 3) {
            throw omnetpp::cRuntimeError("invalid camera orientation '%s'", orientationAttr);
        }

        std::string effectiveId;
        if (id != nullptr && std::strlen(id) > 0) {
            effectiveId = id;
        } else if (selectedId != nullptr) {
            effectiveId = selectedId;
        } else {
            effectiveId = "camera";
        }

        result.emplace_back(
            effectiveId,
            py::Camera(
                mitsuba::Resolve::Point3f(px, py, pz),
                mitsuba::Resolve::Point3f(ox, oy, oz)));
    }

    if (result.empty()) {
        if (selectedId != nullptr) {
            throw omnetpp::cRuntimeError("could not resolve visualizer camera '%s' from cameraConfig", camera_.c_str());
        }
        throw omnetpp::cRuntimeError("could not resolve any visualizer camera from cameraConfig");
    }

    return result;
}

std::filesystem::path SionnaSceneVisualizer::framePath(const std::string& cameraId) const {
    std::ostringstream name;
    name << "frame-" << std::setw(6) << std::setfill('0') << frameIndex_ << ".png";
    return std::filesystem::path(outputDir_) / cameraId / name.str();
}

nanobind::object SionnaSceneVisualizer::resolvePythonRenderer(const std::string& rendererSpec) const {
    nanobind::gil_scoped_acquire gil;

    // Parse "module:function" or "path/to/file.py:function" format
    auto pos = rendererSpec.find_last_of(':');
    if (pos == std::string::npos) {
        throw omnetpp::cRuntimeError(
            "SionnaSceneVisualizer: Invalid pythonRenderer format '%s'. Expected 'module:function' or 'path/to/file.py:function'",
            rendererSpec.c_str());
    }

    std::string moduleName = rendererSpec.substr(0, pos);
    std::string functionName = rendererSpec.substr(pos + 1);

    try {
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
                    "SionnaSceneVisualizer: Python renderer file not found: '%s' (resolved to '%s')",
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
                    "SionnaSceneVisualizer: Failed to create module spec for '%s'",
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
                "SionnaSceneVisualizer: '%s' is not callable in module '%s'",
                functionName.c_str(), moduleName.c_str());
        }

        return func;
    } catch (const nanobind::python_error& error) {
        throw omnetpp::cRuntimeError(
            "SionnaSceneVisualizer: Failed to resolve pythonRenderer '%s': %s",
            rendererSpec.c_str(), error.what());
    }
}

void SionnaSceneVisualizer::scheduleNextRender() {
    if (renderTimer_ == nullptr || renderInterval_ <= omnetpp::SimTime::ZERO) {
        return;
    }

    if (!renderTimer_->isScheduled()) {
        scheduleAt(omnetpp::simTime() + renderInterval_, renderTimer_);
    }
}
