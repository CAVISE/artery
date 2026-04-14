#include "TraciCoordinateTransformer.h"

#include <cmath>

#include <drjit/matrix.h>
#include <omnetpp/cexception.h>

using namespace artery::sionna;

Define_Module(TraciCoordinateTransformer);

namespace {
    constexpr int kNumInitStages = 4;
    constexpr int kLoadDynamicSceneStage = 2;
}

int TraciCoordinateTransformer::numInitStages() const {
    return kNumInitStages;
}

void TraciCoordinateTransformer::initialize(int stage) {
    if (stage != kLoadDynamicSceneStage) {
        return;
    }

    rotation_.emplace(
        par("r00").doubleValue(),
        par("r01").doubleValue(),
        par("r10").doubleValue(),
        par("r11").doubleValue());

    auto det = drjit::det(*rotation_);
    if (std::fabs(toScalar<double>(det)) < 1e-12) {
        throw omnetpp::cRuntimeError("TraciCoordinateTransformer matrix is singular");
    }

    inverseRotation_.emplace(drjit::inverse(*rotation_));
    translation_.emplace(
        par("translationX").doubleValue(),
        par("translationY").doubleValue(),
        par("translationZ").doubleValue());
}

mitsuba::Resolve::Vector3f TraciCoordinateTransformer::fromSumo(const mitsuba::Resolve::Vector3f& sumo) const {
    if (!rotation_ || !translation_) {
        throw omnetpp::cRuntimeError("TraciCoordinateTransformer is not initialized yet");
    }

    auto rotated = (*rotation_) * mitsuba::Resolve::Vector2f(sumo.x(), sumo.y());
    return mitsuba::Resolve::Vector3f(rotated.x(), rotated.y(), sumo.z()) + *translation_;
}

mitsuba::Resolve::Vector3f TraciCoordinateTransformer::fromScene(const mitsuba::Resolve::Vector3f& scene) const {
    if (!inverseRotation_ || !translation_) {
        throw omnetpp::cRuntimeError("TraciCoordinateTransformer is not initialized yet");
    }

    auto unshifted = scene - *translation_;
    auto restored = (*inverseRotation_) * mitsuba::Resolve::Vector2f(unshifted.x(), unshifted.y());
    return mitsuba::Resolve::Vector3f(restored.x(), restored.y(), unshifted.z());
}
