#include "PhysicalEnvironment.h"


using namespace artery;

namespace {
    static constexpr int NUM_INIT_STAGES = 2;

    enum class InitStage : int { LOAD_SCENE, PARSE_CONFIG };

}  // namespace


int sionna::PhysicalEnvironment::numInitStages() const {
    return NUM_INIT_STAGES;
}

void sionna::PhysicalEnvironment::initialize(int stage) {
    InitStage initStage = static_cast<InitStage>(stage);

    switch (initStage) {
        case InitStage::LOAD_SCENE:
            initializeScene();
        case InitStage::PARSE_CONFIG:
            initializeSceneWithConfig();
    }
}

void sionna::PhysicalEnvironment::visitObjects(
    const inet::IVisitor* visitor,
    const inet::LineSegment& lineSegment
) const {
    if (!visitor) {
        return;
    }

    const int count = getNumObjects();
    for (int i = 0; i < count; ++i) {
        const auto *object = getObject(i);
        if (!object) {
            continue;
        }

        const auto *shape = object->getShape();
        if (!shape) {
            continue;
        }

        inet::Coord intersection1;
        inet::Coord intersection2;
        inet::Coord normal1;
        inet::Coord normal2;

        if (shape->computeIntersection(lineSegment, intersection1, intersection2, normal1, normal2)) {
            visitor->visit(dynamic_cast<const omnetpp::cObject *>(object));
        }
    }
}
