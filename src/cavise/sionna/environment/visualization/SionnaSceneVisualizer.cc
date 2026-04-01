#include "SionnaSceneVisualizer.h"

#include <omnetpp/cexception.h>

using namespace artery::sionna;

Define_Module(SionnaSceneVisualizer);

void SionnaSceneVisualizer::initialize() {
    outputDir_ = par("outputDir").stdstringValue();
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
    if (renderTimer_ != nullptr) {
        cancelAndDelete(renderTimer_);
        renderTimer_ = nullptr;
    }
}

void SionnaSceneVisualizer::setScene(py::SionnaScene scene) {
    scene_ = std::move(scene);
    scheduleNextRender();
}

void SionnaSceneVisualizer::renderFrame() {
    if (!scene_.has_value()) {
        return;
    }

    // Placeholder for a debug render dump. Keep frame accounting stable so the
    // first implementation can be swapped for a real renderer later.
    ++frameIndex_;
}

void SionnaSceneVisualizer::scheduleNextRender() {
    if (renderTimer_ == nullptr || renderInterval_ <= omnetpp::SimTime::ZERO) {
        return;
    }

    if (!renderTimer_->isScheduled()) {
        scheduleAt(omnetpp::simTime() + renderInterval_, renderTimer_);
    }
}
