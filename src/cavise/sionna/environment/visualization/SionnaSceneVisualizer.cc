#include "SionnaSceneVisualizer.h"

#include <cavise/sionna/environment/config/dynamic/TraciDynamicSceneConfigProvider.h>
#include <cavise/sionna/environment/PathLoss.h>

#include <omnetpp/cexception.h>
#include <omnetpp/cxmlelement.h>

#include <inet/common/ModuleAccess.h>

#include <cstring>
#include <filesystem>
#include <iomanip>
#include <optional>
#include <sstream>
#include <tuple>
#include <unordered_map>

using namespace artery::sionna;

Define_Module(SionnaSceneVisualizer);

namespace {

    const std::unordered_map<std::string, py::InteractionTypes::TInteraction> interactions = {
        {"none", py::InteractionTypes::none},
        {"specular", py::InteractionTypes::specular},
        {"diffuse", py::InteractionTypes::diffuse},
        {"refraction", py::InteractionTypes::refraction},
        {"transmission", py::InteractionTypes::refraction},
        {"diffraction", py::InteractionTypes::diffraction},
    };

} // namespace

void SionnaSceneVisualizer::initialize() {
    api_ = ISionnaAPI::get(this);

    // Rendering basics.
    outputDir_ = par("outputDir").stdstringValue();

    // Render options.
    renderOptions_.numSamples = par("spp").intValue();
    renderOptions_.width = par("width").intValue();
    renderOptions_.height = par("height").intValue();
    renderOptions_.showDevices = false;

    // Handle colors for rays.
    if (omnetpp::cXMLElement* root = par("interactionColorConfig").xmlValue(); root) {
        for (omnetpp::cXMLElement* child = root->getFirstChild(); child != nullptr; child = child->getNextSibling()) {
            if (std::strcmp(child->getTagName(), "interaction") != 0) {
                continue;
            }

            if (const char* r = child->getAttribute("r"); r == nullptr) {
                throw omnetpp::cRuntimeError("failed to get red component from interaction's color");
            } else if (const char* g = child->getAttribute("g"); g == nullptr) {
                throw omnetpp::cRuntimeError("failed to get green component from interaction's color");
            } else if (const char* b = child->getAttribute("b"); b == nullptr) {
                throw omnetpp::cRuntimeError("failed to get blue component from interaction's color");
            } else if (const char* type = child->getAttribute("type"); type == nullptr) {
                throw omnetpp::cRuntimeError("failed to get type component from interaction");
            } else {
                auto color = std::make_tuple(std::stod(r), std::stod(g), std::stod(b));
                if (auto it = interactions.find(type); it != interactions.end()) {
                    interactionTypeColors_.setColor(it->second, color);
                } else {
                    EV_WARN << "could not set color for interaction type: type " << type << " is unknown";
                }
            }
        }
    }

    // Find and resolve cameras.
    if (omnetpp::cXMLElement* root = par("cameraConfig").xmlValue(); root != nullptr) {
        for (omnetpp::cXMLElement* child = root->getFirstChild(); child != nullptr; child = child->getNextSibling()) {
            if (std::strcmp(child->getTagName(), "camera") == 0) {
                if (const char* id = child->getAttribute("id"); id == nullptr) {
                    throw omnetpp::cRuntimeError("camera ID is null - cannot spawn it without ID");
                } else if (omnetpp::cXMLElement* position = child->getFirstChildWithTag("position"); position == nullptr) {
                    throw omnetpp::cRuntimeError("position is not found for camera %s", id);
                } else if (omnetpp::cXMLElement* orientation = child->getFirstChildWithTag("orientation"); position == nullptr) {
                    throw omnetpp::cRuntimeError("orientation is not found for camera %s", id);
                } else {
                    auto getPoint = [](omnetpp::cXMLElement* element) {
                        auto x = std::stod(element->getAttribute("x"));
                        auto y = std::stod(element->getAttribute("y"));
                        auto z = std::stod(element->getAttribute("z"));

                        return mi::Point3f(x, y, z);
                    };

                    auto cam = std::make_pair(id, py::Camera(getPoint(position), getPoint(orientation)));
                    cameras_.emplace_back(cam);
                }
            } else if (std::strcmp(child->getTagName(), "ref") == 0) {
                if (const char* id = child->getAttribute("id"); id != nullptr) {
                    cameras_.emplace_back(id);
                } else {
                    EV_WARN << "Skipping reference to camera in scene: id is empty";
                }
            }
        }
    }

    pathLoss_ = inet::getModuleFromPar<PathLoss>(par("pathLoss"), this, true);
    pathLoss_->subscribe(PathLoss::pathsSolved, this);
}

void SionnaSceneVisualizer::finish() {
    pathLoss_->unsubscribe(PathLoss::pathsSolved, this);
    api_ = nullptr;
}

void SionnaSceneVisualizer::renderFrame() {
    std::optional<py::Paths> paths = pathLoss_->cachedPaths();

    for (const auto& camera : cameras_) {
        std::visit(
            [this, &paths](const auto& cam) {
                using TCamType = std::decay_t<decltype(cam)>;

                if constexpr (std::is_same_v<TCamType, std::string>) {
                    const auto filename = framePath(cam);
                    std::filesystem::create_directories(filename.parent_path());
                    renderOptions_.paths = paths;
                    api_->scene().renderToFile(cam, filename.string(), renderOptions_);
                } else {
                    const auto filename = framePath(cam.first);
                    std::filesystem::create_directories(filename.parent_path());
                    renderOptions_.paths = paths;
                    api_->scene().renderToFile(cam.second, filename.string(), renderOptions_);
                }
            },
            camera);
    }

    ++frameIndex_;
}

void SionnaSceneVisualizer::receiveSignal(omnetpp::cComponent* /* source */, omnetpp::simsignal_t signal, unsigned long /* value */, omnetpp::cObject* /* details */) {
    if (signal != PathLoss::pathsSolved) {
        throw omnetpp::cRuntimeError("SionnaSceneVisualizer received unknown signal");
    }

    renderFrame();
}

std::filesystem::path SionnaSceneVisualizer::framePath(const std::string& cameraId) const {
    std::ostringstream name;
    name << "frame-" << std::setw(6) << std::setfill('0') << frameIndex_ << ".png";
    return std::filesystem::path(outputDir_) / cameraId / name.str();
}
