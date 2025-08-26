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
