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
        pythonRenderer_.resolve(pythonRendererSpec_);
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

    if (pythonRenderer_.isValid()) {
        // Use custom Python renderer callback
        pythonRenderer_.invoke(*scene_, omnetpp::simTime().dbl(), outputDir_, frameIndex_);
        ++frameIndex_;
        return;
    } else {
        throw omnetpp::cRuntimeError("PythonRendererCallback: Invalid renderer");
    }
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

void SionnaSceneVisualizer::scheduleNextRender() {
    if (renderTimer_ == nullptr || renderInterval_ <= omnetpp::SimTime::ZERO) {
        return;
    }

    if (!renderTimer_->isScheduled()) {
        scheduleAt(omnetpp::simTime() + renderInterval_, renderTimer_);
    }
}
