#include "SionnaSceneVisualizer.h"

#include <omnetpp/cexception.h>
#include <omnetpp/cxmlelement.h>

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

    const auto camera = resolveCamera();

    const auto filename = framePath();
    std::filesystem::create_directories(filename.parent_path());

    scene_->renderToFile(
        camera,
        filename.string(),
        spp_,
        width_,
        height_);

    ++frameIndex_;
}

py::Camera SionnaSceneVisualizer::resolveCamera() const {
    omnetpp::cXMLElement* root = par("cameraConfig").xmlValue();
    if (root == nullptr) {
        throw omnetpp::cRuntimeError("SionnaSceneVisualizer cameraConfig does not resolve to XML; provide xmldoc(...) and select a camera id");
    }

    const char* selectedId = camera_.empty() ? nullptr : camera_.c_str();
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

        return py::Camera(
            mitsuba::Resolve::Point3f(px, py, pz),
            mitsuba::Resolve::Point3f(ox, oy, oz));
    }

    throw omnetpp::cRuntimeError("could not resolve visualizer camera '%s' from cameraConfig", camera_.c_str());
}

std::filesystem::path SionnaSceneVisualizer::framePath() const {
    std::ostringstream name;
    name << "frame-" << std::setw(6) << std::setfill('0') << frameIndex_ << ".png";
    return std::filesystem::path(outputDir_) / name.str();
}

void SionnaSceneVisualizer::scheduleNextRender() {
    if (renderTimer_ == nullptr || renderInterval_ <= omnetpp::SimTime::ZERO) {
        return;
    }

    if (!renderTimer_->isScheduled()) {
        scheduleAt(omnetpp::simTime() + renderInterval_, renderTimer_);
    }
}
