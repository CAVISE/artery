#include "TraciCoordinateTransformer.h"
#include "inet/common/InitStages.h"
#include "omnetpp/cxmlelement.h"

#include <cmath>

#include <drjit/matrix.h>
#include <omnetpp/cexception.h>
#include <string>

using namespace artery::sionna;

Define_Module(TraciCoordinateTransformer);

namespace {

    template <typename Matrix>
    Matrix parseMatrix(omnetpp::cXMLElement* data) {
        Matrix matrix;
        for (const auto& item : data->getElementsByTagName("element")) {
            std::size_t row = std::stoull(item->getAttribute("row"));
            std::size_t column = std::stoull(item->getAttribute("column"));
            matrix[row][column] = std::stod(item->getNodeValue());
        }

        return matrix;
    }

    mitsuba::Resolve::Vector3f parseTranslationVector(omnetpp::cXMLElement* data) {
        return {
            std::stod(data->getFirstChildWithTag("x")->getNodeValue()),
            std::stod(data->getFirstChildWithTag("y")->getNodeValue()),
            std::stod(data->getFirstChildWithTag("z")->getNodeValue())};
    }


} // namespace

int TraciCoordinateTransformer::numInitStages() const {
    return inet::NUM_INIT_STAGES;
}

void TraciCoordinateTransformer::initialize(int stage) {
    // NOTE: We need initialized DrJit for transforms.
    if (stage != inet::INITSTAGE_PHYSICAL_ENVIRONMENT) {
        return;
    }

    remap_ = parseMatrix<mitsuba::Resolve::Matrix3f>(par("remapMatrix").xmlValue());
    rotation_ = parseMatrix<mitsuba::Resolve::Matrix2f>(par("rotationMatrix").xmlValue());
    translation_ = parseTranslationVector(par("translationVector").xmlValue());

    auto remapDet = drjit::det(remap_);
    if (std::fabs(toScalar<double>(remapDet)) < 1e-12) {
        throw omnetpp::cRuntimeError("TraciCoordinateTransformer remap matrix is singular");
    }

    auto det = drjit::det(rotation_);
    if (std::fabs(toScalar<double>(det)) < 1e-12) {
        throw omnetpp::cRuntimeError("TraciCoordinateTransformer matrix is singular");
    }

    inverseRemap_ = drjit::inverse(remap_);
    inverseRotation_ = drjit::inverse(rotation_);
}

mitsuba::Resolve::Vector3f TraciCoordinateTransformer::fromSumo(const mitsuba::Resolve::Vector3f& sumo) const {
    return vectorFromSumo(sumo) + translation_;
}

mitsuba::Resolve::Vector3f TraciCoordinateTransformer::vectorFromSumo(const mitsuba::Resolve::Vector3f& sumo) const {
    auto rotated = rotation_ * mitsuba::Resolve::Vector2f(sumo.x(), sumo.y());
    return mitsuba::Resolve::Vector3f(rotated.x(), rotated.y(), sumo.z());
}

mitsuba::Resolve::Vector3f TraciCoordinateTransformer::fromScene(const mitsuba::Resolve::Vector3f& scene) const {
    auto unshifted = scene - translation_;
    auto restored = inverseRotation_ * mitsuba::Resolve::Vector2f(unshifted.x(), unshifted.y());
    return mitsuba::Resolve::Vector3f(restored.x(), restored.y(), unshifted.z());
}

mitsuba::Resolve::Vector3f TraciCoordinateTransformer::toLocalScene(const mitsuba::Resolve::Vector3f& scene) const {
    return remap_ * scene;
}

mitsuba::Resolve::Vector3f TraciCoordinateTransformer::fromLocalScene(const mitsuba::Resolve::Vector3f& localScene) const {
    return inverseRemap_ * localScene;
}
