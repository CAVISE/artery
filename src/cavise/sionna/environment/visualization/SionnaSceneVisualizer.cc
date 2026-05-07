#include "SionnaSceneVisualizer.h"

#include <cavise/sionna/environment/config/dynamic/TraciDynamicSceneConfigProvider.h>
#include <cavise/sionna/environment/PathLoss.h>

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
    api_ = ISionnaAPI::get(this);
    outputDir_ = par("outputDir").stdstringValue();
    camera_ = par("camera").stdstringValue();
    spp_ = par("spp").intValue();
    width_ = par("width").intValue();
    height_ = par("height").intValue();

    dynamicSceneConfigProvider_ = dynamic_cast<TraciDynamicSceneConfigProvider*>(getModuleByPath("^.dynamicSceneConfigProvider"));
    if (dynamicSceneConfigProvider_ == nullptr) {
        throw omnetpp::cRuntimeError("SionnaSceneVisualizer could not resolve dynamicSceneConfigProvider at path ^.dynamicSceneConfigProvider");
    }

    dynamicSceneConfigProvider_->subscribe(TraciDynamicSceneConfigProvider::sceneEditedSignal, this);

    if (auto* pathLossModule = dynamic_cast<PathLoss*>(getModuleByPath("^.^.radioMedium.pathLoss"))) {
        pathLossModule_ = pathLossModule;
    }
}

void SionnaSceneVisualizer::handleMessage(omnetpp::cMessage* msg) {
    throw omnetpp::cRuntimeError("SionnaSceneVisualizer does not use self-messages, received '%s'", msg->getName());
}

void SionnaSceneVisualizer::finish() {
    if (dynamicSceneConfigProvider_ != nullptr) {
        dynamicSceneConfigProvider_->unsubscribe(TraciDynamicSceneConfigProvider::sceneEditedSignal, this);
        dynamicSceneConfigProvider_ = nullptr;
    }

    if (pathLossModule_ != nullptr) {
        pathLossModule_ = nullptr;
    }

    api_ = nullptr;
}

void SionnaSceneVisualizer::renderFrame() {
    if (api_ == nullptr) {
        return;
    }

    std::optional<py::Paths> paths;
    if (pathLossModule_ != nullptr) {
        paths = pathLossModule_->cachedPaths();
    }

    const auto cameras = resolveCameras();
    for (const auto& [cameraId, camera] : cameras) {
        const auto filename = framePath(cameraId);
        std::filesystem::create_directories(filename.parent_path());

        api_->scene().renderToFile(
            camera,
            filename.string(),
            spp_,
            width_,
            height_,
            std::nullopt,
            paths,
            false);
    }

    ++frameIndex_;
}

void SionnaSceneVisualizer::receiveSignal(omnetpp::cComponent* /* source */, omnetpp::simsignal_t signal, unsigned long /* value */, omnetpp::cObject* /* details */) {
    if (signal != TraciDynamicSceneConfigProvider::sceneEditedSignal) {
        throw omnetpp::cRuntimeError("SionnaSceneVisualizer received unknown signal");
    }

    renderFrame();
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
